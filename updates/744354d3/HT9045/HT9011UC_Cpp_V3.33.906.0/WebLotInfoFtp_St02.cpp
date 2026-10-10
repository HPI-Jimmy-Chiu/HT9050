// ===========================================================================
//  WebLotInfoFtp_St02.cpp -- the body of WS act.lotInfoFtp.<op> (card LI-9 F1): golden TfLotInfo::btnFtpServerClick and
//  the KYEC FTP dialog TfFTPClient (HD and Tester pages), for web/page/ht9045_lotinfo_ftp.js and ht9045_ftpclient.js.
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  Linked into wb_serve and tests/test_li9_ftpclient.cpp; reached through the
//  dispatcher JsonBridge/actions/LotInfoFtp.cpp once W906_St02_LotInfoFtpRegisterBody() ran (wb_serve: the installer
//  WebLotInfoFtpInstall_St02.cpp).  Every op holds FormLock (it can change the setup file and write config.ini).
//
//  value (a JSON object as a string; "{}" or empty for none)          golden
//    open        {"tag":0|1|2|3}                                       uLotInfo.cpp:5001-5126 btnFtpServerClick (Lot Info FTP tab)
//    state       {}                                                    read only: the dialog + the Lot Info FTP buttons
//    filter      {"edit":"hd","text":"<name>"}                         typing in edtHDWaferName -> OnChange -> FilterList (:2117 / :2098)
//    pick        {"list":"hd","index":n,"name":"<item>"}               lstHDFile double click (:2139)
//    loadHD      {}                                                    plLoad 'Load from HD' (:1575 plLoadClick)
//    enterHD     {}                                                    Enter in edtHDWaferName (:2206 edtHDWaferNameKeyPress)
//    upload      {}                                                    plUnload 'Upload to Server' (golden 913 :1128 plUnloadClick; F2-3)
//    filter      {"edit":"server","text":"<name>"}                     typing in edtServerWaferName (POOL-14 MR-A; golden 913 :2167 / :2143)
//    pick        {"list":"server","index":n,"name":"<item>"}           lstServerFile double click (golden 913 :2172)
//    copyName    {}                                                    Panel15 'Copy File Name' (golden 913 :2311 Panel15Click)
//    cleanName   {}                                                    Panel16 'Clean File Name' (golden 913 :2317 Panel16Click; CC_TSMC_TAINAN)
//    testerType  {"index":n} | {"text":"<typed>"}                      cbTesterType OnChange (:1831)
//    testerId    {"index":n} | {"text":"<typed>"}                      cbTesterID OnChange (:1954)
//    inputMethod {"index":0|1}                                         rgInputMethod OnClick (:2067)
//    testerName  {"text":"<name>"}                                     typing in edTesterName (no OnChange in the dfm)
//    saveTester  {"edTesterName":"<name>"?}                            btSafeTasterName 'Save' (:1868)
//    exit        {}                                                    Button3 'Exit' (:1703)
//    memoClear   {}                                                    memoFTP double click (:2062)
//    close       {}                                                    the window's close box = TForm.Close (CloseQuery :2258, FormClose :1655)
//  Reply: {"executed":bool, "op", "guard", "golden", "detail", "opened"?, "messages":[{s1,s2}], "gated":[...], "notes":[...],
//          "vclException"?, "session"? (loadHD), "state":{...}} -- "state" always, so a page redraws from C++'s truth.
//  executed:false = refused before golden ran (guard names the golden line or the port reason); a golden handler that ran
//  and stopped on its own message is executed:true (the message is in "messages").
//  Messages: golden's ShowMyMessage is collected while FormLock is held and shown after it is released (the wb_serve host
//  waits for the operator: W62, "訊息框照 golden"), in golden order -- the same as TrayEditForm.cpp QueueMyMessage.  [W906]
//  For "Tester Name 錯誤" (btSafeTasterNameClick :1913, in the middle of the handler) golden waits before the save; here the
//  save is done first and the box follows.
//  upload (AI(W906-W202) 20261009 (St02-E), card W-202 (2) LI-9 F2-3): golden plUnloadClick runs the whole FTP upload
//    (connect, del / 7z, Gate #4, STOR) synchronously while FormLock is held -- W-203 = B (Steven 20261009): as golden, no added
//    timeout.  "FTP Server is not connected" / "Upload done" are collected like every other golden box and shown after the lock.
// ===========================================================================
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "KYECFTP/FTPClientForm_St02.h"
#include "forms/fLotInfo_Ftp_St02.h"
#include "JsonBridge/actions/LotInfoFtp.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "canary_support.h"         // ShowMyMessage, W906_ShowMyMessage_Hook
#include "cmydef.h"                 // SystemStart, AccessLevel
#include "Config.h"                 // IniConfig.iServerEnable / iHDEnable / bEnableFTP
#include "forms/fLotInfo.h"         // the Lot Info FTP tab's buttons (state)

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:39-40

