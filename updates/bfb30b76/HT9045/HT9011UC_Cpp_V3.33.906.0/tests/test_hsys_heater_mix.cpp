// =============================================================================
//  test_hsys_heater_mix.cpp -- HandlerSys「Heater」分頁的 Q34 方案 D＋Q15 讀寫檔（FileRW/HSys.cpp 第 (4) 段）。
//
//  //AI(W906-FRW-S166) 20260927 [W906]：新檔。裁決 RULINGS_20260926 S166（Q34 方案 D；D-2＝A 依 RULINGS_20260927 第 7 條第 35 題；
//    D-1b D-3a D-4a D-5a D-6a D-7a D-8a D-9a）＋S137（Q15：缺鍵＝3 No Heater、開頁不寫檔）＋S136（Q14＝B）。
//    全文 D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q34.」。
//
//  受測的是 wb_serve 編進去的同一份 FileRW/HSys.cpp（＋它 include 的產生檔 FileRW/HSys.gen.inc）與 C 路共用層
//  FileRW/_EditList.cpp／_EditPage.cpp（沒有 TU-local 替身）。呼叫的是產生檔取代段呼叫的那幾支函式本體：
//    W906_HeaterMixLoaderSystemSet（golden LoaderSystemSet :262-278 的取代＝開頁）、W906_HeaterMixSave（SaveSystemSet :779-794）、
//    W906_HeaterMixTypeClick（rgHeaterTypeClick :1529-1538，經 PageDesc::beforeApply 重播）、W906_HeaterMixSaveCheck（SaveFlow 呼叫）；
//    以及 PageDesc 的 beforeApply／extraJson／saveFlow（只走「檢查擋下」那條路 —— 不跑 golden FormShow／SaveSystemSet 全段：
//    那兩段會讀寫 D:\GPIB9045\system\general.ini 與 D:\RS232Standard\System\Setup.ini）。
//
//  只寫 %TEMP%\ht9045_hsys_heater_mix_test\（asGeneralPath 轉向＋OpenGeneralIniFile）。開跑前後比對
//  D:\HT9045\system\Gerneral.ini 與 D:\HT9045\config\config.ini 的內容，有變就失敗。
//
//  涵蓋：V899 檔（沒有 HeaterInsOpt_）、V912 71 鍵全 -9999 檔（Steven01 這台的形狀：USE_16_HEATER=2、HEATER_CTRL_TYPE=2、沒有模式鍵）、
//    V912 混搭檔（推斷「不同」、缺鍵＝3）、新檔來回（「不同」模式＋站號，存兩次位元組不變）、「相同」模式 Index≠其他與 D-6a、
//    EJ1N 鎖 Index、開頁不寫檔、存檔逐鍵比對、D-7a 範圍、D-8a 重複擋下（SaveFlow 什麼都不寫）、Heater Type 重播不寫檔、extra JSON。
//
//  //AI(W906-FRW-E029) 20261002 [W906]：E029／Q72（Steven 1002「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」
//    「這些全部都要可以選,所以數量也是少了」「使用 EJ1N 或是 DTM 應該是要設定兩個變數」）——71 個通道全部有列、7 個廠牌、
//    5／6 來回、Index 區 EJ1N／DTM 與 [System] USE_16_HEATER 兩個方向連動（同一次存檔兩個鍵都寫）、兩邊都改又對不上擋下、
//    EJ1N／DTM 的台號／站＋CH（HeaterInsCh_）、D-8a 分匯流排（溫控 COM 埠／EJ1N／DTM）、HEATER_CTRL_TYPE 不寫 5／6、
//    DTME08_Control.ini 唯讀（W906_DTME08INI_PATH 轉到暫存）。開頁時頁面的 USE_16_HEATER＝rgHeater 替身（golden :260 讀的檔案值），
//    所以每個情境都用 SetU16 同時設全域與替身。
//
//  另有手動模式（ctest 不跑）：test_hsys_heater_mix --example <Gerneral.ini> [USE_16_HEATER]
//    把指定的檔（只讀）複製到暫存資料夾，照開頁＋什麼都不改直接存檔跑一次，印出逐鍵差異 —— 給交件報告用，不碰原檔。
// =============================================================================
#include "FileRW/HSys_Heater.h"
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"

#include <direct.h>
#include <cstdio>
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_HSys_Boot();

//AI(W906-FRW-E029) 20261002: _putenv（W906_DTME08INI_PATH）——MinGW.org 6.3 嚴格 C++17 不宣告，同 tests/test_agv_e84.cpp:143-163 的寫法
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

// 連結用（不是受測碼）：cprod.cpp（ht9045_globals）會叫 FileRW_IniConfig_ChangeCBListProperty（真的那支在 FileRW/IniConfig.cpp，
//   本測試沒編它）。不在這裡給的話，連結器會去抽 ht9045_globals 的 FileRW/_fallback.cpp 成員，而那個成員同時定義了
//   FileRW_ProxyChecked —— 跟本測試直接編進來的真 FileRW/_EditList.cpp:448 撞名（multiple definition）。
//   內容跟 FileRW/_fallback.cpp 一字不差（＝其他非 wb_serve 程式拿到的行為：IniConfig 沒開機時什麼都不做）；受測的 HSys 程式不呼叫它。
void FileRW_IniConfig_ChangeCBListProperty() {}

using filerw::EL;

namespace {

int g_fail = 0;
int g_pass = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL: %s\n", what.c_str());
}
void CheckEq(const std::string& got, const std::string& want, const std::string& what)
{
    if (got == want) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL: %s\n  got : %s\n  want: %s\n", what.c_str(), got.c_str(), want.c_str());
}
void CheckEqI(int got, int want, const std::string& what)
{
    if (got == want) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL: %s: got %d want %d\n", what.c_str(), got, want);
}

bool ReadFile(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
void WriteFile(const std::string& p, const std::string& s)
{
    std::ofstream f(p.c_str(), std::ios::binary | std::ios::trunc);
    f << s;
}

std::string g_dir;
std::string ScratchDir()
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    std::string d = std::string(t ? t : ".") + "\\" + W906_TestTmpName("ht9045_hsys_heater_mix_test");   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
    _mkdir(d.c_str());
    return d + "\\";
}

// 把 Gerneral.ini 轉到暫存檔（寫入走 INIFileGeneral，讀走 CheckIniData／ReadIniData(asGeneralPath)）
void UseFile(const std::string& name, const std::string& content)
{
    const std::string p = g_dir + name;
    WriteFile(p, content);
    asGeneralPath = AnsiString(p.c_str());
    OpenGeneralIniFile();
}
std::string Cur()
{
    std::string s;
    ReadFile(asGeneralPath.c_str(), &s);
    return s;
}

std::string Name(int ti)            // HeaterInsOpt_<通道> 去掉前綴
{
    std::string s = g_tHeaterInsInfo[ti].GetSaveName().c_str();
    return s.substr(std::strlen("HeaterInsOpt_"));
}
std::string I2S(int v) { char b[32]; std::snprintf(b, sizeof b, "%d", v); return b; }   // MinGW 6.3：沒有 std::to_string
std::string L(const std::string& s) { return s + "\r\n"; }
std::string KV(const std::string& k, int v) { return L(k + "=" + I2S(v)); }

// [TempCtrl] 71 鍵（依序），值由 f(ti) 給；skip(ti)=true 的不放
template <class F, class S>
std::string Keys71(F f, S skip)
{
    std::string s;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (skip(ti)) continue;
        s += KV(g_tHeaterInsInfo[ti].GetSaveName().c_str(), f(ti));
    }
    return s;
}

TComboBox*   Cb(const std::string& n)  { return EL<TComboBox>("THandlerSystem", n.c_str()); }
TEdit*       Ed(const std::string& n)  { return EL<TEdit>("THandlerSystem", n.c_str()); }
TRadioGroup* Rg(const std::string& n)  { return EL<TRadioGroup>("THandlerSystem", n.c_str()); }
TComboBox*   ChCb(int ti)              { return Cb("cbHeaterInsOpt_" + Name(ti)); }
TEdit*       ChEd(int ti)              { return Ed("edHeaterInsAddr_" + Name(ti)); }
void SetCb(TComboBox* c, int i)        { filerw::ELComboIndex(c, i); }
TEdit*       ChCh(int ti)              { return Ed("edHeaterInsCh_" + Name(ti)); }   //AI(W906-FRW-E029) 20261002：EJ1N／DTM 的 CH
// //AI(W906-FRW-E029) 20261002：golden LoaderSystemSet :260 開頁時先把 [System] USE_16_HEATER 讀進 rgHeater，Heater 分頁照它；
//   測試不跑 golden FormShow ⇒ 全域與替身一起設（溫控底層 HeaterInsOpt_Read 用全域，開頁用替身）
void SetU16(int u)                     { USE_16_HEATER = u; Rg("rgHeater")->ItemIndex = u; }
// //AI(W906-FRW-E029) 20261002：Dut Heater Count（rgUse4DUT＝[System] SocketBasedAdd4Temp，golden eDut1ea 0／eDut4ea 1／eDut2ea 2）；
//   Steven 1002 17:1x「socket跟 Dut 1~4也是互斥的」——存檔時照頁面上的 rgUse4DUT（替身）
void SetDut(int d)                     { iSocketBaseTempCount = d; Rg("rgUse4DUT")->ItemIndex = d; }
// 這台有裝（測試自己照 Steven 1002 17:1x 的規則算，不呼叫受測碼）：Head1～4＝4 Heaters；Index 區照組數；Socket＝1 EA；DUT1～2＝2／4 EA；
//   DUT3～4＝4 EA；其他＝開機 golden 可見條件（main() 的開機設定：HotPlate1/2、Shuttle1/2、Chamber 看得到，CCD／熱風槍／2D／LB／ESD／ATC 熱風都關）
bool ActiveNow(int ti, int u, int dut)
{
    const bool b32 = (u == eht32HeaterEJ1N || u == eht32HeaterKT4H || u == eht32HeaterDTME08), b0 = (u == eht4Heater);
    if (ti >= tcHead1 && ti <= tcHead4) return b0;
    if (ti >= tcAa1 && ti <= tcBd2) return !b0;
    if (ti >= tcAe1 && ti <= tcBh2) return b32;
    if (ti == tcSocket) return dut == eDut1ea;
    if (ti == tcDUT1 || ti == tcDUT2) return dut == eDut2ea || dut == eDut4ea;
    if (ti == tcDUT3 || ti == tcDUT4) return dut == eDut4ea;
    return ti == tcHotPlate1 || ti == tcHotPlate2 || ti == tcShuttle1 || ti == tcShuttle2 || ti == tcChamber;
}

// 開頁（golden LoaderSystemSet 的溫控那一段）＋ extra JSON：兩者都不可以寫檔
void OpenNoWrite(const std::string& what)
{
    const std::string before = Cur();
    W906_HeaterMixLoaderSystemSet();
    const filerw::PageDesc* d = filerw::FindPage("HSys");
    if (d && d->extraJson) (void)d->extraJson();
    CheckEq(Cur() == before ? "same" : "CHANGED", "same", what + ": page open (LoaderSystemSet heater part + extra JSON) writes nothing");
}
bool SaveNow()
{
    filerw::SessionBegin("");
    if (!W906_HeaterMixSaveCheck()) return false;
    W906_HeaterMixSave();
    return true;
}
bool SessionHas(const std::string& text) { return filerw::SessionJson().find(text) != std::string::npos; }
//AI(W906-FRW-E029) 20261002：位元組不同時印出第一個不同的行（除錯用，結果照 CheckEq）
std::string SameOrDiff(const std::string& a, const std::string& b)
{
    if (a == b) return "same";
    std::istringstream ia(a), ib(b);
    std::string la, lb;
    int n = 0;
    while (true) {
        const bool ga = static_cast<bool>(std::getline(ia, la)), gb = static_cast<bool>(std::getline(ib, lb));
        ++n;
        if (!ga && !gb) break;
        if (!ga || !gb || la != lb) return "CHANGED at line " + I2S(n) + ": [" + (ga ? la : "<eof>") + "] -> [" + (gb ? lb : "<eof>") + "]";
    }
    return "CHANGED";
}

const std::string kHint = "目前溫控只看 HEATER_CTRL_TYPE";

//AI(W906-FRW-E029) 20261002：golden 的 Index 區接線（golden V912 cmydef.cpp:111-117 iTempCode；
//   D:\HT9045\.claude\skills\ht9045-temperature\references\main-screen-display.md §3 表）——測試自己抄一份，不拿受測碼的 iTempCode 比自己
const int kGoldenTempCode[32] = { tcAa1, tcBa1, tcAb1, tcBb1, tcAc1, tcBc1, tcAd1, tcBd1,
                                  tcAa2, tcBa2, tcAb2, tcBb2, tcAc2, tcBc2, tcAd2, tcBd2,
                                  tcAe1, tcBe1, tcAf1, tcBf1, tcAg1, tcBg1, tcAh1, tcBh1,
                                  tcAe2, tcBe2, tcAf2, tcBf2, tcAg2, tcBg2, tcAh2, tcBh2 };
