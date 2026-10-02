// =============================================================================
//  test_b8_ctl2_timerep.cpp -- B8 CT-L2：Contact 頁 RTC Auto Tuning 勾選看不看得到（golden TfContact::TimerEPTimer）
//
//  //AI(W906-B8-CTL2) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-L2」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp:17177-17232 TimerEPTimer（REAL_TIME_CCD 段 :17191-17217）；
//    TimerEP 由 FormShow :1647 開（EP_Install 3／5）、FormClose :1845 關；COM2->bRTCVerSupportAutoTurnning 只有 rs232.cpp:119 設 false。
//  受測的是 wb_serve 編進去的同一份 FileRW/DeviceForm_File.cpp（＋它 include 的產生檔 FileRW/DeviceForm_File.gen.inc 的 DF_TimerEPTimer）
//    與 C 路共用層 FileRW/_EditList.cpp／_EditPage.cpp／_KitSuck.cpp；god-stack 用 RESCAN 連（同 wb_serve、同 ctest HSys_HeaterMix），
//    沒有 TU-local 的受測替身。入口：FileRW_Contact_Boot（golden 建構）→ FileRW_Contact_TimerEPTick（golden OnTimer 一拍）。
//    [1] 開機值：W906_COM2_bRTCVerSupportAutoTurnning＝false（golden rs232.cpp:119）
//    [2] TimerEP->Enabled=false ⇒ 一拍什麼都不動（golden TTimer 停著不觸發）
//    [3] 沒有 REAL_TIME_CCD ⇒ Visible=false、Checked=false
//    [4] golden 的現實：其他三個條件都成立、成員是 golden 開機值 false ⇒ 一拍之後看不見（計時器有跑的機台「一律隱藏」）
//    [5] 條件本身（不是寫死隱藏）：四個條件 16 種組合，Visible＝四個 AND（成員設 true 只為了驗條件；golden 沒有人設 true）
//    [6] 等級 176：AccessLevel < LevelSet.AccessLevel[176] ⇒ 看不見、>= ⇒ 看得見（Insufficient 的 iMaxLevelItem 照 wb_serve 開機 180）
//    [7] 邊緣（bOldRTCAutoTuning）一般客戶：看不見→看得見那一拍 Checked=false；沒變的拍子不碰勾選；看得見→看不見 Checked=false
//    [8] 邊緣 SIGURD 湖口：Checked＝Visible、Enabled＝!Visible、bRTCAutoTuning＝Visible（兩個方向）
//    [9] 不在這一列：EP 讀值（ADAM_ReadPA）、KYEC_LEE ATC 倒數字 —— 一拍之後 labEPValue／lblATCTempWait 不動（原文在產生檔 #if 0，B8 CT-3）
//    [10] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄，唯讀）：產生檔有 DF_TimerEPTimer 與 golden 條件四項（活碼，不在 #if 0）、
//         FormShow 開／FormClose 關 TimerEP；FileRW/DeviceForm_File.cpp 開頁（DF_FormShow 之後）與 form.event（EvB3Body 之後）都呼叫一拍，
//         而且呼叫在該行 // 之前（接在註解後面＝死碼）
//  NOT COVERED：golden FormShow 本體（讀配方檔、記 Enter 紀錄）不在這裡跑 —— 開頁那一拍的接法由 [10] 的棘輪看；頁面顯示（隱藏）是引擎
//    套 proxies 的 visible（D:\HT9045\web\page\ht9045_wire_engine.js gbApply）。
//  開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\system\ContactInfo.ini、D:\HT9045\config\config.ini 的內容，有變就失敗
//  （本測試不寫任何檔；golden 建構子在預設全域下不走 CheckAndReadIniData）。
// =============================================================================
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "MachineType.h"
#include "atester_shims.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_Contact_Boot();
extern void FileRW_Contact_TimerEPTick();
extern bool W906_COM2_bRTCVerSupportAutoTurnning;
extern int iMaxLevelItem;   // cSecurity.cpp:47（golden cSecurity.cpp:19 檔案層全域；wb_serve W906_SecurityBoot 設 180）