std::string W906_St02_LotInfoFtpOp(const std::string& op, const std::string& payloadJson);

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// ---- ShowMyMessage collected during the op (the wb_serve host shows no box while the hook is not its own) ----
typedef std::vector<std::pair<std::string, std::string> > MsgList;
MsgList* g_msgs = 0;
void CaptureHook(const char* s1, const char* s2)
{
    if (g_msgs) g_msgs->push_back(std::make_pair(std::string(s1 ? s1 : ""), std::string(s2 ? s2 : "")));
}
struct MsgCapture {
    void (*prev)(const char*, const char*);
    explicit MsgCapture(MsgList* m) : prev(W906_ShowMyMessage_Hook) { g_msgs = m; W906_ShowMyMessage_Hook = CaptureHook; }
    ~MsgCapture() { W906_ShowMyMessage_Hook = prev; g_msgs = 0; }
};

// ---- names on disk are ANSI (cp950), JSON is UTF-8 (same as WebRecipeChange.cpp U8 / FromU8) ----
std::string U8(const AnsiString& a)
{
    const std::string s = a.c_str();
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string out((size_t)(n > 0 ? n : 0), '\0');
    if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &out[0], n, NULL, NULL);
    return out;
}
std::string U8s(const std::string& s) { return U8(AnsiString(s.c_str())); }
AnsiString FromU8(const std::string& s)
{
    const int wn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return AnsiString(s.c_str());
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int n = WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string out((size_t)(n > 0 ? n : 0), '\0');
    if (n > 0) WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, &out[0], n, NULL, NULL);
    return AnsiString(out.c_str());
}

// ---- payload fields ----
bool Has(const cJSON* o, const char* k) { return o != 0 && cJSON_GetObjectItemCaseSensitive(o, k) != 0; }
std::string Str(const cJSON* o, const char* k)
{
    const cJSON* it = o ? cJSON_GetObjectItemCaseSensitive(o, k) : 0;
    if (it && cJSON_IsString(it) && it->valuestring) return it->valuestring;
    return std::string();
}
int Int(const cJSON* o, const char* k, int dflt)
{
    const cJSON* it = o ? cJSON_GetObjectItemCaseSensitive(o, k) : 0;
    if (it && cJSON_IsNumber(it)) return it->valueint;
    if (it && cJSON_IsString(it) && it->valuestring && *it->valuestring) return std::atoi(it->valuestring);
    return dflt;
}

struct Res {
    bool executed;
    std::string guard, golden, detail;
    bool hasOpened, opened;
    Res() : executed(false), hasOpened(false), opened(false) {}
    void Refuse(const char* g, const char* gl, const std::string& d) { executed = false; guard = g; golden = gl; detail = d; }
};

bool DialogOpen() { return fFTPClient->W906_Shown() && fFTPClient->W906_bModalOpen; }   // the dialog's own golden bShow (KYECFTP/FTPClientForm_St02.h W906_Shown)