int  GoldenPos(int ti)                     { for (int p = 0; p < 32; ++p) if (kGoldenTempCode[p] == ti) return p; return -1; }
void GoldenEj1n(int ti, int* u, int* c)    { const int p = GoldenPos(ti); *u = p / 4 + 1; *c = p % 4 + 1; }   // EJ1N 第 p/4+1 台 CH p%4+1
void GoldenDtm(int ti, int* s, int* c)     { const int p = GoldenPos(ti); *s = p / 8;     *c = p % 8 + 1; }   // DTM 站 p/8 CH p%8+1
bool InArea(int ti, int u)                 // Index Heater Counts 管的 Index 區（Steven 1002）：Aa1～Bd2，32 組再加 Ae1～Bh2
{
    const bool b32 = (u == eht32HeaterEJ1N || u == eht32HeaterKT4H || u == eht32HeaterDTME08);
    return (ti >= tcAa1 && ti <= tcBd2) || (b32 && ti >= tcAe1 && ti <= tcBh2);
}
bool IsZone(int ti)                        { return (ti >= tcAa1 && ti <= tcBd2) || (ti >= tcAe1 && ti <= tcBh2); }

// ---------------------------------------------------------------------------
void CaseV899()
{
    // V899 升上來的檔：0 個 HeaterInsOpt_、沒有模式鍵 → 相同，Index＝其他＝HEATER_CTRL_TYPE（Q34 ③ 第 1 條）
    SetU16(eht16HeaterEJ1N);
    const std::string head = L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("COM_PORT=COM12") +
                             L("HEATER_CTRL_TYPE=1") + L("AMBIENT_TEMP_CHECK1=1");
    const std::string tail = L("[NUMBER_PANEL]") + L("NUMBER_PANEL_DELAY=1");
    UseFile("v899.ini", head + tail);
    OpenNoWrite("V899");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_SAME, "V899: mode inferred SAME (D-1b: no HeaterInsOpt_ at all)");
    //AI(W906-FRW-E029) 20261002：USE_16_HEATER=2（16 EJ1N）⇒ Index＝Omron EJ1N（以前＝HEATER_CTRL_TYPE、下拉停用）；Index 區 Aa1～Bd2＝5，
    //   Head1～4 與其他通道＝其他位置（HEATER_CTRL_TYPE＝KT4H）
    CheckEqI(g_iHeaterInsIndexOpt, W906_HEATER_INS_EJ1N, "V899 (USE_16_HEATER=EJ1N): Index = Omron EJ1N (E029: Index Heater Counts wins on read)");
    CheckEqI(g_iHeaterInsOtherOpt, 1, "V899: Other = HEATER_CTRL_TYPE");
    bool all1 = true;
    for (int ti = 0; ti < eHeaterType_Count; ++ti)
        all1 = all1 && g_tHeaterInsInfo[ti].GetHeaterInsOpt() == ((ti >= tcAa1 && ti <= tcBd2) ? W906_HEATER_INS_EJ1N : 1);
    Check(all1, "V899: Aa1..Bd2 = EJ1N, the other 55 channels (incl. Head1-4) = KT4H in memory");
    CheckEqI(Rg("rgHeaterType")->ItemIndex, 1, "V899: rgHeaterType shows KT4H");
    CheckEqI(Rg("rgHeaterInsMode")->ItemIndex, 0, "V899: mode proxy = same");
    Check(Cb("cbHeaterInsIndexOpt")->Enabled && Cb("cbHeaterInsIndexOpt")->ItemIndex == W906_HEATER_INS_EJ1N,
          "V899 (USE_16_HEATER=EJ1N): Index drop-down enabled and shows Omron EJ1N (E029, was disabled)");
    // 什麼都不改直接存檔：段尾一次加 3＋71 行，其他不動
    Check(SaveNow(), "V899: unchanged save passes the check");
    const std::string want = L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("COM_PORT=COM12") +
                             L("HEATER_CTRL_TYPE=1") + L("AMBIENT_TEMP_CHECK1=1") +
                             KV("HeaterInsMode", 0) + KV("HeaterInsIndexOpt", W906_HEATER_INS_EJ1N) + KV("HeaterInsOtherOpt", 1) +
                             Keys71([](int ti) { return (ti >= tcAa1 && ti <= tcBd2) ? W906_HEATER_INS_EJ1N : 1; }, [](int) { return false; }) + tail;
    CheckEq(Cur(), want, "V899: unchanged save = 3 mode keys + 71 HeaterInsOpt_ (Aa1..Bd2 = 5 EJ1N, rest 1) appended at the end of [TempCtrl], "
                         "HEATER_CTRL_TYPE stays 1, USE_16_HEATER stays 2 (both variables, E029)");
    Check(!SessionHas(kHint), "V899: no D-9a hint (same, Index==Other, no station)");
    // 存完再開頁仍不寫、再存一次位元組不變
    const std::string after = Cur();
    OpenNoWrite("V899 after save");
    Check(SaveNow(), "V899: second save");
    CheckEq(Cur() == after ? "same" : "CHANGED", "same", "V899: second unchanged save is byte-identical");
}

// ---------------------------------------------------------------------------
std::string Steven01Like(const std::string& opts)
{
    return L("[Version]") + L("Ver=V3.33.906.0") + L("[System]") + L("USE_16_HEATER=2") +   //AI(W906-FRW-E029) 20261002：[System]（W906_HeaterMixSave 也寫 USE_16_HEATER）
           L("[TempCtrl]") + L("   \tUSE_NEW_TEMPCTRL_FUNCTION=0") + L("COM_PORT=COM12") + L("AMBIENT_TEMP_CHECK33=1") +
           L("HEATER_CTRL_TYPE=2") + L("AMBIENT_TEMP_CHECK71=1") + opts + L("  ") +
           L("[NUMBER_PANEL]") + L("   \tNUMBER_PANEL_DELAY=1");
}
void CaseV912AllMinus9999()
{
    // Steven01 這台的形狀：USE_16_HEATER=2（EJ1N）、HEATER_CTRL_TYPE=2（E5DC）、71 鍵全 -9999、沒有模式鍵（Q34 例子第 1 條）
    SetU16(eht16HeaterEJ1N);
    const std::string before = Steven01Like(Keys71([](int) { return INVALID_INT_VAL_NEG; }, [](int) { return false; }));
    UseFile("v912_all9999.ini", before);
    OpenNoWrite("V912 all -9999");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_SAME, "V912 all -9999: mode SAME (D-1b after D-5a)");
    CheckEqI(g_iHeaterInsIndexOpt, W906_HEATER_INS_EJ1N, "V912 all -9999: Index = Omron EJ1N (USE_16_HEATER=2, E029)");
    CheckEqI(g_iHeaterInsOtherOpt, 2, "V912 all -9999: Other = E5DC");
    CheckEqI(g_tHeaterInsInfo[tcAa1].GetHeaterInsOpt(), W906_HEATER_INS_EJ1N, "V912 all -9999: Aa1 = EJ1N (E029; was E5DC by D-5a)");
    CheckEqI(g_tHeaterInsInfo[tcAe1].GetHeaterInsOpt(), 2, "V912 all -9999: Ae1 (outside the 16-heater area) = Other = E5DC");
    CheckEqI(g_tHeaterInsInfo[tcHead1].GetHeaterInsOpt(), 2, "V912 all -9999: Head1 stays on the COM port = Other = E5DC");
    CheckEqI(ChCb(tcHotPlate1)->ItemIndex, 2, "V912 all -9999: HotPlate1 drop-down = E5DC");
    Check(Cb("cbHeaterInsIndexOpt")->Enabled, "V912 all -9999: Index drop-down enabled (E029)");
    Check(SaveNow(), "V912 all -9999: unchanged save passes the check");
    const std::string want = Steven01Like(Keys71([](int ti) { return (ti >= tcAa1 && ti <= tcBd2) ? W906_HEATER_INS_EJ1N : 2; },
                                                 [](int) { return false; }) +
                                          KV("HeaterInsMode", 0) + KV("HeaterInsIndexOpt", W906_HEATER_INS_EJ1N) + KV("HeaterInsOtherOpt", 2));
    CheckEq(Cur(), want, "V912 all -9999: unchanged save = 71 keys -9999 in place (Aa1..Bd2 -> 5 EJ1N, rest -> 2; D-4a, E029) + "
                         "3 new keys after HeaterInsOpt_LBDown, HEATER_CTRL_TYPE stays 2, USE_16_HEATER stays 2, other lines untouched");
}

// ---------------------------------------------------------------------------
void CaseV912Mixed()
{
    // V912 寫過、各通道真的不同：HotPlate1=KT4H、Shuttle1=DTK4848、其餘 -9999（→E5DC）、CCD_2 的鍵被刪掉 → 推斷「不同」，
    //   CCD_2 缺鍵＝3（Q15）；站號用預設（Q34 ③ 第 3 條、例子「Q15 的缺鍵＝3」）
    SetU16(eht16HeaterEJ1N);
    auto val = [](int ti) { return ti == tcHotPlate1 ? 1 : ti == tcShuttle1 ? 4 : INVALID_INT_VAL_NEG; };
    auto skipCcd2 = [](int ti) { return ti == tcCCD_2; };
    const std::string before = Steven01Like(Keys71(val, skipCcd2));
    UseFile("v912_mixed.ini", before);
    OpenNoWrite("V912 mixed");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_DIFF, "V912 mixed: mode inferred DIFF (D-1b)");
    CheckEqI(g_tHeaterInsInfo[tcHotPlate1].GetHeaterInsOpt(), 1, "V912 mixed: HotPlate1 = KT4H");
    CheckEqI(g_tHeaterInsInfo[tcShuttle1].GetHeaterInsOpt(), 4, "V912 mixed: Shuttle1 = DTK4848");
    CheckEqI(g_tHeaterInsInfo[tcHotPlate2].GetHeaterInsOpt(), 2, "V912 mixed: HotPlate2 -9999 -> HEATER_CTRL_TYPE (D-5a)");
    CheckEqI(g_tHeaterInsInfo[tcCCD_2].GetHeaterInsOpt(), NoHeater, "V912 mixed: CCD_2 missing key -> 3 No Heater (Q15)");
    CheckEqI(g_tHeaterInsInfo[tcBd2].GetHeaterInsOpt(), W906_HEATER_INS_EJ1N, "V912 mixed: Bd2 (16 EJ1N area) = EJ1N (E029)");
    //AI(W906-FRW-E029) 20261002：⛔ 以前 Index＝HEATER_CTRL_TYPE（2）；USE_16_HEATER=2 ⇒ Index＝Omron EJ1N（讀檔以 Index Heater Counts 為準）
    CheckEqI(g_iHeaterInsIndexOpt, W906_HEATER_INS_EJ1N, "V912 mixed: Index = Omron EJ1N (USE_16_HEATER=2, E029)");
    CheckEqI(W906_HeaterStationIdx(tcShuttle1), tcShuttle1, "V912 mixed: Shuttle1 station index = default (Addr)");
    CheckEqI(Rg("rgHeaterType")->ItemIndex, 2, "V912 mixed: rgHeaterType shows HEATER_CTRL_TYPE (not all the same, golden :271)");
    Check(SaveNow(), "V912 mixed: unchanged save passes the check");
    // 裝了的通道（開機可見條件：HotPlate1/2、Shuttle1/2、Head1～4、Socket、Chamber）寫預設站號；
    //AI(W906-FRW-E029) 20261002：EJ1N 的 Index 區 Aa1～Bd2 也裝了（EJ1N 匯流排）→ 台號＋CH 照 golden 接線（iTempCode）寫出來
    std::string addr;
    //AI(W906-FRW-E029) 20261002：⛔ Head1～4 拿掉——Steven 1002 17:1x Head1～4 與 Ax／Bx 互斥：16 組（USE_16_HEATER=2）時 Head1～4 沒裝
    const int listed[] = { tcHotPlate1, tcHotPlate2, tcShuttle1, tcShuttle2, tcSocket, tcChamber };
    for (int ti : listed) addr += KV("HeaterInsAddr_" + Name(ti), ti + 1);
    for (int ti = tcAa1; ti <= tcBd2; ++ti) {
        int u = 0, c = 0;
        GoldenEj1n(ti, &u, &c);
        addr += KV("HeaterInsAddr_" + Name(ti), u) + KV("HeaterInsCh_" + Name(ti), c);
    }
    const std::string want = Steven01Like(Keys71([](int ti) { return ti == tcHotPlate1 ? 1 : ti == tcShuttle1 ? 4 :
                                                              (ti >= tcAa1 && ti <= tcBd2) ? W906_HEATER_INS_EJ1N : 2; }, skipCcd2) +
                                          KV("HeaterInsMode", 1) + KV("HeaterInsIndexOpt", W906_HEATER_INS_EJ1N) + KV("HeaterInsOtherOpt", 2) +
                                          KV("HeaterInsOpt_CCD_2", 3) + addr);
    CheckEq(Cur(), want, "V912 mixed: unchanged save = mode 1, -9999 -> 2 (Aa1..Bd2 -> 5 EJ1N), CCD_2=3 appended, installed channels' "
                         "default stations (no Head1-4 at 16 heaters; EJ1N area: golden unit + CH), HEATER_CTRL_TYPE stays 2 (D-6a: most-used COM brand)");
    Check(SessionHas(kHint), "V912 mixed: D-9a hint in the save ack (mode different)");
    const std::string after = Cur();
    OpenNoWrite("V912 mixed after save");
    Check(SaveNow(), "V912 mixed: second save");
    CheckEq(Cur() == after ? "same" : "CHANGED", "same", "V912 mixed: second unchanged save is byte-identical");
}