// 連結用（不是受測碼）：
//  * cprod.cpp（ht9045_globals）會叫 FileRW_IniConfig_ChangeCBListProperty（真的那支在 FileRW/IniConfig.cpp，本測試沒編）。不在這裡給，
//    連結器會去抽 ht9045_globals 的 FileRW/_fallback.cpp 成員，它同時定義 FileRW_ProxyChecked，跟直接編進來的 FileRW/_EditList.cpp 撞名。
//    內容跟 FileRW/_fallback.cpp 一字不差（同 tests/test_hsys_heater_mix.cpp）。
//  * W906_Main_sbContactClickOpen（FileRW/MainClick.cpp，只編進 wb_serve）：只有 golden FormShow 之後（FormShowAndSnap）會叫，本測試不跑那一段。
void FileRW_IniConfig_ChangeCBListProperty() {}
const char* W906_Main_sbContactClickOpen() { return nullptr; }

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

TCheckBox* Cb() { return EL<TCheckBox>("TfContact", "cbRTCAutoTuning"); }
TControl* TimerEP() { return EL<TControl>("TfContact", "TimerEP"); }

// 四個條件（golden :17193-17196 的順序）
void SetConds(bool notDummy, bool d74, bool levelOk, bool ver)
{
    COM2->bCCDDummyRum = !notDummy;
    IniConfig.bD74RTCAutoTuning = d74;
    AccessLevel = levelOk ? 3 : 2;               // LevelSet.AccessLevel[176]＝3（main 裡設）
    W906_COM2_bRTCVerSupportAutoTurnning = ver;
}

// 一個檔案的一行（原始碼棘輪用）：去掉 // 之後的註解（字串裡的 // 這裡不會出現）
std::string CodeOf(const std::string& line)
{
    std::string::size_type p = line.find("//");
    return p == std::string::npos ? line : line.substr(0, p);
}

std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::string cur;
    for (char c : text) {
        if (c == '\n') { if (!cur.empty() && cur.back() == '\r') cur.pop_back(); v.push_back(cur); cur.clear(); }
        else cur += c;
    }
    if (!cur.empty()) v.push_back(cur);
    return v;
}

// 函式本體（從整行等於 head 的定義行到下一個只有 "}" 的行；前置宣告帶 ";" 不算），跳過 #if 0 … #endif
std::vector<std::string> LiveBody(const std::vector<std::string>& L, const std::string& head)
{
    std::vector<std::string> out;
    bool in = false, gated = false;
    for (const std::string& l : L) {
        if (!in) { if (l == head) in = true; continue; }
        if (l == "}") break;
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        out.push_back(l);
    }
    return out;
}