// ---------------------------------------------------------------------------
//  the ops
// ---------------------------------------------------------------------------
void Dispatch(const std::string& op, const cJSON* v, Res& r)
{
    TfFTPClient* F = fFTPClient;
    if (op == "state") { r.executed = true; return; }
    if (op == "open")
    {
        const W906_St02_FtpClick c = W906_St02_LotInfo_btnFtpServerClick(Int(v, "tag", -1));
        r.executed = c.guard.empty();
        r.guard = c.guard; r.golden = c.golden; r.detail = c.detail;
        r.hasOpened = true; r.opened = c.opened;
        return;
    }
    if (op != "filter" && op != "pick" && op != "loadHD" && op != "enterHD" && op != "upload" && op != "testerType" && op != "testerId" &&
        op != "inputMethod" && op != "testerName" && op != "saveTester" && op != "exit" && op != "memoClear" && op != "close" &&
        op != "copyName" && op != "cleanName")
    {
        r.Refuse("unknown-op", "", "op " + op + " is not one of open / state / filter / pick / loadHD / enterHD / upload / testerType / "
                 "testerId / inputMethod / testerName / saveTester / exit / memoClear / close / copyName / cleanName");
        return;
    }
    if (!DialogOpen())
    {
        r.Refuse("not-open", "KYECFTP/FTPClient.cpp:1563 ShowModal", "the FTP dialog is not open (press Server / HD / Tester Name on Lot Info)");
        return;
    }
    const int hd = F->W906_GetHD();
    if (op == "close")                                                          // the window's close box: TForm.Close
    {
        F->Close();
        if (DialogOpen()) r.Refuse("cannot-close", "KYECFTP/FTPClient.cpp:2258-2263 FormCloseQuery", "bCanExit is false (golden keeps the dialog open)");
        else r.executed = true;
        return;
    }
    if (op == "memoClear") { F->memoFTPDblClick(F->memoFTP); r.executed = true; return; }
    // ---- the Server page (AI(W906-P14) 20261010 (St02-E) POOL-14 MR-A; golden 913 FTPClient.cpp / .dfm) ----
    if ((op == "filter" && Str(v, "edit") == "server") || (op == "pick" && Str(v, "list") == "server") || op == "copyName" || op == "cleanName")
    {
        if (hd != 0) { r.Refuse("wrong-page", "golden 913 KYECFTP/FTPClient.cpp:1255-1460", "the Server page is not the one open (ShowFTPModal(0))"); return; }
        if (op == "filter")
        {
            if (F->W906_BarcodeKeys())
            {
                r.Refuse("barcode-keys", "golden 913 KYECFTP/FTPClient.cpp:2238-2241",
                         "CosFunction.bFTPUseBarcodeReader && [N06] Use Barcode: golden swallows typed keys (Key=NULL); the barcode box "
                         "(edtServerWaferNameKeyDown :3831, InputBarcodeNumber) is not ported");
                return;
            }
            F->W906_SetText(F->edtServerWaferName, FromU8(Str(v, "text")));    // VCL: typing -> OnChange (.dfm :496 edtServerWaferNameChange)
            r.executed = true;
            return;
        }
        if (op == "pick")
        {
            if (!F->lstServerFile->Visible || !F->lstServerFile->Enabled)
            {
                r.Refuse("list-disabled", "golden 913 KYECFTP/FTPClient.cpp:1819-1838 FormShow", "lstServerFile is disabled (barcode reader mode)");
                return;
            }
            const int idx = Int(v, "index", -1);
            if (idx < 0 || idx >= F->lstServerFile->Items->Count) { r.Refuse("bad-index", "", "index is not an item of lstServerFile"); return; }
            if (Has(v, "name") && AnsiString(F->lstServerFile->Items->Strings[idx]) != FromU8(Str(v, "name")))
            {
                r.Refuse("stale-list", "", "lstServerFile changed since the page drew it -- redraw and pick again");
                return;
            }
            F->lstServerFile->ItemIndex = idx;                                  // VCL: the first click selects
            F->lstServerFileDblClick(F->lstServerFile);                         // .dfm :509 OnDblClick
            r.executed = true;
            return;
        }
        TPanel* p = (op == "copyName") ? F->Panel15 : F->Panel16;
        if (!p->Visible || !p->Enabled)
        {
            r.Refuse("button-hidden", "golden 913 KYECFTP/FTPClient.cpp:1803-1808 FormShow", "Panel15 / Panel16 is hidden (Greatek / not TSMC) or disabled");
            return;
        }
        if (op == "copyName") F->Panel15Click(p);                               // .dfm :578 OnClick
        else                  F->Panel16Click(p);                               // .dfm :594 OnClick
        r.executed = true;
        return;
    }
    if (op == "filter" || op == "pick" || op == "loadHD" || op == "enterHD" || op == "upload")
    {
        if (hd != 1) { r.Refuse("wrong-page", "KYECFTP/FTPClient.cpp:1442-1479", "the HD page is not the one open (ShowFTPModal(1))"); return; }
        if (op == "filter")
        {
            if (F->W906_BarcodeKeys())
            {
                r.Refuse("barcode-keys", "KYECFTP/FTPClient.cpp:2209-2212",
                         "CosFunction.bFTPUseBarcodeReader && [N06] Use Barcode: golden swallows typed keys (Key=NULL); the barcode box "
                         "(edtHDWaferNameKeyDown :3734-3758, InputBarcodeNumber) is not in F1");
                return;
            }
            F->W906_SetText(F->edtHDWaferName, FromU8(Str(v, "text")));        // VCL: typing -> OnChange (edtHDWaferNameChange)
            r.executed = true;
            return;
        }
        if (op == "pick")
        {
            if (!F->lstHDFile->Visible || !F->lstHDFile->Enabled)
            {
                r.Refuse("list-disabled", "KYECFTP/FTPClient.cpp:1774-1794", "lstHDFile is disabled (barcode reader mode)");
                return;
            }
            const int idx = Int(v, "index", -1);
            if (idx < 0 || idx >= F->lstHDFile->Items->Count) { r.Refuse("bad-index", "", "index is not an item of lstHDFile"); return; }
            if (Has(v, "name") && AnsiString(F->lstHDFile->Items->Strings[idx]) != FromU8(Str(v, "name")))
            {
                r.Refuse("stale-list", "", "lstHDFile changed since the page drew it -- redraw and pick again");
                return;
            }
            F->lstHDFile->ItemIndex = idx;                                      // VCL: the first click selects
            F->lstHDFileDblClick(F->lstHDFile);                                 // .dfm :661 OnDblClick
            r.executed = true;
            return;
        }
        if (op == "loadHD")
        {
            if (!F->plLoad->Visible || !F->plLoad->Enabled)
            {
                r.Refuse("button-hidden", "KYECFTP/FTPClient.cpp:1448 / :1748-1756", "plLoad (Load from HD) is hidden or disabled");
                return;
            }
            F->plLoadClick(F->plLoad);                                          // .dfm :698 OnClick
            r.executed = true;
            return;
        }
        if (op == "upload")                                                     // AI(W906-W202) 20261009 (St02-E) F2-3
        {
            if (!F->plUnload->Visible || !F->plUnload->Enabled)
            {
                r.Refuse("button-hidden", "KYECFTP/FTPClient.cpp:1517-1518 / golden 913 :1171", "plUnload (Upload to Server) is hidden or disabled");
                return;
            }
            if (SystemStart)                                                    // [W906] port-only: golden's dialog is modal, so START on the
            {                                                                   //   main form cannot be pressed under it; the web window is not
                r.Refuse("running", "uLotInfo.cpp:5001-5126 btnFtpServerClick (SystemStart refusal)",   //   modal, so the op re-checks what opening did
                         "the machine is running (SystemStart) -- golden opens the FTP dialog only while stopped");
                return;
            }
            F->plUnloadClick(F->plUnload);                                      // golden 913 FTPClient.dfm :700-715 plUnload OnClick
            r.executed = true;
            return;
        }
        char Key = VK_RETURN;                                                   // enterHD
        F->edtHDWaferNameKeyPress(F->edtHDWaferName, Key);                      // .dfm :650 OnKeyPress
        r.executed = true;
        return;
    }
    // ---- the Taster page ----
    if (hd != 2) { r.Refuse("wrong-page", "KYECFTP/FTPClient.cpp:1480-1515", "the Taster page is not the one open (ShowFTPModal(2))"); return; }
    if (op == "testerType" || op == "testerId")
    {
        TComboBox* cb = (op == "testerType") ? F->cbTesterType : F->cbTesterID;
        if (!F->grpTesterMap->Visible || !cb->Enabled)
        {
            r.Refuse("input-method", "KYECFTP/FTPClient.cpp:1497-1510 / :2069-2083", "the Taster Map group is hidden (Input method = Manually)");
            return;
        }
        if (Has(v, "index"))                                                    // picked from the list (CBN_SELCHANGE)
        {
            const int idx = Int(v, "index", -1);
            if (idx < 0 || idx >= cb->Items->Count) { r.Refuse("bad-index", "", "index is not an item of the list"); return; }
            cb->ItemIndex = idx;
            cb->Text = cb->Items->Strings[idx];
        }
        else                                                                    // typed (csDropDown): the text, no list item selected
        {
            cb->Text = FromU8(Str(v, "text"));
            cb->ItemIndex = -1;
        }
        if (op == "testerType") F->cbTesterTypeChange(cb);                      // .dfm OnChange = cbTesterTypeChange
        else                    F->cbTesterIDChange(cb);                        // .dfm OnChange = cbTesterIDChange
        r.executed = true;
        return;
    }
    if (op == "inputMethod")
    {
        const int idx = Int(v, "index", -1);
        if (idx < 0 || idx >= F->rgInputMethod->Items->Count) { r.Refuse("bad-index", "", "index must be 0 (By List) or 1 (Manually)"); return; }
        F->W906_RadioSetItemIndex(F->rgInputMethod, idx);                       // a click on the other button: OnClick (rgInputMethodClick)
        r.executed = true;
        return;
    }
    if (op == "testerName")
    {
        if (!F->grpTesterName->Enabled || !F->edTesterName->Enabled)
        {
            r.Refuse("input-method", "KYECFTP/FTPClient.cpp:1500 / :2072", "Taster Name is not editable (Input method = By List)");
            return;
        }
        F->edTesterName->Text = FromU8(Str(v, "text"));
        r.executed = true;
        return;
    }
    if (op == "saveTester")
    {
        if (!F->btSafeTasterName->Visible || !F->btSafeTasterName->Enabled) { r.Refuse("button-hidden", "", "btSafeTasterName is hidden or disabled"); return; }
        if (Has(v, "edTesterName") && F->grpTesterName->Enabled && F->edTesterName->Enabled)
            F->edTesterName->Text = FromU8(Str(v, "edTesterName"));            // what the operator typed before pressing Save
        F->btSafeTasterNameClick(F->btSafeTasterName);                          // .dfm :956 OnClick
        r.executed = true;
        return;
    }
    // exit
    F->Button3Click(F->Button3);                                                // .dfm :971 OnClick
    if (DialogOpen()) r.Refuse("cannot-close", "KYECFTP/FTPClient.cpp:2258-2263 FormCloseQuery", "bCanExit is false (golden keeps the dialog open)");
    else r.executed = true;
}

