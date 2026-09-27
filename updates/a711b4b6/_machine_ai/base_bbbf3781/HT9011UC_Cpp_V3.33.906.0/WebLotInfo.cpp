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
//
//  寫入
//    selection.get／selection.save：D:\HT9045\config\Security_new.def [Network]（save 寫 16 個鍵，SCK ART 多 "Auto Retest"、
//      VTEST 多 2 個；get 只在缺鍵時補寫）
//  寫入（barcode.clearCount confirmed=true）
//    D:\HT9045_Log\2DBarCode\YYYY_MM_DD\  目錄（golden :10174 MyForceDirectories；common.cpp 真本體）
//    （.xls 本身不會產生，見上）
//    記憶體：iNeedBarcodeCount／iBarcodeDuplicate／iBarcodeErrorCount／iBarcodePassCount／iBarcodeAutoRetry（cmydef.cpp:4237-4241）歸零、
//            fLotInfo->sgBarcode 重畫。這些計數器沒有落地檔（golden 也沒有），開機時 cmydef.cpp:6031 歸零。
//  不做的鈕：Clear List（btClearBarcodeListClick :10186-10195 會送 CCD 指令 "2DID by lot list"、清 2D 重複碼清單 —— 重複碼判斷會影響分 Bin，
//            V912 BarCode/BarCode.cpp:823／:830 golden 註解「這部分會影響分Bin」）、Change File（btChangeFileClick :10394-10418 重連 CCD／換 CCD job）。
// =============================================================================
#include <cstdio>
#include <string>

#include "WebLotInfo.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fLotInfo.h"
#include "cmydef.h"        // SystemYear…SystemSec、iNeedBarcodeCount 等
#include "cpublic.h"       // GetTimeInfo()
#include "common.h"        // AuthPath（Security_new.def 的資料夾）

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150

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

}  // namespace

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

    if (op != "barcode.clearCount")
        return BadPayload("op 只有 barcode.clearCount／testerLog.get／selection.get／selection.save");

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