bool AnyCode(const std::vector<std::string>& body, const std::string& needle)
{
    for (const std::string& l : body)
        if (CodeOf(l).find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_ctl2_timerep -- B8 CT-L2 golden TfContact::TimerEPTimer (V912 cContact.cpp:17177-17232)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\system\\ContactInfo.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[3];
    bool had[3];
    for (int i = 0; i < 3; ++i) had[i] = ReadAll(kGuard[i], &before[i]);

    // wb_serve 開機的兩件（W906_SecurityBoot：iMaxLevelItem=180；levelset.dat → LevelSet）只取這一格
    iMaxLevelItem = 180;
    LevelSet.AccessLevel[176] = 3;

    FileRW_Contact_Boot();
    Check(filerw::ELFind("TfContact", "cbRTCAutoTuning") != nullptr, "[0] boot: cbRTCAutoTuning proxy exists (golden cContact.h:300)");
    Check(filerw::ELFind("TfContact", "TimerEP") != nullptr, "[0] boot: TimerEP proxy exists (golden cContact.dfm:18634)");
    Check(!TimerEP()->Enabled, "[0] boot: TimerEP DFM Enabled=False (cContact.dfm:18635)");

    std::printf("[1] golden boot value\n");
    Check(W906_COM2_bRTCVerSupportAutoTurnning == false, "[1] W906_COM2_bRTCVerSupportAutoTurnning starts false (golden rs232.cpp:119)");

    std::printf("[2] TimerEP disabled -> no tick\n");
    REAL_TIME_CCD = true;
    SetConds(true, true, true, true);
    Cb()->Visible = true; Cb()->Checked = true; Cb()->Enabled = true;
    TimerEP()->Enabled = false;
    FileRW_Contact_TimerEPTick();
    Check(Cb()->Visible && Cb()->Checked && Cb()->Enabled, "[2] disabled TimerEP: tick changes nothing");

    TimerEP()->Enabled = true;                   // golden FormShow :1647（EP_Install 3／5）

    std::printf("[3] REAL_TIME_CCD off\n");
    REAL_TIME_CCD = false;
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible && !Cb()->Checked, "[3] !REAL_TIME_CCD: Visible=false, Checked=false (golden :17213-17217)");

    std::printf("[4] golden as shipped: member false -> always hidden after one tick\n");
    REAL_TIME_CCD = true;
    SetConds(true, true, true, false);
    Cb()->Visible = true;                        // golden FormShow :1425 Visible=!bCCDDummyRum（計時器第一拍之前）
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible, "[4] !bCCDDummyRum && D74 && level 176 && member=false(golden) -> hidden after the first tick");

    std::printf("[5] the condition itself: 16 combinations\n");
    {
        int ok = 0;
        for (int m = 0; m < 16; ++m) {
            const bool a = (m & 1) != 0, b = (m & 2) != 0, c = (m & 4) != 0, d = (m & 8) != 0;
            SetConds(a, b, c, d);
            FileRW_Contact_TimerEPTick();
            if (Cb()->Visible == (a && b && c && d)) ++ok;
            else std::printf("    mismatch: notDummy=%d D74=%d level=%d member=%d -> Visible=%d\n", a, b, c, d, (int)Cb()->Visible);
        }
        Check(ok == 16, "[5] Visible == !bCCDDummyRum && bD74RTCAutoTuning && Insufficient(176,false) && member, all 16 rows");
    }

    std::printf("[6] level 176\n");
    SetConds(true, true, true, true);
    LevelSet.AccessLevel[176] = 4; AccessLevel = 3;
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible, "[6] AccessLevel 3 < LevelSet.AccessLevel[176] 4 -> hidden");
    AccessLevel = 4;
    FileRW_Contact_TimerEPTick();
    Check(Cb()->Visible, "[6] AccessLevel 4 >= LevelSet.AccessLevel[176] 4 -> visible");
    iMaxLevelItem = 0;                           // GATE (SEC1) 沒開機的值（cSecurity.cpp:47）：Insufficient(176) 一律 false
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible, "[6] iMaxLevelItem 0 (security not booted) -> Insufficient(176) false -> hidden");
    iMaxLevelItem = 180;
    LevelSet.AccessLevel[176] = 3;

    std::printf("[7] edge, ordinary customer\n");
    CUSTOMER_CODE = 0;
    SetConds(true, true, true, false);
    FileRW_Contact_TimerEPTick();                // 看不見（bOld 跟著變成 false）
    FileRW_Contact_TimerEPTick();
    Cb()->Checked = true;                        // 手動設（驗「沒邊緣不碰勾選」）
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible && Cb()->Checked, "[7] hidden, no edge: Checked untouched");
    SetConds(true, true, true, true);
    FileRW_Contact_TimerEPTick();
    Check(Cb()->Visible && !Cb()->Checked, "[7] hidden -> visible edge: Checked=false (golden :17207-17210)");
    Cb()->Checked = true;
    FileRW_Contact_TimerEPTick();
    Check(Cb()->Visible && Cb()->Checked, "[7] visible, no edge: Checked untouched");
    SetConds(true, true, true, false);
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible && !Cb()->Checked, "[7] visible -> hidden edge: Checked=false");

    std::printf("[8] edge, SIGURD HUKOU\n");
    CUSTOMER_CODE = CC_SIGURD_HUKOU;
    bRTCAutoTuning = false;
    Cb()->Enabled = true;
    SetConds(true, true, true, true);
    FileRW_Contact_TimerEPTick();
    Check(Cb()->Visible && Cb()->Checked && !Cb()->Enabled && bRTCAutoTuning,
          "[8] hidden -> visible: Checked=Visible, Enabled=!Visible, bRTCAutoTuning=Visible (golden :17201-17206)");
    SetConds(true, true, true, false);
    FileRW_Contact_TimerEPTick();
    Check(!Cb()->Visible && !Cb()->Checked && Cb()->Enabled && !bRTCAutoTuning,
          "[8] visible -> hidden: Checked=false, Enabled=true, bRTCAutoTuning=false");
    CUSTOMER_CODE = 0;

    std::printf("[9] not in this row (golden text kept in #if 0, B8 CT-3)\n");
    {
        TLabel* ep = EL<TLabel>("TfContact", "labEPValue");
        TLabel* atc = EL<TLabel>("TfContact", "lblATCTempWait");
        ep->Caption = "untouched";
        const int ep0 = EP_Install;
        EP_Install = 3;
        CUSTOMER_CODE = CC_KYEC_LEE; bEnable_KLT_Function = false; bCheckATCTemp = true;
        Temperature.bATCActiveCooling = true; iATCTempWaitTimer = 30;
        FileRW_Contact_TimerEPTick();
        Check(ep->Caption == "untouched", "[9] EP read (ADAM_ReadPA, :17180-17189) not run: labEPValue caption untouched");
        Check(!atc->Visible, "[9] KYEC ATC wait label (:17219-17230) not run: lblATCTempWait stays DFM Visible=False");
        EP_Install = ep0;
        CUSTOMER_CODE = 0; bCheckATCTemp = false; Temperature.bATCActiveCooling = false; iATCTempWaitTimer = 0;
    }

    std::printf("[10] wiring ratchet (source, read-only)\n");
    if (argc < 2) {
        Check(false, "[10] argv[1] (port tree root) missing");
    } else {
        const std::string root = argv[1];
        std::string gen, cpp;
        const bool g = ReadAll(root + "/FileRW/DeviceForm_File.gen.inc", &gen);
        const bool c = ReadAll(root + "/FileRW/DeviceForm_File.cpp", &cpp);
        Check(g && c, "[10] read FileRW/DeviceForm_File.gen.inc and FileRW/DeviceForm_File.cpp");
        const std::vector<std::string> GL = Lines(gen), CL = Lines(cpp);
        const std::vector<std::string> tick = LiveBody(GL, "static void DF_TimerEPTimer()");
        Check(gen.find("// golden cContact.cpp:17177  TfContact::TimerEPTimer(TObject *Sender)") != std::string::npos,
              "[10] generated DF_TimerEPTimer carries the golden head cContact.cpp:17177");
        Check(AnyCode(tick, "->Visible=(!COM2->bCCDDummyRum && IniConfig.bD74RTCAutoTuning && fSecurity->Insufficient(176, false) && W906_COM2_bRTCVerSupportAutoTurnning);"),
              "[10] live code: Visible = the four golden terms in golden order");
        Check(AnyCode(tick, "if(bOldRTCAutoTuning!=EL<TCheckBox>(\"TfContact\", \"cbRTCAutoTuning\")->Visible)"), "[10] live code: bOldRTCAutoTuning edge");
        Check(!AnyCode(tick, "ADAM_ReadPA") && !AnyCode(tick, "lblATCTempWait"), "[10] EP read / KYEC label only inside #if 0");
        const std::vector<std::string> show = LiveBody(GL, "static void DF_FormShow()");
        const std::vector<std::string> close = LiveBody(GL, "static void DF_FormClose()");
        Check(AnyCode(show, "EL<TControl>(\"TfContact\", \"TimerEP\")->Enabled=(EP_Install==3 || EP_Install==5);"), "[10] FormShow enables TimerEP (golden :1647)");
        Check(AnyCode(close, "EL<TControl>(\"TfContact\", \"TimerEP\")->Enabled=false;"), "[10] FormClose disables TimerEP (golden :1845)");
        const std::vector<std::string> snap = LiveBody(CL, "void FormShowAndSnap()");
        Check(AnyCode(snap, "DF_FormShow();  FileRW_Contact_TimerEPTick();"), "[10] page open: one tick right after golden FormShow, before any //");
        const std::vector<std::string> run = LiveBody(CL, "void EvB3Run(TControl* sender)");
        Check(AnyCode(run, "EvB3Body(*e, sender);  FileRW_Contact_TimerEPTick();"), "[10] form.event: one tick after the golden handler, before any //");
    }

    for (int i = 0; i < 3; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    std::printf("test_b8_ctl2_timerep: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
// AI(W906-S09-Q3) 20260930 (St02-E): FileRW/_EditList.cpp now takes the FormJson lock in FileRW_ProxyChecked / FileRW_ProxySet* (St01 R1);
//   this test compiles _EditList.cpp without JsonBridge/FormJson.cpp, so it gives the lock itself (single-threaded:
//   a no-op), as test_b8_os5_sortbuttons.cpp:62-65 does.
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }
