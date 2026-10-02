// =============================================================================
//  test_setupa_kbextra_n04.cpp -- setupA 兩個 C++ 補件：
//    (A) golden TfConfiguration 建構子 [N04] Machine Info 的本機名稱／IP（cConfiguration.cpp:121-147）→ FileRW/IniConfig_N04.cpp
//    (B) TfContact editlist.get extra：golden 小鍵盤的執行期上下限＋edDropWaitTimeMouseDown 輸入後修正要的值 → FileRW/DeviceForm_KbExtra.cpp
//
//  //AI(W906-SETUPA) 20261002 新檔。EastSun 1001「請檢查每個頁面元件…用枚舉 每個東西都檢查」（setupA 枚舉 12 頁）。
//  golden（906 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618，cp950）：
//    cConfiguration.cpp:121-147 WSAStartup／gethostname → edtN04_Host／(KYEC_LEE 且沒 KLT) edN04_ID／gethostbyname → mmoN04_IP->Lines
//    cContact.cpp:19029-19050 edForcePerPinGMouseDown、:18542-18563 edForcePerPinNMouseDown、:15164-15168 edContactOffsetArm1MouseDown、
//      :16867-16874 edShtPickOffset1MouseDown、:18357-18369 edDoubleForceMouseDown、:2089-2239 edDropWaitTimeMouseDown
//    [1] (A) 本機名稱＝GetComputerNameExA(ComputerNameDnsHostname)（同一個名字，不分大小寫）；每個 IP 是 a.b.c.d
//    [2] (A) 替身：edtN04_Host->Text＝本機名稱、Enabled=false（dfm）；mmoN04_IP->Lines 逐筆＝IP、Text＝每行接 CRLF；父層 gbN04
//    [3] (A) KYEC_LEE 且 bEnable_KLT_Function==false：edN04_ID＝本機名稱、停用、IniConfig.SocketHandlerID／sGPIBMachineID；其他客戶不碰
//    [4] (B) 一般客戶：edForcePerPinG／edDieForcePerPinG＝[DOUBLE,4,true,High+10,Low]；N 兩格同；JCET／AMKOR_China 不加 10
//    [5] (B) edContactOffsetArm1/2＝dContactHigh／Low；edShtPickOffset1/2：bChangeKitNoHardStop ? dShuttle* : iOffsetZ*
//    [6] (B) edDoubleForce：kit 3.0 → d=30 → max 36.02、min 2.88（MyFormatFloat 2 位）
//    [7] (B) clamp：hanaMicron、indexDownPos、directContactModeDiffentSpeed=2、tmoveSlowContact、Loader tray X/Y pitch、XDivision、
//        fixedDropSpeed／iFixedDropSpeed、koreaFunction
//    [8] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄，唯讀）：IniConfig.cpp 的 Boot 在 // 之前呼叫 FileRW_IniConfig_N04Ctor；
//        DeviceForm_File.cpp 的 g_page 在 // 之前帶 &FileRW_Contact_KbExtraJson；CMakeLists.txt wb_serve 那一行有兩個新檔
//  不寫任何檔：開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini，有變就失敗。只讀本機網路設定。
// =============================================================================
#include <winsock2.h>
#include <windows.h>

#include "FileRW/_EditList.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

bool W906_IniConfig_N04HostIP(std::string* host, std::vector<std::string>* ips);
void FileRW_IniConfig_N04Ctor();
std::string FileRW_Contact_KbExtraJson();

// 連結用（不是受測碼）：同 tests/test_b8_ctl2_timerep.cpp（cprod.cpp 會叫；不給的話連結器抽 FileRW/_fallback.cpp 跟 _EditList.cpp 撞名）
void FileRW_IniConfig_ChangeCBListProperty() {}
//  * formjson 鎖（JsonBridge，只編進 wb_serve）：_EditList.cpp 的 ELCheckedLocked 用；單執行緒測試給空的（同 tests/test_b8_ctl2_timerep.cpp:289）
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }

using filerw::EL;

namespace {

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

std::string Lower(std::string s) { for (char& c : s) c = (char)std::tolower((unsigned char)c); return s; }

bool IsDottedQuad(const std::string& s)
{
    int parts = 0, digits = 0, val = 0;
    for (std::size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == '.') {
            if (digits == 0 || digits > 3 || val > 255) return false;
            ++parts; digits = 0; val = 0;
            continue;
        }
        if (!std::isdigit((unsigned char)s[i])) return false;
        val = val * 10 + (s[i] - '0'); ++digits;
    }
    return parts == 4;
}

std::string CodeOf(const std::string& line)
{
    std::string::size_type p = line.find("//");
    return p == std::string::npos ? line : line.substr(0, p);
}

