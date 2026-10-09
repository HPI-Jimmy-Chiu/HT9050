// =============================================================================
//  test_w190_heaterins.cpp  --  AI(W906-W190-HEATERINS) 20261009 (Ifor01)
//
//  W-190 第一張（Ifor 1009 14:0x 定位置；Steven 1009 13:5x「913 的修正翻成 cpp、放進 cpp 現有結構，不搬 BCB 檔或檔案配置」）：
//  逐通道溫控器廠牌表與小工具（golden 913 MachineTypeUtility.cpp:14-295＋HandlerSys.cpp:32-49）從 FileRW/HSys.cpp（只編進 wb_serve）
//  搬到 cmydef.cpp 檔尾（ht9045_globals），宣告照舊在 FileRW/HSys_Heater.h。本測試不編任何 FileRW 來源 —— 連得起來本身就是第一項證據
//  （搬之前這支會找不到 IsValEqual_HeaterInsOpt／g_tHeaterInsInfo，跟 golden 913 rs232.cpp／bthermo.cpp 照翻後會遇到的一樣）。
//
//    [1] 表：71 列（tcTotalCount）、名稱與存檔鍵照 golden、23 列 occupy（頁面 23 組替身）、預設廠牌全是 KT4H；廠牌字串 5 個
//    [2] IsValEqual_HeaterInsOpt：逐通道比；通道或廠牌超出範圍＝false；移植樹的 5（EJ1N）／6（DTM）在 golden 的範圍外＝false
//    [3] IsNoHeaterMachine：只看 TC401HeaterControl（[TempCtrl] HEATER_CTRL_TYPE），跟表無關
//    [4] IsAllValEqual／IsAllSame／GetStaTable／IsExistVal：bJustForShowItem 只算 occupy 的 23 列
//    [5] 顯示序表（golden HandlerSys.cpp:32-49，跟著搬來）：GetFirstHeaterInsOpt(true) 讀 g_vecHeaterTypeIdxForShow；TypeIdxToShowIdx
//    [6] GetCtrlItemVisProp：照機型旗標決定通道看不看得到
//    [7] THeaterInsInfo::SetCtrlItemProp 經 MachineType.h 帶進來的 vclcompat TLabel／TComboBox 設 Visible
//    [8] 讀原始碼（argv[1]＝移植樹根目錄，唯讀）：cmydef.cpp 檔尾定義這些、FileRW/HSys.cpp 不再定義、沒有照 BCB 開 MachineTypeUtility.cpp、
//        MachineType.h 沒有搬進宣告
//  只動記憶體，不讀寫任何檔。反向驗證見 MR 說明。
// =============================================================================
#include "FileRW/HSys_Heater.h"           // 宣告（照舊在這個檔；本體在 cmydef.cpp 檔尾）
#include "cmydef.h"                       // TC401HeaterControl, USE_16_HEATER, RTC_TemperNumber, INSTALL_HEAT_GUN, iSocketBaseTempCount ...
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; } else { g_fail++; std::printf("  FAIL [test_w190_heaterins.cpp:%d] %s\n", __LINE__, msg); } } while (0)

