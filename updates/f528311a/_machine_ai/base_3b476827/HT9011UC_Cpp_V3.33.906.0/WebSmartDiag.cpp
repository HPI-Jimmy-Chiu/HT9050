// =============================================================================
//  WebSmartDiag.cpp -- Data.SmartDiagnostic.html <-> golden V912 TfSmartDiagnostic.
//
//  Steven 20260925 (Data.SmartDiagnostic). NOT in golden as a file: the golden
//  bodies are the facade's (forms/fSmartDiagnostic.cpp + the repo-root
//  SmartDiagnostic.cpp, translated from golden V912 SmartDiagnostic.cpp in
//  HT9011UC_Code_V3.33.912.0_20260908_Jimmy); this file only replays the clicks on them and
//  turns the form into JSON for the WS command. wb_serve-only (same reason as
//  WebSortCT.cpp: it needs WebBridge's JsonWriter and cJSON).
//
//  ⚠ DISPATCH NOT WIRED HERE: tools/wb_serve.cpp is the integrator's; the arm to add
//    is in WebSmartDiag.h. The page (web/page/ht9045_smartdiag_web.js) sends
//    `smartdiag.op`, value = JSON string.
//
//  ACTS (value {"act":...}) -- each one is one golden operation:
//    open    golden route to the form + ShowModal: palConfig (main.cpp:29009-29016
//            sbConfigClick: SystemStart -> no; fSecurity->Insufficient(1)) ->
//            sbStartModeClick (:28546-28553, Insufficient(24)) -> Start Condition's
//            sb_Maintenance_SmartDiagnosticFunctionClick (cStartCondition.cpp:1254-
//            1258) -> fSmartDiagnostic->ShowModal(). Then W906_Create (golden's
//            boot-time ctor+OnCreate, once) and FormShow (:274-357).
//    timer   one SmartDiagnosticTimer tick (dfm :1062-1067, no Interval -> VCL
//            1000 ms; Enabled = bStartRecord, ctor :24): SmartDiagnosticTimerTimer
//            :729-783 (paints alarm rows red). Only while the page is open -- golden
//            ticks in the background too; it only changes colours.
//    create  "Create Initial Cylider Name" (:594-623), two-phase confirmation.
//    reset   "Reset Cylider Record" (:692-724) after picking itemIndex in the
//            combo, two-phase. (Golden quirk kept: resets row 1 -- see the body.)
//    save    title "Save" = FormButtonClick Tag==9 (:172-180), two-phase; "edits"
//            = what the operator typed into the two parameter boxes. WRITES
//            system\SmartDiagnosticRecord.txt, SmartDiagnosticRecordReset.txt,
//            SmartDiagnosticPara.ini.
//    cell    click an editable sgCylinder cell (sgCylinderSelectCell :854-865; the
//            integer box MyInputBox answers with "value"). Memory only (golden
//            never persists it -- SaveCylinderData is unreachable).
//
//  TWO-PHASE CONFIRMATION (golden MessageDlg at :174 / :597 / :698):
//    confirmed=false -> handler runs with the answer preset to mrNo; golden asks
//      before touching anything, so this has no effect; the response is
//      {"needConfirm":true,"prompt":["<golden text>"]}.
//    confirmed=true  -> guards re-checked (never trust the browser), the handler runs
//      with mrYes -> {"executed":true,...}.
//  GUARDS on create/reset/save/cell: the same three route checks as open, quietly
//  (fSecurity->Insufficient(n,false): the operator already got past them to open
//  the page; re-asking with the alarm would pop WAR1676 on every click).
//
//  Every response carries "state" (the whole form) so the page just re-renders.
// =============================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>

#include "WebSmartDiag.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fSmartDiagnostic.h"
#include "forms/fSecurity.h"   // fSecurity->Insufficient (cSecurity.cpp, ht9045_sm)
#include "cmydef.h"            // SystemStart
#include "canary_support.h"    // W906_ShowMyMessage_Hook (captured into "messages")

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

const int kMrYes = 6, kMrNo = 7;   // VCL Controls.hpp (same values as forms/fSmartDiagnostic.cpp)

