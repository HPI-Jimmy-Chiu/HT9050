// ===========================================================================
//  tests/test_lane_io_route.cpp
//
//  AI(W906-A4-7) 20260924: TLaneIO 依點位分派三路後端的測試（週末計畫 W3 第 1 支 MyLaneIo）。
//
//  golden 的 IO 方法直接呼叫廠商函式，依點位 ISABase 與全域 IO_CARD_TYPE 分三路
//  （golden MyLaneIo.cpp:143-158、:210-225、:278-293、:397-413、:466-481）：
//      iISABase==ePCI1203                            -> Acm_Daq*Ex    ⇒ pIO1203
//      IO_CARD_TYPE==MotionnetIO_MN200||NewIO_MN200  -> mn_*          ⇒ pIOMN200
//      其他                                          -> _mnet_*       ⇒ pIOMnet
//  唯一例外：IOInputByte 的 MN200 條件是 IO_CARD_TYPE==1（golden :474），NewIO_MN200(2) 會走 _mnet_。
//
//    [1] 預設（建構後、沒有硬體初始化）：三路都是模擬後端 ⇒ 寫了讀得回來（既有測試依賴的語意）
//    [2] 注入三個探針：每個方法 × 每種點位／IO_CARD_TYPE 都落到 golden 指定的那一路，參數逐一相同
//    [3] 1203 的回傳值換算（golden：0==成功 → 1，其他 → -1）：探針回錯誤碼 ⇒ IOInputBit 回 false
//    [4] SelectVendorBackends：SOFT_SIMULTE 建置不換（仍是模擬）；出貨組態換成真後端
//        ⇒ 模擬後端的儲存不再被寫到（這台筆電沒有 SDK，真後端是回 0 的樁）
// ===========================================================================
#include "MyLaneIo.h"
#include "IOBackend.h"
#include "MachineType.h"
#include "cmydef.h"
#include <cstdio>
#include <cstring>

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_lane_io_route.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// 探針後端：記下最後一次呼叫的方法與參數；回傳值可設定
struct Spy : public TIOBackend {
    const char* name;
    int calls, ret;
    char last[16];
    int r, ip, port, bit, val;
    unsigned char readValue;
    explicit Spy(const char* n) : name(n), calls(0), ret(0), r(-1), ip(-1), port(-1), bit(-1), val(-1), readValue(1) { last[0] = 0; }
    void Rec(const char* m, int a, int b, int c, int d, int v) { ++calls; std::strcpy(last, m); r = a; ip = b; port = c; bit = d; val = v; }
    virtual int WriteBit (int R, int I, int P, int B, int V) override { Rec("WriteBit", R, I, P, B, V); return ret; }
    virtual int WriteByte(int R, int I, int P, unsigned int V) override { Rec("WriteByte", R, I, P, -1, (int)V); return ret; }
    virtual int ReadBit  (int R, int I, int P, int B, unsigned char* v) override { Rec("ReadBit", R, I, P, B, -1); if (v) *v = readValue; return ret; }
    virtual int ReadByte (int R, int I, int P, unsigned char* v) override { Rec("ReadByte", R, I, P, -1, -1); if (v) *v = readValue; return ret; }
    void Reset() { calls = 0; last[0] = 0; r = ip = port = bit = val = -1; }
};

static void ResetAll(Spy& a, Spy& b, Spy& c) { a.Reset(); b.Reset(); c.Reset(); }