// ---------------------------------------------------------------------------
void CaseNewFileRoundTrip()
{
    // 「不同」模式＋站號（Q34 例子第 3 條）：Shuttle1 選 DTK4848、站號 21；HotPlate1 選 TC401（站號空白＝第 1 台）；Aa1 選 E5DC
    SetU16(eht16Heater);               // 16 Heaters（KT4H 版）：Index Aa1～Bd2 走溫控 COM 埠 → 列出
    const std::string head = L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("COM_PORT=COM2") + L("HEATER_CTRL_TYPE=1");
    UseFile("newfile.ini", head);
    OpenNoWrite("new file");
    Check(Cb("cbHeaterInsIndexOpt")->Enabled, "new file (USE_16_HEATER=1): Index drop-down enabled");
    Check(W906_HeaterInsListed(tcAa1) && W906_HeaterInsListed(tcBd2) && !W906_HeaterInsListed(tcAe1),
          "D-3a: 16 Heaters lists Aa1..Bd2, not Ae1..Bh2");
    //AI(W906-FRW-E029) 20261002：⛔ 以前「Ae1 隱藏」；Steven 1002「這些全部都要可以選」⇒ 71 個替身全部看得到（存檔照收頁面值）
    Check(ChCb(tcAa1)->Visible && ChCb(tcAe1)->Visible && ChCb(tcLBDown)->Visible && ChCb(tcCCD)->Visible,
          "E029: every channel's combo proxy is visible (Aa1, Ae1, LBDown, golden-hidden CCD)");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_DIFF;
    SetCb(ChCb(tcShuttle1), 4);  ChEd(tcShuttle1)->Text = "21";
    SetCb(ChCb(tcHotPlate1), 0); ChEd(tcHotPlate1)->Text = "";
    SetCb(ChCb(tcAa1), 2);       ChEd(tcAa1)->Text = " ";
    //AI(W906-FRW-S166) 20260928 [W906] R113：Q34 例子第 3 條的「Shuttle1 DTK4848 站號 21」在 16 Heaters 會跟 Ab2（KT4H，預設站號 20+1）
    //   撞站號 —— 不同廠牌、同一個溫控 COM 埠（KT4H 與 DTK4848 都是 Modbus ASCII）。R113 之後擋下；改用沒人用的 50 繼續來回測試。
    {
        const std::string before = Cur();
        filerw::SessionBegin("");
        Check(!W906_HeaterMixSaveCheck(), "R113: new file Shuttle1 DTK4848 station 21 vs Ab2 KT4H default 21 is refused");
        Check(SessionHas("站號 21 被不同廠牌共用：Shuttle1（DTK4848）、Ab2（Panasonic KT4H）"),
              "R113: refusal lists Shuttle1 and Ab2 with brands (channel order)");
        CheckEq(Cur() == before ? "same" : "CHANGED", "same", "R113: refused check writes nothing");
    }
    ChEd(tcShuttle1)->Text = "50";
    Check(SaveNow(), "new file: save passes the check");
    std::string want = head;
    want += KV("HeaterInsMode", 1) + KV("HeaterInsIndexOpt", 1) + KV("HeaterInsOtherOpt", 1);
    want += Keys71([](int ti) { return ti == tcShuttle1 ? 4 : ti == tcHotPlate1 ? 0 : ti == tcAa1 ? 2 : 1; }, [](int) { return false; });
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        if (!ActiveNow(ti, eht16Heater, eDut1ea)) continue;          //AI(W906-FRW-E029) 20261002：以前 W906_HeaterInsListed（含 Head1～4）
        const int st = ti == tcShuttle1 ? 50 /* R113：原本 21 */ : ti == tcHotPlate1 ? 1 /* TC401 第 0/4+1 台 */ : ti + 1;
        want += KV("HeaterInsAddr_" + Name(ti), st);
    }
    CheckEq(Cur(), want, "new file: different mode save writes mode 1, actual brands, station keys for the listed channels "
                         "(blank = default; TC401 = unit), HEATER_CTRL_TYPE=1 (most-used among listed)");
    Check(SessionHas(kHint), "new file: D-9a hint in the save ack");
    const std::string saved = Cur();
    OpenNoWrite("new file reopen");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_DIFF, "new file reopen: mode DIFF");
    CheckEqI(g_tHeaterInsInfo[tcShuttle1].GetHeaterInsOpt(), 4, "new file reopen: Shuttle1 = DTK4848");
    CheckEqI(g_iHeaterInsAddr[tcShuttle1], 50, "new file reopen: Shuttle1 station 50");   //AI(W906-FRW-S166) 20260928 [W906] R113：21 → 50（見上）
    CheckEqI(W906_HeaterStationIdx(tcShuttle1), 49, "W906_HeaterStationIdx(Shuttle1) = 50-1");
    CheckEqI(W906_HeaterStationIdx(tcHotPlate1), 0, "W906_HeaterStationIdx(HotPlate1, TC401 default) = unit 1 - 1 (=Addr/4)");
    CheckEqI(W906_HeaterStationIdx(tcHead2), tcHead2, "W906_HeaterStationIdx(Head2, KT4H default) = Addr");
    CheckEq(ChEd(tcShuttle1)->Text.c_str(), "50", "new file reopen: Shuttle1 station box shows 50");
    CheckEq(ChEd(tcHotPlate1)->Text.c_str(), "", "new file reopen: HotPlate1 station box blank (= default)");
    CheckEqI(ChCb(tcAa1)->ItemIndex, 2, "new file reopen: Aa1 drop-down = E5DC");
    Check(SaveNow(), "new file: second save");
    CheckEq(Cur() == saved ? "same" : "CHANGED", "same", "new file: reopen + unchanged save is byte-identical (round trip)");
}

// ---------------------------------------------------------------------------
std::string OptOf(const std::string& file, const std::string& key)
{
    const std::string k = "\r\n" + key + "=";
    const size_t p = file.find(k);
    if (p == std::string::npos) return "<missing>";
    const size_t b = p + k.size();
    return file.substr(b, file.find("\r\n", b) - b);
}
void CaseSameIndexOther()
{
    // 全機相同、Index≠其他（Q34 例子第 2 條）＋ D-6a 的三種情況
    SetU16(eht16Heater);
    UseFile("same_mix.ini", L("[TempCtrl]") + L("HEATER_CTRL_TYPE=2"));
    OpenNoWrite("same mix");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_SAME;
    SetCb(Cb("cbHeaterInsIndexOpt"), 1);
    SetCb(Cb("cbHeaterInsOtherOpt"), 2);
    Check(SaveNow(), "same mix: save passes");
    std::string f = Cur();
    CheckEq(OptOf(f, "HeaterInsMode") + "/" + OptOf(f, "HeaterInsIndexOpt") + "/" + OptOf(f, "HeaterInsOtherOpt"), "0/1/2",
            "same mix: mode/Index/Other keys");
    CheckEq(OptOf(f, "HeaterInsOpt_Head1") + OptOf(f, "HeaterInsOpt_Aa1") + OptOf(f, "HeaterInsOpt_Bd2") + OptOf(f, "HeaterInsOpt_Ae1") +
            OptOf(f, "HeaterInsOpt_Bh2"), "11111", "same mix: Index group (Head1-4 + 32 zones, D-2a) = KT4H");
    CheckEq(OptOf(f, "HeaterInsOpt_HotPlate1") + OptOf(f, "HeaterInsOpt_Socket") + OptOf(f, "HeaterInsOpt_DUT1") +
            OptOf(f, "HeaterInsOpt_IndexESD") + OptOf(f, "HeaterInsOpt_Door1"), "22222",
            "same mix: other positions (incl. Socket, DUT1, IndexESD, Door1 per D-2=A) = E5DC");
    CheckEq(OptOf(f, "HEATER_CTRL_TYPE"), "2", "D-6a same: HEATER_CTRL_TYPE = Other");
    Check(f.find("HeaterInsAddr_") == std::string::npos, "same mix: no station keys in same mode (D-7a)");
    Check(SessionHas(kHint), "same mix: D-9a hint (Index != Other)");
    SetCb(Cb("cbHeaterInsOtherOpt"), 3);
    Check(SaveNow(), "same mix Other=NoHeater: save passes");
    CheckEq(OptOf(Cur(), "HEATER_CTRL_TYPE"), "1", "D-6a same: Other = No Heater -> HEATER_CTRL_TYPE = Index (machine not treated as no-heater)");
    SetCb(Cb("cbHeaterInsIndexOpt"), 3);
    Check(SaveNow(), "same mix both NoHeater: save passes");
    CheckEq(OptOf(Cur(), "HEATER_CTRL_TYPE"), "3", "D-6a same: all No Heater -> 3");

    // EJ1N：⛔ 20261002 更正（E029）：以前「Index 下拉停用 → Index 跟著其他位置」；現在讀檔以 USE_16_HEATER 為準 → Index＝EJ1N，
    //   Head1～4 照 golden 留在溫控 COM 埠＝其他位置的廠牌
    SetU16(eht16HeaterEJ1N);
    UseFile("same_locked.ini", L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=2") + KV("HeaterInsMode", 0) +
                               KV("HeaterInsIndexOpt", 4) + KV("HeaterInsOtherOpt", 2));
    OpenNoWrite("same locked");
    CheckEqI(g_iHeaterInsIndexOpt, W906_HEATER_INS_EJ1N, "EJ1N: Index = Omron EJ1N on read (USE_16_HEATER wins, E029)");
    CheckEqI(g_tHeaterInsInfo[tcHead1].GetHeaterInsOpt(), 2, "EJ1N: Head1 = Other's brand (stays on the COM port)");
    CheckEqI(g_tHeaterInsInfo[tcAa1].GetHeaterInsOpt(), W906_HEATER_INS_EJ1N, "EJ1N: Aa1 = EJ1N");
    SetCb(Cb("cbHeaterInsOtherOpt"), 1);
    Check(SaveNow(), "EJ1N: save passes");
    CheckEq(OptOf(Cur(), "HeaterInsIndexOpt") + OptOf(Cur(), "HeaterInsOpt_Head1") + OptOf(Cur(), "HeaterInsOpt_Aa1") +
            OptOf(Cur(), "USE_16_HEATER"), "5152", "EJ1N: save writes Index = 5, Head1 = Other, Aa1 = 5, USE_16_HEATER = 2");
}

// ---------------------------------------------------------------------------
void CaseRefused()
{
    // D-8a：同一個溫控 COM 埠同廠牌同站號 → SaveFlow 擋下，整頁什麼都不寫；D-7a 範圍
    //AI(W906-FRW-E029) 20261002：4 Heaters（以前 16 EJ1N）——Steven 1002 17:1x Head1～4 只在 4 Heaters 時有裝，下面的 Head1 才在匯流排上
    SetU16(eht4Heater);
    const std::string base = L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1");
    UseFile("refuse.ini", base);
    OpenNoWrite("refuse");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_DIFF;
    ChEd(tcShuttle1)->Text = "5";                      // Shuttle1 KT4H 站號 5 ＝ Head1 KT4H 的預設站號 5
    const filerw::PageDesc* d = filerw::FindPage("HSys");
    Check(d != nullptr, "PageDesc HSys registered");
    const bool oldModel = bHandlerModel;
    bHandlerModel = true;                              // golden 型號讀取成功（否則 SaveFlow 在 HS_ModelReadError 就停）
    filerw::SessionBegin("");                          // 沒帶 YES：就算檢查沒擋，golden SaveSystemSet 也答 NO、不寫檔
    if (d) d->saveFlow();
    bHandlerModel = oldModel;
    const std::string js = filerw::SessionJson();
    Check(js.find("Shuttle1") != std::string::npos && js.find("Head1") != std::string::npos &&
          js.find("都是 Panasonic KT4H、站號 5") != std::string::npos, "D-8a: refusal message names Shuttle1 and Head1");
    Check(filerw::ELMarked("W906_HeaterMixSaveCheck: refused (nothing written)"), "D-8a: SaveFlow refused before golden save");
    Check(!filerw::ELMarked("SaveBtnClick") && !filerw::ELMarked("SaveSystemSet"), "D-8a: golden SaveBtnClick/SaveSystemSet not run");
    CheckEq(Cur() == base ? "same" : "CHANGED", "same", "D-8a: nothing written");

    // TC401：同一台不同通道＝正常；同一台同一通道＝重複
    ChEd(tcShuttle1)->Text = "";
    SetCb(ChCb(tcHotPlate1), 0); SetCb(ChCb(tcHotPlate2), 0);   // 都是第 1 台（0/4、1/4），通道 0、1
    filerw::SessionBegin("");
    Check(W906_HeaterMixSaveCheck(), "D-8a: TC401 same unit, different channel is allowed");
    SetCb(ChCb(tcHead1), 0); ChEd(tcHead1)->Text = "1";          // Head1（序號 4，通道 0）指定第 1 台 → 跟 HotPlate1 同台同通道
    filerw::SessionBegin("");
    Check(!W906_HeaterMixSaveCheck(), "D-8a: TC401 same unit and channel is refused");
    Check(SessionHas("都是 TC401 第 1 台的通道 0"), "D-8a: TC401 message");
    SetCb(ChCb(tcHead1), 1); ChEd(tcHead1)->Text = "";

    struct { int ti, opt; const char* text; bool ok; const char* what; } r[] = {
        { tcShuttle2, 2, "100", false, "D-7a: E5DC station 100 refused (1-99)" },
        { tcShuttle2, 2, "99",  true,  "D-7a: E5DC station 99 allowed" },
        { tcShuttle2, 1, "248", false, "D-7a: KT4H station 248 refused (1-247)" },
        { tcShuttle2, 1, "247", true,  "D-7a: KT4H station 247 allowed" },
        { tcShuttle2, 1, "0",   false, "D-7a: station 0 refused" },
        { tcShuttle2, 4, "abc", false, "D-7a: non-numeric station refused" },
    };
    for (auto& x : r) {
        SetCb(ChCb(x.ti), x.opt); ChEd(x.ti)->Text = x.text;
        filerw::SessionBegin("");
        Check(W906_HeaterMixSaveCheck() == x.ok, x.what);
    }
    ChEd(tcShuttle2)->Text = "";
    SetCb(ChCb(tcSocket), -1);
    filerw::SessionBegin("");
    Check(!W906_HeaterMixSaveCheck(), "listed channel without a brand is refused");
    CheckEq(Cur() == base ? "same" : "CHANGED", "same", "checks alone write nothing");
}