// ---- ShowMyMessage capture: chain in front of wb_serve's hook for one request ----
std::vector<std::pair<std::string, std::string> >* g_msgs = 0;
void (*g_prevHook)(const char*, const char*) = 0;
void CaptureHook(const char* s1, const char* s2)
{
    if (g_msgs) g_msgs->push_back(std::make_pair(std::string(s1 ? s1 : ""), std::string(s2 ? s2 : "")));
    if (g_prevHook) g_prevHook(s1, s2);
}
struct MsgCapture {
    std::vector<std::pair<std::string, std::string> > msgs;
    MsgCapture()  { g_prevHook = W906_ShowMyMessage_Hook; g_msgs = &msgs; W906_ShowMyMessage_Hook = CaptureHook; }
    ~MsgCapture() { W906_ShowMyMessage_Hook = g_prevHook; g_msgs = 0; g_prevHook = 0; }
};

// ---- text: literals in this tree are UTF-8, file contents are ANSI (cp950) ----
std::string U8(const AnsiString& a)
{
    const std::string s = a.c_str();
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return webbridge::SanitizeToUtf8(s);
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int un = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string u((size_t)(un > 0 ? un : 0), '\0');
    if (un > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &u[0], un, NULL, NULL);
    return u;
}

// TColor (0x00BBGGRR, or 0x800000nn = system colour nn) -> "#rrggbb"
std::string Css(int c)
{
    unsigned u = (unsigned)c;
    if ((u & 0xFF000000u) == 0x80000000u) u = (unsigned)GetSysColor((int)(u & 0xFFu));
    char b[8];
    std::snprintf(b, sizeof(b), "#%02x%02x%02x", u & 0xFF, (u >> 8) & 0xFF, (u >> 16) & 0xFF);
    return b;
}

void WriteGrid(webbridge::JsonWriter& w, TStringGrid* g, TColor** colors, int colorCols, int colorRows)
{
    const int rc = g->RowCount, cc = g->ColCount;
    w.BeginObject();
    w.Key("rowCount").Number((wb_int64)rc);
    w.Key("colCount").Number((wb_int64)cc);
    w.Key("colWidths").BeginArray();
    for (int c = 0; c < cc; ++c) w.Number((wb_int64)(int)g->ColWidths[c]);
    w.EndArray();
    w.Key("cells").BeginArray();
    for (int r = 0; r < rc; ++r) {
        w.BeginArray();
        for (int c = 0; c < cc; ++c) w.String(U8(AnsiString(g->Cells[c][r])));
        w.EndArray();
    }
    w.EndArray();
    if (colorCols > 0) {
        // golden DrawCell (:366-384 / :1036-1053) fills every cell with GridColor[c][r];
        // null = no colour was ever allocated for that cell (the page keeps its default)
        w.Key("colors").BeginArray();
        for (int r = 0; r < rc; ++r) {
            w.BeginArray();
            for (int c = 0; c < cc; ++c) {
                if (colors && c < colorCols && r < colorRows) w.String(Css((int)colors[c][r]));
                else w.Null();
            }
            w.EndArray();
        }
        w.EndArray();
    }
    w.EndObject();
}

void WriteFile(webbridge::JsonWriter& w, const char* key, const AnsiString& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fa;
    const bool ex = GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fa) != 0 &&
                    !(fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    w.BeginObject();
    w.Key("key").String(key);
    w.Key("path").String(U8(path));
    w.Key("exists").Bool(ex);
    if (ex) w.Key("size").Number((wb_int64)fa.nFileSizeLow);
    w.EndObject();
}