bool AnyCodeLine(const std::string& text, const std::string& needle)
{
    std::istringstream ss(text);
    std::string l;
    while (std::getline(ss, l)) if (CodeOf(l).find(needle) != std::string::npos) return true;
    return false;
}

// extra.kb.<id> == [flags, dp, true, a, b]
bool KbIs(cJSON* kb, const char* id, const char* flags, int dp, double a, double b)
{
    cJSON* arr = cJSON_GetObjectItemCaseSensitive(kb, id);
    if (!cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 5) return false;
    cJSON* f = cJSON_GetArrayItem(arr, 0);
    cJSON* d = cJSON_GetArrayItem(arr, 1);
    cJSON* c = cJSON_GetArrayItem(arr, 2);
    cJSON* x = cJSON_GetArrayItem(arr, 3);
    cJSON* y = cJSON_GetArrayItem(arr, 4);
    return cJSON_IsString(f) && std::strcmp(f->valuestring, flags) == 0 && cJSON_IsNumber(d) && (int)d->valuedouble == dp &&
           cJSON_IsTrue(c) && cJSON_IsNumber(x) && std::fabs(x->valuedouble - a) < 1e-9 && cJSON_IsNumber(y) && std::fabs(y->valuedouble - b) < 1e-9;
}

cJSON* Extra(cJSON** root)
{
    *root = cJSON_Parse(FileRW_Contact_KbExtraJson().c_str());
    return *root;
}

double Num(cJSON* o, const char* k) { cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k); return cJSON_IsNumber(v) ? v->valuedouble : -12345.0; }
int    Bool(cJSON* o, const char* k) { cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k); return cJSON_IsBool(v) ? (cJSON_IsTrue(v) ? 1 : 0) : -1; }

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_setupa_kbextra_n04 -- golden cConfiguration.cpp:121-147 [N04] + TfContact keypad limits (cContact.cpp)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);

    // ---- [1] 本機名稱／IP
    std::string host;
    std::vector<std::string> ips;
    const bool ok = W906_IniConfig_N04HostIP(&host, &ips);
    char cn[256] = {0};
    DWORD n = sizeof(cn);
    const bool cnOk = ::GetComputerNameExA(ComputerNameDnsHostname, cn, &n) != 0;
    std::printf("  host '%s', %d IP(s), GetComputerNameExA '%s'\n", host.c_str(), (int)ips.size(), cn);
    Check(ok && !host.empty(), "[1] gethostname ok, non-empty");
    Check(cnOk && Lower(host) == Lower(std::string(cn, n)), "[1] host == GetComputerNameExA(ComputerNameDnsHostname)");
    bool allQuad = !ips.empty();
    for (const std::string& s : ips) if (!IsDottedQuad(s)) allQuad = false;
    Check(allQuad, "[1] at least one IP, every IP is a.b.c.d");

    // ---- [2] 替身
    const int ccSave = CUSTOMER_CODE;
    CUSTOMER_CODE = CC_PTI;
    TEdit* edId = EL<TEdit>("TfConfiguration", "edN04_ID");
    edId->Text = "UNCHANGED";
    const AnsiString sockSave = IniConfig.SocketHandlerID;
    FileRW_IniConfig_N04Ctor();
    TEdit* edHost = EL<TEdit>("TfConfiguration", "edtN04_Host");
    TMemo* mmo = EL<TMemo>("TfConfiguration", "mmoN04_IP");
    Check(std::string(edHost->Text.c_str()) == host, "[2] edtN04_Host->Text == host name (golden :127)");
    Check(edHost->Enabled == false, "[2] edtN04_Host Enabled=false (dfm :19834)");
    bool linesOk = mmo->Lines->Count == (int)ips.size();
    std::string joined;
    for (int i = 0; linesOk && i < mmo->Lines->Count; ++i) {
        if (std::string(mmo->Lines->Strings[i].c_str()) != ips[i]) linesOk = false;
        joined += ips[i] + "\r\n";
    }
    Check(linesOk, "[2] mmoN04_IP->Lines == IPs in order (golden :145)");
    Check(std::string(mmo->Text.c_str()) == joined, "[2] mmoN04_IP->Text == lines + CRLF each (VCL TMemo.Text)");
    Check(filerw::ELEditable("TfConfiguration", "edtN04_Host") == false, "[2] edtN04_Host not editable (Enabled=false)");
    Check(std::string(edId->Text.c_str()) == "UNCHANGED" && IniConfig.SocketHandlerID == sockSave, "[3] CC_PTI: edN04_ID / SocketHandlerID untouched");

    // ---- [3] KYEC_LEE（沒 KLT）
    {
        const bool kltSave = bEnable_KLT_Function;
        const AnsiString gpibSave = IniConfig.sGPIBMachineID;
        CUSTOMER_CODE = CC_KYEC_LEE;
        bEnable_KLT_Function = false;
        mmo->Lines->Clear();
        FileRW_IniConfig_N04Ctor();
        Check(std::string(edId->Text.c_str()) == host && edId->Enabled == false, "[3] KYEC_LEE: edN04_ID = host, disabled (golden :130-131)");
        Check(std::string(IniConfig.SocketHandlerID.c_str()) == host && std::string(IniConfig.sGPIBMachineID.c_str()) == host,
              "[3] KYEC_LEE: SocketHandlerID / sGPIBMachineID = host (golden :132-133)");
        IniConfig.SocketHandlerID = sockSave;
        IniConfig.sGPIBMachineID = gpibSave;
        bEnable_KLT_Function = kltSave;
        CUSTOMER_CODE = CC_PTI;
    }

    // ---- [4]-[7] Contact extra
    const INPUT_LIMIT ilSave = InputLimit;
    InputLimit.dForcePerpinHigh = 100.0;  InputLimit.dForcePerpinLow = 1.5;
    InputLimit.dForcePerpinNHigh = 0.98;  InputLimit.dForcePerpinNLow = 0.01;
    InputLimit.dContactHigh = 2.0;        InputLimit.dContactLow = -0.5;
    InputLimit.iOffsetZHigh = 3;          InputLimit.iOffsetZLow = -3;
    InputLimit.dShuttleHigh = 2.0;        InputLimit.dShuttleLow = -2.0;
    const bool chSave = IniConfig.bChangeKitNoHardStop;
    const double kitSave = DeviceForm_File.dDieForceKitDiameter;
    DeviceForm_File.dDieForceKitDiameter = 3.0;
    IniConfig.bChangeKitNoHardStop = false;
    {
        cJSON* root = nullptr;
        cJSON* kb = cJSON_GetObjectItemCaseSensitive(Extra(&root), "kb");
        Check(KbIs(kb, "edForcePerPinG", "DOUBLE", 4, 110.0, 1.5) && KbIs(kb, "edDieForcePerPinG", "DOUBLE", 4, 110.0, 1.5),
              "[4] CC_PTI: edForcePerPinG / edDieForcePerPinG = [DOUBLE,4,true,High+10,Low] (golden :19048)");
        Check(KbIs(kb, "edForcePerPinN", "DOUBLE", 4, 10.98, 0.01) && KbIs(kb, "edDieForcePerPinN", "DOUBLE", 4, 10.98, 0.01),
              "[4] CC_PTI: edForcePerPinN / edDieForcePerPinN = NHigh+10, NLow (golden :18561)");
        Check(KbIs(kb, "edContactOffsetArm1", "DOUBLE", 2, 2.0, -0.5) && KbIs(kb, "edContactOffsetArm2", "DOUBLE", 2, 2.0, -0.5),
              "[5] edContactOffsetArm1/2 = dContactHigh/Low (golden :15167)");
        Check(KbIs(kb, "edShtPickOffset1", "DOUBLE", 2, 3.0, -3.0) && KbIs(kb, "edShtPickOffset2", "DOUBLE", 2, 3.0, -3.0),
              "[5] bChangeKitNoHardStop=false: edShtPickOffset1/2 = iOffsetZHigh/Low (golden :16873)");
        Check(KbIs(kb, "edDoubleForce", "DOUBLE", 2, 2.88, 36.02), "[6] edDoubleForce kit 3.0 -> [2.88, 36.02] (golden :18360-18367)");
        cJSON_Delete(root);
    }
    IniConfig.bChangeKitNoHardStop = true;
    CUSTOMER_CODE = CC_JCET;
    {
        cJSON* root = nullptr;
        cJSON* kb = cJSON_GetObjectItemCaseSensitive(Extra(&root), "kb");
        Check(KbIs(kb, "edShtPickOffset1", "DOUBLE", 2, 2.0, -2.0), "[5] bChangeKitNoHardStop=true: edShtPickOffset1 = dShuttleHigh/Low (golden :16871)");
        Check(KbIs(kb, "edForcePerPinG", "DOUBLE", 4, 100.0, 1.5) && KbIs(kb, "edForcePerPinN", "DOUBLE", 4, 0.98, 0.01),
              "[4] CC_JCET: no +10 (golden :19044 / :18557)");
        cJSON_Delete(root);
    }
    CUSTOMER_CODE = CC_AMKOR_China;
    {
        cJSON* root = nullptr;
        cJSON* kb = cJSON_GetObjectItemCaseSensitive(Extra(&root), "kb");
        Check(KbIs(kb, "edForcePerPinG", "DOUBLE", 4, 100.0, 1.5), "[4] CC_AMKOR_China: no +10");
        cJSON_Delete(root);
    }
    // [7] clamp
    CUSTOMER_CODE = CC_HANA_MICRON;
    const double idpSave = fIndexDownPos;
    fIndexDownPos = -146.0;
    const int cmSave = DeviceForm.ContactMode;
    DeviceForm.ContactMode = TMoveSlowContact;
    const int ttSave = TrayForm.Loader.iTrayType;
    TrayForm.Loader.iTrayType = 1;
    const double ypSave = UserDefForm_File[1].YPitch, xpSave = UserDefForm_File[1].XPitch;
    UserDefForm_File[1].YPitch = 12.5;
    UserDefForm_File[1].XPitch = 14.25;
    const bool fdSave = CosFunction.bFixedDropSpeed;
    const int ifdSave = CosFunction.iFixedDropSpeed;
    CosFunction.bFixedDropSpeed = true;
    CosFunction.iFixedDropSpeed = 30;
    const bool koSave = IniConfig.bKoreaFunction;
    IniConfig.bKoreaFunction = true;
    {
        cJSON* root = nullptr;
        cJSON* cl = cJSON_GetObjectItemCaseSensitive(Extra(&root), "clamp");
        Check(Bool(cl, "hanaMicron") == 1, "[7] hanaMicron (golden :2103)");
        Check(Num(cl, "indexDownPos") == -146.0, "[7] indexDownPos = fIndexDownPos (golden :2122)");
        Check(Num(cl, "directContactModeDiffentSpeed") == (double)DirectContactModeDiffentSpeed, "[7] directContactModeDiffentSpeed (golden :2127)");
        Check(Bool(cl, "tmoveSlowContact") == 1, "[7] DeviceForm.ContactMode==TMoveSlowContact (golden :2128)");
        Check(Num(cl, "loaderYPitch") == 12.5 && Num(cl, "loaderXPitch") == 14.25, "[7] UserDefForm_File[TrayForm.Loader.iTrayType].Y/XPitch (golden :2159/:2174)");
        Check(Num(cl, "loadFormXDivision") == (double)(LoadForm ? LoadForm->XDivision : 0), "[7] LoadForm->XDivision (golden :2168)");
        Check(Bool(cl, "fixedDropSpeed") == 1 && Num(cl, "iFixedDropSpeed") == 30.0, "[7] bFixedDropSpeed / iFixedDropSpeed (golden :2186-2190)");
        Check(Bool(cl, "koreaFunction") == 1, "[7] bKoreaFunction (golden :2219)");
        cJSON_Delete(root);
    }
    CUSTOMER_CODE = ccSave;
    fIndexDownPos = idpSave;
    DeviceForm.ContactMode = cmSave;
    TrayForm.Loader.iTrayType = ttSave;
    UserDefForm_File[1].YPitch = ypSave;
    UserDefForm_File[1].XPitch = xpSave;
    CosFunction.bFixedDropSpeed = fdSave;
    CosFunction.iFixedDropSpeed = ifdSave;
    IniConfig.bKoreaFunction = koSave;
    IniConfig.bChangeKitNoHardStop = chSave;
    DeviceForm_File.dDieForceKitDiameter = kitSave;
    InputLimit = ilSave;

    // ---- [8] 原始碼棘輪
    if (argc > 1) {
        const std::string root = argv[1];
        std::string ini, dev, cm;
        const bool r1 = ReadAll(root + "\\FileRW\\IniConfig.cpp", &ini);
        const bool r2 = ReadAll(root + "\\FileRW\\DeviceForm_File.cpp", &dev);
        const bool r3 = ReadAll(root + "\\CMakeLists.txt", &cm);
        Check(r1 && AnyCodeLine(ini, "FileRW_IniConfig_N04Ctor();"), "[8] IniConfig.cpp calls FileRW_IniConfig_N04Ctor (live code, before //)");
        Check(r2 && AnyCodeLine(dev, "&FileRW_Contact_KbExtraJson"), "[8] DeviceForm_File.cpp g_page carries &FileRW_Contact_KbExtraJson (live code)");
        Check(r3 && AnyCodeLine(cm, "FileRW/IniConfig_N04.cpp") && AnyCodeLine(cm, "FileRW/DeviceForm_KbExtra.cpp"),
              "[8] CMakeLists.txt lists both new files (live, not in a comment)");
    } else {
        Check(false, "[8] argv[1] (tree root) missing");
    }

    for (int i = 0; i < 2; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("no write: ") + kGuard[i]);
    }
    std::printf("RESULT: %d passed, %d failed -- %s\n", g_pass, g_fail, g_fail ? "FAIL" : "ALL PASS");
    return g_fail ? 1 : 0;
}