// ---------------------------------------------------------------------------
//AI(W906-FRW-S166) 20260928 [W906] R113（Steven 20260928「不同廠牌共用同一個站號要不要擋。 要擋, 目前有遇到 E5DC + KT4H + DTK4848混用了」）：
//   同一個溫控 COM 埠（＝D-3a 列出的通道）上不同廠牌同站號也擋，所有廠牌都算（TC401 的站號＝第幾台）；同廠牌照 D-8a 原規則；
//   「全機相同」模式用預設站號一起檢查。沒列出的通道（EJ1N 的 Index 區走 COM_PORT_OMRON）與 No Heater 不算。
int CountOf(const std::string& s, const std::string& what)
{
    int n = 0;
    for (size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size())) ++n;
    return n;
}
bool CheckNow() { filerw::SessionBegin(""); return W906_HeaterMixSaveCheck(); }
void CaseCrossBrandR113()
{
    //AI(W906-FRW-E029) 20261002：4 Heaters（以前 16 EJ1N）——Steven 1002 17:1x Head1～4 只在 4 Heaters 時有裝
    SetU16(eht4Heater);                                // 有裝：HotPlate1/2、Shuttle1/2、Head1～4、Socket、Chamber
    const std::string base = L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1");
    UseFile("r113.ini", base);
    OpenNoWrite("R113");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_DIFF;

    // (a) R113 題目的例子：Shuttle1 KT4H 站號 5、Head1 DTK4848（預設站號 5）→ 以前可以存，現在擋
    ChEd(tcShuttle1)->Text = "5";
    SetCb(ChCb(tcHead1), 4);
    Check(!CheckNow(), "R113 (a): Shuttle1 KT4H 5 + Head1 DTK4848 default 5 is refused");
    const std::string ja = filerw::SessionJson();
    Check(ja.find("站號 5 被不同廠牌共用：Shuttle1（Panasonic KT4H）、Head1（DTK4848）") != std::string::npos,
          "R113 (a): message lists Shuttle1 (KT4H) and Head1 (DTK4848)");
    Check(ja.find("station 5 is shared by different brands on the same heater COM port: Shuttle1 (Panasonic KT4H), Head1 (DTK4848)") !=
          std::string::npos, "R113 (a): English message");
    CheckEqI(CountOf(ja, "被不同廠牌共用"), 1, "R113 (a): one message per station");
    Check(ja.find("都是 Panasonic KT4H、站號 5") == std::string::npos, "R113 (a): no same-brand (D-8a) message");
    Check(ja.find("「全機相同」模式用的是預設站號") == std::string::npos, "R113 (a): no same-mode hint in different mode");
    ChEd(tcShuttle1)->Text = "";
    SetCb(ChCb(tcHead1), 1);
    Check(CheckNow(), "R113: back to all KT4H default stations is allowed");

    // (b) Steven 遇到的混用：E5DC＋KT4H＋DTK4848 同站號 → 一則訊息列出三個通道；站號分開就可以
    SetCb(ChCb(tcHotPlate1), 2); ChEd(tcHotPlate1)->Text = "30";
    SetCb(ChCb(tcHotPlate2), 1); ChEd(tcHotPlate2)->Text = "30";
    SetCb(ChCb(tcShuttle1), 4);  ChEd(tcShuttle1)->Text = "30";
    Check(!CheckNow(), "R113 (b): E5DC + KT4H + DTK4848 on station 30 is refused");
    const std::string jb = filerw::SessionJson();
    Check(jb.find("站號 30 被不同廠牌共用：HotPlate1（Omron E5DC）、HotPlate2（Panasonic KT4H）、Shuttle1（DTK4848）") != std::string::npos,
          "R113 (b): message lists all three channels with brands");
    CheckEqI(CountOf(jb, "被不同廠牌共用"), 1, "R113 (b): one message for station 30");
    ChEd(tcHotPlate2)->Text = "31";
    ChEd(tcShuttle1)->Text = "32";
    Check(CheckNow(), "R113 (b): same three brands on stations 30 / 31 / 32 are allowed");
    ChEd(tcShuttle1)->Text = "31";
    Check(!CheckNow(), "R113 (b): KT4H 31 + DTK4848 31 (both Modbus ASCII) is refused");
    Check(SessionHas("站號 31 被不同廠牌共用：HotPlate2（Panasonic KT4H）、Shuttle1（DTK4848）"), "R113 (b): station 31 message");
    for (int ti : { tcHotPlate1, tcHotPlate2, tcShuttle1 }) { SetCb(ChCb(ti), 1); ChEd(ti)->Text = ""; }

    // (c) TC401 的站號＝第幾台：TC401 第 1 台與 E5DC 站號 1 也擋；TC401 同一台不同通道照舊可以
    SetCb(ChCb(tcHotPlate1), 0);                       // TC401 第 0/4+1 台，通道 0
    SetCb(ChCb(tcShuttle2), 2); ChEd(tcShuttle2)->Text = "1";
    Check(!CheckNow(), "R113 (c): TC401 unit 1 + E5DC station 1 is refused");
    Check(SessionHas("站號 1 被不同廠牌共用：HotPlate1（TC401 第 1 台通道 0）、Shuttle2（Omron E5DC）"), "R113 (c): TC401 unit shown");
    ChEd(tcShuttle2)->Text = "";                       // E5DC 預設站號 4
    SetCb(ChCb(tcHotPlate2), 0);                       // TC401 第 1 台，通道 1
    Check(CheckNow(), "R113 (c): TC401 unit 1 channels 0 / 1 (same brand, D-8a rule) + E5DC 4 are allowed");
    // 預設站號就會撞：Head1 改 TC401（序號 4 → 第 2 台）、HotPlate2 回 KT4H（預設站號 2）—— V912 逐通道混搭檔沒有站號，會在這裡被擋
    SetCb(ChCb(tcHead1), 0);
    SetCb(ChCb(tcHotPlate2), 1);
    Check(!CheckNow(), "R113 (c): default stations clash (HotPlate2 KT4H 2 vs Head1 TC401 unit 2) is refused");
    Check(SessionHas("站號 2 被不同廠牌共用：HotPlate2（Panasonic KT4H）、Head1（TC401 第 2 台通道 0）"), "R113 (c): default-station clash message");
    ChEd(tcHead1)->Text = "3";                          // 指定 TC401 第 3 台（通道仍是 4%4=0）→ 跟 Shuttle1 KT4H 預設 3 撞
    Check(!CheckNow(), "R113 (c): Head1 TC401 unit 3 vs Shuttle1 KT4H default 3 is refused");
    ChEd(tcHead1)->Text = "40";
    Check(CheckNow(), "R113 (c): Head1 TC401 unit 40 is allowed");
    for (int ti : { tcHotPlate1, tcHotPlate2, tcShuttle2, tcHead1 }) { SetCb(ChCb(ti), 1); ChEd(ti)->Text = ""; }

    // (d) 只看同一個溫控 COM 埠：EJ1N 的 Index 區（沒列出，走 COM_PORT_OMRON）與 No Heater 不算
    //AI(W906-FRW-E029) 20261002：⛔ 以前把 Aa1 改成 DTK4848 站號 5；E029 起 EJ1N 版的 Index 區一律是 EJ1N（Index Heater Counts 一個選項，
    //   改一個會被補回整區），所以改成：Aa1 是 EJ1N（EJ1N 匯流排）指定第 5 台 CH1、Shuttle2 No Heater「5」→ 跟 Head1 KT4H 站號 5 不衝突
    SetU16(eht16HeaterEJ1N);
    OpenNoWrite("R113 (d)");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_DIFF;
    Check(!W906_HeaterInsListed(tcAa1), "R113 (d): EJ1N -- Aa1 is not on the heater COM port (not listed)");
    CheckEqI(ChCb(tcAa1)->ItemIndex, W906_HEATER_INS_EJ1N, "R113 (d): Aa1 shows Omron EJ1N (USE_16_HEATER=2)");
    ChEd(tcAa1)->Text = "5"; ChCh(tcAa1)->Text = "1";   // EJ1N 第 5 台 CH1（Ae1 的 golden 接線；16 組時 Ae1 沒裝）
    ChEd(tcShuttle1)->Text = "5";                        // Shuttle1 KT4H 站號 5（溫控 COM 埠）
    SetCb(ChCb(tcShuttle2), 3); ChEd(tcShuttle2)->Text = "5";   // No Heater
    Check(CheckNow(), "R113 (d): Aa1 EJ1N unit 5 (EJ1N bus), No Heater Shuttle2 5 and Shuttle1 KT4H 5 (COM port) are allowed");
    ChEd(tcAa1)->Text = ""; ChCh(tcAa1)->Text = ""; ChEd(tcShuttle1)->Text = "";
    SetCb(ChCb(tcShuttle2), 1); ChEd(tcShuttle2)->Text = "";

    // (e) 「全機相同」模式：預設站號一起檢查（只有 Index≠其他、其中一邊是 TC401 才撞得到）
    SetU16(eht4Heater);                                // Index 下拉可用、Index 區不列出
    OpenNoWrite("R113 same");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_SAME;
    SetCb(Cb("cbHeaterInsIndexOpt"), 0);               // Index＝TC401：Head1～4＝第 2 台通道 0～3
    SetCb(Cb("cbHeaterInsOtherOpt"), 1);               // 其他＝KT4H：HotPlate2 預設站號 2
    Check(!CheckNow(), "R113 (e): same mode Index=TC401 / Other=KT4H is refused (default stations clash)");
    const std::string je = filerw::SessionJson();
    Check(je.find("站號 2 被不同廠牌共用：HotPlate2（Panasonic KT4H）、Head1（TC401 第 2 台通道 0）、Head2（TC401 第 2 台通道 1）、"
                  "Head3（TC401 第 2 台通道 2）、Head4（TC401 第 2 台通道 3）") != std::string::npos,
          "R113 (e): message lists HotPlate2 and Head1..Head4");
    Check(je.find("請改用「各溫控器不同」模式替通道指定站號") != std::string::npos, "R113 (e): same-mode hint");
    SetCb(Cb("cbHeaterInsIndexOpt"), 1);
    SetCb(Cb("cbHeaterInsOtherOpt"), 0);               // Index＝KT4H（站號 5～8）、其他＝TC401（第 1、3 台）→ 不撞
    Check(CheckNow(), "R113 (e): same mode Index=KT4H / Other=TC401 is allowed (no shared station)");
    SetCb(Cb("cbHeaterInsIndexOpt"), 0);
    Check(CheckNow(), "R113 (e): same mode Index=Other=TC401 is allowed");
    SetCb(Cb("cbHeaterInsIndexOpt"), 1);
    SetCb(Cb("cbHeaterInsOtherOpt"), 4);
    Check(CheckNow(), "R113 (e): same mode Index=KT4H / Other=DTK4848 is allowed (stations = channel + 1, all different)");
    CheckEq(Cur() == base ? "same" : "CHANGED", "same", "R113: checks alone write nothing");
    SetU16(eht16HeaterEJ1N);
}

