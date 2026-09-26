// ===========================================================================
//  tests/test_machine_suckers.cpp
//
//  AI(W906-W3-9) 20260924: 週末計畫 W3 第 9 項（InitialSuckerName／InitSucker）的
//  「第三級：值有被維護」測試。A4-6 把 TMySucker 合回 golden 的單一佈局（mykitsuck.h）之後，
//  InitSucker 的 if（IO_CARD_TYPE==NewIO_MN200 || PCI_P64C64）那半才編得起來。
//
//  golden InitSucker（移植 cinitial.cpp:493 起）對 pSuck 裡每個有 SuckerName 的吸嘴查 IO 表三次：
//      <名稱>        -> Sen*  （SenUsing／SenRing／SenIP／SenPort／SenBit／SenType／SenISABase）＋四個時間 ×10
//      <名稱>_On     -> On*   （OnUsing／OnRing／OnIP／OnPort／OnBit／OnType／OnISABase）
//      <名稱>_Off    -> Off*  （同上 7 欄）
//  ISABase 依 On → Off → Sen 的順序覆寫，最後生效的是 SenISABase。
//
//  三層斷言：
//    (1) 全表：machines/HT9050/IO_Table.csv（argv[1]）裡 49 個吸嘴 × 3 組，逐欄比對，零不符；
//        pSuck 內 53 個有名稱的吸嘴，表裡找不到的 4 個（OutArm2SuckAA／AB、CheckKitSuck_1／_2）照 golden 只 sprintf 不跳訊息。
//    (2) 手算：三顆吸嘴的值直接寫死，數字是從 CSV 逐列讀出來的（不經過程式解析），擋「兩邊用同一個解析器所以一起錯」。
//    (3) Enable：模擬組態（SOFT_SIMULTE）golden 強制全關、兩個 DelayTime 歸 0；出貨組態 Enable = SenUsing!="" && 表的 Enable==1。
//        HT9050 的 49 列 Enable 全是 0，所以另外在 cwd 產生一份「只把 InArmSuckA 的 Enable 改成 1」的表驗 true 那一臂
//        （不改版控的機台表）。
//
//  用法：test_machine_suckers <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"
#include "cinitial.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "mykitsuck.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_machine_suckers.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static TIODATA* Row(const AnsiString& alias)
{
    HSys.mapIOTableIter = HSys.mapIOTable.find(alias);
    if (HSys.mapIOTableIter == HSys.mapIOTable.end()) return 0;
    return HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
}
static AnsiString UseOf(const TIODATA* r) { return (r->iPort == -1) ? AnsiString("") : AnsiString(r->iPort); }

static std::string g_env;   // putenv keeps the pointer
static void LoadTable(const std::string& path)
{
    g_env = std::string("W906_IOTABLE_PATH=") + path;
    HT9045_TEST_PUTENV(const_cast<char*>(g_env.c_str()));
    HSys.LoadIoData();
}

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_machine_suckers <IO_Table.csv>\n"); return 2; }
    LoadTable(argv[1]);
    std::printf("IO_Table rows: %d\n", (int)HSys.IOTable.size());
    CHECK(HSys.IOTable.size() > 0);

    IO_CARD_TYPE     = NewIO_MN200;      // golden 的 if 那半（筆電 Gerneral.ini 的實際值）
    USE_PICKER_COUNT = 1;                // 非 ep16Picker：InArmSuckA..H 命名（golden cinitial.cpp:316-319）
    INDEX_SUCKER_TYPE = 1;               // 負壓：TestSuck 的 OnAlarmTime ÷10 封頂 50（golden :520-528）
    InitSucker();

    // ---- (1) 全表 ----
    int named = 0, notInTable = 0, senRows = 0, senBad = 0, onRows = 0, onBad = 0, offRows = 0, offBad = 0;
    for (int i = 0; i < pSuck->Count; ++i) {
        TMySucker* S = (TMySucker*)pSuck->Items[i];
        if (S->SuckerName == AnsiString("")) continue;
        ++named;
        CHECK(S->SensorName  == S->SuckerName);
        CHECK(S->OnPortName  == S->SensorName + "_On");
        CHECK(S->OffPortName == S->SensorName + "_Off");

        const TIODATA* rs = Row(S->SuckerName);
        const TIODATA* ron = Row(S->OnPortName);
        const TIODATA* roff = Row(S->OffPortName);
        if (!rs && !ron && !roff) ++notInTable;
        if (rs) {
            ++senRows;
            const bool ok = S->SenUsing == UseOf(rs) && S->SenRing == rs->iLane && S->SenIP == rs->iIP &&
                            S->SenPort == rs->iPort && S->SenBit == rs->iBit && S->SenType == rs->iInType &&
                            S->SenISABase == rs->iISABase && S->ISABase == rs->iISABase &&
                            S->OffAlarmTime == rs->iOffAlarmTime * 10;
            if (!ok) { if (senBad < 5) std::printf("  Sen mismatch: %s\n", S->SuckerName.c_str()); ++senBad; }
        }
        if (ron) {
            ++onRows;
            const bool ok = S->OnUsing == UseOf(ron) && S->OnRing == ron->iLane && S->OnIP == ron->iIP &&
                            S->OnPort == ron->iPort && S->OnBit == ron->iBit && S->OnType == ron->iInType &&
                            S->OnISABase == ron->iISABase;
            if (!ok) { if (onBad < 5) std::printf("  On mismatch: %s\n", S->OnPortName.c_str()); ++onBad; }
        }
        if (roff) {
            ++offRows;
            const bool ok = S->OffUsing == UseOf(roff) && S->OffRing == roff->iLane && S->OffIP == roff->iIP &&
                            S->OffPort == roff->iPort && S->OffBit == roff->iBit && S->OffType == roff->iInType &&
                            S->OffISABase == roff->iISABase;
            if (!ok) { if (offBad < 5) std::printf("  Off mismatch: %s\n", S->OffPortName.c_str()); ++offBad; }
        }
    }
    std::printf("suckers named %d (not in table %d) | Sen %d rows %d bad | On %d rows %d bad | Off %d rows %d bad\n",
                named, notInTable, senRows, senBad, onRows, onBad, offRows, offBad);
    CHECK(named == 53);                  // In 8 + Out 8 + F 16 + B 16 + OutArm2 2 + CatchTray 1 + CheckKit 2
    CHECK(notInTable == 4);              // OutArm2SuckAA／AB、CheckKitSuck_1／_2
    CHECK(senRows == 49 && senBad == 0);
    CHECK(onRows == 49 && onBad == 0);
    CHECK(offRows == 49 && offBad == 0);

    // ---- (2) 手算（CSV 逐列讀出） ----
    {   // InArmSuck.Suck[0][0] = "InArmSuckA"：Sen 第 571 列、On 第 583 列、Off 第 582 列
        TMySucker& S = InArmSuck.Suck[0][0];
        CHECK(S.SuckerName == "InArmSuckA");
        CHECK(S.SenUsing == "128" && S.SenRing == 0 && S.SenIP == 32 && S.SenPort == 128 && S.SenBit == 0 && S.SenType == 1 && S.SenISABase == 3);
        CHECK(S.OnUsing == "17" && S.OnIP == 32 && S.OnPort == 17 && S.OnBit == 0 && S.OnISABase == 3);
        CHECK(S.OffUsing == "16" && S.OffIP == 32 && S.OffPort == 16 && S.OffBit == 0 && S.OffISABase == 3);
        CHECK(S.OnAlarmTime == 50 && S.OffAlarmTime == 100);      // 5×10、10×10
        CHECK(S.ISABase == 3);
    }
    {   // InArmSuck.Suck[1][0] = "InArmSuckB"：第 575 列（Port 132 Bit 4；OnDelayTime 1）
        TMySucker& S = InArmSuck.Suck[1][0];
        CHECK(S.SuckerName == "InArmSuckB");
        CHECK(S.SenPort == 132 && S.SenBit == 4);
    }
    {   // FTestSuck.Suck[0][0] = "FTestSuckAA"：Sen 第 304 列、On 第 340 列；OnAlarm 50×10=500 → 負壓 ÷10 = 50（封頂 50）
        TMySucker& S = FTestSuck.Suck[0][0];
        CHECK(S.SuckerName == "FTestSuckAA");
        CHECK(S.SenIP == 64 && S.SenPort == 128 && S.SenBit == 0 && S.SenISABase == 3);
        CHECK(S.OnIP == 64 && S.OnPort == 17 && S.OnBit == 0);
        CHECK(S.OnAlarmTime == 50);
    }
    {   // CatchTraySuck.Suck[0][0] = "CatchSuck"：Sen 第 12 列（ISABase 0）、On 第 29 列、Off 第 32 列；不是 TestSuck ⇒ 500 不除
        TMySucker& S = CatchTraySuck.Suck[0][0];
        CHECK(S.SuckerName == "CatchSuck");
        CHECK(S.SenIP == 1 && S.SenPort == 1 && S.SenBit == 0 && S.SenISABase == 0);
        CHECK(S.OnPort == 3 && S.OnBit == 1 && S.OffPort == 3 && S.OffBit == 2);
        CHECK(S.OnAlarmTime == 500);
        CHECK(S.ISABase == 0);
    }
    CHECK(CheckKitSuck.Suck[0][0].SuckerName == "CheckKitSuck_1");
    CHECK(OutArm2Suck.Suck[0][1].SuckerName == "OutArm2SuckAB");
    CHECK(InArmSuckBackup.Suck[0][0].SuckerName == "InArmSuckA");         // InitialSuckerName 同步命名備份格
    CHECK(FTestSuckBackup.Suck[1][7].SuckerName == "FTestSuckBH");
    CHECK(FTestSuckBackup.Suck[0][0].SenPort == 128);                      // InitSucker 尾段 CopyKitSuck 備份端（golden :821）

    // ---- (2b) W3-9b：格數沒設時備份只到 [0][0]；SetMyKitSuckItemAmount（GATE n4-1 已解）之後涵蓋整張格線 ----
    //   CopyKitSuck 以來源的 iMaxRow／iMaxCol 為邊界（mykitsuck.cpp:2818）；ctor 給 1×1，唯一寫它的是 SetItemAmount。
    //   wb_serve 在 golden ctor 位置（main.cpp:2121）先呼叫 SetMyKitSuckItemAmount，比 InitialHandler→InitSucker 早。
    CHECK(FTestSuck.iMaxRow == 1 && FTestSuck.iMaxCol == 1);               // 本測試到這裡還沒設格數
    CHECK(FTestSuckBackup.Suck[1][3].SenPort != 135);                      // ⇒ 備份沒帶到 [1][3]（W3-9 當時的樣子）
    SetMyKitSuckItemAmount();                                              // Type_HT9045 ⇒ F/BTestSuck 2×4；ep8Picker ⇒ In/OutArmSuck 2×4
    CHECK(FTestSuck.iMaxRow == 2 && FTestSuck.iMaxCol == 4);
    CHECK(InArmSuck.iMaxRow == 2 && InArmSuck.iMaxCol == 4);
    CHECK(TestSocket.iMaxRow == 4 && TestSocket.iMaxCol == 8);
    InitSucker();
    // FTestSuckBD（第 305 列）／InArmSuckH（第 572 列）：Port 135 Bit 7
    CHECK(FTestSuck.Suck[1][3].SuckerName == "FTestSuckBD" && FTestSuck.Suck[1][3].SenPort == 135 && FTestSuck.Suck[1][3].SenBit == 7);
    CHECK(FTestSuckBackup.Suck[1][3].SenPort == 135 && FTestSuckBackup.Suck[1][3].SenBit == 7);
    CHECK(InArmSuck.Suck[1][3].SuckerName == "InArmSuckH" && InArmSuckBackup.Suck[1][3].SenPort == 135 && InArmSuckBackup.Suck[1][3].SenBit == 7);

    // ---- (2c) AI(W906-HT9050-GRID) 20260925：HT9050（Type_HT9050，NEW_MAX_Index_Col=8，database.cpp:517-518）----
    //   NB2 R12 RA-01：它原本落在 SetMyKitSuckItemAmount 的 else（2×4），而 IO 表綁的是 2×8 ⇒ 備份只到 col 3，
    //   ChangeSite 的還原（c22dcb12）會把 ctor 的 0 接線蓋進有效站。修正後格線 2×8，還原不會清掉任何一顆。
    //   32 顆 F/B 測試頭吸嘴在 HT9050 表裡 SenPort 全部非 0（awk 量），所以「還原後有 0」就是被蓋掉。
    MachineTypeChoice = Type_HT9050;
    NEW_MAX_Index_Col = 8;
    SetMyKitSuckItemAmount();
    CHECK(FTestSuck.iMaxRow == 2 && FTestSuck.iMaxCol == 8 && FTestSuckBackup.iMaxCol == 8 && BTestSuck.iMaxCol == 8);
    InitSucker();
    CHECK(FTestSuck.Suck[1][7].SuckerName == "FTestSuckBH" && FTestSuckBackup.Suck[1][7].SenPort == 135 && FTestSuckBackup.Suck[1][7].SenBit == 7);   // 第 CSV 列 FTestSuckBH
    TestIF_File.iTestMode = DualSite;   // 1x2：ChangeSite 的一般分支（cinitial.cpp:17784 起）任何機種都會走，而且拷 Backup[0][5..7]
    ChangeSite();
    {
        int zeroed = 0;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 8; ++j) {
                if (FTestSuck.Suck[i][j].SenPort == 0) ++zeroed;
                if (BTestSuck.Suck[i][j].SenPort == 0) ++zeroed;
            }
        std::printf("after ChangeSite (Type_HT9050, iTestMode=%d): index nozzles with zero wiring = %d\n", TestIF_File.iTestMode, zeroed);
        CHECK(zeroed == 0);
    }

    // ---- (2d) AI(W906-HT9050-FAM) 20260925：HT9050＝HT9046 家族（使用者 20260925，RULINGS_20260925 第 5 條）----
    //   ChangeSite 裡列了 Type_HT9046 的 6 處加了 Type_HT9050：:17603 SingleSite（H 類）、:17830 QualSite1X4、
    //   :18525／:18556／:18598 Octal 12／16 kit、:18640 Octal 80 kit（G 類）。
    //   量法：同一份 2×8 備份格，每個情境跑三次 ChangeSite ——
    //     Type_HT9046（答案）、Type_HT9050（修正後）、Type_HT7080（ChangeSite 全函式沒提到它＝修正前 HT9050 走的路）。
    //   CopySuck 連 SuckerName 一起拷（mykitsuck.cpp:2853），所以每格的名字就是「接線從備份哪一格來」。
    //   斷言：HT9050 與 HT9046 的 32 格逐格同名；會換位的情境 HT7080 至少一格不同（對照組有鑑別力）；
    //   QualSite1X4 非 NS 兩邊都是原位（HT9046 分支是空的、else 是 [r][c]→[r][c]），照實斷言「相同」。
    {
        struct Case { const char* name; int mode; bool o12; bool o16; bool o80; double pitch; bool remaps; };
        const Case cases[] = {
            {"SingleSite (:17603)",             SingleSite,  false, false, false, 20.0, true },
            {"QualSite1X4 (:17830)",            QualSite1X4, false, false, false, 20.0, false},
            {"_8Site2X4+Octal12 (:18525)",      _8Site2X4,   true,  false, false, 20.0, true },
            {"_16Site4X4+Octal12 (:18525)",     _16Site4X4,  true,  false, false, 20.0, true },
            {"_8Site2X4+Octal16 p=40 (:18556)", _8Site2X4,   false, true,  false, 40.0, true },
            {"_8Site2X4+Octal16 p=50 (:18598)", _8Site2X4,   false, true,  false, 50.0, true },
            {"_8Site2X4+Octal80 (:18640)",      _8Site2X4,   false, false, true,  20.0, true },
        };
        auto run = [](int type) {
            MachineTypeChoice = type;
            ChangeSite();
            std::string s[32];
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 8; ++j) {
                    s[i * 8 + j]      = FTestSuck.Suck[i][j].SuckerName.c_str();
                    s[16 + i * 8 + j] = BTestSuck.Suck[i][j].SuckerName.c_str();
                }
            std::string out;
            for (int k = 0; k < 32; ++k) { out += s[k]; out += ','; }
            return out;
        };
        auto slot = [](const std::string& sig, int k) {   // 第 k 格（0..15 F、16..31 B）的名字
            size_t b = 0;
            for (int n = 0; n < k; ++n) b = sig.find(',', b) + 1;
            return sig.substr(b, sig.find(',', b) - b);
        };
        for (const Case& c : cases) {
            TestIF_File.iTestMode = c.mode;
            TestIF_File.bNS7000kit = false;  TestIF_File.bNS7000CS = false;
            TestIF_File.bOctal_12Kit = c.o12; TestIF_File.bOctal_16Kit = c.o16; TestIF_File.bOctal_80Kit = c.o80;
            TestIF_File.dSiteXPitch = c.pitch;
            const std::string a = run(Type_HT9046), b = run(Type_HT9050), old = run(Type_HT7080);
            int moved = 0;
            std::string diff;
            for (int k = 0; k < 32; ++k)
                if (slot(b, k) != slot(old, k)) {
                    ++moved;
                    char buf[96];
                    std::snprintf(buf, sizeof buf, " %s[%d][%d]:%s->%s", k < 16 ? "F" : "B", (k % 16) / 8, k % 8,
                                  slot(old, k).c_str(), slot(b, k).c_str());
                    diff += buf;
                }
            std::printf("(2d) %-34s HT9050==HT9046:%s  vs old path: %d slots changed%s\n", c.name, a == b ? "yes" : "NO", moved, diff.c_str());
            CHECK(a == b);
            CHECK((old != a) == c.remaps);
        }
        // 手算（golden cinitial.cpp:12187-12191 SingleSite；:13154-13170 Octal 80 kit）
        TestIF_File.iTestMode = SingleSite;
        TestIF_File.bOctal_12Kit = TestIF_File.bOctal_16Kit = TestIF_File.bOctal_80Kit = false;
        {
            const std::string b = run(Type_HT9050);
            CHECK(slot(b, 0) == "FTestSuckAF" && slot(b, 5) == "FTestSuckAA");          // F[0][0]<-Backup[0][5]、F[0][5]<-Backup[0][0]
            CHECK(slot(b, 16) == "BTestSuckBF" && slot(b, 16 + 13) == "BTestSuckAA");   // B[0][0]<-Backup[1][5]、B[1][5]<-Backup[0][0]
        }
        TestIF_File.iTestMode = _8Site2X4;  TestIF_File.bOctal_80Kit = true;
        {
            const std::string b = run(Type_HT9050);
            CHECK(slot(b, 0) == "FTestSuckAB" && slot(b, 3) == "FTestSuckAH" && slot(b, 8) == "FTestSuckBB");   // F[0][0]<-[0][1]、F[0][3]<-[0][7]、F[1][0]<-[1][1]
        }
        TestIF_File.bOctal_80Kit = false;  TestIF_File.dSiteXPitch = 0.0;
    }
    MachineTypeChoice = Type_HT9045;   // 後面 (3) 以預設機種繼續
    TestIF_File.iTestMode = 0;
    NEW_MAX_Index_Col = 8;

    // ---- (3) Enable ----
    int enabled = 0;
    for (int i = 0; i < pSuck->Count; ++i) {
        TMySucker* S = (TMySucker*)pSuck->Items[i];
        if (S->Enable || S->OnEnable || S->OffEnable) ++enabled;
    }
    CHECK(enabled == 0);                 // 兩組態都是 0：模擬＝golden 強制關；出貨＝HT9050 表 Enable 全 0
#ifdef SOFT_SIMULTE
    CHECK(InArmSuck.Suck[0][0].OnDelayTime == 0 && InArmSuck.Suck[0][0].OffDelayTime == 0);
#else
    CHECK(InArmSuck.Suck[0][0].OnDelayTime == 100 && InArmSuck.Suck[0][0].OffDelayTime == 10);   // 10×10、1×10
    CHECK(InArmSuck.Suck[1][0].OnDelayTime == 10);                                                // 第 575 列 OnDelayTime=1
#endif

    // true 那一臂：只把 InArmSuckA（感測列）的 Enable 欄改成 1
    {
        std::ifstream in(argv[1], std::ios::binary);
        std::stringstream ss; ss << in.rdbuf();
        std::string all = ss.str(), out, line;
        std::istringstream is(all);
        int patched = 0;
        while (std::getline(is, line)) {
            const std::string key = "Sucker,InArmSuckA,";
            if (line.compare(0, key.size(), key) == 0) {
                // 欄位：IOType,Alias,Lane,ModuleType,IP,Port,Bit,InType,ISABase,Enable,...（Enable 是第 10 欄，索引 9）
                size_t pos = 0; int col = 0;
                while (col < 9 && pos != std::string::npos) { pos = line.find(',', pos); if (pos != std::string::npos) ++pos; ++col; }
                if (pos != std::string::npos && pos < line.size() && line[pos] == '0') { line[pos] = '1'; ++patched; }
            }
            out += line; out += "\n";
        }
        CHECK(patched == 1);
        const std::string tmp = "suckers_enable1_IO_Table.csv";
        std::ofstream(tmp.c_str(), std::ios::binary) << out;
        LoadTable(tmp);
        InitSucker();
        TMySucker& S = InArmSuck.Suck[0][0];
#ifdef SOFT_SIMULTE
        CHECK(S.Enable == false && S.OnEnable == false && S.OffEnable == false);   // golden :497-502 模擬一律關
#else
        CHECK(S.Enable == true && S.OnEnable == true && S.OffEnable == true);
        CHECK(InArmSuck.Suck[1][0].Enable == false && InArmSuck.Suck[1][0].OnEnable == false);   // 沒改的那顆仍被強制關
#endif
        std::remove(tmp.c_str());
    }

    // ---- (4) AI(W906-W3-12c) 20260925: UpdateMyKitSuckDelayTimeToProd 寫進吸嘴的值（W3-12 解開的 n4-5a／5b）----
    //   NB2 R17 RW-01 量到：這支函式在 W3-12 之後仍然「0 個活呼叫者」（DoSetupSystemToProd 的 GATE n2-2 還閘著、
    //   AutoClean 打到檔內 static 空殼）。呼叫邊由本次開閘＋拆殼接上；這一段鎖的是函式本體的值（第三級）。
    //   手算：golden cinitial.cpp:6566-6594／:6620-6635 的 ×100；選二進位可精確表示的數，避開 x87 截斷。
    {
        bRunAutoClean = false;
        ArmSpeed[InArm].dCTAirOn  = 0.25;  ArmSpeed[OutArm].dCTAirOn  = 0.5;
        ArmSpeed[InArm].iDestroyAgainCount = 3;  ArmSpeed[OutArm].iDestroyAgainCount = 4;
        ArmSpeed[InArm].dDestroyAgainTime  = 0.125;  ArmSpeed[OutArm].dDestroyAgainTime = 0.75;
        ArmSpeed[IndexArm].iDestroyAgainCount = 5;  ArmSpeed[IndexArm].dDestroyAgainTime = 0.375;
        InArmSuck.Suck[0][0].DestroyAgainCount = -1;  FTestSuck.Suck[1][7].DestroyAgainCount = -1;   // 哨兵：沒寫到就看得出來
        UpdateMyKitSuckDelayTimeToProd();
        CHECK(InArmSuck.Suck[0][0].OffDelayTime == 25 && OutArmSuck.Suck[0][0].OffDelayTime == 50);
        CHECK(InArmSuck.Suck[0][0].DestroyAgainCount == 3 && OutArmSuck.Suck[0][0].DestroyAgainCount == 4);
        CHECK(InArmSuck.Suck[0][0].DestroyAgainTime == 12.5 && OutArmSuck.Suck[0][0].DestroyAgainTime == 75.0);
        CHECK(FTestSuck.Suck[1][7].DestroyAgainCount == 5 && BTestSuck.Suck[1][7].DestroyAgainTime == 37.5);   // NEW_MAX_Index_Col=8 那一格
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
