// ===========================================================================
//  tests/test_init_dio_status.cpp
//
//  AI(W906-DIO) 20260925: TfMain::InitDIOStstus（cDIOStatus.cpp，golden 906 main.cpp:24357-24375）＋開機輔助
//  W906_BootInitDIOStstus（golden FormShow main.cpp:10106-10108）的「第三級：值有被維護」測試。
//
//  (1) 逐欄複製：TTLCfg 12 個欄位各填不同的值（cModeName 128 個位元組全填，第 20 個是 NUL、後面還有資料 ——
//      擋「用 strcpy／strncpy 抄」），呼叫後 Prod.DIOCfg 逐欄等於呼叫前的 TTLCfg、整個 TTL_DATA memcmp 相同；
//      TTLCfg 本身不變；Prod 裡緊鄰 DIOCfg 的前後欄位（cprod.h:596 iXTrayEmpty1／:598 dFailureLimit[0]）不被波及。
//      兩輪：bNeedOn=false＋iDutType=DUTPosPluse；bNeedOn=true＋iDutType=DUTNONE，iSTLogicMode 1／0。
//  (2) TTL 輸出：18 個點（SwStart0-7／SwDut0-7／SwClear2／SwClear6）在測試裡掛成 ePCI1203、注入記錄用的後端
//      （MyLaneIO.SetBackend），逐點量寫出的值 —— 照 golden 與 TMySwitch 的語意：
//        Start：Type=iSTLogicMode 後 Off()：Type 1 → 寫 0、Type 0 → 寫 1（負邏輯）
//        Dut  ：DUTPosPluse → On() → 1；其他 → Off() → 0（測試把 Dut 的 Type 設 1）
//        Clear：OnOff(bNeedOn) → bNeedOn（Type 1）
//  (3) 真表：argv[1]＝machines/HT9050/IO_Table.csv，InitialSwitch() 之後量這 18 點的 Enable，再跑開機輔助與
//      InitDIOStstus(true)，量寫出次數。HT9050 的表沒有這 18 列 ⇒ 出貨組態 Enable 全 false、0 次寫出。
//  模擬組態（SOFT_SIMULTE）：golden 本體整段 #ifndef SOFT_SIMULTE ⇒ 斷言「Prod.DIOCfg、SW 的 Type 都沒變、0 次寫出」；
//  (3) 另斷言 golden cinitial.cpp:1657-1676 的「模擬時 SwStart0-7／SwDut0-7 強制 Enable」＝16 點。
//
//  不讀寫任何機台檔：InitDIOStstus 本身不碰檔案（出貨組態會呼叫 TTLLog，但它要先建 log 物件才寫，這裡沒建，見 cDIOStatus.cpp）；(3) 只讀 argv[1] 的 CSV
//  （HSys.LoadIoData 的 W906_IOTABLE_PATH 唯讀接縫，同 test_machine_suckers）。
//
//  用法：test_init_dio_status <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "forms/fMain.h"
#include "cprod.h"
#include "cmydef.h"
#include "myswitch.h"
#include "MyLaneIo.h"
#include "IOBackend.h"
#include "database.h"
#include "cinitial.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

extern void W906_BootInitDIOStstus();   // cDIOStatus.cpp

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_init_dio_status.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- 記錄用的 IO 後端 --------------------------------------------------------
struct Write1 { int ring, ip, port, bit, value; };
class TRecBackend : public TIOBackend {
public:
    std::vector<Write1> w;
    int WriteBit(int Ring, int IP, int Port, int Bit, int Value) override
    {
        Write1 x = { Ring, IP, Port, Bit, Value };
        w.push_back(x);
        return 0;                                   // Acm 的 SUCCESS；TLaneIO 對 ePCI1203 把 0 換成 +1
    }
};
static TRecBackend g_rec;