// ---------------------------------------------------------------------------
void CaseHeaterTypeReplayAndExtra()
{
    // Heater Type 重播（BeforeApply → golden rgHeaterTypeClick → W906_HeaterMixTypeClick）：全機相同、Index＝其他＝點的廠牌，不寫檔（Q14＝B）
    SetU16(eht16Heater);
    const std::string base = L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 1);
    UseFile("replay.ini", base);
    OpenNoWrite("replay");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_DIFF, "replay: file mode DIFF");
    CheckEqI(g_tHeaterInsInfo[tcHotPlate1].GetHeaterInsOpt(), NoHeater, "replay: DIFF + missing key = 3 (Q15)");
    const filerw::PageDesc* d = filerw::FindPage("HSys");
    const bool oldModel = bHandlerModel;
    bHandlerModel = true;
    std::vector<std::string> handled;
    const int cur = Rg("rgHeaterType")->ItemIndex;
    const int pick = (cur == 4) ? 0 : 4;
    if (d && d->beforeApply)
        d->beforeApply("{\"rgHeaterType\":{\"itemIndex\":" + I2S(pick) + "}}", &handled);
    bHandlerModel = oldModel;
    Check(handled.size() == 1 && handled[0] == "rgHeaterType", "replay: rgHeaterType handled by beforeApply");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_SAME, "replay: mode -> SAME");
    CheckEqI(g_iHeaterInsIndexOpt * 10 + g_iHeaterInsOtherOpt, pick * 11, "replay: Index = Other = picked brand");
    bool all = true;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) all = all && g_tHeaterInsInfo[ti].GetHeaterInsOpt() == pick;
    Check(all, "replay: all 71 channels = picked brand in memory (no -9999, Q34 5)");
    CheckEqI(ChCb(tcAa1)->ItemIndex, pick, "replay: Index zone drop-down follows");
    CheckEqI(Rg("rgHeaterInsMode")->ItemIndex, 0, "replay: mode proxy = same");
    CheckEq(Cur() == base ? "same" : "CHANGED", "same", "replay: Heater Type click writes nothing (Q14=B)");

    // extra JSON（頁面照這份建畫面）
    W906_HeaterMixLoaderSystemSet();
    std::string ex = (d && d->extraJson) ? d->extraJson() : std::string();
    cJSON* root = cJSON_Parse(ex.c_str());
    const cJSON* h = root ? cJSON_GetObjectItemCaseSensitive(root, "heater") : nullptr;
    const cJSON* mix = h ? cJSON_GetObjectItemCaseSensitive(h, "mix") : nullptr;
    const cJSON* rows = mix ? cJSON_GetObjectItemCaseSensitive(mix, "rows") : nullptr;
    const cJSON* grp = mix ? cJSON_GetObjectItemCaseSensitive(mix, "indexGroup") : nullptr;
    const cJSON* hint = mix ? cJSON_GetObjectItemCaseSensitive(mix, "hint") : nullptr;
    //AI(W906-FRW-E029) 20261002：⛔ 以前 55 列（有下拉的 23 個＋Index 32 區）；Steven 1002「這些全部都要可以選,所以數量也是少了」⇒ 71 列
    CheckEqI(rows ? cJSON_GetArraySize(rows) : -1, eHeaterType_Count, "extra: 71 table rows (all of golden V912 eTempControll, E029)");
    const cJSON* opts = h ? cJSON_GetObjectItemCaseSensitive(h, "options") : nullptr;
    CheckEqI(opts ? cJSON_GetArraySize(opts) : -1, W906_HEATER_INS_OPT_COUNT, "extra: 7 brand options (E029)");
    Check(opts && cJSON_GetArraySize(opts) == 7 && std::string(cJSON_GetArrayItem(opts, 5)->valuestring) == "Omron EJ1N" &&
          std::string(cJSON_GetArrayItem(opts, 6)->valuestring) == "Delta DTM", "extra: options 5 = Omron EJ1N, 6 = Delta DTM");
    CheckEqI(grp ? cJSON_GetArraySize(grp) : -1, 36, "extra: Index group = 36 channels (D-2a)");
    Check(hint && cJSON_IsString(hint) && std::string(hint->valuestring).find(kHint) == 0, "extra: D-9a hint text");
    int listed = 0;
    for (const cJSON* r = rows ? rows->child : nullptr; r; r = r->next)
        if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(r, "listed"))) ++listed;
    CheckEqI(listed, 10 + 16, "extra: listed rows with 16 Heaters = 10 visible golden + Aa1..Bd2");
    if (root) cJSON_Delete(root);
}

// ---------------------------------------------------------------------------
//AI(W906-HSYS-EVT) 20261002: BeforeApply also replays golden rgTTLCardClick (906 HandlerSys.cpp:1246-1255) and rgRotateKit_TypeClick
//   (906 :1132-1136) before the drop of non-editable values, so a value the event re-enables is kept. Proxies only, no file.
void CaseTTLRotateReplay()
{
    const filerw::PageDesc* d = filerw::FindPage("HSys");
    const bool oldModel = bHandlerModel;
    bHandlerModel = true;
    std::vector<std::string> handled;
    // opened with card 3: golden FormShow -> rgTTLCardClick greyed the address and set it to 1
    Rg("rgTTLCard")->ItemIndex = 3;  Rg("rgTTLUseAddress")->ItemIndex = 1;  Rg("rgTTLUseAddress")->Enabled = false;
    Check(!filerw::ELEditable("THandlerSystem", "rgTTLUseAddress"), "TTL: card 3 -> address not editable before the replay");
    if (d && d->beforeApply) d->beforeApply("{\"rgTTLCard\":{\"itemIndex\":1},\"rgTTLUseAddress\":{\"itemIndex\":0}}", &handled);
    Check(handled.size() == 1 && handled[0] == "rgTTLCard", "TTL: rgTTLCard handled by beforeApply");
    CheckEqI(Rg("rgTTLCard")->ItemIndex, 1, "TTL: card set to the page value");
    Check(Rg("rgTTLUseAddress")->Enabled && filerw::ELEditable("THandlerSystem", "rgTTLUseAddress"),
          "TTL: card 1 -> address editable again (golden :1254), so the page's address is applied, not dropped");
    handled.clear();
    if (d && d->beforeApply) d->beforeApply("{\"rgTTLCard\":{\"itemIndex\":3}}", &handled);
    Check(!Rg("rgTTLUseAddress")->Enabled, "TTL: card 3 -> address greyed (golden :1251)");
    CheckEqI(Rg("rgTTLUseAddress")->ItemIndex, 1, "TTL: card 3 -> address = 1 (golden :1250)");
    handled.clear();
    if (d && d->beforeApply) d->beforeApply("{\"rgTTLCard\":{\"itemIndex\":3}}", &handled);
    Check(handled.empty(), "TTL: same value = no click, nothing handled");
    Rg("rgTTLCard")->ItemIndex = 0;  Rg("rgTTLUseAddress")->Enabled = true;  Rg("rgTTLUseAddress")->ItemIndex = 0;

    // rotate kit: type 1 greys In / Out, type 0 enables them (golden :1134-1135)
    Rg("rgRotateKit_Type")->ItemIndex = 0;  Rg("rgRotateKitIn")->Enabled = true;  Rg("rgRotateKitOut")->Enabled = true;
    handled.clear();
    if (d && d->beforeApply) d->beforeApply("{\"rgRotateKit_Type\":{\"itemIndex\":1}}", &handled);
    Check(handled.size() == 1 && handled[0] == "rgRotateKit_Type", "Rotate: rgRotateKit_Type handled by beforeApply");
    Check(!Rg("rgRotateKitIn")->Enabled && !Rg("rgRotateKitOut")->Enabled, "Rotate: type 1 -> In / Out greyed");
    handled.clear();
    if (d && d->beforeApply) d->beforeApply("{\"rgRotateKit_Type\":{\"itemIndex\":0}}", &handled);
    Check(Rg("rgRotateKitIn")->Enabled && Rg("rgRotateKitOut")->Enabled, "Rotate: type 0 -> In / Out enabled");
    bHandlerModel = oldModel;
}

