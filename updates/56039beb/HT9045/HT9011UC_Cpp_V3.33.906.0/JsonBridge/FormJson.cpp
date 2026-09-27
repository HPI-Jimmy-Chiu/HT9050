// ===========================================================================
//  JsonBridge/FormJson.cpp -- S12 本體。說明見 FormJson.h。
//
//  Steven 20260924.  NOT in golden.
//
//  AI(W906-Q4-S126) 20260927 (St02): 第一型退役（Steven S126／Q4；RULINGS_20260926 S126「A」）。
//    第一型是「呼叫移植樹翻好的 TfXxx::DoIniDataToForm()＋兩輪哨兵法找出有賦值的 widget」，
//    產生檔 gen/form_*.gen.cpp（tools/gen_formjson.py）接了 4 頁：TfHotPlate、TfSpeed、
//    TfTrayAssignment、TfDIOFrom。後三頁 20260927 前已經全部改走 C 路（editlist：
//    FileRW/ArmSpeed_File.cpp、FileRW/TrayForm.cpp、FileRW/TTLCfg.cpp），網頁 load() 在 gbStruct()
//    就 return，從來走不到 formOverlay()；HotPlate 走第二型。所以哨兵法、Find／kForms、清單裡
//    的第一型項目、FormPageJson 的第一型退路一起拿掉（產生檔與產生器同一個 commit 刪除）。
//    留下：FormLock／FormUnlock（多個 WS 分派與 WebBuilder／WebLotInfo／WebRecipeChange 都用它）、
//    Guard、第二型的 GET（BridgePageJson）與 form.save（FormSave）。沒有第二型 bridge 的頁 → 404。
// ===========================================================================
#include "JsonBridge/FormJson.h"

#include <windows.h>

#include "WebBridge/JsonWriter.h"
#include "JsonBridge/FormBridge.h"   // Steven 20260924：第二型（golden 原檔產生的 bridge）

