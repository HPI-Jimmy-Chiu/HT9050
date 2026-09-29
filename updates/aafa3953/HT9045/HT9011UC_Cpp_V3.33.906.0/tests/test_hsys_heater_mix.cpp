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
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_HSys_Boot();

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
    std::string d = std::string(t ? t : ".") + "\\ht9045_hsys_heater_mix_test";
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

const std::string kHint = "目前溫控只看 HEATER_CTRL_TYPE";

// ---------------------------------------------------------------------------
void CaseV899()
{
    // V899 升上來的檔：0 個 HeaterInsOpt_、沒有模式鍵 → 相同，Index＝其他＝HEATER_CTRL_TYPE（Q34 ③ 第 1 條）
    USE_16_HEATER = eht16HeaterEJ1N;
    const std::string head = L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("COM_PORT=COM12") +
                             L("HEATER_CTRL_TYPE=1") + L("AMBIENT_TEMP_CHECK1=1");
    const std::string tail = L("[NUMBER_PANEL]") + L("NUMBER_PANEL_DELAY=1");
    UseFile("v899.ini", head + tail);
    OpenNoWrite("V899");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_SAME, "V899: mode inferred SAME (D-1b: no HeaterInsOpt_ at all)");
    CheckEqI(g_iHeaterInsIndexOpt, 1, "V899: Index = HEATER_CTRL_TYPE");
    CheckEqI(g_iHeaterInsOtherOpt, 1, "V899: Other = HEATER_CTRL_TYPE");
    bool all1 = true;
    for (int ti = 0; ti < eHeaterType_Count; ++ti) all1 = all1 && g_tHeaterInsInfo[ti].GetHeaterInsOpt() == 1;
    Check(all1, "V899: all 71 channels = KT4H in memory (same as today)");
    CheckEqI(Rg("rgHeaterType")->ItemIndex, 1, "V899: rgHeaterType shows KT4H");
    CheckEqI(Rg("rgHeaterInsMode")->ItemIndex, 0, "V899: mode proxy = same");
    Check(!Cb("cbHeaterInsIndexOpt")->Enabled, "V899 (USE_16_HEATER=EJ1N): Index drop-down disabled");
    // 什麼都不改直接存檔：段尾一次加 3＋71 行，其他不動
    Check(SaveNow(), "V899: unchanged save passes the check");
    const std::string want = L("[System]") + L("USE_16_HEATER=2") + L("[TempCtrl]") + L("COM_PORT=COM12") +
                             L("HEATER_CTRL_TYPE=1") + L("AMBIENT_TEMP_CHECK1=1") +
                             KV("HeaterInsMode", 0) + KV("HeaterInsIndexOpt", 1) + KV("HeaterInsOtherOpt", 1) +
                             Keys71([](int) { return 1; }, [](int) { return false; }) + tail;
    CheckEq(Cur(), want, "V899: unchanged save = 3 mode keys + 71 HeaterInsOpt_=1 appended at the end of [TempCtrl], HEATER_CTRL_TYPE stays 1");
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
    return L("[Version]") + L("Ver=V3.33.906.0") +
           L("[TempCtrl]") + L("   \tUSE_NEW_TEMPCTRL_FUNCTION=0") + L("COM_PORT=COM12") + L("AMBIENT_TEMP_CHECK33=1") +
           L("HEATER_CTRL_TYPE=2") + L("AMBIENT_TEMP_CHECK71=1") + opts + L("  ") +
           L("[NUMBER_PANEL]") + L("   \tNUMBER_PANEL_DELAY=1");
}
void CaseV912AllMinus9999()
{
    // Steven01 這台的形狀：USE_16_HEATER=2（EJ1N）、HEATER_CTRL_TYPE=2（E5DC）、71 鍵全 -9999、沒有模式鍵（Q34 例子第 1 條）
    USE_16_HEATER = eht16HeaterEJ1N;
    const std::string before = Steven01Like(Keys71([](int) { return INVALID_INT_VAL_NEG; }, [](int) { return false; }));
    UseFile("v912_all9999.ini", before);
    OpenNoWrite("V912 all -9999");
    CheckEqI(g_iHeaterInsMode, HEATER_INS_MODE_SAME, "V912 all -9999: mode SAME (D-1b after D-5a)");
    CheckEqI(g_iHeaterInsIndexOpt, 2, "V912 all -9999: Index = E5DC");
    CheckEqI(g_iHeaterInsOtherOpt, 2, "V912 all -9999: Other = E5DC");
    CheckEqI(g_tHeaterInsInfo[tcAa1].GetHeaterInsOpt(), 2, "V912 all -9999: Aa1 = E5DC (D-5a)");
    CheckEqI(ChCb(tcHotPlate1)->ItemIndex, 2, "V912 all -9999: HotPlate1 drop-down = E5DC");
    Check(!Cb("cbHeaterInsIndexOpt")->Enabled, "V912 all -9999: Index drop-down disabled (EJ1N)");
    Check(SaveNow(), "V912 all -9999: unchanged save passes the check");
    const std::string want = Steven01Like(Keys71([](int) { return 2; }, [](int) { return false; }) +
                                          KV("HeaterInsMode", 0) + KV("HeaterInsIndexOpt", 2) + KV("HeaterInsOtherOpt", 2));
    CheckEq(Cur(), want, "V912 all -9999: unchanged save = 71 keys -9999 -> 2 in place (D-4a, incl. the 48 without a drop-down) + "
                         "3 new keys after HeaterInsOpt_LBDown, HEATER_CTRL_TYPE stays 2, indentation / blank line / other sections untouched");
}