void WriteState(webbridge::JsonWriter& w, TfSmartDiagnostic* f)
{
    w.BeginObject();
    w.Key("created").Bool(f->bW906Created);
    w.Key("activePage").Number((wb_int64)f->pc_SmartDiagnostic->ActivePageIndex);
    w.Key("timeCaption").String(U8(f->pn_SmartDiagnostic_Time->Caption));
    w.Key("timerEnabled").Bool(f->SmartDiagnosticTimer->Enabled);
    w.Key("startRecord").Bool(f->bStartRecord);
    w.Key("limitCountValue").Number((wb_int64)f->iLimitCountValue);
    w.Key("limitCheckTime").Number((wb_int64)f->iLimitCheckTime);
    w.Key("edits").BeginObject();
    w.Key("ed_SmartDiagnostic_LimitCountValue").String(U8(f->ed_SmartDiagnostic_LimitCountValue->Text));
    w.Key("ed_SmartDiagnostic_LimitCheckTime").String(U8(f->ed_SmartDiagnostic_LimitCheckTime->Text));
    w.EndObject();
    w.Key("visible").BeginObject();
    w.Key("pn_SmartDiagnostic_Parameter").Bool(f->pn_SmartDiagnostic_Parameter->Visible);
    w.Key("pn_SmartDiagnostic_CyliderManagement").Bool(f->pn_SmartDiagnostic_CyliderManagement->Visible);
    w.EndObject();
    w.Key("combo").BeginObject();
    w.Key("items").BeginArray();
    for (int i = 0; i < f->cob_SmartDiagnostic_CyliderName->Items->Count; ++i)
        w.String(U8(f->cob_SmartDiagnostic_CyliderName->Items->Strings[i]));
    w.EndArray();
    w.Key("itemIndex").Number((wb_int64)f->cob_SmartDiagnostic_CyliderName->ItemIndex);
    w.EndObject();
    w.Key("summary");    WriteGrid(w, f->sg_SmartDiagnostic_Summary, f->GridColor, 6, f->iW906GridColorRows);
    w.Key("management"); WriteGrid(w, f->sg_SmartDiagnostic_CyliderManagement, 0, 0, 0);
    w.Key("cylinder");   WriteGrid(w, f->sgCylinder, f->CylinderGridColor, f->CylinderGridColor ? 9 : 0, 11);
    w.Key("files").BeginArray();
    WriteFile(w, "record", f->aSmartDiagnosticFilePath);
    WriteFile(w, "reset",  f->aSmartDiagnosticResetFilePath);
    WriteFile(w, "para",   f->aSmartDiagnosticParaPath);
    WriteFile(w, "preAlm", f->aCylinderPreAlm);
    w.EndArray();
    w.EndObject();
}

struct Guard { const char* guard; const char* goldenLine; const char* detail; };

// golden route to the form; alarm=true only for "open" (the operator's first click)
const Guard* RouteGuard(bool alarm)
{
    static const Guard kRun  = { "SystemStart", "golden V912 main.cpp:29012-29013 sbConfigClick",
        "機台運轉中不能開（Start Condition 的入口 sbStartMode 在 palConfig 裡；DoMainPadProcess :3970-3978 運轉中把 palConfig 藏起來）" };
    static const Guard kLv1  = { "not-authorized", "golden V912 main.cpp:29015 sbConfigClick fSecurity->Insufficient(1)",
        "權限不足（[1] Config，system\\levelset.dat）" };
    static const Guard kLv24 = { "not-authorized", "golden V912 main.cpp:28549 sbStartModeClick fSecurity->Insufficient(24)",
        "權限不足（[24] Start Condition；SmartDiagnostic 由 Start Condition 的 sb_Maintenance_SmartDiagnosticFunction 開，cStartCondition.cpp:1254-1258）" };
    if (SystemStart) return &kRun;
    if (fSecurity == 0 || fSecurity->Insufficient(1, alarm) == false) return &kLv1;
    if (fSecurity->Insufficient(24, alarm) == false) return &kLv24;
    return 0;
}

bool IntItem(const cJSON* root, const char* key, int* out)
{
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!j || !cJSON_IsNumber(j)) return false;
    const double d = j->valuedouble;
    if (d != (double)(int)d) return false;
    *out = (int)d;
    return true;
}

bool OkEditText(const std::string& s)
{
    if (s.size() > 64) return false;
    for (size_t i = 0; i < s.size(); ++i) if ((unsigned char)s[i] < 0x20) return false;
    return true;
}

}  // namespace

