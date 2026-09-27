// ===========================================================================
//  JsonBridge/FormJson.cpp -- S12 本體。說明見 FormJson.h。
//
//  Steven 20260924.  NOT in golden.
//
//  ---------------------------------------------------------------------------
//  怎麼知道 DoIniDataToForm() 這次「有賦值」哪些屬性：兩輪哨兵法
//  ---------------------------------------------------------------------------
//  golden 的 DoIniDataToForm() 大量用條件分支（IniConfig／CosFunction 旗標）
//  決定要不要碰某個 widget。沒碰到的 widget 在 VCL 裡保留 dfm 的值；
//  在這裡保留的是 vclcompat 建構子的值（Text=""、Visible=true），兩者不同。
//  所以不能把所有 widget 照送，也不能靠靜態分析（分支是執行期決定的）。
//
//    第 1 輪：值屬性放哨兵（Text/Caption = kSentinel、ItemIndex/Position = kIntSentinel），
//             布林屬性（Checked/Down/Visible/Enabled）全放 false，然後呼叫。
//             值屬性 != 哨兵 ⇒ 有賦值。
//    第 2 輪：只在有布林屬性時跑。布林屬性全放 true，值屬性再放哨兵，再呼叫一次。
//             布林屬性兩輪結果相同 ⇒ 有賦值（沒賦值的會一輪 false、一輪 true）。
//    收尾：  沒賦值的屬性還原成呼叫前的值，有賦值的保留（等於 golden 呼叫後的狀態）。
//
//  ⚠ 代價：DoIniDataToForm() 可能被呼叫兩次。golden 自己也這樣做過
//    （cSetUp.cpp:1861/:1863 連叫兩次，註解「再做一次，不然 Pitch 的數字會被改掉」），
//    而 25 個 golden 本體的副作用只有 MyForceDirectories（冪等）與少數全域字串
//    （例：cContact 的 asContactHeight[]，第二輪寫入同一個值）。
//  ⚠ 假設 DoIniDataToForm() 對同一份結構是確定性的。`X->Checked = !X->Checked`
//    這種讀自己再寫回的寫法會被判成「沒賦值」—— 目前 11 個已翻本體裡沒有。
// ===========================================================================
#include "JsonBridge/FormJson.h"

#include <windows.h>

#include <climits>
#include <cstring>

#include "WebBridge/JsonWriter.h"
#include "JsonBridge/FormBridge.h"   // Steven 20260924：第二型（golden 原檔產生的 bridge）