#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace ht9045 {
namespace formjson {

namespace {

CRITICAL_SECTION g_lock;
struct LockInit {
    LockInit() { ::InitializeCriticalSection(&g_lock); }
} g_lockInit;

}  // namespace

void FormLock()   { ::EnterCriticalSection(&g_lock); }
void FormUnlock() { ::LeaveCriticalSection(&g_lock); }

namespace {
// Steven 20260924 審查更正（高級審查員 #2）：golden 本體丟例外時鎖一定要放，否則 /api/form 從此卡死。
struct Guard {
    Guard()  { FormLock(); }
    ~Guard() { FormUnlock(); }
};
}  // namespace

int FormListJson(std::string* json) {
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("note").String("golden-bridge pages only: golden's own display code fills the widget state "
                         "(FormBridge.h; type 1 retired 20260927, S126)");
    w.Key("forms").BeginArray();
    for (std::size_t i = 0; i < formbridge::BridgeCount(); ++i) {
        const formbridge::BridgeDesc* b = formbridge::BridgeAt(i);
        w.BeginObject();
        w.Key("page").String(b->page);
        w.Key("form").String(b->formClass);
        w.Key("kind").String("golden-bridge");
        w.Key("golden").String(b->golden);
        w.Key("saveable").Bool(b->saveFlow != nullptr && b->sourceGap[0] == '\0');
        w.Key("sourceGap").String(b->sourceGap);
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    *json = w.Str();
    return 200;
}

// ---------------------------------------------------------------------------
//  第二型：golden 原檔產生的 bridge（FormBridge.h）。有執行到的賦值才進 J，不需要哨兵。
// ---------------------------------------------------------------------------
static int BridgePageJson(const formbridge::BridgeDesc& b, std::string* json) {
    formbridge::FormState J;
    std::string err;
    {
        Guard lock;
        try {
            b.display(J);
        } catch (const std::exception& e) {
            err = e.what();
        } catch (...) {
            err = "non-std exception";
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("page").String(b.page);
    w.Key("form").String(b.formClass);
    w.Key("kind").String("golden-bridge");
    w.Key("golden").String(b.golden);
    w.Key("available").Bool(err.empty());
    if (!err.empty()) w.Key("why").String("golden display threw: " + err);
    // ⚠ sourceGap 非空：值是結構初值，頁面不可拿它蓋檔案值、也不可走 form.save。
    w.Key("sourceGap").String(b.sourceGap);
    w.Key("saveable").Bool(b.saveFlow != nullptr && b.sourceGap[0] == '\0');
    // form.save 要送的 widget（golden SaveSetupFile 讀到的全部）。頁面照這張送，少一個伺服器就拒寫。
    w.Key("saveReads").BeginArray();
    {
        std::stringstream ss(b.saveReads);
        for (std::string id; std::getline(ss, id, ',');) if (!id.empty()) w.String(id);
    }
    w.EndArray();
    w.Key("widgets").RawValue(J.WidgetsJson());
    w.Key("messages").RawValue(J.MessagesJson());
    w.Key("todo").RawValue(J.TodoJson());
    w.EndObject();
    *json = w.Str();
    return 200;
}

int FormSave(const std::string& page, const std::string& widgetsJson,
             std::string* ackJson, std::string* err) {
    const formbridge::BridgeDesc* b = formbridge::FindBridge(page);
    if (!b)              { *err = "no golden bridge for this page (GET /api/form/ lists them)"; return 404; }
    if (!b->saveFlow)    { *err = "this page's bridge has no save flow"; return 405; }
    if (b->sourceGap[0]) {
        // golden SaveSetupFile 會把整頁寫回檔案；讀檔端不在，頁面上的值不可信 —— 拒寫。
        *err = std::string("refused: reader not ported -- ") + b->sourceGap +
               " (save through recipe.doc.put until it lands)";
        return 409;
    }
    formbridge::FormState J;
    if (!J.FromJson(widgetsJson, err)) return 400;
    // 寫檔前先空跑 golden SaveSetupFile 一次，寫到 %TEMP% 的暫存夾，記下「golden 這次真的讀了、
    // 頁面卻沒送」的屬性。有缺就整筆拒寫、真檔一個鍵都不動 —— golden 會把整頁寫回檔案，缺值＝寫預設值進配方。
    // Steven 20260924：原本用產生器靜態抽的 saveReads 檢查，會把條件分支裡才讀的欄位也算進去
    // （例：HotPlate 的 chkTrayHotplateCheck 只在 bVTESTFunction 時才存），非 VTEST 機台會永遠存不了。
    if (b->save) {
        char tmp[MAX_PATH];
        ::GetTempPathA(MAX_PATH, tmp);
        const std::string dry = std::string(tmp) + "wb_formsave_dry";
        ::CreateDirectoryA(dry.c_str(), 0);
        formbridge::FormState D = J;
        std::string what;
        {
            Guard lock;
            try {
                b->save(D, AnsiString(dry.c_str()), AnsiString("dry"));
            } catch (const std::exception& e) {
                what = e.what();
            } catch (...) {
                what = "non-std exception";
            }
            // ⚠ 這裡**不可以**呼叫 CloseIniFile()：移植樹照 golden 的原樣，它 delete INIFile 之後不把指標設 NULL
            //   （common.cpp:588「faithful bug」），golden 只在 OpenIniFile 重新 new 之前呼叫它。
            //   在這裡多呼叫一次，實跑的第一個 WriteIniData 會在 OpenIniFile 讀到已釋放的 INIFile 而當掉
            //   （Steven 20260924 實測：HotPlate form.save 讓 wb_serve 直接終止）。
            //   不呼叫也沒問題：實跑寫的是別的檔名，OpenIniFile 會自己關掉暫存檔的快取再開真檔。
        }
        if (!what.empty()) {
            *err = "refused: golden save threw during the dry run (nothing written): " + what;
            return 400;
        }
        if (!D.MissingReads().empty()) {
            char n[32];
            std::snprintf(n, sizeof(n), "%u", (unsigned)D.MissingReads().size());   // MinGW 6.3：不用 std::to_string
            *err = std::string("refused: golden save reads ") + n +
                   " value(s) the page did not send (nothing written):";
            for (std::size_t i = 0; i < D.MissingReads().size(); ++i)
                *err += (i ? ", " : " ") + D.MissingReads()[i];
            return 400;
        }
    }
    {
        Guard lock;
        std::string what;
        try {
            b->saveFlow(J);
        } catch (const std::exception& e) {
            what = e.what();
        } catch (...) {
            what = "non-std exception";
        }
        if (!what.empty()) {
            // golden 在 VCL 裡是 EConvertError 對話框（例：數字欄位填了字）。這裡回報，不讓伺服器掛掉。
            // ⚠ 例外之前已寫的鍵不會回滾 —— 與 golden 相同（WriteIniData 逐鍵落地）。
            *err = "golden save threw: " + what +
                   " (keys written before the throw are NOT rolled back, same as golden)";
            return 500;
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("page").String(page);
    // 高級審查員第二輪：golden 存檔鈕可能在呼叫 SaveSetupFile 之前就 return（沒寫檔）
    w.Key("saved").Bool(J.M("saved") != 0);
    w.Key("closed").Bool(J.M("closed") != 0);
    w.Key("messages").RawValue(J.MessagesJson());
    w.Key("todo").RawValue(J.TodoJson());
    w.EndObject();
    *ackJson = w.Str();
    return 200;
}

int FormPageJson(const std::string& page, std::string* json) {
    if (const formbridge::BridgeDesc* b = formbridge::FindBridge(page)) return BridgePageJson(*b, json);
    webbridge::JsonWriter w;                  // 第一型退役（S126）：沒有第二型 bridge 的頁一律 404
    w.BeginObject();
    w.Key("page").String(page);
    w.Key("error").String("no golden bridge for this page (GET /api/form/ lists them)");
    w.EndObject();
    *json = w.Str();
    return 404;
}

}  // namespace formjson
}  // namespace ht9045