// 18 個 TTL 點：[0..7] Start、[8..15] Dut、[16] SwClear2、[17] SwClear6
static int Pt(int k)
{
    if (k < 8)  return TTL_StartData[k];
    if (k < 16) return TTL_Dut[k - 8];
    return (k == 16) ? SwClear2 : SwClear6;
}
static const int kTestIP = 2;                       // 測試用位址：Ring 0／IP 2／Port k/8／Bit k%8（都在 IO_MAX* 範圍內）
static int LastValue(int k)                          // 該點最後一次寫出的值；沒寫過回 -1
{
    int v = -1;
    for (size_t i = 0; i < g_rec.w.size(); ++i)
        if (g_rec.w[i].ring == 0 && g_rec.w[i].ip == kTestIP && g_rec.w[i].port == k / 8 && g_rec.w[i].bit == k % 8)
            v = g_rec.w[i].value;
    return v;
}
static void BindFixturePoints()
{
    for (int k = 0; k < 18; ++k) {
        TMySwitch& s = SW[Pt(k)];
        s.Enable = true;  s.ISABase = ePCI1203;
        s.Ring = 0;  s.IP = kTestIP;  s.Port = k / 8;  s.Bit = k % 8;
        s.Type = 1;                                 // Start 的 Type 會被 InitDIOStstus 改成 iSTLogicMode
        s.OutValue = false;
    }
}

// ---- TTLCfg fixture -----------------------------------------------------------
static void FillTTLCfg(int seed, int iSTLogicMode, int iDutType)
{
    for (int k = 0; k < (int)sizeof(TTLCfg.cModeName); ++k)
        TTLCfg.cModeName[k] = (char)('A' + (k * 7 + seed) % 26);
    TTLCfg.cModeName[19] = '\0';                   // 中間的 NUL：後面 108 個位元組也要被抄（memcpy，不是字串複製）
    TTLCfg.iSTLogicMode   = iSTLogicMode;
    TTLCfg.iStartType     = 100 + seed;
    TTLCfg.iOneSTChannel  = 200 + seed;
    TTLCfg.iSTPluseWidth  = 0x80000000u + (unsigned)seed;   // unsigned 的高位元
    TTLCfg.iDutType       = iDutType;
    TTLCfg.iDutBfOnTime   = 300 + seed;
    TTLCfg.iDutAfOffTime  = 400 + seed;
    TTLCfg.iCateLogicMode = 500 + seed;
    TTLCfg.iCateBitLength = 600 + seed;
    TTLCfg.iCateParity    = 700 + seed;
    TTLCfg.iCateDataType  = 800 + seed;
}

static const int    kSentBefore = 0x13579BDF;
static const double kSentAfter  = -4321.25;
static void PoisonProd()
{
    std::memset(&Prod.DIOCfg, 0xCD, sizeof(Prod.DIOCfg));
    Prod.iXTrayEmpty1     = kSentBefore;            // cprod.h:596（DIOCfg 前一欄）
    Prod.dFailureLimit[0] = kSentAfter;             // cprod.h:598（DIOCfg 後一欄）
}
static bool ProdStillPoisoned()
{
    const unsigned char* p = (const unsigned char*)&Prod.DIOCfg;
    for (size_t i = 0; i < sizeof(Prod.DIOCfg); ++i) if (p[i] != 0xCD) return false;
    return true;
}

static void CheckCopied(const TTL_DATA& want)
{
    CHECK(std::memcmp(Prod.DIOCfg.cModeName, want.cModeName, sizeof(want.cModeName)) == 0);
    CHECK(Prod.DIOCfg.iSTLogicMode   == want.iSTLogicMode);
    CHECK(Prod.DIOCfg.iStartType     == want.iStartType);
    CHECK(Prod.DIOCfg.iOneSTChannel  == want.iOneSTChannel);
    CHECK(Prod.DIOCfg.iSTPluseWidth  == want.iSTPluseWidth);
    CHECK(Prod.DIOCfg.iDutType       == want.iDutType);
    CHECK(Prod.DIOCfg.iDutBfOnTime   == want.iDutBfOnTime);
    CHECK(Prod.DIOCfg.iDutAfOffTime  == want.iDutAfOffTime);
    CHECK(Prod.DIOCfg.iCateLogicMode == want.iCateLogicMode);
    CHECK(Prod.DIOCfg.iCateBitLength == want.iCateBitLength);
    CHECK(Prod.DIOCfg.iCateParity    == want.iCateParity);
    CHECK(Prod.DIOCfg.iCateDataType  == want.iCateDataType);
    CHECK(std::memcmp(&Prod.DIOCfg, &want, sizeof(TTL_DATA)) == 0);   // 整個結構（含欄位間的填充位元組）
}

