// ===========================================================================
//  tests/test_ioweb_watch.cpp
//
//  AI(W906-ONSITE-1) 20260926: ONSITE-1 的純邏輯測試 —— 兩件現場量測前要先在桌上驗的事：
//    [1] tools/ioweb_watch.h DiffAndAdvance：`ioweb_probe <outDir> <seconds> watch`
//        判斷「哪些 DI 位元變了」。讀失敗的那一輪不比、保留上一次的好值（中間讀失敗
//        也不會漏掉開門）；卡片對應表改了站號的 slot 重新取基準、不報變化。
//    [2] RowsForDiBit / AddressMatches / RowCaveat / 文字與 CSV 格式：哪些 IO_Table 列
//        對到這個位元（ISABase 3：Lane = ring、IP = station、Port = stationChan*8 + bit），
//        Enable=0 也列出；Lane 空白、輸出型別的列列出但附說明；另一個 ring 的列不列；
//        沒有列時印 "(no ePCI1203 row)"（20260926 審查後改字：只查 ISABase 3 的列，
//        而這張表的門有列、只是在 MotionNet 位址上）。
//    [2b] 審查後新增：BitColumnCandidates（舊式「Port = byte、Bit = bit」讀法，另列、
//        不混進主清單）、DoorRowsOffPci1203（名字含 door、不是 ISABase 3 的列）、
//        CSV 新欄 bitColumnCandidates 與舊表頭判別、TorqueLimitLine 一行格式。
//    [3] 與網頁本身的規則對帳：讀版控的 machines/HT9050/IO_Table.csv（argv[1]），每一個
//        ISABase 3 的輸入列，用 JsonBridge/ChanIoPoints.cpp ResolveIoPoint 驗「網頁讀的就是
//        這個位元」，再驗 RowsForDiBit 在這個位元找得到它、在同 byte 的其他 7 個位元找不到它。
//    [4] Pci1203TorqueLimitReadText 與 Pci1203AxisSample 的 60E0h/60E1h 讀回欄位預設值：
//        "ok" / "readFailed" / "notRead" 三態分得開（SUCCESS 是 0，只看 ret 分不出來）。
//
//  不開卡、不帶 HAVE_PCI1203（廠商呼叫根本沒編進來），只讀版控的 IO_Table.csv，不寫任何檔案。
//  用法：test_ioweb_watch <IO_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "JsonBridge/ChanIo.h"
#include "database.h"
#include "EtherCAT/Pci1203Monitor.h"
#include "tools/ioweb_watch.h"

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

using namespace ht9045::iowatch;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_ioweb_watch.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static DiByte B(int slot, int ring, int station, int chan, unsigned char v, bool valid = true)
{
    DiByte b; b.valid = valid; b.ring = ring; b.station = station; b.stationChan = chan; b.slot = slot; b.byteData = v;
    return b;
}

static IoRow R(int row, const char* alias, const char* type, int dir, int isa, int lane, int ip, int port,
               int bitCol, int enable)
{
    IoRow r; r.row = row; r.alias = alias; r.type = type; r.dir = dir; r.isaBase = isa; r.lane = lane;
    r.ip = ip; r.port = port; r.bitCol = bitCol; r.enable = enable; r.inType = 1;
    return r;
}

static bool Has(const std::vector<int>& v, int x)
{
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i] == x) return true;
    return false;
}

static ht9045::sjson::IoByteSample S(int ring, int station, int chan, unsigned char b)
{
    ht9045::sjson::IoByteSample s; s.valid = true; s.ring = ring; s.station = station; s.stationChan = chan; s.byteData = b;
    return s;
}

int main(int argc, char** argv)
{
    // ---------------------------------------------------------------- [1]
    std::printf("[1] DiffAndAdvance\n");
    {
        std::vector<DiByte> base, cur;
        DiffStats st;
        cur.push_back(B(0, 1, 80, 0, 0x00));
        cur.push_back(B(1, 1, 80, 1, 0x0F));
        cur.push_back(B(2, 1, 81, 0, 0x00, false));   // not read on the first poll
        std::vector<BitChange> ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.empty());                             // the first poll is a baseline, never a change
        CHECK(st.adopted == 2 && st.invalid == 1 && st.compared == 0 && st.readdressed == 0);
        CHECK(base.size() == 3);

        cur[0].byteData = 0x21;                        // bits 0 and 5 rise
        cur[1].byteData = 0x0B;                        // bit 2 falls
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.size() == 3);
        CHECK(st.compared == 2 && st.invalid == 1);
        if (ch.size() == 3) {
            CHECK(ch[0].slot == 0 && ch[0].bit == 0 && ch[0].oldV == 0 && ch[0].newV == 1);
            CHECK(ch[1].slot == 0 && ch[1].bit == 5 && ch[1].oldV == 0 && ch[1].newV == 1);
            CHECK(ch[2].slot == 1 && ch[2].bit == 2 && ch[2].oldV == 1 && ch[2].newV == 0);
            CHECK(ch[0].ring == 1 && ch[0].station == 80 && ch[0].stationChan == 0);
        }

        ch = DiffAndAdvance(base, cur, &st);           // nothing changed
        CHECK(ch.empty() && st.compared == 2);

        // a door opened DURING a failed read is not lost: the baseline is the last GOOD value
        cur[0].valid = false; cur[0].byteData = 0xFF;  // failed read, garbage byte
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.empty() && st.invalid == 2);
        cur[0].valid = true; cur[0].byteData = 0x20;   // bit 0 fell while the read was failing
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.size() == 1 && ch[0].slot == 0 && ch[0].bit == 0 && ch[0].oldV == 1 && ch[0].newV == 0);

        // the slot that was never valid becomes a baseline when it first reads -- not a change
        cur[2].valid = true; cur[2].byteData = 0x80;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.empty() && st.adopted == 1);
        cur[2].byteData = 0x00;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.size() == 1 && ch[0].slot == 2 && ch[0].bit == 7 && ch[0].newV == 0);

        // re-attribution by the card map (station changed): re-based, never reported as a door
        cur[1].station = 81; cur[1].byteData = 0xF0;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.empty() && st.readdressed == 1);
        cur[1].byteData = 0xF1;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.size() == 1 && ch[0].station == 81 && ch[0].bit == 0);

        // a ring change on the same slot is a re-address too
        cur[1].ring = 0;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.empty() && st.readdressed == 1);

        // cur shrinks: the missing slots keep their baseline; growing back compares against it
        std::vector<DiByte> shrunk(cur.begin(), cur.begin() + 1);   // not `small`: MinGW.org's rpcndr.h #defines it (oracle line)
        ch = DiffAndAdvance(base, shrunk, &st);
        CHECK(ch.empty() && base.size() == 3 && st.compared == 1);
        cur[2].byteData = 0x01;
        ch = DiffAndAdvance(base, cur, &st);
        CHECK(ch.size() == 1 && ch[0].slot == 2 && ch[0].bit == 0);

        CHECK(DiffAndAdvance(base, cur, 0).empty());   // stats pointer is optional
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] RowsForDiBit / text\n");
    {
        std::vector<IoRow> rows;
        rows.push_back(R(10, "SnSafeDoor1",     "Sensor", kDirIn,  3, 1, 2, 25, 1, 1));   // 0: resolves
        rows.push_back(R(11, "SnSafeDoorIndex", "Sensor", kDirIn,  3, 1, 2, 25, 1, 0));   // 1: same point, Enable=0
        rows.push_back(R(12, "SwWrongType",     "Switch", kDirOut, 3, 1, 2, 25, 1, 1));   // 2: output-typed
        rows.push_back(R(13, "SnNoLane",        "Sensor", kDirIn,  3, -1, 2, 25, 1, 1));  // 3: Lane empty
        rows.push_back(R(14, "SnRing0",         "Sensor", kDirIn,  3, 0, 2, 25, 1, 1));   // 4: the other ring
        rows.push_back(R(15, "SnMotionNet",     "Sensor", kDirIn,  0, 1, 2, 25, 1, 1));   // 5: not ePCI1203
        rows.push_back(R(16, "SnNextBit",       "Sensor", kDirIn,  3, 1, 2, 26, 2, 1));   // 6: bit 2
        rows.push_back(R(17, "SnBitColOff",     "Sensor", kDirIn,  3, 1, 2, 25, 3, 1));   // 7: Bit column disagrees
        rows.push_back(R(18, "",                "Weird",  kDirUnknown, 3, 1, 2, 25, 1, 1)); // 8: unknown type
        rows.push_back(R(19, "SnNoPort",        "Sensor", kDirIn,  3, 1, 2, -1, 1, 1));   // 9: no Port

        const std::vector<int> h = RowsForDiBit(rows, 1, 2, 3, 1);          // Port 25 = chan 3, bit 1
        CHECK(h.size() == 6);
        CHECK(Has(h, 0) && Has(h, 1) && Has(h, 2) && Has(h, 3) && Has(h, 7) && Has(h, 8));
        CHECK(!Has(h, 4) && !Has(h, 5) && !Has(h, 6) && !Has(h, 9));
        CHECK(h.size() >= 2 && h[0] == 0 && h[1] == 1);                    // table order

        const std::vector<int> r0 = RowsForDiBit(rows, 0, 2, 3, 1);         // ring 0: the ring-0 row and the laneless one
        CHECK(Has(r0, 4) && Has(r0, 3) && !Has(r0, 0));
        const std::vector<int> fl = RowsForDiBit(rows, -1, 2, 3, 1);        // flat read: lane not compared (ResolveIoPoint)
        CHECK(Has(fl, 0) && Has(fl, 4));
        CHECK(RowsForDiBit(rows, 1, 2, 3, 2) == std::vector<int>(1, 6));   // Port 26 = chan 3 bit 2
        CHECK(RowsForDiBit(rows, 1, -1, 3, 1).empty());                     // unattributed byte: nothing
        CHECK(RowsForDiBit(rows, 1, 2, -1, 1).empty());
        CHECK(RowsForDiBit(rows, 1, 2, 3, 8).empty());
        CHECK(RowsForDiBit(rows, 1, 9, 3, 1).empty());

        CHECK(PageReadsHere(rows[0]) && PageReadsHere(rows[1]));
        CHECK(!PageReadsHere(rows[2]) && !PageReadsHere(rows[3]) && !PageReadsHere(rows[8]));
        CHECK(std::strcmp(RowCaveat(rows[0]), "") == 0);
        CHECK(std::strstr(RowCaveat(rows[2]), "output-typed") != 0);
        CHECK(std::strstr(RowCaveat(rows[3]), "Lane empty") != 0);
        CHECK(std::strstr(RowCaveat(rows[8]), "neither input nor output") != 0);

        CHECK(DescribeRow(rows[0]) == "row 10 SnSafeDoor1 [Sensor] Lane=1 IP=2 Port=25 Bit=1 Enable=1 InType=1");
        CHECK(DescribeRow(rows[1]).find("Enable=0: the page shows it as disabled") != std::string::npos);
        CHECK(DescribeRow(rows[7]).find("Bit column 3 differs from Port%8 = 1") != std::string::npos);
        CHECK(DescribeRow(rows[8]).find("(no alias) [Weird]") != std::string::npos);

        CHECK(RowsText(rows, std::vector<int>()) == "(no ePCI1203 row)");
        CHECK(std::strcmp(NoRowText(), "(no ePCI1203 row)") == 0);
        std::vector<int> two; two.push_back(0); two.push_back(6);
        CHECK(RowsText(rows, two) == DescribeRow(rows[0]) + " | " + DescribeRow(rows[6]));

        CHECK(CsvQuote("a\"b,c") == "\"a\"\"b,c\"");
        BitChange c = { 25, 1, 2, 3, 1, 0, 1 };
        CHECK(ChangeCsvLine("2026-09-26 10:00:00", 12.44, 62ul, c, "(no ePCI1203 row)", "") ==
              "\"2026-09-26 10:00:00\",12.4,62,1,2,3,1,25,0,1,\"(no ePCI1203 row)\",\"\"\r\n");
        CHECK(ChangeCsvLine("s", 1.0, 5ul, c, "a", "b,c") == "\"s\",1.0,5,1,2,3,1,25,0,1,\"a\",\"b,c\"\r\n");
        CHECK(std::string(ChangeCsvHeader()) ==
              "session,tSec,poll,ring,station,stationChan,bit,slot,old,new,ioTableRows,bitColumnCandidates\r\n");
        CHECK(ChangeHeadline(12.44, c) == "[   12.4 s] DI ring 1 station 2 chan 3 bit 1 (slot 25): 0 -> 1");

        // the log's header: this version's (with or without CR/LF) is accepted, the earlier build's is not
        CHECK(IsChangeCsvHeader("session,tSec,poll,ring,station,stationChan,bit,slot,old,new,ioTableRows,bitColumnCandidates"));
        CHECK(IsChangeCsvHeader("session,tSec,poll,ring,station,stationChan,bit,slot,old,new,ioTableRows,bitColumnCandidates\r"));
        CHECK(IsChangeCsvHeader(ChangeCsvHeader()));
        CHECK(!IsChangeCsvHeader("session,tSec,poll,ring,station,stationChan,bit,slot,old,new,ioTableRows\r"));
        CHECK(!IsChangeCsvHeader(""));
    }

    // ---------------------------------------------------------------- [2b]
    std::printf("[2b] Bit-column candidates / door rows off the 1203 / torque-limit line\n");
    {
        std::vector<IoRow> rows;
        rows.push_back(R(20, "SnLegacy",      "Sensor", kDirIn,  3, 1, 32, 1, 2, 0));   // 0: legacy: byte 1, bit 2
        rows.push_back(R(21, "SnPageRule",    "Sensor", kDirIn,  3, 1, 32, 10, 2, 1));  // 1: page rule: 10 = chan 1 bit 2
        rows.push_back(R(22, "SnLegacyRing0", "Sensor", kDirIn,  3, 0, 32, 1, 2, 1));   // 2: legacy, the other ring
        rows.push_back(R(23, "SnLegacyMN",    "Sensor", kDirIn,  0, 1, 32, 1, 2, 1));   // 3: legacy shape, not ePCI1203
        rows.push_back(R(24, "SnNoBitCol",    "Sensor", kDirIn,  3, 1, 32, 1, -1, 1));  // 4: Bit column empty
        rows.push_back(R(25, "SnZero",        "Sensor", kDirIn,  3, 1, 32, 0, 0, 1));   // 5: Port 0 Bit 0: both readings agree
        rows.push_back(R(26, "SnLegacyNoLane","Sensor", kDirIn,  3, -1, 32, 1, 2, 1));  // 6: legacy, Lane empty

        // ring 1 station 32 chan 1 bit 2: the page rule names row 1; the legacy reading names rows 0 and 6
        const std::vector<int> prim = RowsForDiBit(rows, 1, 32, 1, 2);
        const std::vector<int> cand = BitColumnCandidates(rows, 1, 32, 1, 2);
        CHECK(prim == std::vector<int>(1, 1));
        CHECK(cand.size() == 2 && Has(cand, 0) && Has(cand, 6));
        CHECK(!Has(cand, 1) && !Has(cand, 2) && !Has(cand, 3) && !Has(cand, 4));
        CHECK(LegacyAddressMatches(rows[0], 1, 32, 1, 2) && !AddressMatches(rows[0], 1, 32, 1, 2));
        CHECK(LegacyAddressMatches(rows[2], 0, 32, 1, 2) && !LegacyAddressMatches(rows[2], 1, 32, 1, 2));
        CHECK(LegacyAddressMatches(rows[0], -1, 32, 1, 2));                       // flat read: lane not compared
        CHECK(!LegacyAddressMatches(rows[0], 1, 32, 1, 8) && !LegacyAddressMatches(rows[0], 1, -1, 1, 2));
        // a row both readings put on the same bit is shown ONCE, in the primary list
        CHECK(Has(RowsForDiBit(rows, 1, 32, 0, 0), 5) && !Has(BitColumnCandidates(rows, 1, 32, 0, 0), 5));
        CHECK(!LegacyAddressMatches(rows[5], 1, 32, 0, 0));   //AI(W906-ONSITE-1) 20260926: Bit == Port%8 is a page-form row, never a legacy candidate
        {   // a page-form row with Port 3 / Bit 3 (page: chan 0 bit 3) must NOT turn up at chan 3 bit 3 (review 20260926)
            std::vector<IoRow> pr;
            pr.push_back(R(27, "SnPagePort3", "Sensor", kDirIn, 3, 1, 32, 3, 3, 1));
            CHECK(BitColumnCandidates(pr, 1, 32, 3, 3).empty());
            CHECK(Has(RowsForDiBit(pr, 1, 32, 0, 3), 0));
        }
        CHECK(CandidatesText(rows, std::vector<int>()).empty());
        CHECK(CandidatesText(rows, cand) == RowsText(rows, cand));
        CHECK(std::strstr(BitColumnCandidatesLabel(), "Bit-column candidates") == BitColumnCandidatesLabel());
        CHECK(std::strstr(BitColumnCandidatesLabel(), "NOT how the web page reads them") != 0);

        // door rows that are not ISABase 3
        CHECK(AliasHasDoor("SnSafeDoor1") && AliasHasDoor("SnHeaterDOOR") && AliasHasDoor("door") &&
              AliasHasDoor("C_SafeDoorIndexLock"));
        CHECK(!AliasHasDoor("SnDoo") && !AliasHasDoor("") && !AliasHasDoor("SnMotorPower"));
        std::vector<IoRow> d;
        d.push_back(R(30, "SnSafeDoor1",   "Sensor", kDirIn, 3, 1, 2, 25, 1, 1));   // 0: on the 1203 -> not listed
        d.push_back(R(31, "SnSafeDoor2",   "Sensor", kDirIn, 0, 1, 4, 0, 1, 0));    // 1: MotionNet -> listed
        d.push_back(R(32, "SnMotorPower",  "Sensor", kDirIn, 0, 1, 2, 30, 6, 1));   // 2: not a door
        d.push_back(R(33, "SnHeaterdoor3", "Sensor", kDirIn, 1, -1, 32, 0, 2, 1));  // 3: other ISABase, lower-case -> listed
        const std::vector<int> off = DoorRowsOffPci1203(d);
        CHECK(off.size() == 2 && off[0] == 1 && off[1] == 3);
        CHECK(DescribeOffRow(d[1]) == "row 31 SnSafeDoor2 [Sensor] ISABase=0 Lane=1 IP=4 Port=0 Bit=1 Enable=0");

        // one axis's torque-limit line
        TrqLimHalf h[2];
        h[0].valid = true;  h[0].val = 3000; h[0].ret = 0ul; h[0].readText = "ok";
        h[1].valid = false; h[1].val = 0;    h[1].ret = 0x80000009ul; h[1].readText = "readFailed"; h[1].retText = "SDO abort";
        CHECK(TorqueLimitLine(3, 2, 0, 0x000u, h) ==
              "ax3 station 2 axis A: 60E0h pos=3000 (300.0 %) ok ret=0x00000000 | 60E1h neg=- readFailed ret=0x80000009 \"SDO abort\"");
        CHECK(TorqueLimitLine(4, 2, 1, 0x800u, h).find("68E0h pos=3000") != std::string::npos);
        CHECK(TorqueLimitLine(4, 2, 1, 0x800u, h).find("axis B: ") != std::string::npos);
        CHECK(TorqueLimitLine(4, 2, 1, 0x800u, h).find("68E1h neg=-") != std::string::npos);
        TrqLimHalf n[2];
        n[0].readText = "notRead"; n[1].readText = "notRead";
        CHECK(TorqueLimitLine(0, -1, -1, 0x000u, n) ==
              "ax0 station -1 axis ?: 60E0h pos=- notRead ret=- | 60E1h neg=- notRead ret=-");
    }

    // ---------------------------------------------------------------- [3]
    if (argc < 2) { std::printf("usage: test_ioweb_watch <IO_Table.csv>\n"); return 2; }
    std::printf("[3] agreement with ResolveIoPoint on %s\n", argv[1]);
    {
        const std::string env = std::string("W906_IOTABLE_PATH=") + argv[1];
        HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
        HSys.LoadIoData();
        std::printf("IO_Table rows loaded: %d\n", (int)HSys.IOTable.size());
        CHECK(!HSys.IOTable.empty());

        std::vector<IoRow> rows;   // the same copy the probe makes (tools/ioweb_probe.cpp CopyIoRows)
        for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
            const TIODATA* t = HSys.IOTable[i];
            if (!t) continue;
            IoRow r;
            r.row = (int)i; r.alias = t->Alias.c_str(); r.type = t->Type.c_str();
            r.dir = (int)ht9045::sjson::IoDirectionOfType(r.type);
            r.isaBase = t->iISABase; r.lane = t->iLane; r.ip = t->iIP; r.port = t->iPort;
            r.bitCol = t->iBit; r.enable = t->iEnable; r.inType = t->iInType;
            rows.push_back(r);
        }

        int nIn = 0, agreeOn = 0, agreeOff = 0, found = 0, strayHits = 0, nOut = 0, outListed = 0, outPageDi = 0;
        const std::vector<ht9045::sjson::IoByteSample> none;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const IoRow& r = rows[i];
            if (r.isaBase != kIsaPci1203 || r.lane < 0 || r.ip < 0 || r.port < 0) continue;
            const int chan = r.port / 8, bit = r.port % 8;
            if (r.dir == kDirIn) {
                ++nIn;
                std::vector<ht9045::sjson::IoByteSample> on, off;
                on.push_back(S(r.lane, r.ip, chan, (unsigned char)(1u << bit)));
                off.push_back(S(r.lane, r.ip, chan, (unsigned char)(0xFFu & ~(1u << bit))));
                const ht9045::sjson::IoPointState a = ht9045::sjson::ResolveIoPoint(
                    r.isaBase, r.lane, r.ip, r.port, 1, 1, ht9045::sjson::kIoDirIn, on, none);
                const ht9045::sjson::IoPointState b = ht9045::sjson::ResolveIoPoint(
                    r.isaBase, r.lane, r.ip, r.port, 1, 1, ht9045::sjson::kIoDirIn, off, none);
                if (a.raw == 1) ++agreeOn;       // the page reads exactly this bit ...
                if (b.raw == 0) ++agreeOff;      // ... and no other bit of the byte
                if (Has(RowsForDiBit(rows, r.lane, r.ip, chan, bit), (int)i) && PageReadsHere(r)) ++found;
                for (int ob = 0; ob < 8; ++ob)
                    if (ob != bit && Has(RowsForDiBit(rows, r.lane, r.ip, chan, ob), (int)i)) ++strayHits;
            } else if (r.dir == kDirOut) {
                ++nOut;
                if (Has(RowsForDiBit(rows, r.lane, r.ip, chan, bit), (int)i) && !PageReadsHere(r)) ++outListed;
                std::vector<ht9045::sjson::IoByteSample> on;
                on.push_back(S(r.lane, r.ip, chan, 0xFF));
                const ht9045::sjson::IoPointState o = ht9045::sjson::ResolveIoPoint(
                    r.isaBase, r.lane, r.ip, r.port, 1, 1, ht9045::sjson::kIoDirOut, on, none);
                if (o.raw < 0) ++outPageDi;      // the page does NOT read an output row from DI
            }
        }
        std::printf("ePCI1203 input rows %d: ResolveIoPoint on=%d off=%d, found by RowsForDiBit=%d, stray=%d | "
                    "output rows %d: listed-with-caveat=%d, page-not-from-DI=%d\n",
                    nIn, agreeOn, agreeOff, found, strayHits, nOut, outListed, outPageDi);
        CHECK(nIn > 0 && nOut > 0);
        CHECK(agreeOn == nIn && agreeOff == nIn && found == nIn && strayHits == 0);
        CHECK(outListed == nOut && outPageDi == nOut);

        // the pair the on-site session is about: two door rows on one input (IP 2 Port 25 = chan 3 bit 1)
        const std::vector<int> door = RowsForDiBit(rows, 1, 2, 3, 1);
        bool d1 = false, dIdx = false;
        for (std::size_t k = 0; k < door.size(); ++k) {
            if (rows[(std::size_t)door[k]].alias == "SnSafeDoor1") d1 = true;
            if (rows[(std::size_t)door[k]].alias == "SnSafeDoorIndex") dIdx = true;
        }
        std::printf("ring 1 station 2 chan 3 bit 1 -> %s\n", RowsText(rows, door).c_str());
        CHECK(d1 && dIdx);

        //AI(W906-ONSITE-1) 20260926 (review): every legacy-form ISABase 3 row (Bit column != Port % 8)
        //  is a Bit-column candidate at its legacy address, and never in the primary list there.
        int nLegacy = 0, legacyFound = 0, legacyInPrimary = 0;
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const IoRow& r = rows[i];
            if (r.isaBase != kIsaPci1203 || r.ip < 0 || r.port < 0 || r.bitCol < 0 || r.bitCol > 7) continue;
            if (r.bitCol == r.port % 8) continue;
            ++nLegacy;
            if (Has(BitColumnCandidates(rows, r.lane, r.ip, r.port, r.bitCol), (int)i)) ++legacyFound;
            if (Has(RowsForDiBit(rows, r.lane, r.ip, r.port, r.bitCol), (int)i)) ++legacyInPrimary;
        }
        std::printf("legacy-form ISABase 3 rows %d: found as Bit-column candidates %d, in the primary list %d\n",
                    nLegacy, legacyFound, legacyInPrimary);
        CHECK(nLegacy > 0 && legacyFound == nLegacy && legacyInPrimary == 0);

        //  door-named rows that are not ISABase 3: listed, none of them ISABase 3, the 1203 door rows not listed
        const std::vector<int> offDoors = DoorRowsOffPci1203(rows);
        bool sd2 = false, sd1Listed = false, any1203 = false;
        for (std::size_t k = 0; k < offDoors.size(); ++k) {
            const IoRow& r = rows[(std::size_t)offDoors[k]];
            if (r.isaBase == kIsaPci1203) any1203 = true;
            if (r.alias == "SnSafeDoor2") sd2 = true;
            if (r.alias == "SnSafeDoor1") sd1Listed = true;
        }
        std::printf("door-named rows off the 1203: %d%s%s\n", (int)offDoors.size(),
                    offDoors.empty() ? "" : ", first: ", offDoors.empty() ? "" : DescribeOffRow(rows[(std::size_t)offDoors[0]]).c_str());
        CHECK(!offDoors.empty() && sd2 && !sd1Listed && !any1203);
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] torque-limit read-back states\n");
    {
        CHECK(std::strcmp(ht9045::Pci1203TorqueLimitReadText(true, 0ul), "ok") == 0);
        CHECK(std::strcmp(ht9045::Pci1203TorqueLimitReadText(false, 0x80000009ul), "readFailed") == 0);
        CHECK(std::strcmp(ht9045::Pci1203TorqueLimitReadText(false, 0ul), "notRead") == 0);
        const ht9045::Pci1203AxisSample a;
        for (int k = 0; k < 2; ++k) {
            CHECK(!a.trqLimValid[k] && a.trqLimRet[k] == 0ul && a.trqLimVal[k] == 0);
            CHECK(a.trqLimRetText[k].empty());   // AI(W906-ONSITE-1) 20260926: no vendor text before any read
            CHECK(std::strcmp(ht9045::Pci1203TorqueLimitReadText(a.trqLimValid[k], a.trqLimRet[k]), "notRead") == 0);
        }
    }

    std::printf("RESULT: %s (%d/%d)\n", g_fail ? "FAIL" : "ALL PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