// ---------------------------------------------------------------------------
//AI(W906-FRW-E029) 20261002 [W906]：E029／Q72。Steven 1002「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」
//   「這些全部都要可以選,所以數量也是少了」「使用 EJ1N 或是 DTM 應該是要設定兩個變數」。規則：FileRW/HSys_Heater.h 檔尾 E029。
std::string AllBrands(const std::string& f, std::string* bad, int (*want)(int, void*), void* ctx)
{
    std::string ok = "ok";
    for (int ti = 0; ti < eHeaterType_Count; ++ti) {
        const std::string got = OptOf(f, "HeaterInsOpt_" + Name(ti));
        if (got != I2S(want(ti, ctx))) { ok = "BAD"; *bad += Name(ti) + "=" + got + "(want " + I2S(want(ti, ctx)) + ") "; }
    }
    return ok;
}
struct SameCtx { int index, other, u; };
int WantSame(int ti, void* v)              // 「全機相同」的 71 個通道（Index 選 EJ1N／DTM 時 Head1～4 與 Index 區以外跟著其他位置）
{
    const SameCtx* c = static_cast<SameCtx*>(v);
    if (!W906_HeaterInsIsIndexGroup(ti)) return c->other;
    const bool area = c->index == W906_HEATER_INS_EJ1N || c->index == W906_HEATER_INS_DTM;
    if (!area) return c->index;
    return (IsZone(ti) && InArea(ti, c->u)) ? c->index : c->other;
}
void CaseE029()
{
    // ---- (1) Heater 分頁 → USE_16_HEATER（「全機相同」的 Index 下拉；頁面沒連動時伺服器連動），兩個變數同一次存檔
    struct { int u0, pick, u1; const char* what; } a[] = {
        { eht16Heater,       W906_HEATER_INS_EJ1N, eht16HeaterEJ1N,   "16 KT4H + Index EJ1N -> USE_16_HEATER 2" },
        { eht32HeaterKT4H,   W906_HEATER_INS_EJ1N, eht32HeaterEJ1N,   "32 KT4H + Index EJ1N -> 3" },
        { eht4Heater,        W906_HEATER_INS_EJ1N, eht16HeaterEJ1N,   "4 Heaters + Index EJ1N -> 16 EJ1N (2)" },
        { eht16Heater,       W906_HEATER_INS_DTM,  eht16HeaterDTME08, "16 KT4H + Index DTM -> 5" },
        { eht32HeaterKT4H,   W906_HEATER_INS_DTM,  eht32HeaterDTME08, "32 KT4H + Index DTM -> 6" },
        { eht4Heater,        W906_HEATER_INS_DTM,  eht16HeaterDTME08, "4 Heaters + Index DTM -> 16 DTME08 (5)" },
        { eht16HeaterEJ1N,   KT4H,                 eht16Heater,       "16 EJ1N + Index KT4H -> 1" },
        { eht32HeaterEJ1N,   E5DC,                 eht32HeaterKT4H,   "32 EJ1N + Index E5DC -> 4" },
        { eht16HeaterDTME08, DTK4848,              eht16Heater,       "16 DTME08 + Index DTK4848 -> 1" },   // （TC401＋其他 E5DC 會撞 R113，見 CaseCrossBrandR113 (e)）
        { eht32HeaterDTME08, KT4H,                 eht32HeaterKT4H,   "32 DTME08 + Index KT4H -> 4" },
        { eht16HeaterEJ1N,   W906_HEATER_INS_DTM,  eht16HeaterDTME08, "16 EJ1N + Index DTM -> 5" },
    };
    for (auto& x : a) {
        SetU16(x.u0);
        UseFile("e029_tab.ini", L("[System]") + KV("USE_16_HEATER", x.u0) + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=2"));
        OpenNoWrite(std::string("E029 (1) ") + x.what);
        Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_SAME;
        SetCb(Cb("cbHeaterInsOtherOpt"), E5DC);
        SetCb(Cb("cbHeaterInsIndexOpt"), x.pick);
        Check(SaveNow(), std::string("E029 (1) ") + x.what + ": save passes");
        const std::string f = Cur();
        CheckEq(OptOf(f, "USE_16_HEATER"), I2S(x.u1), std::string("E029 (1) ") + x.what + ": [System] USE_16_HEATER written in the same save");
        CheckEq(OptOf(f, "HeaterInsIndexOpt"), I2S(x.pick), std::string("E029 (1) ") + x.what + ": HeaterInsIndexOpt");
        SameCtx c = { x.pick, E5DC, x.u1 };
        std::string bad;
        CheckEq(AllBrands(f, &bad, &WantSame, &c), "ok", std::string("E029 (1) ") + x.what +
                ": 71 HeaterInsOpt_ (Index area = pick, Head1-4 / other zones = Other when EJ1N / DTM) " + bad);
        CheckEq(OptOf(f, "HEATER_CTRL_TYPE"), "2", std::string("E029 (1) ") + x.what + ": HEATER_CTRL_TYPE = Other's COM brand (never 5 / 6)");
        Check(SessionHas("Index Heater Counts（USE_16_HEATER）跟著改成"), std::string("E029 (1) ") + x.what + ": ack says the server linked it");
    }

    // ---- (2) USE_16_HEATER → Heater 分頁（Index Heater Counts 改了、Heater 分頁沒動）
    struct { int u0, u1, idx; const char* what; } b[] = {
        { eht16Heater,       eht32HeaterDTME08, W906_HEATER_INS_DTM,  "16 KT4H -> 32 DTME08: Index area = DTM" },
        { eht16Heater,       eht16HeaterEJ1N,   W906_HEATER_INS_EJ1N, "16 KT4H -> 16 EJ1N: Index area = EJ1N" },
        { eht32HeaterEJ1N,   eht32HeaterKT4H,   KT4H,                 "32 EJ1N -> 32 KT4H: EJ1N -> KT4H" },
        { eht16HeaterDTME08, eht4Heater,        KT4H,                 "16 DTME08 -> 4 Heaters: DTM -> KT4H" },
        { eht32HeaterEJ1N,   eht32HeaterDTME08, W906_HEATER_INS_DTM,  "32 EJ1N -> 32 DTME08: EJ1N -> DTM" },
    };
    for (auto& x : b) {
        SetU16(x.u0);
        UseFile("e029_u16.ini", L("[System]") + KV("USE_16_HEATER", x.u0) + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
        OpenNoWrite(std::string("E029 (2) ") + x.what);
        Rg("rgHeater")->ItemIndex = x.u1;                     // 頁面只改了 Index Heater Counts
        Check(SaveNow(), std::string("E029 (2) ") + x.what + ": save passes");
        const std::string f = Cur();
        CheckEq(OptOf(f, "USE_16_HEATER"), I2S(x.u1), std::string("E029 (2) ") + x.what + ": USE_16_HEATER");
        CheckEq(OptOf(f, "HeaterInsIndexOpt"), I2S(x.idx), std::string("E029 (2) ") + x.what + ": HeaterInsIndexOpt follows");
        SameCtx c = { x.idx, KT4H, x.u1 };
        std::string bad;
        CheckEq(AllBrands(f, &bad, &WantSame, &c), "ok", std::string("E029 (2) ") + x.what + ": 71 HeaterInsOpt_ in the same save " + bad);
        Check(SessionHas("Heater 分頁的 Index 區跟著改成"), std::string("E029 (2) ") + x.what + ": ack says the server linked it");
    }

    // ---- (3) 「各溫控器不同」：Index 區任一通道選 EJ1N／DTM＝整個 Index 區；改回 COM 埠廠牌＝回 1／4
    SetU16(eht16Heater);
    UseFile("e029_diff.ini", L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 1) +
                             Keys71([](int) { return KT4H; }, [](int) { return false; }));
    OpenNoWrite("E029 (3) diff");
    SetCb(ChCb(tcAb1), W906_HEATER_INS_EJ1N);
    Check(SaveNow(), "E029 (3): one Index channel EJ1N in different mode passes");
    {
        const std::string f = Cur();
        CheckEq(OptOf(f, "USE_16_HEATER"), "2", "E029 (3): USE_16_HEATER -> 2 (16 EJ1N)");
        std::string zs, os;
        for (int ti = tcAa1; ti <= tcBd2; ++ti) zs += OptOf(f, "HeaterInsOpt_" + Name(ti));
        for (int ti = tcAe1; ti <= tcBh2; ++ti) os += OptOf(f, "HeaterInsOpt_" + Name(ti));
        CheckEq(zs, std::string(16, '5'), "E029 (3): the whole 16-heater Index area = EJ1N (Steven: one channel = the whole area)");
        CheckEq(os, std::string(16, '1'), "E029 (3): Ae1..Bh2 (outside the area, not installed) keep KT4H");
        CheckEq(OptOf(f, "HeaterInsOpt_Head1"), "1", "E029 (3): Head1 stays KT4H on the COM port");
        int u = 0, c = 0;
        GoldenEj1n(tcBb1, &u, &c);
        CheckEq(OptOf(f, "HeaterInsAddr_Bb1") + "/" + OptOf(f, "HeaterInsCh_Bb1"), I2S(u) + "/" + I2S(c),
                "E029 (3): Bb1 EJ1N unit / CH = golden wiring (iTempCode p=3: unit 1 CH4)");
        CheckEq(OptOf(f, "HeaterInsAddr_Bb1") + "/" + OptOf(f, "HeaterInsCh_Bb1"), "1/4", "E029 (3): Bb1 = EJ1N unit 1 CH4 (literal)");
        CheckEq(OptOf(f, "HeaterInsCh_Head1"), "<missing>", "E029 (3): no HeaterInsCh_ for a COM-port brand");
        CheckEq(OptOf(f, "HeaterInsCh_Ae1"), "<missing>", "E029 (3): no address keys for a not-installed channel");
        Check(SessionHas(kHint), "E029 (3): D-9a hint");
    }
    // 改回 COM 埠廠牌（頁面把整區一起改）→ USE_16_HEATER 回 1；只改一個（頁面沒連動）→ 補回整區
    SetU16(eht16HeaterEJ1N);
    OpenNoWrite("E029 (3) reopen");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_DIFF, "E029 (3) reopen: diff");
    CheckEq(ChEd(tcBb1)->Text.c_str() + std::string("/") + ChCh(tcBb1)->Text.c_str(), "/", "E029 (3) reopen: golden default unit / CH shown blank");
    {
        const std::string saved = Cur();
        Check(SaveNow(), "E029 (3) reopen: unchanged save");
        CheckEq(SameOrDiff(saved, Cur()), "same", "E029 (3): EJ1N area round trip is byte-identical");
    }
    SetCb(ChCb(tcAa1), E5DC);                                 // 只改一個，其他 15 個還是 EJ1N
    Check(SaveNow(), "E029 (3): one channel back to E5DC (page did not link) passes");
    CheckEq(OptOf(Cur(), "HeaterInsOpt_Aa1") + OptOf(Cur(), "USE_16_HEATER"), "52",
            "E029 (3): the area stays EJ1N (one option for the whole area), USE_16_HEATER stays 2");
    for (int ti = tcAa1; ti <= tcBd2; ++ti) SetCb(ChCb(ti), DTK4848);   // 整區改回 COM 埠的廠牌
    Check(SaveNow(), "E029 (3): whole area -> DTK4848 passes");
    CheckEq(OptOf(Cur(), "USE_16_HEATER") + OptOf(Cur(), "HeaterInsOpt_Aa1") + OptOf(Cur(), "HeaterInsOpt_Bd2"), "144",
            "E029 (3): USE_16_HEATER -> 1 (16 KT4H-COM), the area keeps DTK4848");
    {   // 同時有 EJ1N 與 DTM → 擋下
        SetU16(eht16Heater);
        OpenNoWrite("E029 (3) mixed area");
        SetCb(ChCb(tcAa1), W906_HEATER_INS_EJ1N);
        SetCb(ChCb(tcAb1), W906_HEATER_INS_DTM);
        const std::string before = Cur();
        Check(!CheckNow(), "E029 (3): EJ1N and DTM in the same Index area is refused");
        Check(SessionHas("Index 區同時有 Omron EJ1N 與 Delta DTM 的通道"), "E029 (3): mixed-area message");
        CheckEq(Cur() == before ? "same" : "CHANGED", "same", "E029 (3): refused check writes nothing");
    }

    // ---- (4) 兩邊都改又對不上 → 擋下、整頁什麼都不寫
    SetU16(eht16Heater);
    UseFile("e029_both.ini", L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
    OpenNoWrite("E029 (4)");
    Rg("rgHeater")->ItemIndex = eht16HeaterEJ1N;
    SetCb(Cb("cbHeaterInsIndexOpt"), W906_HEATER_INS_DTM);
    {
        const std::string before = Cur();
        Check(!CheckNow(), "E029 (4): Index Heater Counts EJ1N + Heater tab DTM (both changed) is refused");
        Check(SessionHas("兩邊都改過、對不上"), "E029 (4): message");
        CheckEq(Cur() == before ? "same" : "CHANGED", "same", "E029 (4): nothing written");
    }
    SetCb(Cb("cbHeaterInsIndexOpt"), W906_HEATER_INS_EJ1N);   // 兩邊一致（頁面連動過）＝照存
    Check(SaveNow(), "E029 (4): both changed and agreeing passes");
    CheckEq(OptOf(Cur(), "USE_16_HEATER") + OptOf(Cur(), "HeaterInsIndexOpt"), "25", "E029 (4): both written");
    Check(!SessionHas("跟著改成"), "E029 (4): page already linked -> no server-link note");

    // ---- (5) 讀檔以 USE_16_HEATER 為準
    SetU16(eht32HeaterDTME08);
    UseFile("e029_read.ini", L("[System]") + L("USE_16_HEATER=6") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 0) +
                             KV("HeaterInsIndexOpt", 1) + KV("HeaterInsOtherOpt", 2) + Keys71([](int) { return 1; }, [](int) { return false; }));
    OpenNoWrite("E029 (5)");
    CheckEqI(g_iHeaterInsIndexOpt, W906_HEATER_INS_DTM, "E029 (5): file Index KT4H + USE_16_HEATER 6 -> Index = Delta DTM");
    CheckEqI(g_tHeaterInsInfo[tcAa1].GetHeaterInsOpt() * 100 + g_tHeaterInsInfo[tcBh2].GetHeaterInsOpt() * 10 + g_tHeaterInsInfo[tcHead1].GetHeaterInsOpt(),
             662, "E029 (5): Aa1 = Bh2 = DTM (32 area), Head1 = Other");
    CheckEqI(Cb("cbHeaterInsIndexOpt")->ItemIndex, W906_HEATER_INS_DTM, "E029 (5): Index drop-down shows Delta DTM");
    SetU16(eht16Heater);
    UseFile("e029_read2.ini", L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 0) +
                              KV("HeaterInsIndexOpt", 5) + KV("HeaterInsOtherOpt", 2));
    OpenNoWrite("E029 (5b)");
    CheckEqI(g_iHeaterInsIndexOpt, KT4H, "E029 (5): file Index EJ1N + USE_16_HEATER 1 (COM) -> Index = KT4H");
    W906_HeaterMixReadFile();                                 // 溫控底層的讀法：全域 USE_16_HEATER
    CheckEqI(g_iHeaterInsIndexOpt, KT4H, "E029 (5): HeaterInsOpt_Read (global USE_16_HEATER) agrees");

    // ---- (6) 5／6 來回：Index 區以外的通道（HT9050 的 Hotplate／Shuttle／DUT 走 DTM，D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md）
    SetU16(eht16HeaterDTME08);
    UseFile("e029_rt.ini", L("[System]") + L("USE_16_HEATER=5") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 1) +
                           Keys71([](int) { return KT4H; }, [](int) { return false; }));
    OpenNoWrite("E029 (6)");
    CheckEqI(ChCb(tcAa1)->ItemIndex, W906_HEATER_INS_DTM, "E029 (6): DTM area shows Delta DTM");
    SetCb(ChCb(tcHotPlate1), W906_HEATER_INS_DTM);  ChEd(tcHotPlate1)->Text = "0"; ChCh(tcHotPlate1)->Text = "1";
    Check(!CheckNow(), "E029 (6): HotPlate1 DTM station 0 CH1 vs Aa1 (DTM area default station 0 CH1) is refused (same DTM bus)");
    Check(SessionHas("HotPlate1 與 Aa1 都是 Delta DTM 站 0 CH1") || SessionHas("Aa1 與 HotPlate1 都是 Delta DTM 站 0 CH1"), "E029 (6): DTM duplicate message");
    ChEd(tcHotPlate1)->Text = "2";
    SetCb(ChCb(tcHotPlate2), W906_HEATER_INS_DTM);  ChEd(tcHotPlate2)->Text = "2"; ChCh(tcHotPlate2)->Text = "2";
    SetCb(ChCb(tcShuttle1), W906_HEATER_INS_EJ1N);  ChEd(tcShuttle1)->Text = "3"; ChCh(tcShuttle1)->Text = "2";
    SetCb(ChCb(tcDUT1), W906_HEATER_INS_DTM);       ChEd(tcDUT1)->Text = "3";     ChCh(tcDUT1)->Text = "5";   // DUT1 這台沒裝（DUT 1 顆），指定了站號＝要用
    Check(SaveNow(), "E029 (6): HotPlate1 / HotPlate2 / DUT1 DTM + Shuttle1 EJ1N with stations passes");
    {
        const std::string f = Cur();
        CheckEq(OptOf(f, "HeaterInsOpt_HotPlate1") + "/" + OptOf(f, "HeaterInsAddr_HotPlate1") + "/" + OptOf(f, "HeaterInsCh_HotPlate1"), "6/2/1",
                "E029 (6): HotPlate1 = 6 (DTM), station 2, CH1");
        CheckEq(OptOf(f, "HeaterInsOpt_Shuttle1") + "/" + OptOf(f, "HeaterInsAddr_Shuttle1") + "/" + OptOf(f, "HeaterInsCh_Shuttle1"), "5/3/2",
                "E029 (6): Shuttle1 = 5 (EJ1N), unit 3, CH2");
        CheckEq(OptOf(f, "HeaterInsOpt_DUT1") + "/" + OptOf(f, "HeaterInsAddr_DUT1") + "/" + OptOf(f, "HeaterInsCh_DUT1"), "6/3/5",
                "E029 (6): DUT1 (not installed, station given) = 6, station 3, CH5");
        CheckEq(OptOf(f, "USE_16_HEATER") + OptOf(f, "HEATER_CTRL_TYPE"), "51", "E029 (6): USE_16_HEATER stays 5, HEATER_CTRL_TYPE = KT4H");
        Check(SessionHas(kHint), "E029 (6): D-9a hint (per-channel EJ1N / DTM need the lower layer)");
    }
    {
        const std::string saved = Cur();
        OpenNoWrite("E029 (6) reopen");
        CheckEqI(g_tHeaterInsInfo[tcHotPlate1].GetHeaterInsOpt(), W906_HEATER_INS_DTM, "E029 (6) reopen: HotPlate1 = 6 in memory");
        int u = -1, c = -1;
        Check(W906_HeaterInsUnitCh(tcHotPlate1, &u, &c) && u == 2 && c == 1, "E029 (6) reopen: W906_HeaterInsUnitCh(HotPlate1) = station 2 CH1");
        Check(W906_HeaterInsUnitCh(tcShuttle1, &u, &c) && u == 3 && c == 2, "E029 (6) reopen: W906_HeaterInsUnitCh(Shuttle1) = unit 3 CH2");
        Check(W906_HeaterInsUnitCh(tcBa1, &u, &c) && u == 0 && c == 2, "E029 (6) reopen: W906_HeaterInsUnitCh(Ba1) = DTM golden station 0 CH2");
        Check(!W906_HeaterInsUnitCh(tcHead3, &u, &c), "E029 (6) reopen: Head3 (KT4H) has no unit / CH");
        CheckEqI(W906_HeaterInsBus(tcHotPlate1) * 100 + W906_HeaterInsBus(tcShuttle1) * 10 + W906_HeaterInsBus(tcHead1), 321,
                 "E029 (6) reopen: buses DTM / EJ1N / COM");
        CheckEq(std::string(ChEd(tcHotPlate1)->Text.c_str()) + "/" + ChCh(tcHotPlate1)->Text.c_str(), "2/1", "E029 (6) reopen: HotPlate1 boxes show 2 / 1");
        Check(SaveNow(), "E029 (6) reopen: unchanged save");
        CheckEq(SameOrDiff(saved, Cur()), "same", "E029 (6): codes 5 / 6 + stations round trip is byte-identical");
    }

    // ---- (7) D-8a 分匯流排（EJ1N 同台同 CH、DTM 同站同 CH；不同匯流排不比）＋範圍
    SetCb(ChCb(tcShuttle2), W906_HEATER_INS_EJ1N); ChEd(tcShuttle2)->Text = "3"; ChCh(tcShuttle2)->Text = "2";
    Check(!CheckNow(), "E029 (7): Shuttle1 and Shuttle2 both EJ1N unit 3 CH2 are refused");
    Check(SessionHas("Shuttle1 與 Shuttle2 都是 Omron EJ1N 第 3 台 CH2"), "E029 (7): EJ1N duplicate message");
    ChCh(tcShuttle2)->Text = "3";
    Check(CheckNow(), "E029 (7): same EJ1N unit, different CH is allowed");
    SetCb(ChCb(tcShuttle2), KT4H); ChEd(tcShuttle2)->Text = "3"; ChCh(tcShuttle2)->Text = "";
    Check(CheckNow(), "E029 (7): KT4H station 3 (COM port) vs EJ1N unit 3 (EJ1N bus) is allowed (different buses)");
    ChEd(tcShuttle2)->Text = "";
    struct { int ti, opt; const char* st; const char* ch; bool ok; const char* what; } r[] = {
        { tcShuttle2, W906_HEATER_INS_EJ1N, "16", "1", false, "E029 (7): EJ1N unit 16 refused (SW1 1-15)" },
        { tcShuttle2, W906_HEATER_INS_EJ1N, "0",  "1", false, "E029 (7): EJ1N unit 0 refused" },
        { tcShuttle2, W906_HEATER_INS_EJ1N, "15", "4", true,  "E029 (7): EJ1N unit 15 CH4 allowed" },
        { tcShuttle2, W906_HEATER_INS_EJ1N, "15", "5", false, "E029 (7): EJ1N CH5 refused (TC4 CH1-4)" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "3",  "8", true,  "E029 (7): DTM station 3 CH8 allowed" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "4",  "1", false, "E029 (7): DTM station 4 refused (0-3)" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "3",  "9", false, "E029 (7): DTM CH9 refused (1-8)" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "3",  "0", false, "E029 (7): DTM CH0 refused" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "3",  "",  false, "E029 (7): station without CH refused" },
        { tcShuttle2, W906_HEATER_INS_DTM,  "3",  "x", false, "E029 (7): non-numeric CH refused" },
        { tcShuttle2, W906_HEATER_INS_EJ1N, "",   "",  false, "E029 (7): installed non-Index EJ1N without unit / CH refused (no golden default)" },
        { tcAa1,      W906_HEATER_INS_DTM,  "",   "",  true,  "E029 (7): Index-area DTM without station = golden default, allowed" },
    };
    for (auto& x : r) {
        SetCb(ChCb(x.ti), x.opt); ChEd(x.ti)->Text = x.st; ChCh(x.ti)->Text = x.ch;
        Check(CheckNow() == x.ok, x.what);
        if (x.ti == tcShuttle2) { SetCb(ChCb(tcShuttle2), KT4H); ChEd(tcShuttle2)->Text = ""; ChCh(tcShuttle2)->Text = ""; }   // 每列各自獨立
    }
    SetCb(ChCb(tcShuttle2), KT4H); ChEd(tcShuttle2)->Text = ""; ChCh(tcShuttle2)->Text = "";
    SetCb(ChCb(tcShuttle2), W906_HEATER_INS_EJ1N);
    Check(!CheckNow() && SessionHas("Shuttle2：選了 Omron EJ1N，要填台號與 CH"), "E029 (7): missing-address message names the channel");
    SetCb(ChCb(tcShuttle2), KT4H);

    // ---- (8) HEATER_CTRL_TYPE 永遠不寫 5／6，有 EJ1N／DTM 時也不寫 3
    SetU16(eht16HeaterEJ1N);
    UseFile("e029_ctrl.ini", L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=2"));
    OpenNoWrite("E029 (8)");
    Rg("rgHeaterInsMode")->ItemIndex = HEATER_INS_MODE_SAME;
    SetCb(Cb("cbHeaterInsOtherOpt"), NoHeater);
    Check(SaveNow(), "E029 (8): Index EJ1N + Other No Heater passes");
    CheckEq(OptOf(Cur(), "HEATER_CTRL_TYPE"), "2", "E029 (8): only EJ1N heaters -> HEATER_CTRL_TYPE keeps 2 (not 3: golden starts EJ1N only when != NoHeater)");
    //   全 DTM（HT9050 的樣子，「各溫控器不同」）＋檔案 HEATER_CTRL_TYPE=3 → KT4H（golden 預設），不寫 6
    SetU16(eht16HeaterDTME08);
    UseFile("e029_ctrl2.ini", L("[System]") + L("USE_16_HEATER=5") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=3") + KV("HeaterInsMode", 1) +
                              Keys71([](int) { return NoHeater; }, [](int) { return false; }));
    OpenNoWrite("E029 (8b)");
    SetCb(ChCb(tcHotPlate1), W906_HEATER_INS_DTM); ChEd(tcHotPlate1)->Text = "2"; ChCh(tcHotPlate1)->Text = "1";
    SetCb(ChCb(tcHotPlate2), W906_HEATER_INS_DTM); ChEd(tcHotPlate2)->Text = "2"; ChCh(tcHotPlate2)->Text = "2";
    Check(SaveNow(), "E029 (8b): all DTM (HT9050 style, different mode) passes");
    CheckEq(OptOf(Cur(), "HEATER_CTRL_TYPE"), "1", "E029 (8b): no COM-port brand + file HEATER_CTRL_TYPE 3 -> KT4H (golden default), never 6, not 3");
    CheckEq(OptOf(Cur(), "HeaterInsOpt_HotPlate1") + OptOf(Cur(), "HeaterInsOpt_Aa1") + OptOf(Cur(), "HeaterInsOpt_Shuttle1"), "663",
            "E029 (8b): HotPlate1 / Aa1 = 6, Shuttle1 = No Heater");
    //   Steven 1002 17:1x：「全機相同」的「其他位置溫控器」只有 golden 5 個 —— 替身只有 5 項，選 6 → 沒有選 → 擋下
    SetU16(eht16Heater);
    UseFile("e029_other.ini", L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
    OpenNoWrite("E029 (8c)");
    CheckEqI(Cb("cbHeaterInsOtherOpt")->Items->Count * 10 + Cb("cbHeaterInsIndexOpt")->Items->Count, 57,
             "E029 (8c): same mode lists -- Other has the 5 golden brands, Index has 7 (EJ1N / DTM only at Index)");
    CheckEqI(ChCb(tcHotPlate1)->Items->Count * 10 + ChCb(tcLBDown)->Items->Count, 77, "E029 (8c): per-channel lists have 7 (different mode)");
    SetCb(Cb("cbHeaterInsOtherOpt"), W906_HEATER_INS_DTM);
    CheckEqI(Cb("cbHeaterInsOtherOpt")->ItemIndex, -1, "E029 (8c): Other cannot take Delta DTM (5 items)");
    {
        const std::string before = Cur();
        Check(!CheckNow() && SessionHas("只能選 golden 的 5 個"), "E029 (8c): Other without a golden brand is refused");
        CheckEq(Cur() == before ? "same" : "CHANGED", "same", "E029 (8c): nothing written");
    }
    UseFile("e029_other2.ini", L("[System]") + L("USE_16_HEATER=1") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsOtherOpt", 6));
    OpenNoWrite("E029 (8d)");
    CheckEqI(g_iHeaterInsOtherOpt, KT4H, "E029 (8d): a file with HeaterInsOtherOpt=6 reads as KT4H (Other is golden 0-4 only)");

    // ---- (9) Heater Type 重播：USE_16_HEATER 是 EJ1N 時 Index 區留 EJ1N（golden 按 Heater Type 不動 USE_16_HEATER）
    SetU16(eht16HeaterEJ1N);
    UseFile("e029_type.ini", L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
    OpenNoWrite("E029 (9)");
    {
        const filerw::PageDesc* d = filerw::FindPage("HSys");
        const bool oldModel = bHandlerModel;
        bHandlerModel = true;
        std::vector<std::string> handled;
        const int pick = (Rg("rgHeaterType")->ItemIndex == E5DC) ? DTK4848 : E5DC;
        if (d && d->beforeApply) d->beforeApply("{\"rgHeaterType\":{\"itemIndex\":" + I2S(pick) + "}}", &handled);
        bHandlerModel = oldModel;
        CheckEqI(g_iHeaterInsIndexOpt * 10 + g_iHeaterInsOtherOpt, W906_HEATER_INS_EJ1N * 10 + pick, "E029 (9): Heater Type -> Index stays EJ1N, Other = pick");
        CheckEqI(g_tHeaterInsInfo[tcAa1].GetHeaterInsOpt() * 10 + g_tHeaterInsInfo[tcHead1].GetHeaterInsOpt(), W906_HEATER_INS_EJ1N * 10 + pick,
                 "E029 (9): Aa1 = EJ1N, Head1 = pick");
    }

    // ---- (10) extra JSON：71 列、7 個廠牌、Index 區的 golden 接線、DTM 連線設定（唯讀，W906_DTME08INI_PATH）、全部替身看得到
    {
        const filerw::PageDesc* d = filerw::FindPage("HSys");
        const std::string ex = (d && d->extraJson) ? d->extraJson() : std::string();
        cJSON* root = cJSON_Parse(ex.c_str());
        const cJSON* h = root ? cJSON_GetObjectItemCaseSensitive(root, "heater") : nullptr;
        const cJSON* mix = h ? cJSON_GetObjectItemCaseSensitive(h, "mix") : nullptr;
        const cJSON* rows = mix ? cJSON_GetObjectItemCaseSensitive(mix, "rows") : nullptr;
        const cJSON* conn = mix ? cJSON_GetObjectItemCaseSensitive(mix, "conn") : nullptr;
        const cJSON* dtm = conn ? cJSON_GetObjectItemCaseSensitive(conn, "dtm") : nullptr;
        const cJSON* mo = mix ? cJSON_GetObjectItemCaseSensitive(mix, "options") : nullptr;
        CheckEqI(rows ? cJSON_GetArraySize(rows) : -1, 71, "E029 (10): 71 rows");
        CheckEqI(mo ? cJSON_GetArraySize(mo) : -1, 7, "E029 (10): mix.options = 7");
        const cJSON* oo = mix ? cJSON_GetObjectItemCaseSensitive(mix, "otherOptions") : nullptr;
        CheckEqI(oo ? cJSON_GetArraySize(oo) : -1, 5, "E029 (10): mix.otherOptions = 5 golden brands");
        Check(mix && !cJSON_GetObjectItemCaseSensitive(mix, "indexLocked"), "E029 (10): no indexLocked any more (Index chooses EJ1N / DTM itself)");
        int okZone = 0, okOther = 0, vis = 0, okPair = 0, okActive = 0;
        for (const cJSON* r = rows ? rows->child : nullptr; r; r = r->next) {
            const int ti = cJSON_GetObjectItemCaseSensitive(r, "typeIdx")->valueint;
            const int eu = cJSON_GetObjectItemCaseSensitive(r, "ej1nUnit")->valueint, ec = cJSON_GetObjectItemCaseSensitive(r, "ej1nCh")->valueint;
            const int ds = cJSON_GetObjectItemCaseSensitive(r, "dtmStation")->valueint, dc = cJSON_GetObjectItemCaseSensitive(r, "dtmCh")->valueint;
            if (IsZone(ti)) {
                int u = 0, c = 0, s = 0, c2 = 0;
                GoldenEj1n(ti, &u, &c); GoldenDtm(ti, &s, &c2);
                if (eu == u && ec == c && ds == s && dc == c2) ++okZone;
            } else if (eu == -1 && ec == -1 && ds == -1 && dc == -1) ++okOther;
            const char* pair = cJSON_GetObjectItemCaseSensitive(r, "pair")->valuestring;
            const std::string wantPair = (ti >= tcHead1 && ti <= tcHead4) ? "head" : (ti >= tcAa1 && ti <= tcBd2) ? "zone16" :
                                         (ti >= tcAe1 && ti <= tcBh2) ? "zone32" : ti == tcSocket ? "socket" :
                                         (ti == tcDUT1 || ti == tcDUT2) ? "dut12" : (ti == tcDUT3 || ti == tcDUT4) ? "dut34" : "";
            if (wantPair == pair) ++okPair;
            if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(r, "active")) == ActiveNow(ti, eht16HeaterEJ1N, eDut1ea)) ++okActive;
            const cJSON* chEd = cJSON_GetObjectItemCaseSensitive(r, "chEd");
            if (chEd && cJSON_IsString(chEd) && Ed(chEd->valuestring)->Visible && ChCb(ti)->Visible) ++vis;
        }
        CheckEqI(okZone, 32, "E029 (10): 32 Index zones carry golden EJ1N / DTM wiring (iTempCode)");
        CheckEqI(okOther, 39, "E029 (10): the other 39 channels have no EJ1N / DTM default");
        CheckEqI(vis, 71, "E029 (10): all 71 combo + CH proxies are visible (all selectable; the page hides the exclusive pairs)");
        CheckEqI(okPair, 71, "E029 (10): rows carry the two exclusive pairs (head / zone16 / zone32 / socket / dut12 / dut34)");
        CheckEqI(okActive, 71, "E029 (10): rows' active = Steven's rules at 16 EJ1N + 1 EA");
        Check(dtm && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(dtm, "exists")) &&
              std::string(cJSON_GetObjectItemCaseSensitive(dtm, "address")->valuestring) == "172.16.8.50" &&
              std::string(cJSON_GetObjectItemCaseSensitive(dtm, "port")->valuestring) == "502",
              "E029 (10): conn.dtm reads DTME08_Control.ini [SocketSetting] (scratch copy via W906_DTME08INI_PATH)");
        Check(conn && std::string(cJSON_GetObjectItemCaseSensitive(conn, "comOmronWidget")->valuestring) == "cbComTempOmron",
              "E029 (10): conn names the page's COM_PORT_OMRON widget");
        if (root) cJSON_Delete(root);
    }
    // ---- (11) 兩組互斥（Steven 1002 17:1x「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的」「socket跟 Dut 1~4也是互斥的」）：
    //   「各溫控器不同」存檔只替「這台有裝」的通道寫站號 ⇒ 站號鍵在不在＝有沒有裝；兩個方向都試（開頁時就是／頁面上改了 rgHeater、rgUse4DUT 才存）
    struct { int u, dut, uPage, dutPage; const char* what; } q[] = {
        { eht4Heater,      eDut1ea, eht4Heater,      eDut1ea, "4 Heaters + 1 EA" },
        { eht16Heater,     eDut2ea, eht16Heater,     eDut2ea, "16 Heaters + 2 EA" },
        { eht32HeaterKT4H, eDut4ea, eht32HeaterKT4H, eDut4ea, "32 KT4H + 4 EA" },
        { eht4Heater,      eDut1ea, eht16Heater,     eDut4ea, "opened 4 Heaters + 1 EA, page -> 16 + 4 EA" },
        { eht32HeaterKT4H, eDut4ea, eht4Heater,      eDut1ea, "opened 32 + 4 EA, page -> 4 Heaters + 1 EA" },
    };
    for (auto& x : q) {
        SetU16(x.u); SetDut(x.dut);
        UseFile("e029_pair.ini", L("[System]") + KV("USE_16_HEATER", x.u) + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 1) +
                                 Keys71([](int) { return KT4H; }, [](int) { return false; }));
        OpenNoWrite(std::string("E029 (11) ") + x.what);
        Rg("rgHeater")->ItemIndex = x.uPage; Rg("rgUse4DUT")->ItemIndex = x.dutPage;   // 頁面上點的（engine 送回來的值）
        Check(SaveNow(), std::string("E029 (11) ") + x.what + ": save passes");
        const std::string f = Cur();
        std::string got, want;
        const int probe[] = { tcHead1, tcHead4, tcAa1, tcBd2, tcAe1, tcBh2, tcSocket, tcDUT1, tcDUT2, tcDUT3, tcDUT4 };
        for (int ti : probe) {
            got  += Name(ti) + (OptOf(f, "HeaterInsAddr_" + Name(ti)) == "<missing>" ? "-" : "+") + " ";
            want += Name(ti) + (ActiveNow(ti, x.uPage, x.dutPage) ? "+" : "-") + " ";
        }
        CheckEq(got, want, std::string("E029 (11) ") + x.what + ": installed channels (station keys) follow rgHeater / rgUse4DUT on the page");
        CheckEq(OptOf(f, "USE_16_HEATER") + "/" + OptOf(f, "SocketBasedAdd4Temp"), I2S(x.uPage) + "/<missing>",
                std::string("E029 (11) ") + x.what + ": USE_16_HEATER written (SocketBasedAdd4Temp is golden SaveSystemSet's, not run here)");
    }
    {   // D-8a 跟著互斥：Head1 KT4H 預設站號 5 vs Shuttle1 指定 5 —— 4 Heaters 擋、16 組時 Head1 沒裝 → 可以
        SetU16(eht4Heater); SetDut(eDut1ea);
        UseFile("e029_pair2.ini", L("[System]") + L("USE_16_HEATER=0") + L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1") + KV("HeaterInsMode", 1) +
                                  Keys71([](int) { return KT4H; }, [](int) { return false; }));
        OpenNoWrite("E029 (11) D-8a");
        ChEd(tcShuttle1)->Text = "5";
        Check(!CheckNow(), "E029 (11): 4 Heaters -- Head1 KT4H 5 vs Shuttle1 KT4H 5 is refused");
        Rg("rgHeater")->ItemIndex = eht16Heater;
        Check(CheckNow(), "E029 (11): page -> 16 Heaters -- Head1 not installed any more, Shuttle1 KT4H 5 is allowed");
        Rg("rgHeater")->ItemIndex = eht4Heater;
        ChEd(tcShuttle1)->Text = "";
        // Socket TC401（第 8/4+1＝3 台）vs Shuttle1 KT4H 預設站號 3（R113）：1 EA 有裝 → 擋；頁面改 4 EA → Socket 沒裝 → 可以
        SetCb(ChCb(tcSocket), TC401);
        Rg("rgUse4DUT")->ItemIndex = eDut1ea;
        Check(!CheckNow(), "E029 (11): 1 EA -- Socket TC401 unit 3 vs Shuttle1 KT4H 3 is refused");
        Rg("rgUse4DUT")->ItemIndex = eDut4ea;
        Check(CheckNow(), "E029 (11): page -> 4 EA -- Socket not installed, allowed");
        // DUT1 TC401（第 29/4+1＝8 台）vs Head4 KT4H 預設站號 8：4 EA 有裝 → 擋；頁面改 1 EA → DUT1 沒裝 → 可以
        SetCb(ChCb(tcDUT1), TC401);
        Check(!CheckNow(), "E029 (11): 4 EA -- DUT1 TC401 unit 8 vs Head4 KT4H 8 is refused");
        Rg("rgUse4DUT")->ItemIndex = eDut2ea;
        Check(!CheckNow(), "E029 (11): page -> 2 EA -- DUT1 still installed, refused");
        Rg("rgUse4DUT")->ItemIndex = eDut1ea;
        SetCb(ChCb(tcSocket), KT4H);
        Check(CheckNow(), "E029 (11): page -> 1 EA (Socket back to KT4H) -- DUT1 not installed, allowed");
    }
    SetU16(eht16HeaterEJ1N); SetDut(eDut1ea);

    // 港樹的 iTempCode 跟 golden 一樣（受測碼的預設接線拿它算）
    bool same = true;
    for (int p = 0; p < 32; ++p) same = same && iTempCode[p] == kGoldenTempCode[p];
    Check(same, "E029: port tree cmydef.cpp iTempCode == golden V912 cmydef.cpp:111-117");
    CheckEqI(W906_HeaterInsU16For(W906_HEATER_INS_EJ1N, eht4Heater) * 10 + W906_HeaterInsU16For(-1, eht32HeaterDTME08), 24,
             "E029: W906_HeaterInsU16For(EJ1N, 4 Heaters) = 2, (COM, 32 DTME08) = 4");
    SetU16(eht16HeaterEJ1N);
}

