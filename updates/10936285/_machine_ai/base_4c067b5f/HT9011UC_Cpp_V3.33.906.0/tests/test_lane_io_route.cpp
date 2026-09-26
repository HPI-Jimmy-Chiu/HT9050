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

    IO_CARD_TYPE = savedCard;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