// 一輪 (1)＋(2)
static void Round(int seed, bool bNeedOn, int iSTLogicMode, int iDutType)
{
    std::printf("-- round seed=%d bNeedOn=%d iSTLogicMode=%d iDutType=%d\n", seed, (int)bNeedOn, iSTLogicMode, iDutType);
    FillTTLCfg(seed, iSTLogicMode, iDutType);
    const TTL_DATA want = TTLCfg;
    PoisonProd();
    BindFixturePoints();
    g_rec.w.clear();  TestIF.iTestType = TTL_MODE + 1;   // AI(W906-W150) 20261008: TTLLog is live now (cDIOStatus.cpp:71) and golden TTLLog reads SW[...].Status(), which reloads OutValue from IOOutBitStatus -- false for these ePCI1203 Ring-0 fixture points (MyLaneIo.cpp:455-458); outside TTL_MODE it returns at once.  TTLLog itself: St02_W150LogSplit2 [3]

    fMain->InitDIOStstus(bNeedOn);

    CHECK(std::memcmp(&TTLCfg, &want, sizeof(TTL_DATA)) == 0);        // 來源不變
    CHECK(Prod.iXTrayEmpty1 == kSentBefore);                            // 前鄰欄不被波及
    CHECK(Prod.dFailureLimit[0] == kSentAfter);                         // 後鄰欄不被波及
#ifndef SOFT_SIMULTE
    CheckCopied(want);
    CHECK(g_rec.w.size() == 18);                                        // 8 Start ＋ 8 Dut ＋ SwClear2 ＋ SwClear6，各一次
    for (int k = 0; k < 8; ++k) {
        CHECK(SW[TTL_StartData[k]].Type == iSTLogicMode);
        CHECK(SW[TTL_StartData[k]].OutValue == false);                  // Off()
        CHECK(LastValue(k) == (iSTLogicMode ? 0 : 1));                  // Type 1：Off → IOBitOff；Type 0：Off → IOBitOn
        const bool dutOn = (iDutType == DUTPosPluse);
        CHECK(SW[TTL_Dut[k]].OutValue == dutOn);
        CHECK(SW[TTL_Dut[k]].Type == 1);                                // golden 只改 Start 的 Type
        CHECK(LastValue(8 + k) == (dutOn ? 1 : 0));
    }
    CHECK(SW[SwClear2].OutValue == bNeedOn);
    CHECK(SW[SwClear6].OutValue == bNeedOn);
    CHECK(LastValue(16) == (bNeedOn ? 1 : 0));
    CHECK(LastValue(17) == (bNeedOn ? 1 : 0));
#else
    CHECK(ProdStillPoisoned());                                         // golden #ifndef SOFT_SIMULTE：沒有複製
    CHECK(g_rec.w.empty());                                             // 也沒有任何輸出
    for (int k = 0; k < 18; ++k) {
        CHECK(SW[Pt(k)].Type == 1);
        CHECK(SW[Pt(k)].OutValue == false);
    }
#endif
}

static std::string g_env;   // putenv 保留指標

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_init_dio_status <IO_Table.csv>\n"); return 2; }
#ifndef SOFT_SIMULTE
    std::printf("config: SHIP (SOFT_SIMULTE undefined) -- golden body active\n");
#else
    std::printf("config: SIM (SOFT_SIMULTE defined) -- golden body is empty; expecting no effect\n");