// ---------------------------------------------------------------------------
void CaseV912Mixed()
{
    // V912 寫過、各通道真的不同：HotPlate1=KT4H、Shuttle1=DTK4848、其餘 -9999（→E5DC）、CCD_2 的鍵被刪掉 → 推斷「不同」，
    //   CCD_2 缺鍵＝3（Q15）；站號用預設（Q34 ③ 第 3 條、例子「Q15 的缺鍵＝3」）
    USE_16_HEATER = eht16HeaterEJ1N;
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
    CheckEqI(g_iHeaterInsIndexOpt, 2, "V912 mixed: Index falls back to HEATER_CTRL_TYPE");
    CheckEqI(W906_HeaterStationIdx(tcShuttle1), tcShuttle1, "V912 mixed: Shuttle1 station index = default (Addr)");
    CheckEqI(Rg("rgHeaterType")->ItemIndex, 2, "V912 mixed: rgHeaterType shows HEATER_CTRL_TYPE (not all the same, golden :271)");
    Check(SaveNow(), "V912 mixed: unchanged save passes the check");
    // 列出的通道（開機可見條件：HotPlate1/2、Shuttle1/2、Head1～4、Socket、Chamber；EJ1N → Index 區不列）寫預設站號
    std::string addr;
    const int listed[] = { tcHotPlate1, tcHotPlate2, tcShuttle1, tcShuttle2, tcHead1, tcHead2, tcHead3, tcHead4, tcSocket, tcChamber };
    for (int ti : listed) addr += KV("HeaterInsAddr_" + Name(ti), ti + 1);
    const std::string want = Steven01Like(Keys71([](int ti) { return ti == tcHotPlate1 ? 1 : ti == tcShuttle1 ? 4 : 2; }, skipCcd2) +
                                          KV("HeaterInsMode", 1) + KV("HeaterInsIndexOpt", 2) + KV("HeaterInsOtherOpt", 2) +
                                          KV("HeaterInsOpt_CCD_2", 3) + addr);
    CheckEq(Cur(), want, "V912 mixed: unchanged save = mode 1, -9999 -> 2, CCD_2=3 appended, listed channels' default stations, "
                         "HEATER_CTRL_TYPE stays 2 (D-6a: most-used among listed)");
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
    USE_16_HEATER = eht16Heater;       // 16 Heaters（KT4H 版）：Index Aa1～Bd2 走溫控 COM 埠 → 列出
    const std::string head = L("[TempCtrl]") + L("COM_PORT=COM2") + L("HEATER_CTRL_TYPE=1");
    UseFile("newfile.ini", head);
    OpenNoWrite("new file");
    Check(Cb("cbHeaterInsIndexOpt")->Enabled, "new file (USE_16_HEATER=1): Index drop-down enabled");
    Check(W906_HeaterInsListed(tcAa1) && W906_HeaterInsListed(tcBd2) && !W906_HeaterInsListed(tcAe1),
          "D-3a: 16 Heaters lists Aa1..Bd2, not Ae1..Bh2");
    Check(ChCb(tcAa1)->Visible && !ChCb(tcAe1)->Visible, "D-3a: Aa1 combo proxy visible, Ae1 hidden");
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
        if (!W906_HeaterInsListed(ti)) continue;
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
    USE_16_HEATER = eht16Heater;
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

    // EJ1N：Index 下拉停用 → Index 跟著其他位置（讀檔與存檔）
    USE_16_HEATER = eht16HeaterEJ1N;
    UseFile("same_locked.ini", L("[TempCtrl]") + L("HEATER_CTRL_TYPE=2") + KV("HeaterInsMode", 0) +
                               KV("HeaterInsIndexOpt", 4) + KV("HeaterInsOtherOpt", 2));
    OpenNoWrite("same locked");
    CheckEqI(g_iHeaterInsIndexOpt, 2, "EJ1N: Index follows Other on read");
    CheckEqI(g_tHeaterInsInfo[tcHead1].GetHeaterInsOpt(), 2, "EJ1N: Head1 = Other's brand");
    SetCb(Cb("cbHeaterInsOtherOpt"), 1);
    Check(SaveNow(), "EJ1N: save passes");
    CheckEq(OptOf(Cur(), "HeaterInsIndexOpt") + OptOf(Cur(), "HeaterInsOpt_Head1"), "11", "EJ1N: save writes Index = Other");
}