// ---------------------------------------------------------------------------
// 手動：--example <Gerneral.ini> [USE_16_HEATER] —— 複製到暫存、開頁＋不改直接存、印逐鍵差異（原檔只讀）
std::map<std::string, std::string> TempCtrlKeys(const std::string& f, std::vector<std::string>* order)
{
    std::map<std::string, std::string> m;
    std::istringstream in(f);
    std::string line;
    bool in_tc = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t b = line.find_first_not_of(" \t");
        std::string t = b == std::string::npos ? "" : line.substr(b);
        if (!t.empty() && t[0] == '[') { in_tc = (t.compare(0, 10, "[TempCtrl]") == 0); continue; }
        if (!in_tc) continue;
        const size_t eq = t.find('=');
        if (eq == std::string::npos) continue;
        const std::string k = t.substr(0, eq);
        if (!m.count(k) && order) order->push_back(k);
        m[k] = t.substr(eq + 1);
    }
    return m;
}
int Example(const std::string& src, int use16)
{
    std::string in;
    if (!ReadFile(src, &in)) { std::printf("cannot read %s\n", src.c_str()); return 2; }
    SetU16(use16);
    UseFile("example.ini", in);
    W906_HeaterMixLoaderSystemSet();
    const bool same = (Cur() == in);
    std::printf("example: %s (USE_16_HEATER=%d) copied to %s\n", src.c_str(), use16, asGeneralPath.c_str());
    std::printf("page open: file %s; mode=%d Index=%d Other=%d indexArea=%d\n", same ? "unchanged" : "CHANGED",
                g_iHeaterInsMode, g_iHeaterInsIndexOpt, g_iHeaterInsOtherOpt, (int)W906_HeaterInsU16Area(USE_16_HEATER));
    if (!SaveNow()) { std::printf("save refused: %s\n", filerw::SessionJson().c_str()); return 1; }
    const std::string out = Cur();
    std::vector<std::string> o0, o1;
    std::map<std::string, std::string> a = TempCtrlKeys(in, &o0), b = TempCtrlKeys(out, &o1);
    int changed = 0, added = 0;
    for (const std::string& k : o1) {
        if (!a.count(k)) { std::printf("  + %s=%s\n", k.c_str(), b[k].c_str()); ++added; }
        else if (a[k] != b[k]) { std::printf("  ~ %s: %s -> %s\n", k.c_str(), a[k].c_str(), b[k].c_str()); ++changed; }
    }
    std::printf("[TempCtrl]: %d keys changed, %d keys added; bytes %lu -> %lu; lines outside [TempCtrl] %s\n", changed, added,
                (unsigned long)in.size(), (unsigned long)out.size(), "(see file)");
    std::printf("session: %s\n", filerw::SessionJson().c_str());
    return 0;
}