#endif
    CHECK(sizeof(Prod.DIOCfg) == sizeof(TTLCfg));                       // golden memcpy 用 sizeof(TTLCfg)，兩邊同型別
    CHECK(fMain != 0);

    MyLaneIO.SetBackend(&g_rec);

    // ---- (1)＋(2) fixture ----
    Round(1, false, 1, DUTPosPluse);
    Round(2, true,  0, DUTNONE);
    Round(3, true,  1, DUTPosLevel);                                    // 非 pulse 的另一個值：Dut 仍然 Off

    // ---- (3) 真表：machines/HT9050/IO_Table.csv ----
    g_env = std::string("W906_IOTABLE_PATH=") + argv[1];
    HT9045_TEST_PUTENV(const_cast<char*>(g_env.c_str()));
    HSys.LoadIoData();
    std::printf("IO_Table rows: %d (%s)\n", (int)HSys.IOTable.size(), argv[1]);
    CHECK(HSys.IOTable.size() > 0);

    IO_CARD_TYPE      = NewIO_MN200;                                    // golden InitialSwitch 的 if 那半（筆電 Gerneral.ini 的實際值）
    iControlPanelMode = 0;
    InitialSwitch();                                                    // 先跑它：名字是它裡面的 InitialSwitchName 給的
    int enabled = 0, inTable = 0;
    for (int k = 0; k < 18; ++k) {
        CHECK(SW[Pt(k)].Name != AnsiString(""));                        // InitialSwitchName 有給這 18 點名字
        if (HSys.mapIOTable.find(SW[Pt(k)].Name) != HSys.mapIOTable.end()) ++inTable;
        if (SW[Pt(k)].Enable) ++enabled;
    }
    std::printf("HT9050 table: %d of 18 TTL points have an IO_Table row; %d of 18 Enable after InitialSwitch\n",
                inTable, enabled);
    CHECK(inTable == 0);                                                // 量測：HT9050 的表沒有 SwStart*/SwDut*/SwClear2/6
#ifndef SOFT_SIMULTE
    CHECK(enabled == 0);                                                // ⇒ 出貨組態全部 Enable=false
#else
    CHECK(enabled == 16);                                               // golden cinitial.cpp:1657-1676 模擬時強制 SwStart0-7／SwDut0-7
    CHECK(!SW[SwClear2].Enable && !SW[SwClear6].Enable);
#endif

    FillTTLCfg(4, 1, DUTPosPluse);
    const TTL_DATA want = TTLCfg;
    PoisonProd();
    g_rec.w.clear();
    W906_BootInitDIOStstus();                                           // 開機那一處（golden FormShow :10106-10108）
    //AI(W906-DIO-BOOTGATE) 20260925: 開機那一處已閘住（開機讀 DIO 檔沒翻，見 cDIOStatus.cpp）⇒ 兩個組態都不碰 Prod.DIOCfg  [RETIRED 20260928 AI(W906-GB-P8) 20260928 (St02-E helper): the boot gate is lifted (B2, cDIOStatus.cpp:97) -- ship now copies at boot too, as golden FormShow :10107, sim still does not (body empty)]
    const bool bootCopied = std::memcmp(&Prod.DIOCfg, &want, sizeof(TTL_DATA)) == 0;  const bool bootUntouched = ProdStillPoisoned();  PoisonProd();   // AI(W906-GB-P8) 20260928 (St02-E helper): B2, measured here and asserted per configuration below, then re-poisoned so the save call is measured alone
    fMain->InitDIOStstus(true);                                         // DIO 設定頁存檔那一處（golden sbDioSetClick :27670-27672）
#ifndef SOFT_SIMULTE
    CheckCopied(want);  CHECK(bootCopied && !bootUntouched);                                                  // 存檔那一處照 golden 活著：TTLCfg → Prod.DIOCfg
#else
    CHECK(ProdStillPoisoned());  CHECK(bootUntouched && !bootCopied);                                         // 模擬：本體是空的
#endif
    std::printf("HT9050 table: IO writes from boot + save calls = %d\n", (int)g_rec.w.size());
    CHECK(g_rec.w.empty());                                             // 出貨：18 點都沒綁；模擬：本體是空的

    std::printf("%s: %d checks, %d failed\n", g_fail ? "FAIL" : "PASS", g_total, g_fail);
    return g_fail ? 1 : 0;
}