#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace ht9045 {
namespace formjson {

extern const FormDesc* const kForms[];
extern const std::size_t kFormCount;

namespace {

const char* const kSentinel = "\x01" "S12-unset" "\x01";
const int kIntSentinel = INT_MIN + 7;

CRITICAL_SECTION g_lock;
struct LockInit {
    LockInit() { ::InitializeCriticalSection(&g_lock); }
} g_lockInit;

// 一個 widget 在某一刻的全部屬性。只存會用到的，照 kind 決定哪些有意義。
struct Snap {
    AnsiString text, caption;
    int  itemIndex = 0, position = 0;
    bool checked = false, down = false, visible = true, enabled = true;
};

bool HasText(WidgetKind k)    { return k == kEdit || k == kComboBox; }
bool HasCaption(WidgetKind k) {
    return k == kCheckBox || k == kRadioButton || k == kLabel ||
           k == kPanel || k == kGroupBox || k == kSpeedButton || k == kButton;
}
bool HasIndex(WidgetKind k)   { return k == kRadioGroup || k == kComboBox || k == kListBox; }
bool HasChecked(WidgetKind k) { return k == kCheckBox || k == kRadioButton; }

AnsiString& TextRef(const WidgetRef& w) {
    if (w.kind == kComboBox) return static_cast<TComboBox*>(w.ctl)->Text;
    return static_cast<TCustomEdit*>(w.ctl)->Text;
}
AnsiString& CaptionRef(const WidgetRef& w) {
    switch (w.kind) {
    case kCheckBox:    return static_cast<TCheckBox*>(w.ctl)->Caption;
    case kRadioButton: return static_cast<TRadioButton*>(w.ctl)->Caption;
    case kLabel:       return static_cast<TLabel*>(w.ctl)->Caption;
    case kPanel:       return static_cast<TPanel*>(w.ctl)->Caption;
    case kGroupBox:    return static_cast<TGroupBox*>(w.ctl)->Caption;
    case kSpeedButton: return static_cast<TSpeedButton*>(w.ctl)->Caption;
    default:           return static_cast<TButton*>(w.ctl)->Caption;   // kButton：TButton／TBitBtn 都有 Caption
    }
}
int& IndexRef(const WidgetRef& w) {
    if (w.kind == kRadioGroup) return static_cast<TRadioGroup*>(w.ctl)->ItemIndex;
    if (w.kind == kComboBox)   return static_cast<TComboBox*>(w.ctl)->ItemIndex;
    return static_cast<TListBox*>(w.ctl)->ItemIndex;
}
bool& CheckedRef(const WidgetRef& w) {
    if (w.kind == kCheckBox) return static_cast<TCheckBox*>(w.ctl)->Checked;
    return static_cast<TRadioButton*>(w.ctl)->Checked;
}

Snap Take(const WidgetRef& w) {
    Snap s;
    if (HasText(w.kind))    s.text = TextRef(w);
    if (HasCaption(w.kind)) s.caption = CaptionRef(w);
    if (HasIndex(w.kind))   s.itemIndex = IndexRef(w);
    if (HasChecked(w.kind)) s.checked = CheckedRef(w);
    if (w.kind == kScrollBar)   s.position = static_cast<vclcompat::TScrollBar*>(w.ctl)->Position;
    if (w.kind == kSpeedButton) s.down = static_cast<TSpeedButton*>(w.ctl)->Down;
    s.visible = w.ctl->Visible;
    s.enabled = w.ctl->Enabled;
    return s;
}

void Put(const WidgetRef& w, const Snap& s) {
    if (HasText(w.kind))    TextRef(w) = s.text;
    if (HasCaption(w.kind)) CaptionRef(w) = s.caption;
    if (HasIndex(w.kind))   IndexRef(w) = s.itemIndex;
    if (HasChecked(w.kind)) CheckedRef(w) = s.checked;
    if (w.kind == kScrollBar)   static_cast<vclcompat::TScrollBar*>(w.ctl)->Position = s.position;
    if (w.kind == kSpeedButton) static_cast<TSpeedButton*>(w.ctl)->Down = s.down;
    w.ctl->Visible = s.visible;
    w.ctl->Enabled = s.enabled;
}

Snap Primed(bool b) {
    Snap s;
    s.text = AnsiString(kSentinel);
    s.caption = AnsiString(kSentinel);
    s.itemIndex = kIntSentinel;
    s.position = kIntSentinel;
    s.checked = s.down = s.visible = s.enabled = b;
    return s;
}

bool IsSentinel(const AnsiString& a) { return std::strcmp(a.c_str(), kSentinel) == 0; }

const FormDesc* Find(const std::string& page) {
    for (std::size_t i = 0; i < kFormCount; ++i)
        if (page == kForms[i]->page) return kForms[i];
    return nullptr;
}

void Head(webbridge::JsonWriter& w, const FormDesc& f) {
    w.Key("page").String(f.page);
    w.Key("form").String(f.formClass);
    w.Key("portSource").String(f.portSource);
    w.Key("golden").String(f.golden);
}

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
    w.Key("note").String("C++ runs TfXxx::DoIniDataToForm() and sends the widget state "
                         "(decisions.md 二之二, phases.md S12)");
    w.Key("forms").BeginArray();
    // 第二型優先：同一頁若兩型都有，GET 走第二型（FormPageJson 同順序）。
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
    for (std::size_t i = 0; i < kFormCount; ++i) {
        if (formbridge::FindBridge(kForms[i]->page)) continue;
        w.BeginObject();
        Head(w, *kForms[i]);
        w.Key("kind").String("port-DoIniDataToForm");
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
    const FormDesc* f = Find(page);
    webbridge::JsonWriter w;
    w.BeginObject();
    if (!f) {
        w.Key("page").String(page);
        w.Key("error").String("no C++ form is wired to this page yet (GET /api/form/ lists them)");
        w.EndObject();
        *json = w.Str();
        return 404;
    }
    Head(w, *f);

    std::vector<WidgetRef> ws;
    Guard lock;                        // 審查更正 #2：整段持鎖，例外也會放
    const bool live = f->collect(&ws);
    if (!live) {
        w.Key("available").Bool(false);
        w.Key("why").String("form instance is NULL in this process");
        w.EndObject();
        *json = w.Str();
        return 200;
    }

    const std::size_t n = ws.size();
    std::vector<Snap> before(n), a(n), b(n);
    bool anyBool = false;
    for (std::size_t i = 0; i < n; ++i) before[i] = Take(ws[i]);

    // 每個 widget 至少有 Visible／Enabled 兩個布林屬性，所以第 2 輪一定要跑。
    anyBool = n > 0;
    int runs = 1;
    std::string threw;
    try {
        // 第 1 輪
        for (std::size_t i = 0; i < n; ++i) Put(ws[i], Primed(false));
        f->run();
        for (std::size_t i = 0; i < n; ++i) a[i] = Take(ws[i]);
        if (anyBool) {
            for (std::size_t i = 0; i < n; ++i) Put(ws[i], Primed(true));
            f->run();
            for (std::size_t i = 0; i < n; ++i) b[i] = Take(ws[i]);
            runs = 2;
        }
    } catch (const std::exception& e) {
        threw = e.what();
    } catch (...) {
        threw = "non-std exception";
    }
    if (!threw.empty()) {
        // 審查更正 #2：丟例外時 widget 還停在哨兵值 —— 一定先還原成呼叫前，別讓 tick 執行緒讀到哨兵。
        for (std::size_t i = 0; i < n; ++i) Put(ws[i], before[i]);
        w.Key("available").Bool(false);
        w.Key("why").String("DoIniDataToForm threw: " + threw);
        w.EndObject();
        *json = w.Str();
        return 200;
    }

    // 收尾：有賦值的保留第 1 輪的值，沒賦值的還原成呼叫前。
    int assigned = 0;
    w.Key("available").Bool(true);
    w.Key("runs").Number((wb_int64)runs);
    w.Key("widgets").BeginObject();
    for (std::size_t i = 0; i < n; ++i) {
        const WidgetRef& r = ws[i];
        const Snap& p = before[i];
        Snap keep = p;
        bool opened = false;
        auto open = [&]() {
            if (!opened) { w.Key(r.name).BeginObject(); opened = true; }
        };
        if (HasText(r.kind) && !IsSentinel(a[i].text)) {
            keep.text = a[i].text; open(); w.Key("text").String(a[i].text.c_str());
        }
        if (HasCaption(r.kind) && !IsSentinel(a[i].caption)) {
            keep.caption = a[i].caption; open(); w.Key("caption").String(a[i].caption.c_str());
        }
        if (HasIndex(r.kind) && a[i].itemIndex != kIntSentinel) {
            keep.itemIndex = a[i].itemIndex; open(); w.Key("itemIndex").Number((wb_int64)a[i].itemIndex);
        }
        if (r.kind == kScrollBar && a[i].position != kIntSentinel) {
            keep.position = a[i].position; open(); w.Key("position").Number((wb_int64)a[i].position);
        }
        if (runs == 2) {
            if (HasChecked(r.kind) && a[i].checked == b[i].checked) {
                keep.checked = a[i].checked; open(); w.Key("checked").Bool(a[i].checked);
            }
            if (r.kind == kSpeedButton && a[i].down == b[i].down) {
                keep.down = a[i].down; open(); w.Key("down").Bool(a[i].down);
            }
            if (a[i].visible == b[i].visible) {
                keep.visible = a[i].visible; open(); w.Key("visible").Bool(a[i].visible);
            }
            if (a[i].enabled == b[i].enabled) {
                keep.enabled = a[i].enabled; open(); w.Key("enabled").Bool(a[i].enabled);
            }
        }
        if (opened) { w.EndObject(); ++assigned; }
        Put(r, keep);
    }
    w.EndObject();

    w.Key("widgetCount").Number((wb_int64)n);
    w.Key("assignedCount").Number((wb_int64)assigned);
    w.Key("skipped").String(f->skipped);
    w.EndObject();
    *json = w.Str();
    return 200;
}

}  // namespace formjson
}  // namespace ht9045
