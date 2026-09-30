// =============================================================================
//  WebBuilder.cpp -- Data.Builder.html <-> golden V912 TfBuilder (cBuilder.cpp).
//
//  Steven 20260925 (Data.Builder). NOT in golden as a file: the golden bodies are
//  the facade's (forms/fBuilder.cpp, translated from cBuilder.cpp in
//  HT9011UC_Code_V3.33.912.0_20260908_Jimmy); this file replays the operator's
//  clicks on them and turns the form into JSON for the WS command. wb_serve-only
//  (JsonWriter / cJSON), same as WebSortCT.cpp.
//
//  ⚠ DISPATCH NOT WIRED HERE: the tools/wb_serve.cpp arm is in WebBuilder.h. The
//    page (web/page/ht9045_builder_web.js) sends `builder.op`, value = JSON string.
//
//  ACTS (value {"act":...}):
//    open    golden route + ShowModal: main.cpp:29009-29016 sbConfigClick (SystemStart
//            -> no; fSecurity->Insufficient(1)) -> :28515-28527 sbBuilderClick
//            (Insufficient(23); Barcode_Reader(bcBuilder) -- KYEC customers only,
//            skipped per Steven, see below; NewRecordProcess("MES2179","Enter
//            Builder")) -> FormShow (cBuilder.cpp:23-33).
//    state   the form as it is (no golden action) -- for a page refresh.
//    change  the operator picks cbSourceFile / cbDeleteFile or types in
//            edNewFileName: sets the widget, then the dfm OnChange handler
//            (:35-49 / :194-200). edNewFileName text goes through golden's
//            OnKeyPress filter OnlyMakeFileDataInPut (:495-500) first.
//    create  {source,name}: sets both widgets like "change", then Create
//            (btCreateSetupFileClick :72-159). Two-phase ("Do you create ''X'' as
//            the name", :95). WRITES IniData\Data\<name> and IniData\Offset\<name>.
//    delete  {name}: selects it, then Delete (btDeleteSetupFileClick :202-230;
//            golden's own in-use guard :211-215). Two-phase (:218). DELETES
//            IniData\Data\<name> and IniData\Offset\<name> (Recycle Bin).
//    dir     {path}: the operator double-clicks a folder in DirectoryListBox1
//            (VCL TDirectoryListBox.OpenCurrent -> SetDirectory) -> OnChange
//            DirectoryListBox1Change (:262-265).
//    drive   {drive:"e"}: DriveComboBox1 (dfm DirList=DirectoryListBox1) ->
//            TDirectoryListBox.DriveChange: that drive's current directory.
//    import  Import (spbImportClick :475-482 -> CopySourTarget :274-335), two-phase.
//            WRITES into IniData\Data.
//    export  {checked:[names]}: the ticked CheckListBox1 items, then Export
//            (spbExportClick :396-473), two-phase. DELETES + WRITES <dir>\<name>.
//    close   Exit (spbExitClick :484 -> Close -> FormClose :502-505, fShow=false).
//
//  TWO-PHASE CONFIRMATION: confirmed=false runs the handler with the OK/CANCEL box
//  preset to IDCANCEL (TfBuilder::W906_MessageBox) and answers
//  {"needConfirm":true,"prompt":[...]} when golden got as far as asking;
//  confirmed=true re-checks every guard and runs it with IDOK. Every golden body
//  here asks before it writes. (Delete is the one whose cancel path still does
//  something -- golden :223-229 re-lists the recipes and clears the selection after
//  the box whatever the answer; that is reproduced.)
//
//  GUARDS
//   golden: the route (above) at open, re-checked quietly (Insufficient(n,false))
//           before every create/delete/import/export; golden's own checks inside
//           the handlers (empty selection, in-use recipe).
//   port (not in golden, each reported to Steven):
//    P1 names must come from the lists the form holds (csDropDownList in the dfm,
//       CheckListBox items) -- a request cannot invent a folder to delete/export.
//    P2 new name: golden's key filter (OnlyMakeFileDataInPut) plus the characters
//       Windows refuses in a folder name that the filter lets through ('\\', '>',
//       control codes), all-dots/spaces, a trailing dot/space, DOS device names.
//       Without it "..\\x" (typeable on a hardware keyboard in golden) would put a
//       copy of the recipe outside IniData\Data.
//    P3 in-use, case-insensitive: golden compares the name with
//       fMain->cbSetupFileName->Text byte for byte (:211); NTFS is case-insensitive,
//       so "abc" vs "ABC" passes golden's check and deletes the running recipe. Also
//       checked against GetLastOpenFN() (the recipe setup.inf says is loaded).
//    P4 path length: golden copies paths into char[256] (char[128] for Import) with
//       strncpy -- a longer path loses its terminator. Refused up front instead.
//    P5 export target: a browsed directory that is (or is inside) IniData\Data /
//       IniData\Offset, or that makes <dir>\<name> contain them, is refused -- there
//       golden's :448 FO_DELETE removes the very recipe it is about to copy.
//   Not guarded (golden behaviour, reported): Create onto an existing name copies
//   INTO it; Import overwrites same-named recipes, the running one included.
//
//  CUSTOMER-SPECIFIC CODE SKIPPED (Steven: 客戶專屬條件先跳過、只註記):
//    Barcode_Reader(bcBuilder) (main.cpp:28522; asks KYEC_LEE/KYEC_XILINX operators
//    for a barcode, returns 2 = go on for everyone else); the SPIL RMS block after
//    ShowModal (main.cpp:28529-28543, IniConfig.bSPILFunction && bEnableRms);
//    Create2DCodeWorkFile (cBuilder.cpp:97, CC_KYEC_XILINX); the ASE_KaohSiung
//    JOBFILE copies (compiled out, macro not defined).
// =============================================================================
#include <windows.h>
#include <shellapi.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "WebBuilder.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fBuilder.h"
#include "forms/fMain.h"       // fMain->cbSetupFileName (golden's "current recipe")
#include "forms/fSecurity.h"   // fSecurity->Insufficient (cSecurity.cpp, ht9045_sm)
#include "common.h"            // DataPath / OffsetPath / GetLastOpenFN / OnlyMakeFileDataInPut
#include "cmydef.h"            // SystemStart
#include "canary_support.h"    // W906_ShowMyMessage_Hook (captured into "messages")

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   extern bool authMainForm[12];   // JsonBridge/FormJson.cpp:149-150；AI(W906-AUTHMAINFORM) 20260930: authMainForm＝cAuthority.h:56（本體 cAuthority.cpp:119，開機 GetMainAuth 讀 Security_new.def [Main]），接在同一行、不移動行號
// cMyDB.h:129 (body acatchtray_shims.cpp). Declared here instead of including cMyDB.h: that header and
// canary_support.h both give RecordProcess / MyDBIProcessNew default arguments, which is an error in one TU.
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// No "There is no disk in the drive" box from the shell while listing drives/folders
// (VCL VolumeID does the same, BCB6 Source\vcl\filectrl.pas:544).
struct QuietErrors {
    UINT old;
    QuietErrors()  { old = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX); }
    ~QuietErrors() { SetErrorMode(old); }
};

// ---- ShowMyMessage capture (chained in front of wb_serve's hook for one request) ----
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

// ---- text: file-system names are ANSI (cp950); JSON is UTF-8 ----
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
// UTF-8 -> ANSI; false when a character has no ANSI form (it could not be a folder
// name the ANSI file APIs golden uses can reach).
bool Acp(const std::string& u, AnsiString* out)
{
    bool ascii = true;
    for (size_t i = 0; i < u.size(); ++i) if ((unsigned char)u[i] >= 0x80) { ascii = false; break; }
    if (ascii) { *out = u.c_str(); return true; }
    const int wn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u.c_str(), (int)u.size(), NULL, 0);
    if (wn <= 0) return false;
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, u.c_str(), (int)u.size(), &w[0], wn);
    BOOL usedDef = FALSE;
    const int an = WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, NULL, 0, NULL, &usedDef);
    if (an <= 0 || usedDef) return false;
    std::string a((size_t)an, '\0');
    WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, &a[0], an, NULL, &usedDef);
    if (usedDef) return false;
    *out = a.c_str();
    return true;
}

bool CiEq(const AnsiString& a, const AnsiString& b)
{
    return CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, a.c_str(), -1, b.c_str(), -1) == CSTR_EQUAL;
}

// full path, no trailing '\' except "X:\"
AnsiString NormDir(const AnsiString& p)
{
    char buf[MAX_PATH * 2] = "";
    const DWORD n = GetFullPathNameA(p.c_str(), (DWORD)sizeof(buf), buf, NULL);
    std::string s = (n > 0 && n < sizeof(buf)) ? std::string(buf) : std::string(p.c_str());
    while (s.size() > 3 && (s[s.size() - 1] == '\\' || s[s.size() - 1] == '/')) s.erase(s.size() - 1);
    return AnsiString(s.c_str());
}
std::string UpperKey(const AnsiString& p)
{
    std::string s = NormDir(p).c_str();
    if (!s.empty()) CharUpperBuffA(&s[0], (DWORD)s.size());
    if (s.empty() || s[s.size() - 1] != '\\') s += '\\';
    return s;
}
bool SameOrInside(const AnsiString& child, const AnsiString& parent)
{
    const std::string c = UpperKey(child), p = UpperKey(parent);
    return c.size() >= p.size() && c.compare(0, p.size(), p) == 0;
}

int IndexOf(TStringList* l, const AnsiString& s)
{
    for (int i = 0; i < l->Count; ++i) if (l->Strings[i] == s) return i;
    return -1;
}
bool CiInList(TStringList* l, const AnsiString& s)
{
    for (int i = 0; i < l->Count; ++i) if (CiEq(l->Strings[i], s)) return true;
    return false;
}

bool DirExists(const AnsiString& p)
{
    const DWORD a = GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

// golden edNewFileNameKeyPress (:495-500): a key failing OnlyMakeFileDataInPut is
// dropped (Key=NULL). DBCS pairs pass whole (a trail byte may be 0x5C).
AnsiString KeyFilter(const AnsiString& in, bool* dropped)
{
    std::string s = in.c_str(), o;
    *dropped = false;
    for (size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        if (IsDBCSLeadByte(c) && i + 1 < s.size()) { o += s[i]; o += s[i + 1]; ++i; continue; }
        if (OnlyMakeFileDataInPut(c)) o += s[i]; else *dropped = true;
    }
    return AnsiString(o.c_str());
}

// port guard P2 (+ P4 for the create paths); "" = acceptable
std::string NewNameProblem(const AnsiString& name, const AnsiString& source)
{
    std::string s = name.c_str();
    bool onlyDotsSpaces = true;
    for (size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        if (IsDBCSLeadByte(c) && i + 1 < s.size()) { ++i; onlyDotsSpaces = false; continue; }
        if (c < 0x20) return "新名稱有控制字元";
        if (c == '\\' || c == '>') return "新名稱有 Windows 不接受的字元 '\\' 或 '>'（golden 的 OnlyMakeFileDataInPut 沒擋這兩個）";
        if (c != '.' && c != ' ') onlyDotsSpaces = false;
    }
    if (onlyDotsSpaces) return "新名稱不能只有 '.' 或空白（'..' 會把配方複製到 IniData\\Data 外面）";
    const char last = s[s.size() - 1];
    if (last == '.' || last == ' ') return "新名稱不能以 '.' 或空白結尾（Windows 會把它去掉，建出來的資料夾名字對不上）";
    std::string base = s.substr(0, s.find('.'));
    CharUpperBuffA(&base[0], (DWORD)base.size());
    static const char* kDev[] = { "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                                  "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9" };
    for (size_t i = 0; i < sizeof(kDev) / sizeof(kDev[0]); ++i) if (base == kDev[i]) return "新名稱是 DOS 裝置名稱";
    const AnsiString roots[2] = { DataPath, OffsetPath };
    for (int i = 0; i < 2; ++i) {
        if ((roots[i] + source + "\\*.*").Length() >= 255 || (roots[i] + name).Length() >= 255)
            return "路徑超過 254 字（golden 的 char cStr1[256]/cStr2[256]，cBuilder.cpp:87）";
    }
    return "";
}

struct Guard { const char* guard; const char* goldenLine; const char* detail; };
const Guard* RouteGuard(bool alarm)
{
    static const Guard kRun  = { "SystemStart", "golden V912 main.cpp:29012-29013 sbConfigClick",
        "機台運轉中不能開 Builder（sbBuilder 在 palConfig 裡；DoMainPadProcess :3970-3978 運轉中把 palConfig 藏起來）" };
    static const Guard kLv1  = { "not-authorized", "golden V912 main.cpp:29015 sbConfigClick fSecurity->Insufficient(1)",
        "權限不足（[1] Config，system\\levelset.dat）" };
    static const Guard kLv23 = { "not-authorized", "golden V912 main.cpp:28519 sbBuilderClick fSecurity->Insufficient(23)",
        "權限不足（[23] Config - Builder，cSecurity.cpp:51）" };
    if (SystemStart) return &kRun;
    if (!authMainForm[1]) { static const Guard kAuth1 = { "disabled", "golden V912 main.cpp:12952 ChangeLevelAttr sbConfig->Enabled=(AccessLevel>=LevelSet.AccessLevel[1] && authMainForm[1])", "設定選單鈕停用、按不到（D:\\HT9045\\config\\Security_new.def [Main] Maintance=0 ⇒ authMainForm[1] 關；golden Timer2Timer :21675 每拍重算 ⇒ sbConfigClick 不會跑，也不會跳 WAR1676）" }; return &kAuth1; }   // AI(W906-AUTHMAINFORM) 20260930: 放在 Insufficient(1) 之前（同 FileRW/_EditPage.cpp GConfigMenu）
    if (fSecurity == 0 || fSecurity->Insufficient(1, alarm) == false) return &kLv1;
    if (fSecurity->Insufficient(23, alarm) == false) return &kLv23;
    return 0;
}

// ---- DirectoryListBox1 as VCL TDirectoryListBox.BuildList draws it (filectrl.pas:991-1051) ----
void WriteDirTree(webbridge::JsonWriter& w, const AnsiString& dir)
{
    QuietErrors q;
    const std::string d = dir.c_str();
    w.BeginArray();
    if (d.size() < 2 || d[1] != ':') { w.EndArray(); return; }
    const std::string root = d.substr(0, 2) + "\\";
    int level = 0;
    w.BeginObject(); w.Key("name").String(U8(AnsiString(root.c_str()))); w.Key("path").String(U8(AnsiString(root.c_str())));
    w.Key("level").Number((wb_int64)level); w.Key("kind").String(d.size() <= 3 ? "current" : "open"); w.EndObject();
    ++level;
    if (d.size() > 3) {
        std::string rest = d.substr(3), acc = root;
        size_t pos;
        while ((pos = rest.find('\\')) != std::string::npos) {
            const std::string part = rest.substr(0, pos);
            acc += part;
            w.BeginObject(); w.Key("name").String(U8(AnsiString(part.c_str()))); w.Key("path").String(U8(AnsiString(acc.c_str())));
            w.Key("level").Number((wb_int64)level); w.Key("kind").String("open"); w.EndObject();
            acc += "\\"; rest.erase(0, pos + 1); ++level;
        }
        w.BeginObject(); w.Key("name").String(U8(AnsiString(rest.c_str()))); w.Key("path").String(U8(dir));
        w.Key("level").Number((wb_int64)level); w.Key("kind").String("current"); w.EndObject();
        ++level;
    }
    // ReadDirectoryNames (filectrl.pas:965-990): FindFirst(<dir>\*.*, faDirectory) -> hidden/system folders
    // are filtered out by the attribute rule; Siblings.Sorted := True (case-insensitive)
    std::vector<std::string> kids;
    WIN32_FIND_DATAA fd;
    const std::string pat = (d.size() == 3 ? d : d + "\\") + "*.*";
    HANDLE h = FindFirstFileA(pat.c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (fd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) continue;
            if (!std::strcmp(fd.cFileName, ".") || !std::strcmp(fd.cFileName, "..")) continue;
            kids.push_back(fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    std::sort(kids.begin(), kids.end(), [](const std::string& a, const std::string& b) {
        return CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, a.c_str(), -1, b.c_str(), -1) == CSTR_LESS_THAN; });
    for (size_t i = 0; i < kids.size(); ++i) {
        const std::string p = (d.size() == 3 ? d : d + "\\") + kids[i];
        w.BeginObject(); w.Key("name").String(U8(AnsiString(kids[i].c_str()))); w.Key("path").String(U8(AnsiString(p.c_str())));
        w.Key("level").Number((wb_int64)level); w.Key("kind").String("closed"); w.EndObject();
    }
    w.EndArray();
}

// DriveComboBox1 (TDriveComboBox.BuildList filectrl.pas:681-713): floppy/removable "e:",
// fixed/CD/RAM "c: [label]", network "x:" (VCL adds the share name via WNet -- not linked here)
void WriteDrives(webbridge::JsonWriter& w)
{
    QuietErrors q;
    const DWORD bits = GetLogicalDrives();
    w.BeginArray();
    for (int i = 0; i < 26; ++i) {
        if (!(bits & (1u << i))) continue;
        const char letter = (char)('a' + i);
        char rootp[4] = { (char)('A' + i), ':', '\\', 0 };
        const UINT t = GetDriveTypeA(rootp);
        const char* kind = t == DRIVE_REMOVABLE ? "removable" : t == DRIVE_FIXED ? "fixed" : t == DRIVE_REMOTE ? "network" :
                           t == DRIVE_CDROM ? "cdrom" : t == DRIVE_RAMDISK ? "ram" : 0;
        if (!kind) continue;                                           // VCL lists no other type
        std::string label = std::string(1, letter) + ":";
        if (t == DRIVE_FIXED || t == DRIVE_CDROM || t == DRIVE_RAMDISK) {
            char vol[MAX_PATH + 1] = "";
            DWORD notUsed = 0, flags = 0;
            if (!GetVolumeInformationA(rootp, vol, sizeof(vol), NULL, &notUsed, &flags, NULL, 0)) vol[0] = 0;
            CharLowerBuffA(vol, (DWORD)std::strlen(vol));
            label += std::string(" [") + vol + "]";
        }
        w.BeginObject();
        w.Key("letter").String(std::string(1, letter));
        w.Key("label").String(U8(AnsiString(label.c_str())));
        w.Key("type").String(kind);
        w.EndObject();
    }
    w.EndArray();
}

void WriteList(webbridge::JsonWriter& w, TStringList* l)
{
    w.BeginArray();
    for (int i = 0; i < l->Count; ++i) w.String(U8(l->Strings[i]));
    w.EndArray();
}

void WriteState(webbridge::JsonWriter& w, TfBuilder* f)
{
    w.BeginObject();
    w.Key("cbSourceFile").BeginObject();
    w.Key("items"); WriteList(w, f->cbSourceFile->Items);
    w.Key("text").String(U8(f->cbSourceFile->Text));
    w.Key("itemIndex").Number((wb_int64)f->cbSourceFile->ItemIndex);
    w.Key("enabled").Bool(f->cbSourceFile->Enabled);
    w.EndObject();
    w.Key("edNewFileName").BeginObject();
    w.Key("text").String(U8(f->edNewFileName->Text));
    w.Key("enabled").Bool(f->edNewFileName->Enabled);
    w.EndObject();
    w.Key("btCreateSetupFile").BeginObject().Key("enabled").Bool(f->btCreateSetupFile->Enabled).EndObject();
    w.Key("cbDeleteFile").BeginObject();
    w.Key("items"); WriteList(w, f->cbDeleteFile->Items);
    w.Key("text").String(U8(f->cbDeleteFile->Text));
    w.Key("itemIndex").Number((wb_int64)f->cbDeleteFile->ItemIndex);
    w.Key("enabled").Bool(f->cbDeleteFile->Enabled);
    w.EndObject();
    w.Key("btDeleteSetupFile").BeginObject().Key("enabled").Bool(f->btDeleteSetupFile->Enabled).EndObject();
    w.Key("CheckListBox1").BeginObject();
    w.Key("items"); WriteList(w, f->CheckListBox1->Items);
    w.Key("checked").BeginArray();
    for (int i = 0; i < f->CheckListBox1->Items->Count; ++i) w.Bool(f->CheckListBox1->Checked[i]);
    w.EndArray();
    w.EndObject();
    w.Key("labDir").String(U8(f->labDir->Caption));
    w.Key("directory").String(U8(f->DirectoryListBox1->Directory));
    w.Key("dirTree"); WriteDirTree(w, f->DirectoryListBox1->Directory);
    w.Key("drives"); WriteDrives(w);
    {
        std::string d = f->DirectoryListBox1->Directory.c_str();
        w.Key("drive").String(d.size() >= 2 && d[1] == ':' ? std::string(1, (char)tolower((unsigned char)d[0])) : std::string());
    }
    w.Key("currentRecipe").String(U8(fMain ? fMain->cbSetupFileName->Text : AnsiString("")));
    w.Key("dataPath").String(U8(DataPath));
    w.Key("offsetPath").String(U8(OffsetPath));
    w.Key("fShow").Bool(f->fShow);   //AI(W906-FSHOW-B2) 20260929: 回報門面成員本身（close 後 false），不問頁面表
    w.Key("asBackupCreate").String(U8(f->asBackupCreate));
    w.EndObject();
}

const char* FuncName(int wFunc) { return wFunc == FO_COPY ? "copy" : wFunc == FO_DELETE ? "delete" : wFunc == FO_MOVE ? "move" : "other"; }

bool StrItem(const cJSON* root, const char* key, std::string* out, bool* present)
{
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, key);
    if (present) *present = (j != 0);
    if (!j) return true;
    if (!cJSON_IsString(j) || !j->valuestring) return false;
    *out = j->valuestring;
    return true;
}

}  // namespace

std::string W906_BuilderOp(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;
    TfBuilder* f = fBuilder;

    std::string act = "state", err, widget, text, source, name, path, drive;
    bool confirmed = false, hasText = false, hasChecked = false;
    std::vector<std::string> checked;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (!root) err = "value 不是合法 JSON";
        else {
            if (!StrItem(root, "act", &act, 0))       err = "act 要是字串";
            const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
            confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));              // 明確 true 才算確認過
            if (!StrItem(root, "widget", &widget, 0)) err = "widget 要是字串";
            if (!StrItem(root, "text", &text, &hasText)) err = "text 要是字串";
            if (!StrItem(root, "source", &source, 0)) err = "source 要是字串";
            if (!StrItem(root, "name", &name, 0))     err = "name 要是字串";
            if (!StrItem(root, "path", &path, 0))     err = "path 要是字串";
            if (!StrItem(root, "drive", &drive, 0))   err = "drive 要是字串";
            const cJSON* jk = cJSON_GetObjectItemCaseSensitive(root, "checked");
            if (jk) {
                hasChecked = true;
                if (!cJSON_IsArray(jk)) err = "checked 要是字串陣列";
                else for (const cJSON* it = jk->child; it; it = it->next) {
                    if (!cJSON_IsString(it) || !it->valuestring) { err = "checked 要是字串陣列"; break; }
                    checked.push_back(it->valuestring);
                }
            }
            cJSON_Delete(root);
        }
    }
    static const char* kActs[] = { "open", "state", "change", "create", "delete", "dir", "drive", "import", "export", "close" };
    bool known = false;
    for (size_t i = 0; i < sizeof(kActs) / sizeof(kActs[0]); ++i) if (act == kActs[i]) known = true;
    if (err.empty() && !known) err = "unknown act '" + act + "'";
    if (f == 0 || fMain == 0) err = "fBuilder / fMain is NULL";
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
    std::string pGuard, pDetail;       // port guard (not golden)
    bool executed = false, needConfirm = false, reached = false, dropped = false, nameExists = false;
    std::string prompt, excText;
    bool badArg = false;
    std::vector<std::pair<std::string, bool> > after;   // path -> exists, after the operation
    f->W906_ResetTrace();

    try {
        if (act == "open") {
            g = RouteGuard(true);
            if (!g) {
                // Barcode_Reader(bcBuilder) (main.cpp:28522) -- KYEC customers only, skipped (header)
                NewRecordProcess("MES2179", "Enter Builder", " ");                              // main.cpp:28526 (cMyDB.h default Debug=" ")
                f->FormShow(NULL);                                                              // main.cpp:28527 ShowModal -> OnShow
                executed = true;
            }
        } else if (act == "state") {
            executed = true;
        } else if (act == "close") {
            f->spbExitClick(NULL);                                                              // :484-487 Close()
            f->FormClose();                                                                     // OnClose :502-505
            executed = true;
        } else if (act == "change") {
            AnsiString t;
            if (!Acp(text, &t)) throw std::invalid_argument("text 有 ANSI（cp950）表示不出來的字");
            if (widget == "cbSourceFile" || widget == "cbDeleteFile") {
                TComboBox* cb = widget == "cbSourceFile" ? f->cbSourceFile : f->cbDeleteFile;
                const int idx = t == "" ? -1 : IndexOf(cb->Items, t);
                if (t != "" && idx < 0) throw std::invalid_argument(widget + " 是 csDropDownList（cBuilder.dfm），只能選清單裡的項目");
                cb->Text = t; cb->ItemIndex = idx;
                if (widget == "cbSourceFile") f->cbSourceFileChange(NULL); else f->cbDeleteFileChange(NULL);
            } else if (widget == "edNewFileName") {
                f->edNewFileName->Text = KeyFilter(t, &dropped);                                // golden OnKeyPress :495-500
                f->edNewFileNameChange(NULL);                                                   // TEdit OnChange (:43-49)
            } else {
                throw std::invalid_argument("widget 要是 cbSourceFile / edNewFileName / cbDeleteFile");
            }
            executed = true;
        } else if (act == "dir" || act == "drive") {
            AnsiString target;
            QuietErrors q;
            if (act == "dir") {
                if (!Acp(path, &target) || path.empty()) throw std::invalid_argument("path 要是 ANSI 表示得出來的路徑");
            } else {
                if (drive.size() != 1 || !isalpha((unsigned char)drive[0])) throw std::invalid_argument("drive 要是一個字母");
                const int di = tolower((unsigned char)drive[0]) - 'a';
                if (!(GetLogicalDrives() & (1u << di))) throw std::invalid_argument("沒有這個磁碟機");
                // TDirectoryListBox.DriveChange (filectrl.pas:887-902): ChDir(drive) + GetDir -> that drive's
                // current directory; GetFullPathName("X:") answers the same without changing ours
                char spec[3] = { (char)('A' + di), ':', 0 };
                char full[MAX_PATH] = "";
                const DWORD n = GetFullPathNameA(spec, MAX_PATH, full, NULL);
                target = (n > 0 && n < MAX_PATH) ? AnsiString(full) : AnsiString(std::string(spec).append("\\").c_str());
            }
            target = NormDir(target);
            if (!DirExists(target)) {
                pGuard = "dir-missing";
                pDetail = std::string("資料夾不存在或磁碟機沒有就緒：") + U8(target);
            } else {
                // VCL SetDir (filectrl.pas:922-932) also does ChDir(NewDirectory) -- the PROCESS working
                // directory. Not reproduced: wb_serve resolves relative paths (e.g. the web root) against it.
                f->DirectoryListBox1->Directory = target;
                f->DirectoryListBox1Change(NULL);                                               // OnChange :262-265
                executed = true;
            }
        } else {
            // create / delete / import / export: golden route re-checked quietly
            g = RouteGuard(false);
            if (!g) {
                f->iW906MsgBoxAnswer = confirmed ? IDOK : IDCANCEL;
                if (act == "create") {
                    AnsiString src, nm;
                    if (!Acp(source, &src) || !Acp(name, &nm)) throw std::invalid_argument("source/name 有 ANSI（cp950）表示不出來的字");
                    if (src != "" && IndexOf(f->cbSourceFile->Items, src) < 0) {                // P1
                        pGuard = "not-in-list"; pDetail = "來源不在 cbSourceFile 清單裡（csDropDownList，只能選 IniData\\Data 下的配方）";
                    } else {
                        bool drop = false;
                        const AnsiString filtered = KeyFilter(nm, &drop);
                        std::string prob = drop ? "新名稱有 golden edNewFileNameKeyPress 會擋掉的字元（' / : * ? \" < |，OnlyMakeFileDataInPut）" : "";
                        if (prob.empty() && nm != "") prob = NewNameProblem(nm, src);                // P2 / P4
                        if (!prob.empty()) { pGuard = "bad-name"; pDetail = prob; }
                        else {
                            f->cbSourceFile->Text = src; f->cbSourceFile->ItemIndex = IndexOf(f->cbSourceFile->Items, src);
                            f->cbSourceFileChange(NULL);
                            f->edNewFileName->Text = filtered;
                            f->edNewFileNameChange(NULL);
                            if (!f->btCreateSetupFile->Enabled) {                               // a disabled VCL button takes no click
                                pGuard = "button-disabled"; pDetail = "Create 鈕是停用的（edNewFileName 空白，golden :43-49）";
                            } else {
                                nameExists = CiInList(f->cbSourceFile->Items, nm);
                                const AnsiString before = f->edNewFileName->Text;
                                f->btCreateSetupFileClick(NULL);
                                if (f->edNewFileName->Text != before) f->edNewFileNameChange(NULL); // golden :152 Text="" fires TEdit OnChange
                                reached = f->bW906MsgBoxReached;
                                if (reached && confirmed) {
                                    executed = true;
                                    after.push_back(std::make_pair(U8(DataPath + nm), DirExists(DataPath + nm)));
                                    after.push_back(std::make_pair(U8(OffsetPath + nm), DirExists(OffsetPath + nm)));
                                } else if (reached) needConfirm = true;
                            }
                        }
                    }
                } else if (act == "delete") {
                    AnsiString nm;
                    if (!Acp(name, &nm)) throw std::invalid_argument("name 有 ANSI（cp950）表示不出來的字");
                    if (nm != "" && IndexOf(f->cbDeleteFile->Items, nm) < 0) {                  // P1
                        pGuard = "not-in-list"; pDetail = "要刪的配方不在 cbDeleteFile 清單裡（csDropDownList）";
                    } else {
                        const AnsiString cur = fMain->cbSetupFileName->Text;
                        if (nm != "" && nm != cur) {                                            // golden's exact check :211 would not fire
                            const AnsiString last = GetLastOpenFN();
                            if (CiEq(nm, cur) || CiEq(nm, last)) {                              // P3
                                pGuard = "in-use";
                                pDetail = "這是目前使用中的配方（大小寫不同，golden :211 逐位元比較擋不到；NTFS 不分大小寫）：current=" +
                                          U8(cur) + " setup.inf=" + U8(last);
                            }
                        }
                        if (pGuard.empty()) {
                            const AnsiString roots[2] = { DataPath, OffsetPath };
                            for (int i = 0; i < 2 && pGuard.empty(); ++i)
                                if ((roots[i] + nm).Length() >= 255) { pGuard = "path-too-long"; pDetail = "路徑超過 254 字（golden char str1[256]，cBuilder.cpp:234）"; }
                        }
                        if (pGuard.empty()) {
                            f->cbDeleteFile->Text = nm; f->cbDeleteFile->ItemIndex = IndexOf(f->cbDeleteFile->Items, nm);
                            f->cbDeleteFileChange(NULL);
                            if (!f->btDeleteSetupFile->Enabled) {
                                pGuard = "button-disabled"; pDetail = "Delete 鈕是停用的（cbDeleteFile 空白，golden :194-200）";
                            } else {
                                f->btDeleteSetupFileClick(NULL);
                                reached = f->bW906MsgBoxReached;
                                if (reached && confirmed) {
                                    executed = true;
                                    after.push_back(std::make_pair(U8(DataPath + nm), DirExists(DataPath + nm)));
                                    after.push_back(std::make_pair(U8(OffsetPath + nm), DirExists(OffsetPath + nm)));
                                } else if (reached) needConfirm = true;
                            }
                        }
                    }
                } else if (act == "import") {
                    const AnsiString dir = f->DirectoryListBox1->Directory;
                    if (!DirExists(dir)) { pGuard = "dir-missing"; pDetail = "瀏覽的資料夾不存在"; }
                    else if ((dir + "\\*.*").Length() >= 127) {                                 // P4
                        pGuard = "path-too-long"; pDetail = "資料夾路徑太長（golden CopySourTarget 的 char Orgstr[128]，cBuilder.cpp:276）";
                    } else {
                        f->spbImportClick(NULL);
                        reached = f->bW906MsgBoxReached;
                        if (reached && confirmed) executed = true; else if (reached) needConfirm = true;
                    }
                } else if (act == "export") {
                    const AnsiString dir = f->DirectoryListBox1->Directory;
                    std::vector<AnsiString> names;
                    for (size_t i = 0; i < checked.size() && pGuard.empty(); ++i) {
                        AnsiString nm;
                        if (!Acp(checked[i], &nm) || IndexOf(f->CheckListBox1->Items, nm) < 0) {  // P1
                            pGuard = "not-in-list"; pDetail = "勾選的項目不在 CheckListBox1 裡：" + checked[i];
                        } else names.push_back(nm);
                    }
                    if (pGuard.empty() && !DirExists(dir)) { pGuard = "dir-missing"; pDetail = "瀏覽的資料夾不存在"; }
                    if (pGuard.empty() && (SameOrInside(dir, DataPath) || SameOrInside(dir, OffsetPath))) {   // P5
                        pGuard = "export-into-recipes";
                        pDetail = "匯出目的地是配方資料夾本身或在它裡面：golden :448 會先把要匯出的配方 FO_DELETE 掉（" + U8(dir) + "）";
                    }
                    for (size_t i = 0; i < names.size() && pGuard.empty(); ++i) {
                        const AnsiString np = dir + "\\" + names[i];
                        if (SameOrInside(DataPath, np) || SameOrInside(OffsetPath, np)) {
                            pGuard = "export-into-recipes"; pDetail = "匯出路徑 " + U8(np) + " 包含配方資料夾，golden :448 的 FO_DELETE 會刪到它";
                        } else if (np.Length() >= 255 || (DataPath + names[i]).Length() >= 255) {
                            pGuard = "path-too-long"; pDetail = "路徑超過 254 字（golden char cStr1[256]/cStr2[256]，cBuilder.cpp:404）";
                        }
                    }
                    if (pGuard.empty()) {
                        for (int i = 0; i < f->CheckListBox1->Items->Count; ++i)             // the operator's ticks
                            f->CheckListBox1->Checked[i] = std::find(names.begin(), names.end(), AnsiString(f->CheckListBox1->Items->Strings[i])) != names.end();
                        f->spbExportClick(NULL);
                        reached = f->bW906MsgBoxReached;
                        if (reached && confirmed) {
                            executed = true;
                            for (size_t i = 0; i < names.size(); ++i)
                                after.push_back(std::make_pair(U8(dir + "\\" + names[i]), DirExists(dir + "\\" + names[i])));
                        } else if (reached) needConfirm = true;
                    }
                }
            }
        }
    } catch (const std::invalid_argument& e) {
        excText = e.what(); badArg = true;
    } catch (const std::exception& e) {
        excText = e.what();
    } catch (...) {
        excText = "exception in golden TfBuilder";
    }

    w.BeginObject();
    w.Key("act").String(act);
    if (!excText.empty()) {
        w.Key("executed").Bool(false);
        w.Key("guard").String(badArg ? "bad-payload" : "exception");
        w.Key("detail").String(excText);
    } else if (g) {
        w.Key("executed").Bool(false);
        w.Key("guard").String(g->guard);
        w.Key("goldenLine").String(g->goldenLine);
        w.Key("detail").String(g->detail);
    } else if (!pGuard.empty()) {
        w.Key("executed").Bool(false);
        w.Key("guard").String(pGuard);
        w.Key("portGuard").Bool(true);                                  // not golden -- see the header P1..P5
        w.Key("detail").String(pDetail);
    } else {
        w.Key("executed").Bool(executed);
        if (needConfirm) {
            w.Key("needConfirm").Bool(true);
            w.Key("prompt").BeginArray().String(U8(f->asW906MsgBoxText)).EndArray();
            w.Key("caption").String(U8(f->asW906MsgBoxCaption));
            if (act == "create") w.Key("nameExists").Bool(nameExists);
        }
        if (act == "create" || act == "delete" || act == "import" || act == "export") {
            w.Key("confirmReached").Bool(reached);
            if (!reached)                                               // golden stopped before its question: its message says why
                w.Key("goldenStopped").Bool(true);
        }
        if (act == "change") w.Key("keyFiltered").Bool(dropped);
        if (ok) *ok = executed || needConfirm;
    }
    w.Key("shellOps").BeginArray();
    for (size_t i = 0; i < f->vW906ShOps.size(); ++i) {
        const TfBuilderShOp& o = f->vW906ShOps[i];
        w.BeginObject();
        w.Key("op").String(FuncName(o.wFunc));
        w.Key("from").String(U8(o.from));
        w.Key("to").String(U8(o.to));
        w.Key("flags").Number((wb_int64)o.flags);
        w.Key("ret").Number((wb_int64)o.ret);
        w.Key("aborted").Bool(o.aborted);
        w.EndObject();
    }
    w.EndArray();
    w.Key("infoBoxes").BeginArray();
    for (size_t i = 0; i < f->vW906InfoBoxes.size(); ++i) w.String(U8(f->vW906InfoBoxes[i]));
    w.EndArray();
    w.Key("after").BeginArray();
    for (size_t i = 0; i < after.size(); ++i) {
        w.BeginObject(); w.Key("path").String(after[i].first); w.Key("exists").Bool(after[i].second); w.EndObject();
    }
    w.EndArray();
    w.Key("messages").BeginArray();
    for (size_t i = 0; i < cap.msgs.size(); ++i) {
        w.BeginObject();
        w.Key("s1").String(U8(AnsiString(cap.msgs[i].first.c_str())));
        w.Key("s2").String(U8(AnsiString(cap.msgs[i].second.c_str())));
        w.EndObject();
    }
    w.EndArray();
    w.Key("state");
    WriteState(w, f);
    w.EndObject();

    if (act != "state" && act != "change")
        std::printf("builder.op act=%s confirmed=%d -> %s (%u shell ops)\n", act.c_str(), confirmed ? 1 : 0,
                    !excText.empty() ? excText.c_str() : g ? g->guard : !pGuard.empty() ? pGuard.c_str() :
                    needConfirm ? "needConfirm" : executed ? "executed" : "golden-stopped", (unsigned)f->vW906ShOps.size());
    (void)hasText; (void)hasChecked;
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