// ---------------------------------------------------------------------------
//  state
// ---------------------------------------------------------------------------
void Ctl(webbridge::JsonWriter& w, const char* key, const TControl* c, const AnsiString* caption)
{
    w.Key(key).BeginObject();
    w.Key("visible").Bool(c && c->Visible);
    w.Key("enabled").Bool(c && c->Enabled);
    if (caption) w.Key("caption").String(U8(*caption));
    w.EndObject();
}
void Items(webbridge::JsonWriter& w, const TStringList* l)
{
    w.Key("items").BeginArray();
    for (int i = 0; l && i < l->Count; ++i) w.String(U8(l->GetString(i)));
    w.EndArray();
}
void List(webbridge::JsonWriter& w, const char* key, const TListBox* l)
{
    w.Key(key).BeginObject();
    w.Key("visible").Bool(l->Visible);
    w.Key("enabled").Bool(l->Enabled);
    w.Key("itemIndex").Number((wb_int64)l->ItemIndex);
    Items(w, l->Items);
    w.EndObject();
}
void Combo(webbridge::JsonWriter& w, const char* key, const TComboBox* c)
{
    w.Key(key).BeginObject();
    w.Key("visible").Bool(c->Visible);
    w.Key("enabled").Bool(c->Enabled);
    w.Key("text").String(U8(c->Text));
    w.Key("itemIndex").Number((wb_int64)c->ItemIndex);
    Items(w, c->Items);
    w.EndObject();
}
void Edit(webbridge::JsonWriter& w, const char* key, const TEdit* e)
{
    w.Key(key).BeginObject();
    w.Key("visible").Bool(e->Visible);
    w.Key("enabled").Bool(e->Enabled);
    w.Key("text").String(U8(e->Text));
    w.EndObject();
}
void LotBtn(webbridge::JsonWriter& w, const char* id, const TButton* b)
{
    w.BeginObject();
    w.Key("id").String(id);
    w.Key("tag").Number((wb_int64)(b ? b->Tag : -1));
    w.Key("caption").String(b ? U8(b->Caption) : std::string());
    w.Key("visible").Bool(b && b->Visible);
    w.Key("enabled").Bool(b && b->Enabled);
    w.EndObject();
}

void WriteState(webbridge::JsonWriter& w)
{
    const TfFTPClient* F = fFTPClient;
    const int hd = F->W906_GetHD();
    w.Key("state").BeginObject();
    w.Key("open").Bool(DialogOpen());
    w.Key("bShow").Bool(F->W906_Shown());
    w.Key("modal").Bool(F->W906_bModalOpen);
    w.Key("iHD").Number((wb_int64)hd);
    w.Key("page").String(!DialogOpen() ? "" : hd == 0 ? "server" : hd == 1 ? "hd" : hd == 2 ? "tester" : "");
    w.Key("caption").String(U8(F->Caption));
    w.Key("activePageIndex").Number((wb_int64)F->PageControl1->ActivePageIndex);
    w.Key("tabs").BeginArray();
    const TTabSheet* tabs[4] = { F->TabSheet1, F->TabSheet2, F->TabSheet3, F->TabSheet4 };
    const char* names[4] = { "TabSheet1", "TabSheet2", "TabSheet3", "TabSheet4" };
    for (int i = 0; i < 4; ++i)
    {
        w.BeginObject();
        w.Key("name").String(names[i]);
        w.Key("caption").String(U8(tabs[i]->Caption));
        w.Key("visible").Bool(tabs[i]->TabVisible);
        w.EndObject();
    }
    w.EndArray();
    w.Key("bCanExit").Bool(W906_St02_FtpCanExit());
    w.Key("barcodeKeys").Bool(F->W906_BarcodeKeys());
    w.Key("lastLoadMode").Number((wb_int64)F->W906_iLastLoadMode);
    w.Key("vclError").String(U8s(F->W906_LastVclError));

    w.Key("hd").BeginObject();
    Edit(w, "edtHDWaferName", F->edtHDWaferName);
    List(w, "lstHDFile", F->lstHDFile);
    Ctl(w, "plLoad", F->plLoad, &F->plLoad->Caption);
    Ctl(w, "plUnload", F->plUnload, &F->plUnload->Caption);
    Ctl(w, "plUnloadALL", F->plUnloadALL, &F->plUnloadALL->Caption);
    Ctl(w, "labN06_DownloadPath1", F->labN06_DownloadPath1, &F->labN06_DownloadPath1->Caption);
    Edit(w, "FTP_DownPath1", F->FTP_DownPath1);
    Ctl(w, "labN06_UploadPath1", F->labN06_UploadPath1, &F->labN06_UploadPath1->Caption);
    Edit(w, "FTP_UpLdPath1", F->FTP_UpLdPath1);
    w.EndObject();

    w.Key("server").BeginObject();                                              // POOL-14 MR-A: the Server list (ShowFTPModal(0)); plSLoad = MR-C
    Edit(w, "edtServerWaferName", F->edtServerWaferName);
    List(w, "lstServerFile", F->lstServerFile);
    Ctl(w, "plSLoad", F->plSLoad, &F->plSLoad->Caption);
    Ctl(w, "Panel15", F->Panel15, &F->Panel15->Caption);
    Ctl(w, "Panel16", F->Panel16, &F->Panel16->Caption);
    w.Key("downloadPorted").Bool(false);                                        // plSLoadClick 'Download to Handler' = POOL-14 MR-C
    w.EndObject();

    w.Key("tester").BeginObject();
    Edit(w, "edHandlerType", F->edHandlerType);
    Edit(w, "edHandlerID", F->edHandlerID);
    w.Key("rgInputMethod").BeginObject();
    w.Key("enabled").Bool(F->rgInputMethod->Enabled);
    w.Key("itemIndex").Number((wb_int64)F->rgInputMethod->ItemIndex);
    Items(w, F->rgInputMethod->Items);
    w.EndObject();
    Ctl(w, "grpTesterMap", F->grpTesterMap, &F->grpTesterMap->Caption);
    Ctl(w, "grpTesterName", F->grpTesterName, &F->grpTesterName->Caption);
    Combo(w, "cbTesterType", F->cbTesterType);
    Combo(w, "cbTesterID", F->cbTesterID);
    Combo(w, "cbTasterIp", F->cbTasterIp);
    Edit(w, "edTesterName", F->edTesterName);
    Ctl(w, "btSafeTasterName", F->btSafeTasterName, &F->btSafeTasterName->Caption);
    Ctl(w, "Button3", F->Button3, &F->Button3->Caption);
    w.EndObject();

    w.Key("memo").BeginArray();                                                 // memoFTP (the last 200 lines)
    const int n = F->memoFTP->Lines->Count;
    for (int i = (n > 200 ? n - 200 : 0); i < n; ++i) w.String(U8(F->memoFTP->Lines->GetString(i)));
    w.EndArray();

    w.Key("lotInfo").BeginObject();                                             // the Lot Info FTP tab (uLotInfo.dfm:1803-1949)
    w.Key("tsFTPVisible").Bool(fLotInfo && fLotInfo->tsFTP && fLotInfo->tsFTP->TabVisible);
    w.Key("buttons").BeginArray();
    LotBtn(w, "btnFtpServer", fLotInfo ? fLotInfo->btnFtpServer : 0);
    LotBtn(w, "btnDataFTPSaveToData", fLotInfo ? fLotInfo->btnDataFTPSaveToData : 0);
    LotBtn(w, "btnFtpHD", fLotInfo ? fLotInfo->btnFtpHD : 0);
    LotBtn(w, "btnFtpTester", fLotInfo ? fLotInfo->btnFtpTester : 0);
    LotBtn(w, "btnFTPTryConnect", fLotInfo ? fLotInfo->btnFTPTryConnect : 0);
    w.EndArray();
    w.Key("lbFTPStatus").BeginObject();
    w.Key("visible").Bool(fLotInfo && fLotInfo->lbFTPStatus && fLotInfo->lbFTPStatus->Visible);
    w.Key("caption").String(fLotInfo && fLotInfo->lbFTPStatus ? U8(fLotInfo->lbFTPStatus->Caption) : std::string());
    w.EndObject();
    w.Key("systemStart").Bool(SystemStart);
    w.Key("accessLevel").Number((wb_int64)AccessLevel);
    w.Key("iServerEnable").Number((wb_int64)IniConfig.iServerEnable);
    w.Key("iHDEnable").Number((wb_int64)IniConfig.iHDEnable);
    w.Key("bEnableFTP").Bool(IniConfig.bEnableFTP);
    w.EndObject();

    w.Key("f2").String("Server list = POOL-14 MR-A (golden 913 ShowFTPModal case 0); Download to Handler = POOL-14 MR-C (not available yet); "
                       "Upload to Server = F2-3");
    w.EndObject();
}

void StrArray(webbridge::JsonWriter& w, const char* key, const std::vector<std::string>& v)
{
    w.Key(key).BeginArray();
    for (std::size_t i = 0; i < v.size(); ++i) w.String(U8s(v[i]));
    w.EndArray();
}

}  // namespace

// ---------------------------------------------------------------------------
//  the body (installed into JsonBridge/actions/LotInfoFtp.cpp)
// ---------------------------------------------------------------------------
std::string W906_St02_LotInfoFtpOp(const std::string& op, const std::string& payloadJson)
{
    cJSON* root = 0;
    bool badPayload = false;
    if (!payloadJson.empty())
    {
        root = cJSON_Parse(payloadJson.c_str());
        if (root == 0 || !cJSON_IsObject(root)) badPayload = true;
    }
    Res r;
    W906_St02_FtpReport rep;
    MsgList msgs;
    std::string vclErr, exc;
    webbridge::JsonWriter w;
    {
        FormLockGuard lock;
        {
            MsgCapture cap(&msgs);
            W906_St02_FtpCurrentReport = &rep;
            if (badPayload)
                r.Refuse("bad-payload", "", "value must be a JSON object (as a string)");
            else
            {
                try {
                    Dispatch(op, root, r);
                } catch (const W906_St02_VclError& e) {                         // VCL's default handler would show it; the handler stopped there
                    vclErr = e.msg;
                    fFTPClient->W906_LastVclError = e.msg;
                    r.Refuse("vcl-exception", "", e.msg);
                } catch (const std::exception& e) {
                    exc = std::string("exception in golden TfFTPClient: ") + e.what();
                    r.Refuse("exception", "", exc);
                } catch (...) {
                    exc = "exception in golden TfFTPClient (non-std)";
                    r.Refuse("exception", "", exc);
                }
            }
            W906_St02_FtpCurrentReport = 0;
        }
        w.BeginObject();
        w.Key("executed").Bool(r.executed);
        w.Key("op").String(op);
        w.Key("guard").String(r.guard);
        w.Key("golden").String(r.golden);
        w.Key("detail").String(U8s(r.detail));
        if (r.hasOpened) w.Key("opened").Bool(r.opened);
        if (!vclErr.empty()) w.Key("vclException").String(U8s(vclErr));
        w.Key("messages").BeginArray();
        for (std::size_t i = 0; i < msgs.size(); ++i)
        {
            w.BeginObject();
            w.Key("s1").String(U8s(msgs[i].first));
            w.Key("s2").String(U8s(msgs[i].second));
            w.EndObject();
        }
        w.EndArray();
        StrArray(w, "gated", rep.gated);
        StrArray(w, "notes", rep.notes);
        if (!rep.sessionJson.empty()) w.Key("session").RawValue(rep.sessionJson);
        WriteState(w);
        w.EndObject();
    }
    if (root) cJSON_Delete(root);
    if (op != "state")
        std::printf("act.lotInfoFtp.%s -> %s%s (messages %u)\n", op.c_str(), r.executed ? "done" : "refused ",
                    r.guard.c_str(), (unsigned)msgs.size());
    for (std::size_t i = 0; i < msgs.size(); ++i)                               // golden ShowMyMessage, now outside FormLock (file banner)
        ShowMyMessage(AnsiString(msgs[i].first.c_str()), AnsiString(msgs[i].second.c_str()));
    if (!w.Ok()) return "{\"executed\":false,\"guard\":\"json-writer-misuse\"}";
    return w.Str();
}

void W906_St02_LotInfoFtpRegisterBody()
{
    ht9045::sjson::SetLotInfoFtpBody(&W906_St02_LotInfoFtpOp);
}