// 真檔守衛
struct RealGuard { std::string path, bytes; bool had; };

}  // namespace

int main(int argc, char** argv)
{
    std::vector<RealGuard> guard = { { "D:\\HT9045\\system\\Gerneral.ini", "", false }, { "D:\\HT9045\\config\\config.ini", "", false },
                                     { "D:\\HT9045\\system\\DTME08_Control.ini", "", false } };   //AI(W906-FRW-E029) 20261002
    for (RealGuard& g : guard) g.had = ReadFile(g.path, &g.bytes);

    g_dir = ScratchDir();
    //AI(W906-FRW-E029) 20261002：W906_HeaterMixExtraJson 唯讀 DTME08_Control.ini ⇒ 轉到暫存（真檔不讀）
    {
        const std::string dtm = g_dir + "DTME08_Control.ini";
        WriteFile(dtm, L("[SocketSetting]") + L("asAddress=172.16.8.50") + L("asPort=502"));
        const std::string env = "W906_DTME08INI_PATH=" + dtm;
        HT9045_TEST_PUTENV(env.c_str());
    }
    asGeneralPath = AnsiString((g_dir + "boot.ini").c_str());   // 開機建替身之前就轉走（建替身不讀寫檔，保險）
    WriteFile(asGeneralPath.c_str(), L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
    OpenGeneralIniFile();

    // golden 可見條件（開機算一次）：只開 USE_16_HEATER（Head1～4 看得到），其他全關 → 有下拉而看得到的＝10 個
    USE_16_HEATER = eht16HeaterEJ1N;                   // （開機時替身還沒建，只設全域；建完再 SetU16）
    RTC_TemperNumber = 0; INSTALL_HEAT_GUN = 0; INSTALL_ATC_HEAT_GUN = 0; iSocketBaseTempCount = eDut1ea;
    CCD2_TEMPER = false; LB_TEMP = false; Index_ESDAir = false;
    FileRW_HSys_Boot();
    SetDut(eDut1ea);                                   //AI(W906-FRW-E029) 20261002：開頁時的 Dut Heater Count（替身）＝開機的 iSocketBaseTempCount

    int rc = 0;
    if (argc >= 3 && std::string(argv[1]) == "--example") {
        rc = Example(argv[2], argc >= 4 ? std::atoi(argv[3]) : eht16HeaterEJ1N);
    } else {
        CaseV899();
        CaseV912AllMinus9999();
        CaseV912Mixed();
        CaseNewFileRoundTrip();
        CaseSameIndexOther();
        CaseRefused();
        CaseCrossBrandR113();   //AI(W906-FRW-S166) 20260928 [W906] R113
        CaseHeaterTypeReplayAndExtra();
        CaseTTLRotateReplay();   //AI(W906-HSYS-EVT) 20261002
        CaseE029();              //AI(W906-FRW-E029) 20261002
    }

    for (const RealGuard& g : guard) {
        std::string now;
        const bool had = ReadFile(g.path, &now);
        if (had != g.had || now != g.bytes) { ++g_fail; std::printf("FAIL: real machine file changed: %s\n", g.path.c_str()); }
    }
    W906_TestTmpRemoveTree(g_dir);   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h): remove this run's scratch dir
    std::printf("test_hsys_heater_mix: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : rc;
}
// AI(W906-S09-Q3) 20260930 (St02-E): FileRW/_EditList.cpp now takes the FormJson lock in FileRW_ProxyChecked / FileRW_ProxySet* (St01 R1);
//   this test compiles _EditList.cpp without JsonBridge/FormJson.cpp, so it gives the lock itself (single-threaded:
//   a no-op), as test_b8_os5_sortbuttons.cpp:62-65 does.
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }
