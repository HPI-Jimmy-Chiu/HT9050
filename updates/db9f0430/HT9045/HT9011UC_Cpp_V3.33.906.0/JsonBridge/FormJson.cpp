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
void FormUnlock() { ::LeaveCriticalSection(&g_lock); }  bool FormTryLock() { return ::TryEnterCriticalSection(&g_lock) != 0; }   //AI(W906-FASTCLK) 20261003: the non-blocking acquire for the tick thread (FileRW/_ProxyTry.cpp; RULINGS_20261002 #7 「主迴圈不該等 HTTP 執行緒」): true = acquired (free, or already this thread's -- the lock is recursive), pair it with FormUnlock; false = another thread (the HTTP socket thread) holds it. Same line, no line below moves

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
static bool FormSaveRunningRefused(std::string* err);   // AI(W906-FRW-S158) 20260927 [W906]: R87（運轉中 form.save 也擋），定義在檔尾；佔用原本的空行
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
    if (!b->saveFlow)    { *err = "this page's bridge has no save flow"; return 405; }   if (FormSaveRunningRefused(err)) return 409;   // AI(W906-FRW-S158) 20260927 [W906]: R87 —— 運轉中 form.save 也擋（同 editlist.save 的 RULINGS_20260927 #7，tools/wb_serve.cpp:5303），在任何解析與寫檔之前；接在同一行
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
            // 這裡不必呼叫 CloseIniFile()（原本是「不可以」）：golden 原樣 delete INIFile 之後不把指標設 NULL，
            //   移植樹 4d3468a3（20260928）起改成設 nullptr（common.cpp:587-588）；golden 也只在 OpenIniFile 重新 new 之前呼叫它。
            //   4d3468a3 之前在這裡多呼叫一次，實跑的第一個 WriteIniData 會在 OpenIniFile 讀到已釋放的 INIFile 而當掉
            //   （Steven 20260924 實測：HotPlate form.save 讓 wb_serve 直接終止）；現在指標會歸零，照程式看不會再當，仍不呼叫。
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

// ===========================================================================
//  AI(W906-FRW-S158) 20260927 [W906]: decisions-pending R87（St01 建議 A，Steven 沒反對就照 A）—— WS form.save（A 形狀，目前只有
//    Setup.HotPlate.html：FileRW/HotPlateForm_File.cpp）運轉中也擋，跟 editlist.save 同一條規則：RULINGS_20260927 第 2 條第 7 題 A
//    （Steven 0927 07:2x）「機台運轉中，網頁一律不能存設定」，editlist.save 那一道在 tools/wb_serve.cpp:5303（f45b92f5），當時註明
//    form.save「另列給 Steven 決定」＝R87。golden 運轉中打不開設定畫面：V912 main.cpp:3970-3978 DoMainPadProcess SystemStart 時把
//    palSetup／palConfig 藏起來。
//  做在 FormSave（上面 :119 同一行呼叫），不改 tools/wb_serve.cpp：form.save 的唯一入口就是 FormSave。位置在「找不到 bridge／沒有存檔流程」
//    之後、sourceGap 與空跑（寫 %TEMP%）之前 —— 任何解析與寫檔之前擋，同 wb_serve.cpp:5303 的位置規則。
//  回的字：碼 "running:"（同 FileRW/_EditPage.cpp 開窗閘的碼）＋ wb_serve.cpp:5303 的兩句拒絕字樣（SystemStart／SoftStart 各一句），
//    後面註明是 form.save（R87）。wb_serve 的 form.save 分支會印 "form.save <page> -> 409 running: …"（:5362），不另外印。
//    HTTP 式狀態碼 409（同 sourceGap 拒寫）。
// ===========================================================================
extern bool SystemStart;   // cmydef.h:221（cmydef.cpp:286）；不 include cmydef.h（同 FileRW/_FormEvent.cpp:29-30）
extern bool SoftStart;     // cmydef.h:223（cmydef.cpp:288）

namespace ht9045 {
namespace formjson {

static bool FormSaveRunningRefused(std::string* err) {
    if (!(SystemStart || SoftStart)) return false;
    *err = SystemStart
        ? "running: 機台運轉中（SystemStart）不能從網頁存設定 —— golden 運轉中打不開設定畫面（V912 main.cpp:3970-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來）（form.save，R87）"
        : "running: 機台正要啟動或回原點（SoftStart）——這時不能從網頁存設定；golden 在設定畫面開著時根本不會進入這個狀態（form.save，R87）";
    return true;
}

}  // namespace formjson
}  // namespace ht9045