static std::string Slurp(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
// drop // and /* */ comments and string contents' line comments are kept as-is (good enough for definition pins)
static std::string CodeOnly(const std::string& s)
{
    std::string o; o.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        if (s.compare(i, 2, "//") == 0) { while (i < s.size() && s[i] != '\n') ++i; continue; }
        if (s.compare(i, 2, "/*") == 0) { size_t e = s.find("*/", i + 2); i = (e == std::string::npos) ? s.size() : e + 2; continue; }
        if (s[i] == '"') { o += s[i++]; while (i < s.size() && s[i] != '"') { if (s[i] == '\\' && i + 1 < s.size()) o += s[i++]; o += s[i++]; } if (i < s.size()) o += s[i++]; continue; }
        o += s[i++];
    }
    return o;
}
// #if 0 ... #endif blocks (top level) removed -- FileRW/HSys.cpp keeps golden originals there
static std::string DropIf0(const std::string& s)
{
    std::string o; std::istringstream in(s); std::string l; int depth = 0;
    while (std::getline(in, l)) {
        std::string t = l; size_t a = t.find_first_not_of(" \t"); t = (a == std::string::npos) ? "" : t.substr(a);
        if (depth == 0 && t.compare(0, 5, "#if 0") == 0) { depth = 1; continue; }
        if (depth > 0) {
            if (t.compare(0, 3, "#if") == 0) ++depth;
            else if (t.compare(0, 6, "#endif") == 0) --depth;
            continue;
        }
        o += l; o += '\n';
    }
    return o;
}
static void SetAll(int opt) { for (int i = 0; i < eHeaterType_Count; ++i) g_tHeaterInsInfo[i].m_iHeaterInsOpt = opt; }