int main()
{
    const int R = 1, I = 2, P = 3, B = 4;
    const int savedCard = IO_CARD_TYPE;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] default = simulation on all three paths\n");
    {
        TLaneIO io;
        IO_CARD_TYPE = NewIO_MN200;
        io.IOBitOn(R, I, P, B, eMotionNet, "t");
        CHECK(io.IOInputBit(R, I, P, B, eMotionNet, "t") == true);
        io.IOBitOff(R, I, P, B, eMotionNet, "t");
        CHECK(io.IOInputBit(R, I, P, B, eMotionNet, "t") == false);
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] per-point dispatch follows golden\n");
    {
        TLaneIO io;
        Spy s1203("1203"), sMN("MN200"), sMnet("mnet");
        io.SetBackends(&s1203, &sMN, &sMnet);

        // IOBitOn / IOBitOff / IOByteOut / IOInputBit：ePCI1203 永遠走 1203
        const int cards[3] = {0, MotionnetIO_MN200, NewIO_MN200};
        for (int k = 0; k < 3; ++k) {
            IO_CARD_TYPE = cards[k];
            ResetAll(s1203, sMN, sMnet);
            io.IOBitOn(R, I, P, B, ePCI1203, "t");
            CHECK(s1203.calls == 1 && std::strcmp(s1203.last, "WriteBit") == 0 && sMN.calls == 0 && sMnet.calls == 0);
            CHECK(s1203.r == R && s1203.ip == I && s1203.port == P && s1203.bit == B && s1203.val == 1);
            ResetAll(s1203, sMN, sMnet);
            io.IOBitOff(R, I, P, B, ePCI1203, "t");
            CHECK(s1203.calls == 1 && s1203.val == 0 && sMN.calls == 0 && sMnet.calls == 0);
            ResetAll(s1203, sMN, sMnet);
            io.IOByteOut(R, I, P, 0x5A, ePCI1203);
            CHECK(s1203.calls == 1 && std::strcmp(s1203.last, "WriteByte") == 0 && s1203.val == 0x5A);
            ResetAll(s1203, sMN, sMnet);
            io.IOInputBit(R, I, P, B, ePCI1203, "t");
            CHECK(s1203.calls == 1 && std::strcmp(s1203.last, "ReadBit") == 0 && sMN.calls == 0 && sMnet.calls == 0);
        }

        // 非 1203 點：IO_CARD_TYPE 為 1 或 2 ⇒ MN200；0 ⇒ _mnet_
        for (int k = 0; k < 3; ++k) {
            IO_CARD_TYPE = cards[k];
            Spy& want = (cards[k] == MotionnetIO_MN200 || cards[k] == NewIO_MN200) ? sMN : sMnet;
            Spy& other = (&want == &sMN) ? sMnet : sMN;
            ResetAll(s1203, sMN, sMnet);
            io.IOBitOn(R, I, P, B, eMotionNet, "t");
            CHECK(want.calls == 1 && want.val == 1 && other.calls == 0 && s1203.calls == 0);
            ResetAll(s1203, sMN, sMnet);
            io.IOBitOff(R, I, P, B, eMotionNet, "t");
            CHECK(want.calls == 1 && want.val == 0 && other.calls == 0 && s1203.calls == 0);
            ResetAll(s1203, sMN, sMnet);
            io.IOByteOut(R, I, P, 0x3C, eMotionNet);
            CHECK(want.calls == 1 && std::strcmp(want.last, "WriteByte") == 0 && other.calls == 0 && s1203.calls == 0);
            ResetAll(s1203, sMN, sMnet);
            io.IOInputBit(R, I, P, B, eMotionNet, "t");
            CHECK(want.calls == 1 && std::strcmp(want.last, "ReadBit") == 0 && other.calls == 0 && s1203.calls == 0);
        }

        // IOInputByte：golden :474 只認 IO_CARD_TYPE==1
        IO_CARD_TYPE = 1;            ResetAll(s1203, sMN, sMnet); io.IOInputByte(R, I, P, eMotionNet);
        CHECK(sMN.calls == 1 && sMnet.calls == 0);
        IO_CARD_TYPE = NewIO_MN200;  ResetAll(s1203, sMN, sMnet); io.IOInputByte(R, I, P, eMotionNet);
        CHECK(sMnet.calls == 1 && sMN.calls == 0);       // 照 golden 原文：2 走 _mnet_
        IO_CARD_TYPE = NewIO_MN200;  ResetAll(s1203, sMN, sMnet); io.IOInputByte(R, I, P, ePCI1203);
        CHECK(s1203.calls == 1 && sMN.calls == 0 && sMnet.calls == 0);

        // ---------------------------------------------------------------- [3]
        std::printf("[3] 1203 return-code remap\n");
        IO_CARD_TYPE = NewIO_MN200;
        s1203.ret = 0;  s1203.readValue = 1;
        CHECK(io.IOInputBit(R, I, P, B, ePCI1203, "t") == true);      // 0 == 成功
        s1203.ret = 0x80000001; s1203.readValue = 1;
#ifdef SOFT_SIMULTE
        CHECK(io.IOInputBit(R, I, P, B, ePCI1203, "t") == true);      // golden：SOFT_SIMULTE 下讀失敗回 true
#else
        CHECK(io.IOInputBit(R, I, P, B, ePCI1203, "t") == false);     // 出貨組態：讀失敗回 false（並記 MNetLog）
#endif
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] SelectVendorBackends\n");
    {
        TLaneIO io;
        TSimIOBackend sim;
        io.SetBackend(&sim);          // 三路都指向這個模擬後端
        io.SelectVendorBackends();
        IO_CARD_TYPE = NewIO_MN200;
        io.IOBitOn(R, I, P, B, eMotionNet, "t");
        unsigned char v = 0;
        sim.ReadBit(R, I, P, B, &v);
#ifdef SOFT_SIMULTE
        CHECK(v == 1);                // 模擬建置：不換，寫進模擬後端
#else
        CHECK(v == 0);                // 出貨組態：已換成真後端，模擬後端不再被寫到
#endif
    }

    // ---------------------------------------------------------------- [5]
    //  AI(W906-LANEIO-CACHE) 20260926: golden 的輸出命令快取 OutPortData[4][64][4] 裝不下 1203 的輸出位址，
    //  machines/HT9050/IO_Table.csv 的 206 列 1203 輸出（Switch／Cylinder_On／_Off／Sucker_On／_Off）量出 14 組
    //  「兩個不同點位落在同一個快取位元」（flat = Lane*256 + IP*4 + Port、同一個 Bit）；Lane 換成現場表的 1。
    //  golden 缺陷，Jimmy 20260925 裁決修正（RULINGS_20260925 第 18／29 條）。修正前第二個 IOOutBitStatus 會讀到 true。
    std::printf("[5] 1203 output cache: the 14 golden-aliased pairs are independent\n");
    {
        TLaneIO io;
        static const int pr[14][5] = {   // {IP, Port, Bit, 另一個點位的 IP, Port}
            {80,17,1, 84,1}, {80,16,0, 84,0}, {80,23,3, 81,19}, {81,23,3, 82,19}, {16,17,1, 18,9},
            {16,22,6, 18,14}, {16,23,7, 18,15}, {16,29,5, 18,21}, {80,19,3, 84,3}, {16,24,0, 18,16},
            {16,25,1, 18,17}, {16,26,2, 18,18}, {16,27,3, 18,19}, {16,28,4, 18,20} };
        for (int n = 0; n < 14; ++n) {
            const int ip = pr[n][0], port = pr[n][1], bit = pr[n][2], ip2 = pr[n][3], port2 = pr[n][4];
            CHECK(ip * 4 + port == ip2 * 4 + port2);                                // 對照：在 golden 的 [4][64][4] 裡確實是同一格
            io.IOBitOn(1, ip, port, bit, ePCI1203, "t");
            CHECK(io.IOOutBitStatus(1, ip, port, bit, ePCI1203, "t") == true);
            CHECK(io.IOOutBitStatus(1, ip2, port2, bit, ePCI1203, "t") == false);   // 修正前讀到另一個點位的命令
            io.IOBitOff(1, ip, port, bit, ePCI1203, "t");
            CHECK(io.IOOutBitStatus(1, ip, port, bit, ePCI1203, "t") == false);
        }
        CHECK(io.CheckPortRangeErr(true,  ePCI1203, 1, 80, 32, 0) == 2);            // 輸出超出快取：擋下，不再越界寫
        CHECK(io.CheckPortRangeErr(true,  ePCI1203, 1, 256, 0, 0) == 2);
        CHECK(io.CheckPortRangeErr(true,  ePCI1203, 1, 84, 31, 7) == 0);            // 表上最大的輸出位址照常
        CHECK(io.CheckPortRangeErr(false, ePCI1203, 1, 64, 135, 7) == 0);           // 輸入端 Sucker 列（Port 128～135）不受影響
        CHECK(io.CheckPortRangeErr(true,  eMotionNet, 1, 2, 3, 4) == 0);            // MotionNet：沒初始化卡（InitialOK=false）照 golden 回 0
    }

    IO_CARD_TYPE = savedCard;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