std::string W906_SmartDiagOp(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;
    TfSmartDiagnostic* f = fSmartDiagnostic;

    std::string act = "open", err;
    bool confirmed = false, hasValue = false;
    int itemIndex = -2, col = -1, row = -1, value = 0;
    bool hasItemIndex = false;
    std::vector<std::pair<std::string, std::string> > edits;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (!root) err = "value 不是合法 JSON";
        else {
            const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "act");
            if (ja && cJSON_IsString(ja) && ja->valuestring) act = ja->valuestring;
            else if (ja) err = "act 要是字串";
            const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
            confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));              // 明確 true 才算確認過
            hasItemIndex = IntItem(root, "itemIndex", &itemIndex);
            IntItem(root, "col", &col);
            IntItem(root, "row", &row);
            const cJSON* jv = cJSON_GetObjectItemCaseSensitive(root, "value");
            if (jv && !cJSON_IsNull(jv)) {
                if (!IntItem(root, "value", &value)) err = "value 要是整數（golden MyInputBox 是整數框，INPUT.cpp:84-102）";
                else hasValue = true;
            }
            const cJSON* je = cJSON_GetObjectItemCaseSensitive(root, "edits");
            if (je && cJSON_IsObject(je)) {
                for (const cJSON* it = je->child; it; it = it->next) {
                    const std::string k = it->string ? it->string : "";
                    if (k != "ed_SmartDiagnostic_LimitCountValue" && k != "ed_SmartDiagnostic_LimitCheckTime") { err = "edits 只收兩個參數框"; break; }
                    if (!cJSON_IsString(it) || !OkEditText(it->valuestring)) { err = "edits." + k + " 要是 64 字以內、沒有控制字元的字串"; break; }
                    edits.push_back(std::make_pair(k, std::string(it->valuestring)));
                }
            }
            cJSON_Delete(root);
        }
    }
    if (err.empty() && act != "open" && act != "timer" && act != "create" && act != "reset" && act != "save" && act != "cell")
        err = "unknown act '" + act + "'";
    if (f == 0) err = "fSmartDiagnostic is NULL";

    if (!err.empty()) {
        w.BeginObject();
        w.Key("act").String(act);
        w.Key("executed").Bool(false);
        w.Key("guard").String("bad-payload");
        w.Key("detail").String(err);
        w.EndObject();
        return w.Str();
    }

    FormLockGuard lock;
    MsgCapture cap;
    const Guard* g = 0;
    bool executed = false, needConfirm = false, full = true, reached = false, badArg = false;
    std::string prompt, excText;
    std::vector<std::string> writes;

    try {
        if (act == "open") {
            g = RouteGuard(true);
            if (!g) {
                f->W906_Create();
                f->FormShow(NULL);
                executed = true;
            }
        } else if (act == "timer") {
            full = false;
            if (f->bW906Created && f->SmartDiagnosticTimer->Enabled) f->SmartDiagnosticTimerTimer(NULL);
            executed = true;
        } else {
            g = RouteGuard(false);
            if (!g) {
                f->W906_Create();
                f->bW906DlgReached = false;
                f->asW906DlgPrompt = "";
                f->iW906DlgAnswer = confirmed ? kMrYes : kMrNo;
                if (act == "create") {
                    f->sb_SmarDiagnostic_CreateCyliderNameClick(NULL);
                } else if (act == "reset") {
                    const int n = f->cob_SmartDiagnostic_CyliderName->Items->Count;
                    if (!hasItemIndex || itemIndex < -1 || itemIndex >= n)
                        throw std::invalid_argument("itemIndex 要在 -1.." + std::to_string(n - 1) + "（combo 目前的項目）");
                    f->cob_SmartDiagnostic_CyliderName->ItemIndex = itemIndex;                      // 操作員在下拉選的那一項
                    f->cob_SmartDiagnostic_CyliderName->Text = (itemIndex >= 0) ? AnsiString(f->cob_SmartDiagnostic_CyliderName->Items->Strings[itemIndex]) : AnsiString("");
                    f->sb_SmartDiagnostic_ResetRecordCountClick(NULL);
                } else if (act == "save") {
                    for (size_t i = 0; i < edits.size(); ++i) {                                     // 操作員在兩個參數框打的字（golden 的框沒有 OnKeyPress，照收）
                        TEdit* e = edits[i].first == "ed_SmartDiagnostic_LimitCountValue" ? f->ed_SmartDiagnostic_LimitCountValue : f->ed_SmartDiagnostic_LimitCheckTime;
                        e->Text = edits[i].second.c_str();
                    }
                    f->FormButtonClick(f->sb_SmartDiagnostic_Save);                                  // Tag==9（SearchChangePageButton 綁的 OnClick）
                } else if (act == "cell") {
                    if (col < 0 || col >= f->sgCylinder->ColCount || row < 0 || row >= f->sgCylinder->RowCount)
                        throw std::invalid_argument("col/row 不在 sgCylinder 裡");
                    f->bW906InputHas = hasValue;
                    f->iW906InputValue = value;
                    bool canSelect = true;
                    f->sgCylinderSelectCell(NULL, col, row, canSelect);
                    f->bW906InputHas = false;
                    executed = true;
                }
                if (act != "cell") {
                    reached = f->bW906DlgReached;
                    prompt = f->asW906DlgPrompt.c_str();
                    if (reached && confirmed) executed = true;
                    else if (reached) needConfirm = true;
                }
                if (act == "save" && executed) {
                    writes.push_back(U8(f->aSmartDiagnosticFilePath));
                    writes.push_back(U8(f->aSmartDiagnosticResetFilePath));
                    writes.push_back(U8(f->aSmartDiagnosticParaPath));
                }
            }
        }
    } catch (const std::invalid_argument& e) {
        excText = e.what();
        badArg = true;
    } catch (const std::exception& e) {
        excText = e.what();
    } catch (...) {
        excText = "exception in golden TfSmartDiagnostic";
    }

    w.BeginObject();
    w.Key("act").String(act);
    w.Key("full").Bool(full);
    if (!excText.empty()) {
        w.Key("executed").Bool(false);
        w.Key("guard").String(badArg ? "bad-payload" : "exception");
        w.Key("detail").String(U8(AnsiString(excText.c_str())));
    } else if (g) {
        w.Key("executed").Bool(false);
        w.Key("guard").String(g->guard);
        w.Key("goldenLine").String(g->goldenLine);
        w.Key("detail").String(g->detail);
    } else {
        w.Key("executed").Bool(executed);
        if (needConfirm) {
            w.Key("needConfirm").Bool(true);
            w.Key("prompt").BeginArray().String(U8(AnsiString(prompt.c_str()))).EndArray();
        }
        if (act != "open" && act != "timer" && act != "cell") w.Key("confirmReached").Bool(reached);
        if (!writes.empty()) {
            w.Key("writes").BeginArray();
            for (size_t i = 0; i < writes.size(); ++i) w.String(writes[i]);
            w.EndArray();
        }
        if (ok) *ok = true;
    }
    w.Key("messages").BeginArray();
    for (size_t i = 0; i < cap.msgs.size(); ++i) {
        w.BeginObject();
        w.Key("s1").String(U8(AnsiString(cap.msgs[i].first.c_str())));
        w.Key("s2").String(U8(AnsiString(cap.msgs[i].second.c_str())));
        w.EndObject();
    }
    w.EndArray();
    if (full) {
        w.Key("state");
        WriteState(w, f);
    } else {
        w.Key("timerEnabled").Bool(f->SmartDiagnosticTimer->Enabled);
        w.Key("summaryColors").BeginArray();
        const int rc = f->sg_SmartDiagnostic_Summary->RowCount;
        for (int r = 0; r < rc; ++r) {
            w.BeginArray();
            for (int c = 0; c < 6; ++c) {
                if (f->GridColor && r < f->iW906GridColorRows) w.String(Css((int)f->GridColor[c][r]));
                else w.Null();
            }
            w.EndArray();
        }
        w.EndArray();
    }
    w.EndObject();

    if (act != "timer")
        std::printf("smartdiag.op act=%s confirmed=%d -> %s\n", act.c_str(), confirmed ? 1 : 0,
                    !excText.empty() ? excText.c_str() : g ? g->guard : needConfirm ? "needConfirm" : executed ? "executed" : "not-executed");
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