int main(int argc, char** argv)
{
    std::printf("[W190_HeaterIns] heater brand table + helpers at the end of cmydef.cpp (ht9045_globals; no FileRW source in this executable)\n");
#if !EN_HEATER_SHEET
    CHECK(false, "EN_HEATER_SHEET must be 1 from FileRW/HSys_Heater.h");
#else
    // ---------------------------------------------------------------- [1]
    std::printf(" 1. table\n");
    CHECK(eHeaterType_Count == 71 && tcTotalCount == 71, "71 rows = tcTotalCount (golden enum eTempControll)");
    CHECK(eHeaterInsOpt_Count == 5, "eHeaterInsOpt_Count 5 (TC401/KT4H/E5DC/NoHeater/DTK4848)");
    int occ = 0, kt4h = 0;
    for (int i = 0; i < eHeaterType_Count; ++i) { if (g_tHeaterInsInfo[i].GetOccupy()) ++occ; if (g_tHeaterInsInfo[i].GetHeaterInsOpt() == KT4H) ++kt4h; }
    CHECK(occ == 23, "23 occupied rows (the page's 23 proxies)");
    CHECK(kt4h == 71 && DEFAULT_HEATER_INS_OPT == KT4H, "every row starts at DEFAULT_HEATER_INS_OPT = KT4H");
    CHECK(g_tHeaterInsInfo[tcHotPlate1].GetShowName() == "HotPlate1" && g_tHeaterInsInfo[tcHotPlate1].GetSaveName() == "HeaterInsOpt_HotPlate1", "row 0 HotPlate1");
    CHECK(g_tHeaterInsInfo[tcCCD_2].GetSaveName() == "HeaterInsOpt_CCD_2" && g_tHeaterInsInfo[tcCCD_2].GetOccupy(), "row 52 CCD_2 occupied");
    CHECK(g_tHeaterInsInfo[tcLBDown].GetSaveName() == "HeaterInsOpt_LBDown" && !g_tHeaterInsInfo[tcLBDown].GetOccupy(), "row 70 LBDown not occupied");
    CHECK(!g_tHeaterInsInfo[tcBase1].GetOccupy() && !g_tHeaterInsInfo[11].GetOccupy() && g_tHeaterInsInfo[tcDUT4].GetOccupy(), "Base1 / Aa1 off, DUT4 on");
    CHECK(g_HeaterInsOptStr[0] == "TC401" && g_HeaterInsOptStr[1] == "Panasonic KT4H" && g_HeaterInsOptStr[2] == "Omron E5DC"
          && g_HeaterInsOptStr[3] == "No Heater" && g_HeaterInsOptStr[4] == "DTK4848", "g_HeaterInsOptStr = golden's 5 brands");
    CHECK(TC401 == 0 && KT4H == 1 && E5DC == 2 && NoHeater == 3 && DTK4848 == 4, "brand codes match the string table");

    // ---------------------------------------------------------------- [2]
    std::printf(" 2. IsValEqual_HeaterInsOpt\n");
    SetAll(KT4H);
    g_tHeaterInsInfo[tcShuttle1].m_iHeaterInsOpt = E5DC;
    CHECK(IsValEqual_HeaterInsOpt(tcShuttle1, E5DC) && !IsValEqual_HeaterInsOpt(tcShuttle1, KT4H), "Shuttle1 = E5DC, not KT4H");
    CHECK(IsValEqual_HeaterInsOpt(tcHotPlate1, KT4H) && !IsValEqual_HeaterInsOpt(tcHotPlate1, E5DC), "HotPlate1 stays KT4H (per channel)");
    CHECK(!IsValEqual_HeaterInsOpt(-1, KT4H) && !IsValEqual_HeaterInsOpt(eHeaterType_Count, KT4H), "channel out of range = false");
    CHECK(!IsValEqual_HeaterInsOpt(tcHotPlate1, -1) && !IsValEqual_HeaterInsOpt(tcHotPlate1, eHeaterInsOpt_Count), "brand out of range = false");
    g_tHeaterInsInfo[tcHead1].m_iHeaterInsOpt = 6;   // port-only Delta DTM code (FileRW/HSys_Heater.h E029)
    CHECK(!IsValEqual_HeaterInsOpt(tcHead1, 6) && !IsValEqual_HeaterInsOpt(tcHead1, KT4H), "port code 6 (DTM) is outside golden's range: never equal");
    TC401HeaterControl = E5DC;
    CHECK(IsValEqual_HeaterInsOpt(tcHotPlate1, KT4H), "the table wins over TC401HeaterControl (EN_HEATER_SHEET=1)");

    // ---------------------------------------------------------------- [3]
    std::printf(" 3. IsNoHeaterMachine\n");
    TC401HeaterControl = NoHeater; SetAll(KT4H);
    CHECK(IsNoHeaterMachine(), "HEATER_CTRL_TYPE = No Heater -> true");
    TC401HeaterControl = KT4H; SetAll(NoHeater);
    CHECK(!IsNoHeaterMachine(), "table all No Heater but HEATER_CTRL_TYPE = KT4H -> false (machine-level only)");

    // ---------------------------------------------------------------- [4]
    std::printf(" 4. all-equal / statistics\n");
    SetAll(KT4H);
    int same = -1, first = -1;
    CHECK(IsAllValEqual_HeaterInsOpt(KT4H, false, &same) && same == KT4H, "all KT4H");
    CHECK(IsAllSame_HeaterInsOpt(false, &first) && first == KT4H, "IsAllSame -> KT4H");
    g_tHeaterInsInfo[11].m_iHeaterInsOpt = E5DC;   // Aa1, not occupied
    CHECK(!IsAllValEqual_HeaterInsOpt(KT4H, false) && IsAllValEqual_HeaterInsOpt(KT4H, true), "a hidden row differs: all rows no, shown rows yes");
    int sta[eHeaterType_Count];
    CHECK(GetStaTable_HeaterInsOpt(sta, true) && sta[KT4H] == 23 && sta[E5DC] == 0, "shown rows: 23 x KT4H");
    CHECK(GetStaTable_HeaterInsOpt(sta, false) && sta[KT4H] == 70 && sta[E5DC] == 1, "all rows: 70 x KT4H + 1 x E5DC");
    g_tHeaterInsInfo[12].m_iHeaterInsOpt = 6;      // Ab1 = port-only code
    CHECK(!GetStaTable_HeaterInsOpt(sta, false) && GetStaTable_HeaterInsOpt(sta, true), "an out-of-range code in a hidden row: all rows false, shown rows true");
    CHECK(!GetStaTable_HeaterInsOpt(NULL, true), "NULL table -> false");
    g_bGetStaTable_HeaterInsOpt_Already = false;
    CHECK(IsExistVal_HeaterInsOpt(KT4H, true) && !IsExistVal_HeaterInsOpt(E5DC, true) && g_bGetStaTable_HeaterInsOpt_Already, "IsExistVal builds the cached table once");

    // ---------------------------------------------------------------- [5]
    std::printf(" 5. show-order table (golden HandlerSys.cpp:32-49, moved with it)\n");
    SetAll(KT4H);
    g_tHeaterInsInfo[tcShuttle2].m_iHeaterInsOpt = DTK4848;
    g_vecHeaterTypeIdxForShow.clear(); g_mapHeaterTypeIdxToHeaterShowIdx.clear(); g_mapHeaterShowIdxToHeaterTypeIdx.clear();
    CHECK(GetFirstHeaterInsOpt(true) == -1 && GetFirstHeaterInsOpt(false) == KT4H, "empty show list: -1; all rows: row 0");
    CHECK(TypeIdxToShowIdx(tcShuttle2) == INVALID_INT_VAL_NEG && ShowIdxToTypeIdx(0) == INVALID_INT_VAL_NEG, "empty maps -> INVALID_INT_VAL_NEG (-9999)");
    g_vecHeaterTypeIdxForShow.push_back(tcShuttle2);
    g_mapHeaterTypeIdxToHeaterShowIdx[tcShuttle2] = 0; g_mapHeaterShowIdxToHeaterTypeIdx[0] = tcShuttle2;
    CHECK(GetFirstHeaterInsOpt(true) == DTK4848, "first shown row = Shuttle2 -> DTK4848");
    CHECK(TypeIdxToShowIdx(tcShuttle2) == 0 && ShowIdxToTypeIdx(0) == tcShuttle2, "maps answer both ways");
    CHECK(INVALID_INT_VAL_NEG == -9999 && INVALID_INT_VAL_POS == 9999, "golden cmydef.h:168-169 values (FileRW/HSys_Heater.h)");
    g_vecHeaterTypeIdxForShow.clear(); g_mapHeaterTypeIdxToHeaterShowIdx.clear(); g_mapHeaterShowIdxToHeaterTypeIdx.clear();

    // ---------------------------------------------------------------- [6]
    std::printf(" 6. GetCtrlItemVisProp\n");
    CHECK(!GetCtrlItemVisProp(-1) && !GetCtrlItemVisProp(eHeaterType_Count) && !GetCtrlItemVisProp(tcBase1), "out of range / not occupied -> hidden");
    CHECK(GetCtrlItemVisProp(tcChamber) && GetCtrlItemVisProp(tcShuttle1), "Chamber / Shuttle1 always shown");
    USE_16_HEATER = eht4Heater;  CHECK(!GetCtrlItemVisProp(tcHead1), "Head1 hidden on a 4-heater index");
    USE_16_HEATER = eht16Heater; CHECK(GetCtrlItemVisProp(tcHead1), "Head1 shown on a 16-heater index");
    USE_16_HEATER = eht4Heater;
    RTC_TemperNumber = 0; CHECK(!GetCtrlItemVisProp(tcCCD) && !GetCtrlItemVisProp(tcCCD_2), "no RTC sensor: CCD / CCD_2 hidden");
    RTC_TemperNumber = 2; CHECK(GetCtrlItemVisProp(tcCCD) && GetCtrlItemVisProp(tcCCD_2), "two RTC sensors: both shown");
    RTC_TemperNumber = 1;
    INSTALL_HEAT_GUN = 0; CHECK(!GetCtrlItemVisProp(tcHeatGun1), "no heat gun: hidden");
    INSTALL_HEAT_GUN = 1; CHECK(GetCtrlItemVisProp(tcHeatGun2), "heat gun: shown");
    INSTALL_HEAT_GUN = 0;
    const int sock0 = iSocketBaseTempCount;
    iSocketBaseTempCount = eDut2ea; CHECK(GetCtrlItemVisProp(tcDUT2) && !GetCtrlItemVisProp(tcDUT3), "2 DUT: DUT2 shown, DUT3 hidden");
    iSocketBaseTempCount = eDut4ea; CHECK(GetCtrlItemVisProp(tcDUT3) && GetCtrlItemVisProp(tcDUT4), "4 DUT: DUT3 / DUT4 shown");
    iSocketBaseTempCount = sock0;

    // ---------------------------------------------------------------- [7]
    std::printf(" 7. THeaterInsInfo with vclcompat controls\n");
    {
        TLabel lb; TComboBox cb; lb.Visible = true; cb.Visible = true;
        THeaterInsInfo t(true, "X", "HeaterInsOpt_X", E5DC);
        t.SetCtrlItemProp(false, &lb, &cb);
        CHECK(!lb.Visible && !cb.Visible && !t.GetShow() && t.GetCtrlItem_Lb() == &lb && t.GetCtrlItem_Cb() == &cb, "SetCtrlItemProp(false) hides both");
        t.SetCtrlItemVis(true);
        CHECK(lb.Visible && cb.Visible && t.GetHeaterInsOpt() == E5DC, "SetCtrlItemVis(true) shows both");
        THeaterInsInfo d;
        CHECK(!d.GetOccupy() && d.GetHeaterInsOpt() == KT4H && d.GetCtrlItem_Lb() == NULL, "default row: not occupied, KT4H, no controls");
    }
    SetAll(KT4H); TC401HeaterControl = KT4H;
#endif

    // ---------------------------------------------------------------- [8]
    std::printf(" 8. source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string hs   = CodeOnly(DropIf0(Slurp(root + "/FileRW/HSys.cpp")));
        const std::string cmd  = Slurp(root + "/cmydef.cpp");
        const std::string cmdc = CodeOnly(cmd);
        const std::string mt   = CodeOnly(Slurp(root + "/MachineType.h"));
        CHECK(!hs.empty() && !cmd.empty() && !mt.empty(), "sources readable");
        const char* const defs[] = { "bool IsValEqual_HeaterInsOpt(", "bool IsNoHeaterMachine(", "THeaterInsInfo g_tHeaterInsInfo[",
                                     "int GetFirstHeaterInsOpt(", "bool GetCtrlItemVisProp(", "std::vector<int>   g_vecHeaterTypeIdxForShow;",
                                     "int TypeIdxToShowIdx(", "AnsiString g_HeaterInsOptStr[" };
        const size_t tc401 = cmdc.find("int TC401HeaterControl");
        for (size_t i = 0; i < sizeof(defs) / sizeof(defs[0]); ++i) {
            CHECK(hs.find(defs[i]) == std::string::npos, (std::string("FileRW/HSys.cpp no longer defines ") + defs[i]).c_str());
            const size_t at = cmdc.find(defs[i]);
            CHECK(at != std::string::npos && tc401 != std::string::npos && at > tc401, (std::string("cmydef.cpp defines (after TC401HeaterControl) ") + defs[i]).c_str());
        }
        CHECK(hs.find("void HeaterInsOpt_Read()") != std::string::npos, "HeaterInsOpt_Read stays on the page (Q34 / Q15 read)");
        CHECK(cmd.find("#include \"FileRW/HSys_Heater.h\"") != std::string::npos, "cmydef.cpp takes the declarations from FileRW/HSys_Heater.h");
        std::ifstream mtuf((root + "/MachineTypeUtility.cpp").c_str());
        CHECK(!mtuf.good(), "no BCB-layout MachineTypeUtility.cpp in the cpp tree (Steven 1009 13:5x)");
        CHECK(mt.find("IsValEqual_HeaterInsOpt") == std::string::npos && mt.find("EN_HEATER_SHEET") == std::string::npos,
              "MachineType.h unchanged: the declarations stay in FileRW/HSys_Heater.h");
    } else {
        std::printf("  (no argv[1]: source pins skipped)\n");
    }

    std::printf("[W190_HeaterIns] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