// ---------------------------------------------------------------------------
void CaseRefused()
{
    // D-8a：同一個溫控 COM 埠同廠牌同站號 → SaveFlow 擋下，整頁什麼都不寫；D-7a 範圍
    USE_16_HEATER = eht16HeaterEJ1N;
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
    USE_16_HEATER = eht16HeaterEJ1N;                   // 列出：HotPlate1/2、Shuttle1/2、Head1～4、Socket、Chamber（Index 區走 EJ1N 埠）
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
    Check(!W906_HeaterInsListed(tcAa1), "R113 (d): EJ1N -- Aa1 is not on the heater COM port (not listed)");
    SetCb(ChCb(tcAa1), 4); ChEd(tcAa1)->Text = "5";   // Head1 KT4H 預設 5
    SetCb(ChCb(tcShuttle2), 3); ChEd(tcShuttle2)->Text = "5";   // No Heater
    Check(CheckNow(), "R113 (d): unlisted Aa1 DTK4848 5 and No Heater Shuttle2 5 vs Head1 KT4H 5 are allowed");
    SetCb(ChCb(tcAa1), 1); ChEd(tcAa1)->Text = "";
    SetCb(ChCb(tcShuttle2), 1); ChEd(tcShuttle2)->Text = "";

    // (e) 「全機相同」模式：預設站號一起檢查（只有 Index≠其他、其中一邊是 TC401 才撞得到）
    USE_16_HEATER = eht4Heater;                        // Index 下拉可用、Index 區不列出
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
    USE_16_HEATER = eht16HeaterEJ1N;
}

// ---------------------------------------------------------------------------
void CaseHeaterTypeReplayAndExtra()
{
    // Heater Type 重播（BeforeApply → golden rgHeaterTypeClick → W906_HeaterMixTypeClick）：全機相同、Index＝其他＝點的廠牌，不寫檔（Q14＝B）
    USE_16_HEATER = eht16Heater;
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
    CheckEqI(rows ? cJSON_GetArraySize(rows) : -1, 55, "extra: 55 table rows (23 golden drop-downs + 32 Index zones)");
    CheckEqI(grp ? cJSON_GetArraySize(grp) : -1, 36, "extra: Index group = 36 channels (D-2a)");
    Check(hint && cJSON_IsString(hint) && std::string(hint->valuestring).find(kHint) == 0, "extra: D-9a hint text");
    int listed = 0;
    for (const cJSON* r = rows ? rows->child : nullptr; r; r = r->next)
        if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(r, "listed"))) ++listed;
    CheckEqI(listed, 10 + 16, "extra: listed rows with 16 Heaters = 10 visible golden + Aa1..Bd2");
    if (root) cJSON_Delete(root);
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
    USE_16_HEATER = use16;
    UseFile("example.ini", in);
    W906_HeaterMixLoaderSystemSet();
    const bool same = (Cur() == in);
    std::printf("example: %s (USE_16_HEATER=%d) copied to %s\n", src.c_str(), use16, asGeneralPath.c_str());
    std::printf("page open: file %s; mode=%d Index=%d Other=%d locked=%d\n", same ? "unchanged" : "CHANGED",
                g_iHeaterInsMode, g_iHeaterInsIndexOpt, g_iHeaterInsOtherOpt, (int)W906_HeaterInsIndexLocked());
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
    std::vector<RealGuard> guard = { { "D:\\HT9045\\system\\Gerneral.ini", "", false }, { "D:\\HT9045\\config\\config.ini", "", false } };
    for (RealGuard& g : guard) g.had = ReadFile(g.path, &g.bytes);

    g_dir = ScratchDir();
    asGeneralPath = AnsiString((g_dir + "boot.ini").c_str());   // 開機建替身之前就轉走（建替身不讀寫檔，保險）
    WriteFile(asGeneralPath.c_str(), L("[TempCtrl]") + L("HEATER_CTRL_TYPE=1"));
    OpenGeneralIniFile();

    // golden 可見條件（開機算一次）：只開 USE_16_HEATER（Head1～4 看得到），其他全關 → 有下拉而看得到的＝10 個
    USE_16_HEATER = eht16HeaterEJ1N;
    RTC_TemperNumber = 0; INSTALL_HEAT_GUN = 0; INSTALL_ATC_HEAT_GUN = 0; iSocketBaseTempCount = eDut1ea;
    CCD2_TEMPER = false; LB_TEMP = false; Index_ESDAir = false;
    FileRW_HSys_Boot();

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
    }

    for (const RealGuard& g : guard) {
        std::string now;
        const bool had = ReadFile(g.path, &now);
        if (had != g.had || now != g.bytes) { ++g_fail; std::printf("FAIL: real machine file changed: %s\n", g.path.c_str()); }
    }
    std::printf("test_hsys_heater_mix: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : rc;
}
