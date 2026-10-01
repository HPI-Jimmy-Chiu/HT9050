// =============================================================================
//  test_s98_cfgtrayplate.cpp -- todo E-003 ①（S98）：Configuration 頁 Tray／Hot Plate 分頁的 WS cfgtrayplate.op
//
//  //AI(W906-S98) 20261001 [W906] St01 新檔。依據 RULINGS_20260926 S98＋S169（「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」，
//    蓋過 Q41 盤點 docs/Q41_INVENTORY_20260927.md 五、「S98，屬 Q41 的『新頁面先不做』」＝S158）。
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp（cp950）：:6945-7217 兩個分頁的處理器
//    （SelectCell、DblClick、btnModify*、btnAdd*、btnDelete*、sbtReload*、sbUpdate*）、:138-141 iSel* 初值、FormShow :5381-5382 權限；
//    myQwertyKeyBoard.cpp:169-302 ShowQwertyKey 的尾段。
//  受測的是 wb_serve 編進去的同一份 FileRW/CfgTrayPlate.cpp（W906_CfgTrayPlateOp）＋ FileRW/_EditPage.cpp（PageShownLevel、PageJson、
//    PageWindowClosed）＋ FileRW/_EditList.cpp（替身、ELOperable）；god-stack 用 RESCAN 連（同 TA5_ScrollBars）。
//    [0]  沙盒：W906_TRAYFORMCSV_PATH／W906_PLATEFORMCSV_PATH 有設、不是真檔（沒有就拒跑）；本測試換到沙盒底下自己的資料夾
//    [1]  測試縫：有設＝環境變數；沒設／空字串＝golden 全域 TrayTablePath／PlateTablePath（字面值 D:\HT9045\System\*.csv，只比字串）
//    [2]  CreateForm 之前 not-ready；CreateForm＋MainFormShowFlags＋FormShow 的兩次 Load Data（golden :4676-4684）之後 get：16 欄、
//         列數＝檔案列數、FixedRows 1／FixedCols 0、目前格 (0,1)（第一次 Load：FixedCols 1→0 Initialize）、iSel (0,0)、格子內容
//    [3]  守衛：運轉中（SystemStart／SoftStart）、沒開頁、關窗邊緣、開頁之後等級變了、不在這一頁、按鈕停用／格子停用
//         ——每一個都什麼都不動（格子、檔案）；權限不夠時點格與連點兩下照 golden 還可以（:6954 btnModifyTray->Click() 不看 Enabled）
//    [4]  點格：固定列／外面 ⇒ bad-cell；(3,2) ⇒ iSel＝(3,2)、目前格＝(3,2)
//    [5]  Modify Data：沒選格 ⇒ keypad open:false、沒變；double 欄開鍵盤規格（N_DOUBLE、dp 2、0～1000、目前值）、格子在 step 0 不變；
//         step 1 照 golden 尾段 atof→CheckRange→AnsiString（"12.345"、"2000"→"1000"、"-5"→"0"、Cancel "6.350"→"6.35"）；
//         打不出來的字 bad-value（整數欄 "12.5"、文字欄 ","）格子不動；沒 step 0 就 step 1 ⇒ keypad；中間表被重讀 ⇒ stale；JCET 收 ','
//    [6]  Add：多一列、新列清空、選到新列（:6993 Row= → OnSelectCell）；Delete：刪選的列、後面往上；刪最後一列選取往上；沒選（列 0）不動；
//         golden 怪處照留：Load Data 讓表變短之後 iSel 停在表外 ⇒ Delete 刪最後一列
//    [7]  Save：沙盒 TrayForm.csv 照 BCB6 CommaText（空欄不加引號、CRLF）、存完照 golden 重讀 ⇒ 第 16 欄掉（R34）；
//         HP Save 寫 PlateForm.csv、重讀的是 Tray 表（R33：Tray 沒存的改動被蓋回檔案值、HP 畫面留著）
//    [8]  VCL 例外：Load Data 找不到檔 ⇒ EFOpenError、格子清空、列數不變；Save 寫不了檔（資料夾不存在）⇒ EFCreateError、沒建檔、格子不變
//    [9]  FormJson 鎖：每一次都拿、都放；參數錯（不是 JSON、op／table 不對、負數、modify 沒 step）
//    [10] 原始碼棘輪（argv[1]＝移植樹根目錄、argv[2]＝web\page，唯讀）：wb_serve 分派在 form.event 那一行、redirect 名單、
//         本檔沒有直接用 TrayTablePath 讀寫、頁面在 st01_ev 之後載入新 JS、tests/CMakeLists.txt 的沙盒
//    [11] 真檔 D:\HT9045\System\TrayForm.csv／PlateForm.csv 前後大小與修改時間相同（只 stat，不開檔）
//  每個 case 開頭 Baseline()：旗標、等級、替身、分頁、兩個沙盒檔與兩張表、iSel／目前格、鎖計數全部放回同一個起點。
// =============================================================================
#include "FileRW/CfgTrayPlate.h"
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "common.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

// 連結用（不是受測碼）：FileRW_IniConfig_ChangeCBListProperty 同 tests/test_ta5_scrollbars.cpp；FormJson 鎖只在 wb_serve ⇒ 這裡記深度。
void FileRW_IniConfig_ChangeCBListProperty() {}
static int g_lockDepth = 0, g_lockTaken = 0, g_lockMax = 0;
namespace ht9045 {
namespace formjson {
void FormLock() { ++g_lockDepth; ++g_lockTaken; if (g_lockDepth > g_lockMax) g_lockMax = g_lockDepth; }
void FormUnlock() { --g_lockDepth; }
}  // namespace formjson
}  // namespace ht9045

void W906_EditPageWindowClosed(const char* goldenForm);   // FileRW/_EditPage.cpp（頁面表的關窗邊緣；tools/wb_serve.cpp 用同一個入口）
using filerw::EL;

namespace {

const char* const kF = "TfConfiguration";
int g_pass = 0;
int g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
bool WriteAll(const std::string& p, const std::string& s)
{
    std::ofstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    f << s;
    return (bool)f;
}
bool Exists(const std::string& p) { struct stat st; return stat(p.c_str(), &st) == 0; }
struct Meta { bool exists; long long size; long long mtime; };
Meta MetaOf(const char* p)
{
    struct stat st;
    Meta m = {false, 0, 0};
    if (stat(p, &st) == 0) { m.exists = true; m.size = (long long)st.st_size; m.mtime = (long long)st.st_mtime; }
    return m;
}
bool SameMeta(const Meta& a, const Meta& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s;
}
std::string Num(int v) { char b[24]; std::snprintf(b, sizeof(b), "%d", v); return b; }
void SetEnv(const std::string& kv) { HT9045_TEST_PUTENV(kv.c_str()); }   // Windows：putenv("K=") 會把 K 拿掉

// ---- 沙盒檔（這個測試自己的夾具，不是機台的檔；表頭照 golden 讀者用到的欄名：cTrayForm.cpp:589-600、cHotPlate.cpp:430-436、
//      btnModify* :6962-6971 看的 Package Type／Group／Memo／Columns (X)／Rows (Y)／BlockNumberX／Y）----
const char* const kTrayHead[16] = {"Package Type", "X Start Pos", "Y Start Pos", "X Pitch", "Y Pitch", "Columns (X)", "Rows (Y)",
                                   "X Width", "Y Width", "Z Tray Tickness", "Group", "Memo", "BlockNumberX", "BlockNumberY",
                                   "BlockPitchX", "BlockPitchY"};
std::string Row16(const char* const* v)   // 每一格後面都有 ','（這台 TrayForm.csv 的樣子，CfgTrayPlate.cpp 檔頭 (b)）
{
    std::string s;
    for (int i = 0; i < 16; ++i) { s += v[i]; s += ','; }
    return s + "\r\n";
}
std::string TraySeed()
{
    const char* const r1[16] = {"QFN-48", "10.5", "20.25", "6.35", "6.350", "8", "12", "3.2", "3.2", "5", "", "memo1", "1", "1", "0", "0"};
    const char* const r2[16] = {"BGA", "11", "21", "7", "7", "10", "20", "4", "4", "6", "G1", "", "2", "2", "30", "40"};
    const char* const r3[16] = {"LGA", "12", "22", "8", "8", "5", "6", "5", "5", "7", "", "", "1", "1", "0", "0"};
    return Row16(kTrayHead) + Row16(r1) + Row16(r2) + Row16(r3);
}
const char* const kHpHead[16] = {"Package Type", "X Start Pos", "Y Start Pos", "X Pitch", "Y Pitch", "Columns (X)", "Rows (Y)",
                                 "Z Height", "Group", "Memo", "P10", "P11", "P12", "P13", "P14", "P15"};
std::string HpSeed()
{
    const char* const r1[16] = {"HP-A", "1", "2", "3", "4", "5", "6", "7", "", "m", "", "", "", "", "", ""};
    const char* const r2[16] = {"HP-B", "9", "8", "7", "6", "5", "4", "3", "g", "", "", "", "", "", "", ""};
    return Row16(kHpHead) + Row16(r1) + Row16(r2);
}

// ---- 替身／別名頁（IniConfig.cpp 沒編進來：這裡照它開機建的樣子建 golden TfConfiguration 的那幾個）----
bool AliasBooted() { return true; }
void AliasNoop() {}
const filerw::PageDesc kAlias = {
    "Config.Configuration", "TfConfiguration", "Config.Configuration.html",
    nullptr, nullptr, 0, nullptr, 0,
    &AliasNoop, &AliasNoop, "none", &AliasNoop, &AliasBooted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_aliasReg(&kAlias);
const char* const kParents[][2] = {   // FileRW/IniConfig.gen.inc:1923／:1935／:1960／:1961／:2012／:2084（golden cConfiguration.dfm）
    {"pnlHP", "tsHPData"}, {"pnlTray", "tsTrayData"}, {"sbtReloadHP", "pnlHP"}, {"sbtReloadTray", "pnlTray"},
    {"tsHPData", "PageControl1"}, {"tsTrayData", "PageControl1"},
};
const char* const kPc1[] = {"tsSoftSimu", "tsTempComm", "tsConfig", "tsTrayData", "tsHPData"};   // FileRW/IniConfig.cpp:973-976

TPanel* Pnl(const char* n) { return EL<TPanel>(kF, n); }
TTabSheet* Tab(const char* n) { return EL<TTabSheet>(kF, n); }
TPageControl* Pc() { return EL<TPageControl>(kF, "PageControl1"); }

bool OpenPage()   // IniConfig 開頁的收尾：PageJson(kEvPage) 記下「在這個等級開過頁」（FileRW/IniConfig.cpp IC_EvAfterFormShow）
{
    std::string json;
    return filerw::PageJson(kAlias, &json) == 200;
}

std::string g_dir, g_tray, g_hp;
TfConfigurationTrayPlate* F() { return W906_CfgTrayPlate(); }

// ---- 呼叫 ----
struct R { bool ok; std::string raw; cJSON* j; };
R Op(const std::string& v)
{
    R r;
    r.ok = false;
    r.raw = W906_CfgTrayPlateOp(v, &r.ok);
    r.j = r.ok ? cJSON_Parse(r.raw.c_str()) : nullptr;
    return r;
}
void Free(R& r) { if (r.j) cJSON_Delete(r.j); r.j = nullptr; }
const cJSON* G(const cJSON* o, const char* k) { return o ? cJSON_GetObjectItemCaseSensitive(o, k) : nullptr; }
int I(const cJSON* o, const char* k) { const cJSON* v = G(o, k); return v && cJSON_IsNumber(v) ? (int)v->valuedouble : -999; }
bool B(const cJSON* o, const char* k) { const cJSON* v = G(o, k); return v && cJSON_IsTrue(v); }
std::string S(const cJSON* o, const char* k) { const cJSON* v = G(o, k); return v && cJSON_IsString(v) ? v->valuestring : std::string("<none>"); }
std::string Cell(const cJSON* grid, int c, int r)
{
    const cJSON* cells = G(grid, "cells");
    const cJSON* row = cells ? cJSON_GetArrayItem(cells, r) : nullptr;
    const cJSON* v = row ? cJSON_GetArrayItem(row, c) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string("<none>");
}
std::string CellNow(bool hp, int c, int r) { return std::string((hp ? F()->strngrdHP : F()->strngrdTray)->Cells[c][r].c_str()); }
int Rows(bool hp) { return (int)(hp ? F()->strngrdHP : F()->strngrdTray)->RowCount; }
std::string Tbl(bool hp) { return hp ? "\"hp\"" : "\"tray\""; }
std::string Sel(bool hp, int c, int r) { return "{\"op\":\"select\",\"table\":" + Tbl(hp) + ",\"col\":" + Num(c) + ",\"row\":" + Num(r) + "}"; }
std::string Simple(const char* op, bool hp) { return std::string("{\"op\":\"") + op + "\",\"table\":" + Tbl(hp) + "}"; }
std::string Mod0(bool hp, const char* via) { return "{\"op\":\"modify\",\"table\":" + Tbl(hp) + ",\"step\":0,\"via\":\"" + via + "\"}"; }
std::string Mod1(bool hp, const std::string& text) { return "{\"op\":\"modify\",\"table\":" + Tbl(hp) + ",\"step\":1,\"text\":\"" + text + "\"}"; }
std::string Mod1Cancel(bool hp) { return "{\"op\":\"modify\",\"table\":" + Tbl(hp) + ",\"step\":1,\"cancel\":true}"; }
bool Starts(const std::string& s, const char* p) { return s.compare(0, std::strlen(p), p) == 0; }

// 每一個 case 的起點：停機、等級 5、替身全開、在 Tray 頁、兩個沙盒檔重寫、兩張表照 FormShow 重讀、iSel／目前格歸位、鎖計數歸零
void Baseline()
{
    SystemStart = false; SoftStart = false;
    CUSTOMER_CODE = 0;
    AccessLevel = 5;
    SetEnv("W906_TRAYFORMCSV_PATH=" + g_tray);
    SetEnv("W906_PLATEFORMCSV_PATH=" + g_hp);
    WriteAll(g_tray, TraySeed());
    WriteAll(g_hp, HpSeed());
    const char* const on[] = {"pnlTray", "pnlHP", "tsTrayData", "tsHPData", "PageControl1", "sbtReloadTray", "sbtReloadHP"};
    for (std::size_t i = 0; i < sizeof(on) / sizeof(on[0]); ++i) {
        TControl* c = filerw::ELFind(kF, on[i]);
        if (c) { c->Enabled = true; c->Visible = true; }
        if (TTabSheet* t = dynamic_cast<TTabSheet*>(c)) t->TabVisible = true;
    }
    Pc()->ActivePageIndex = 3;               // tsTrayData（頁面切頁籤送 form.event PageControl1 之後伺服器的值）
    W906_CfgTrayPlate_MainFormShowFlags();   // golden main.cpp:9987-9988（經測試縫＝沙盒）
    TfConfigurationTrayPlate* f = F();
    if (bHasTrayCSV) f->sbtReloadTray->Click();    // golden FormShow :4676-4684（C 路 IniConfig.gen.inc 同兩行）
    if (bHasPlateCSV) f->sbtReloadHP->Click();
    f->iSelTrayRow = 0; f->iSelTrayCol = 0; f->iSelHPRow = 0; f->iSelHPCol = 0;   // golden 建構子 :138-141
    f->curTray.col = 0; f->curTray.row = 1; f->curHP.col = 0; f->curHP.row = 1;   // 第一次 Load Data 之後的 VCL 目前格
    OpenPage();
    { R g = Op("{\"op\":\"get\"}"); Free(g); }     // 丟掉上一個 case 開著的小鍵盤
    g_lockDepth = 0; g_lockTaken = 0; g_lockMax = 0;
}

bool CodeHas(const std::string& text, const std::string& needle)   // 去掉 // 註解之後才比（同一行插入在 // 之後＝死碼）
{
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) {
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}
std::string LineWith(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) if (l.find(needle) != std::string::npos) return l;
    return std::string();
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_s98_cfgtrayplate -- S98 Configuration Tray / Hot Plate tabs, WS cfgtrayplate.op (golden V912 cConfiguration.cpp:6945-7217)\n");
    const char* const kRealTray = "D:\\HT9045\\System\\TrayForm.csv";
    const char* const kRealHp = "D:\\HT9045\\System\\PlateForm.csv";
    const Meta realTray0 = MetaOf(kRealTray), realHp0 = MetaOf(kRealHp);

    std::printf("[0] sandbox\n");
    const char* e1 = std::getenv("W906_TRAYFORMCSV_PATH");
    const char* e2 = std::getenv("W906_PLATEFORMCSV_PATH");
    const std::string gt = e1 ? e1 : "", gp = e2 ? e2 : "";
    if (gt.empty() || gp.empty() || Lower(gt) == Lower(kRealTray) || Lower(gp) == Lower(kRealHp) ||
        gt.find_last_of("\\/") == std::string::npos || gp.find_last_of("\\/") == std::string::npos) {
        std::printf("  FAIL: W906_TRAYFORMCSV_PATH / W906_PLATEFORMCSV_PATH are not sandbox paths (tests/CMakeLists.txt _ht9045_env_extra sets them) -- refusing to run\n");
        return 1;
    }
    const std::string root = gt.substr(0, gt.find_last_of("\\/"));
    g_dir = root + "/S98_CfgTrayPlate";
    g_tray = g_dir + "/TrayForm.csv";
    g_hp = g_dir + "/PlateForm.csv";
    _mkdir(root.c_str());
    _mkdir(g_dir.c_str());
    std::remove(g_tray.c_str());
    std::remove(g_hp.c_str());
    Check(!Exists(g_tray) && !Exists(g_hp), "[0] own sandbox folder, both files removed before the run: " + g_dir);

    std::printf("[1] test seam\n");
    SetEnv("W906_TRAYFORMCSV_PATH=" + g_tray);
    SetEnv("W906_PLATEFORMCSV_PATH=" + g_hp);
    Check(std::string(W906_TrayTablePath().c_str()) == g_tray && std::string(W906_PlateTablePath().c_str()) == g_hp, "[1] set -> the environment variables");
    SetEnv("W906_TRAYFORMCSV_PATH=");
    SetEnv("W906_PLATEFORMCSV_PATH=");
    Check(std::string(W906_TrayTablePath().c_str()) == kRealTray && std::string(W906_PlateTablePath().c_str()) == kRealHp &&
          std::string(TrayTablePath.c_str()) == kRealTray && std::string(PlateTablePath.c_str()) == kRealHp,
          "[1] unset / empty -> golden globals TrayTablePath / PlateTablePath = the golden literals (strings only, not opened)");
    SetEnv("W906_TRAYFORMCSV_PATH=" + g_tray);
    SetEnv("W906_PLATEFORMCSV_PATH=" + g_hp);

    std::printf("[2] boot, FormShow, get\n");
    {
        R r = Op("{\"op\":\"get\"}");
        Check(!r.ok && Starts(r.raw, "not-ready:"), "[2] before CreateForm: not-ready (" + r.raw.substr(0, 60) + ")");
        Free(r);
    }
    // IniConfig 開機建的替身（golden DFM）＋頁序；CreateForm 要在 sbtReload* 被別人建之前（CfgTrayPlate.cpp W906_CfgTrayPlate_CreateForm）
    Pc(); Tab("tsSoftSimu"); Tab("tsTempComm"); Tab("tsConfig"); Tab("tsTrayData"); Tab("tsHPData"); Pnl("pnlTray"); Pnl("pnlHP");
    filerw::ELSetParents(kF, kParents, (int)(sizeof(kParents) / sizeof(kParents[0])));
    filerw::ELSetPageOrder(kF, "PageControl1", kPc1, 5);
    bHasTrayCSV = false; bHasPlateCSV = false;                    // golden cmydef.cpp:5119-5120 初值：CreateForm 那一刻還沒設（檔頭 ①）
    W906_CfgTrayPlate_CreateForm();
    Check(filerw::ELFind(kF, "sbtReloadTray") == F()->sbtReloadTray && filerw::ELFind(kF, "sbtReloadHP") == F()->sbtReloadHP,
          "[2] CreateForm registers the two Load Data buttons as TfConfiguration proxies");
    Check(F()->strngrdTray->FixedRows == 1 && F()->strngrdTray->FixedCols == 1 && F()->curTray.col == 1 && F()->curTray.row == 1 &&
          F()->iSelTrayRow == 0 && F()->iSelTrayCol == 0 && F()->edtTemp && std::string(F()->edtTemp->Text.c_str()) == "edtTemp",
          "[2] VCL design-time defaults FixedRows 1 / FixedCols 1, current cell (1,1); golden ctor :138-141 iSel 0; edtTemp.Text dfm");
    SetEnv("W906_TRAYFORMCSV_PATH=" + g_tray);
    WriteAll(g_tray, TraySeed());
    W906_CfgTrayPlate_MainFormShowFlags();
    Check(bHasTrayCSV, "[2] golden TfMain::FormShow :9987 bHasTrayCSV = FileExists(seam path)");
    F()->sbtReloadTray->Click();                                   // 第一次 Load Data（golden FormShow :4678）
    Check(F()->strngrdTray->FixedCols == 0 && F()->strngrdTray->FixedRows == 1 && F()->curTray.col == 0 && F()->curTray.row == 1 &&
          F()->iSelTrayRow == 0 && (int)F()->strngrdTray->RowCount == 4,
          "[2] first Load Data: FixedCols 1->0 = VCL Initialize -> current cell (0,1) without OnSelectCell (iSel stays 0)");
    Baseline();
    {
        R r = Op("{\"op\":\"get\"}");
        const cJSON* t = G(r.j, "tray");
        const cJSON* h = G(r.j, "hp");
        Check(r.ok && S(r.j, "op") == "get" && !B(r.j, "ran") && B(r.j, "pageOpen") && !B(r.j, "running"), "[2] get ok, ran=false, pageOpen, not running");
        Check(I(t, "rowCount") == 4 && I(t, "colCount") == 16 && I(t, "fixedRows") == 1 && I(t, "fixedCols") == 0,
              "[2] tray 4 x 16, FixedRows 1, FixedCols 0 (golden :7043 / :7055 / :7075-7077)");
        Check(I(G(t, "cursor"), "col") == 0 && I(G(t, "cursor"), "row") == 1 && I(G(t, "sel"), "row") == 0,
              "[2] current cell (0,1) after the first Load Data (FixedCols 1->0 Initialize); iSel row 0");
        Check(Cell(t, 0, 0) == "Package Type" && Cell(t, 15, 0) == "BlockPitchY" && Cell(t, 4, 1) == "6.350" && Cell(t, 10, 1) == "" &&
              Cell(t, 0, 3) == "LGA", "[2] cells read the golden way (every field followed by ',')");
        const cJSON* cw = G(t, "colWidths");
        Check(cw && cJSON_GetArraySize(cw) == 16 && (int)cJSON_GetArrayItem(cw, 0)->valuedouble == 200 &&
              (int)cJSON_GetArrayItem(cw, 1)->valuedouble == 80, "[2] ColWidths[0]=200, DefaultColWidth 80 (golden :7045-7046)");
        Check(I(h, "rowCount") == 3 && Cell(h, 0, 2) == "HP-B", "[2] hp 3 rows");
        Check(B(G(t, "operable"), "grid") && B(G(t, "operable"), "buttons") && B(G(t, "operable"), "reload") && B(t, "onTab") && !B(h, "onTab"),
              "[2] operable grid / buttons / reload; the server is on the Tray page");
        Check(S(t, "file") == g_tray && B(t, "fileExists"), "[2] file = the sandbox path");
        Free(r);
    }

    std::printf("[3] guards (nothing changes)\n");
    {
        Baseline();
        const std::string before = TraySeed();
        const int rows0 = Rows(false);
        SystemStart = true;
        R a = Op(Simple("add", false));
        Check(!a.ok && Starts(a.raw, "running:"), "[3] SystemStart -> running");
        SystemStart = false; SoftStart = true;
        R b = Op(Simple("save", false));
        Check(!b.ok && Starts(b.raw, "running:"), "[3] SoftStart -> running");
        SoftStart = false;
        R g = Op("{\"op\":\"get\"}");
        Check(g.ok, "[3] get is answered while running too (read only)");
        W906_EditPageWindowClosed("fConfiguration");               // 頁面表的關窗邊緣（golden 物件名 fConfiguration → TfConfiguration）
        R c = Op(Simple("delete", false));
        Check(!c.ok && c.raw.find("reload page") != std::string::npos, "[3] window closed -> reload page");
        OpenPage();
        AccessLevel = 7;
        R d = Op(Simple("add", false));
        Check(!d.ok && d.raw.find("reload page") != std::string::npos, "[3] access level changed after the open -> reload page");
        AccessLevel = 5;
        Pc()->ActivePageIndex = 2;
        R e = Op(Simple("add", false));
        Check(!e.ok && Starts(e.raw, "not-on-tab:"), "[3] server on tsConfig -> not-on-tab");
        Pc()->ActivePageIndex = 3;
        R e2 = Op(Simple("add", true));
        Check(!e2.ok && Starts(e2.raw, "not-on-tab:"), "[3] hp op while the server is on tsTrayData -> not-on-tab");
        Pnl("pnlTray")->Enabled = false;                           // golden FormShow :5381 等級不夠
        R f1 = Op(Simple("add", false));
        R f2 = Op(Simple("reload", false));
        R f3 = Op(Mod0(false, "button"));
        Check(!f1.ok && Starts(f1.raw, "not-operable:") && !f2.ok && Starts(f2.raw, "not-operable:") && !f3.ok && Starts(f3.raw, "not-operable:"),
              "[3] pnlTray disabled -> add / Load Data / Modify Data button: not-operable");
        R f4 = Op(Sel(false, 1, 2));
        R f5 = Op(Mod0(false, "dblclick"));
        const cJSON* kp = G(f5.j, "keypad");
        Check(f4.ok && f5.ok && B(kp, "open"), "[3] pnlTray disabled: click a cell and double click still work (golden :6954 btnModifyTray->Click() ignores Enabled)");
        R f6 = Op(Mod1Cancel(false));
        Check(f6.ok, "[3] ... and its keypad Cancel");
        Tab("tsTrayData")->Enabled = false;                        // golden PageControl1Change :6113 Security_new.def 鎖整頁
        R f7 = Op(Sel(false, 1, 2));
        Check(!f7.ok && Starts(f7.raw, "not-operable:"), "[3] tsTrayData disabled -> even a cell click: not-operable");
        std::string file;
        ReadAll(g_tray, &file);
        Check(file == before && Rows(false) == rows0 && CellNow(false, 1, 2) == "11", "[3] file and table untouched by every refusal");
        Free(a); Free(b); Free(g); Free(c); Free(d); Free(e); Free(e2); Free(f1); Free(f2); Free(f3); Free(f4); Free(f5); Free(f6); Free(f7);
    }

    std::printf("[4] select\n");
    {
        Baseline();
        R a = Op(Sel(false, 3, 0));
        R b = Op(Sel(false, 16, 1));
        R c = Op(Sel(false, 0, 4));
        Check(!a.ok && Starts(a.raw, "bad-cell:") && !b.ok && Starts(b.raw, "bad-cell:") && !c.ok && Starts(c.raw, "bad-cell:"),
              "[4] fixed row 0 / column 16 / row 4 -> bad-cell (VCL MouseDown does not move the current cell)");
        R d = Op(Sel(false, 3, 2));
        const cJSON* t = G(d.j, "tray");
        Check(d.ok && I(G(t, "sel"), "col") == 3 && I(G(t, "sel"), "row") == 2 && I(G(t, "cursor"), "col") == 3 && I(G(t, "cursor"), "row") == 2 &&
              F()->iSelTrayCol == 3 && F()->iSelTrayRow == 2 && S(d.j, "golden").find(":6945") != std::string::npos,
              "[4] (3,2) -> golden strngrdTraySelectCell :6945 iSel = (3,2), current cell (3,2)");
        Free(a); Free(b); Free(c); Free(d);
    }

    std::printf("[5] Modify Data (keypad in two steps)\n");
    {
        Baseline();
        R a = Op(Mod0(false, "button"));
        Check(a.ok && !B(G(a.j, "keypad"), "open"), "[5] nothing selected (iSel row 0, golden :6959) -> keypad open:false");
        R b = Op(Mod1(false, "1"));
        Check(!b.ok && Starts(b.raw, "keypad:"), "[5] step 1 without an open keypad -> keypad");
        Op(Sel(false, 3, 1));                                      // X Pitch（double 欄）
        R c = Op(Mod0(false, "button"));
        const cJSON* kp = G(c.j, "keypad");
        Check(c.ok && B(kp, "open") && S(kp, "kind") == "double" && I(kp, "dp") == 2 && B(kp, "checkRange") && I(kp, "min") == 0 &&
              I(kp, "max") == 1000 && S(kp, "current") == "6.35" && S(kp, "header") == "X Pitch" && CellNow(false, 3, 1) == "6.35",
              "[5] X Pitch: golden :6977 N_DOUBLE dp 2, 0..1000, current value; the cell is unchanged at step 0");
        R d = Op(Mod1(false, "12.345"));
        Check(d.ok && CellNow(false, 3, 1) == "12.345" && Cell(G(d.j, "tray"), 3, 1) == "12.345", "[5] OK \"12.345\" -> cell 12.345");
        Op(Sel(false, 5, 2));                                      // Columns (X)（整數欄）
        R e = Op(Mod0(false, "dblclick"));
        Check(e.ok && S(G(e.j, "keypad"), "kind") == "int" && S(e.j, "golden").find(":6952") != std::string::npos, "[5] Columns (X) via double click: N_INTEGER (golden :6952 -> :6973)");
        R f = Op(Mod1(false, "2000"));
        Check(f.ok && CellNow(false, 5, 2) == "1000", "[5] \"2000\" -> CheckRange -> \"1000\"");
        Op(Mod0(false, "button"));
        R g = Op(Mod1(false, "-5"));
        Check(g.ok && CellNow(false, 5, 2) == "0", "[5] \"-5\" -> \"0\"");
        Op(Mod0(false, "button"));
        R h = Op(Mod1(false, "12.5"));
        Check(!h.ok && Starts(h.raw, "bad-value:") && CellNow(false, 5, 2) == "0", "[5] \"12.5\" in an N_INTEGER column -> bad-value, cell unchanged");
        Op(Sel(false, 4, 1));                                      // Y Pitch "6.350"
        Op(Mod0(false, "button"));
        R i = Op(Mod1Cancel(false));
        Check(i.ok && CellNow(false, 4, 1) == "6.35", "[5] Cancel on \"6.350\" -> \"6.35\" (golden runs the tail on sBackup too, :285-292)");
        Op(Sel(false, 0, 2));                                      // Package Type（文字欄）
        Op(Mod0(false, "button"));
        R j = Op(Mod1(false, "QFN-48_A (x)"));
        Check(j.ok && CellNow(false, 0, 2) == "QFN-48_A (x)", "[5] text column: letters, digits, space, - _ ( ) accepted as typed");
        Op(Mod0(false, "button"));
        R k = Op(Mod1(false, "a,b"));
        Check(!k.ok && Starts(k.raw, "bad-value:") && CellNow(false, 0, 2) == "QFN-48_A (x)", "[5] ',' in a text column -> bad-value (golden N_NO_SYMBOL cannot type it)");
        CUSTOMER_CODE = CC_JCET;
        Op(Mod0(false, "button"));
        R l = Op(Mod1(false, "a,b"));
        Check(l.ok && CellNow(false, 0, 2) == "a,b", "[5] CC_JCET: physical keyboard not filtered (golden :518) -> ',' accepted");
        CUSTOMER_CODE = 0;
        Op(Mod0(false, "button"));
        F()->sbtReloadTray->Click();                               // 別的頁（TrayForm／Cleaning FormShow）重讀了表
        R m = Op(Mod1(false, "X"));
        Check(!m.ok && Starts(m.raw, "stale:") && CellNow(false, 0, 2) == "BGA", "[5] table reloaded between step 0 and 1 -> stale, nothing written");
        Op(Mod0(false, "button"));
        R n = Op(Simple("add", false));
        Check(n.ok && G(n.j, "todo") && cJSON_GetArraySize(G(n.j, "todo")) == 1, "[5] another op while the keypad is open drops it (todo)");
        R o = Op(Mod1(false, "X"));
        Check(!o.ok && Starts(o.raw, "keypad:"), "[5] ... so the late step 1 is refused");
        Free(a); Free(b); Free(c); Free(d); Free(e); Free(f); Free(g); Free(h); Free(i); Free(j); Free(k); Free(l); Free(m); Free(n); Free(o);
    }

    std::printf("[6] Add / Delete\n");
    {
        Baseline();
        Op(Sel(false, 2, 1));
        R a = Op(Simple("add", false));
        const cJSON* t = G(a.j, "tray");
        Check(a.ok && I(t, "rowCount") == 5 && Cell(t, 0, 4) == "" && Cell(t, 15, 4) == "" && I(G(t, "sel"), "row") == 4 &&
              I(G(t, "sel"), "col") == 2 && I(G(t, "cursor"), "row") == 4, "[6] Add: 5 rows, new row empty, selected (golden :6993 Row= -> OnSelectCell, column kept)");
        Op(Sel(false, 0, 2));
        R b = Op(Simple("delete", false));
        t = G(b.j, "tray");
        Check(b.ok && I(t, "rowCount") == 4 && Cell(t, 0, 2) == "LGA" && Cell(t, 0, 3) == "" && F()->Tag == 0 && I(G(t, "sel"), "row") == 2,
              "[6] Delete row 2 (BGA): rows below move up; Tag=atoi(\"BGA\")=0 (golden :7000)");
        Op(Sel(false, 0, 3));
        R c = Op(Simple("delete", false));
        t = G(c.j, "tray");
        Check(c.ok && I(t, "rowCount") == 3 && I(G(t, "sel"), "row") == 2 && I(G(t, "cursor"), "row") == 2,
              "[6] Delete the last row -> VCL moves the current cell up -> OnSelectCell (iSel row 2)");
        F()->iSelTrayRow = 0;
        R d = Op(Simple("delete", false));
        Check(d.ok && Rows(false) == 3, "[6] iSel row 0 -> golden :6998 returns, nothing deleted");
        // Load Data 讓表變短：目前格被擠掉 ⇒ OnSelectCell
        Baseline();
        const char* const rOnly[16] = {"ONLY", "1", "1", "1", "1", "1", "1", "1", "1", "1", "", "", "1", "1", "0", "0"};
        const char* const rLast[16] = {"LAST", "1", "1", "1", "1", "1", "1", "1", "1", "1", "", "", "1", "1", "0", "0"};
        Op(Sel(false, 0, 3));
        WriteAll(g_tray, Row16(kTrayHead) + Row16(rOnly) + Row16(rLast));
        R e = Op(Simple("reload", false));                          // 3 列：目前格 (0,3) 被擠到 (0,2)
        Check(e.ok && F()->iSelTrayRow == 2 && F()->curTray.row == 2, "[6] Load Data shrinking below the current cell -> VCL MoveCurrent -> OnSelectCell (iSel row 2)");
        // golden 怪處（照留）：只剩表頭的檔 ⇒ RowCount 1 ⇒ FixedRows 1->0（Initialize 只搬反白，iSel 不變）⇒ 檔案又變長之後 iSel 停在表外
        Baseline();
        Op(Sel(false, 0, 3));
        WriteAll(g_tray, Row16(kTrayHead));
        R f = Op(Simple("reload", false));
        Check(f.ok && Rows(false) == 1 && F()->strngrdTray->FixedRows == 0 && F()->curTray.row == 0 && F()->iSelTrayRow == 3,
              "[6] header-only file: RowCount 1, FixedRows 0, current cell (0,0) by Initialize, iSel still row 3");
        WriteAll(g_tray, Row16(kTrayHead) + Row16(rOnly) + Row16(rLast));
        R f2 = Op(Simple("reload", false));
        Check(f2.ok && Rows(false) == 3 && F()->curTray.row == 1 && F()->iSelTrayRow == 3,
              "[6] 3 rows again: highlight on row 1 (Initialize), iSel row 3 = outside the table");
        R f3 = Op(Mod0(false, "button"));
        Check(f3.ok && B(G(f3.j, "keypad"), "open"), "[6] Modify Data still opens the keypad for the cell outside the table (golden :6959 only checks > 0)");
        Op(Mod1Cancel(false));
        R g = Op(Simple("delete", false));
        Check(g.ok && Rows(false) == 2 && CellNow(false, 0, 1) == "ONLY",
              "[6] golden quirk kept: iSel beyond RowCount -> Delete removes the LAST row (LAST; :7001 loop skipped, :7008), not the highlighted ONLY");
        Free(f2); Free(f3);
        Free(a); Free(b); Free(c); Free(d); Free(e); Free(f); Free(g);
    }

    std::printf("[7] Save\n");
    {
        Baseline();
        Op(Sel(false, 0, 1));
        Op(Mod0(false, "button"));
        Op(Mod1(false, "QFN 48"));
        R a = Op(Simple("save", false));
        std::string file;
        ReadAll(g_tray, &file);
        const std::string head = "\"Package Type\",\"X Start Pos\",\"Y Start Pos\",\"X Pitch\",\"Y Pitch\",\"Columns (X)\",\"Rows (Y)\",\"X Width\","
                                 "\"Y Width\",\"Z Tray Tickness\",Group,Memo,BlockNumberX,BlockNumberY,BlockPitchX,BlockPitchY\r\n";
        const std::string row1 = "\"QFN 48\",10.5,20.25,6.35,6.350,8,12,3.2,3.2,5,,memo1,1,1,0,0\r\n";
        Check(a.ok && S(a.j, "wrote") == g_tray && file.compare(0, head.size(), head) == 0 && file.compare(head.size(), row1.size(), row1) == 0,
              "[7] TrayForm.csv written with BCB6 CommaText (quotes only for space / comma / quote; empty fields bare), CRLF");
        const cJSON* t = G(a.j, "tray");
        Check(Cell(t, 15, 0) == "" && Cell(t, 14, 0) == "BlockPitchX" && Cell(t, 0, 1) == "QFN 48",
              "[7] golden reload after save (:7033): column 16 dropped (R34), quotes stripped");
        Op(Sel(false, 1, 2));                                      // Tray 還沒存的改動
        Op(Mod0(false, "button"));
        Op(Mod1(false, "99"));
        Check(CellNow(false, 1, 2) == "99", "[7] tray cell changed, not saved");
        Pc()->ActivePageIndex = 4;
        Op(Sel(true, 1, 1));
        Op(Mod0(true, "button"));
        Op(Mod1(true, "5.5"));
        R b = Op(Simple("save", true));
        std::string hp;
        ReadAll(g_hp, &hp);
        Check(b.ok && S(b.j, "wrote") == g_hp && hp.find("HP-A,5.5,2,3,4,5,6,7,,m,,,,,,\r\n") != std::string::npos,
              "[7] PlateForm.csv written");
        Check(CellNow(false, 1, 2) == "11" && CellNow(true, 1, 1) == "5.5",
              "[7] HP Save reloads the TRAY table (golden :7215, R33): the unsaved tray change is gone, the HP screen keeps its values");
        Free(a); Free(b);
    }

    std::printf("[8] VCL exceptions\n");
    {
        Baseline();
        std::remove(g_tray.c_str());
        R a = Op(Simple("reload", false));
        const cJSON* t = G(a.j, "tray");
        Check(a.ok && S(G(a.j, "error"), "class") == "EFOpenError" && I(t, "rowCount") == 4 && Cell(t, 0, 0) == "" && Cell(t, 0, 1) == "" &&
              !B(t, "fileExists"), "[8] Load Data, file missing -> EFOpenError; cells cleared (:7048-7052), row count kept (stops at :7054)");
        Baseline();
        const std::string nodir = g_dir + "/no_such_dir/TrayForm.csv";
        SetEnv("W906_TRAYFORMCSV_PATH=" + nodir);
        Op(Sel(false, 0, 1));
        R b = Op(Simple("save", false));
        Check(b.ok && S(G(b.j, "error"), "class") == "EFCreateError" && !Exists(nodir) && S(b.j, "wrote") == "<none>" &&
              CellNow(false, 0, 1) == "QFN-48", "[8] Save into a missing folder -> EFCreateError, no file, table unchanged");
        SetEnv("W906_TRAYFORMCSV_PATH=" + g_tray);
        // op 以外（開頁 FormShow）照舊：找不到檔 vclcompat 回空表、不丟例外
        std::remove(g_tray.c_str());
        bool threw = false;
        try { F()->sbtReloadTray->Click(); } catch (...) { threw = true; }
        Check(!threw, "[8] outside the op (FormShow readers) a missing file does not throw (unchanged behaviour)");
        Free(a); Free(b);
    }

    std::printf("[9] lock, bad payloads\n");
    {
        Baseline();
        R a = Op(Sel(false, 1, 1));
        R b = Op(Simple("add", false));
        Check(a.ok && b.ok && g_lockTaken == 2 && g_lockDepth == 0 && g_lockMax == 1, "[9] FormJson lock taken once per op and released");
        R c = Op("not json");
        R d = Op("{\"op\":\"nope\"}");
        R e = Op("{\"op\":\"add\",\"table\":\"x\"}");
        R f = Op("{\"op\":\"select\",\"table\":\"tray\",\"col\":-1,\"row\":1}");
        R g = Op("{\"op\":\"modify\",\"table\":\"tray\"}");
        R h = Op("{\"op\":\"modify\",\"table\":\"tray\",\"step\":0,\"via\":\"keyboard\"}");
        Check(!c.ok && !d.ok && !e.ok && !f.ok && !g.ok && !h.ok && Starts(c.raw, "bad-payload:") && Starts(d.raw, "bad-payload:") &&
              Starts(e.raw, "bad-payload:") && Starts(f.raw, "bad-payload:") && Starts(g.raw, "bad-payload:") && Starts(h.raw, "bad-payload:"),
              "[9] not JSON / unknown op / bad table / negative col / modify without step / bad via -> bad-payload");
        Free(a); Free(b); Free(c); Free(d); Free(e); Free(f); Free(g); Free(h);
    }

    std::printf("[10] source ratchets\n");
    if (argc < 3) {
        Check(false, "[10] argv[1] = port root, argv[2] = web\\page");
    } else {
        const char* o1 = std::getenv("W906_S98_SRC_ROOT");        // 對照組：指到改之前的檔案副本，[10] 一定要紅
        const char* o2 = std::getenv("W906_S98_PAGE_DIR");
        const std::string rootDir = (o1 && *o1) ? o1 : argv[1], page = (o2 && *o2) ? o2 : argv[2];
        std::string wb, cfg, html, cm;
        const bool okr = ReadAll(rootDir + "/tools/wb_serve.cpp", &wb) && ReadAll(rootDir + "/FileRW/CfgTrayPlate.cpp", &cfg) &&
                         ReadAll(page + "/Config.Configuration.html", &html) && ReadAll(rootDir + "/tests/CMakeLists.txt", &cm);
        Check(okr, "[10] read wb_serve.cpp / CfgTrayPlate.cpp / Config.Configuration.html / tests/CMakeLists.txt");
        const std::string disp = LineWith(wb, "wc.cmd == \"cfgtrayplate.op\"");
        Check(CodeHas(disp, "wc.cmd == \"cfgtrayplate.op\"") && CodeHas(disp, "W906_CfgTrayPlateOp(payload, &tpOk)") &&
              disp.find("wc.cmd == \"form.event\"") != std::string::npos,
              "[10] wb_serve.cpp: cfgtrayplate.op dispatched in code (not a comment) on the form.event line (same-line insert)");
        const std::string red = LineWith(wb, "\"W906_A01CONFIGINI_PATH\", \"W906_AGVINI_PATH\"");
        Check(CodeHas(red, "\"W906_PLATEFORMCSV_PATH\"") && CodeHas(red, "\"W906_TRAYFORMCSV_PATH\""), "[10] W906_PrintDataRedirects lists both seams");
        Check(!CodeHas(cfg, "(TrayTablePath)") && !CodeHas(cfg, "(PlateTablePath)"), "[10] CfgTrayPlate.cpp reads / writes only through the seam");
        const std::string::size_type iEng = html.find("<script src=\"ht9045_wire_engine.js\">"),
                                     iEv = html.find("<script src=\"ht9045_config_st01_ev.js\">"),
                                     iTp = html.find("<script src=\"ht9045_config_trayplate.js\">");
        Check(iEng != std::string::npos && iEv != std::string::npos && iTp != std::string::npos && iEng < iEv && iEv < iTp,
              "[10] Config.Configuration.html loads ht9045_config_trayplate.js after the engine and after ht9045_config_st01_ev.js");
        Check(CodeHas(cm, "W906_TRAYFORMCSV_PATH=${CMAKE_CURRENT_BINARY_DIR}") && CodeHas(cm, "W906_PLATEFORMCSV_PATH=${CMAKE_CURRENT_BINARY_DIR}"),
              "[10] tests/CMakeLists.txt gives every ctest a sandbox for both CSVs (_ht9045_env_extra)");
    }

    std::printf("[11] real files\n");
    Check(SameMeta(realTray0, MetaOf(kRealTray)) && SameMeta(realHp0, MetaOf(kRealHp)),
          "[11] D:\\HT9045\\System\\TrayForm.csv / PlateForm.csv size and mtime unchanged (stat only)");

    std::printf("test_s98_cfgtrayplate: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
