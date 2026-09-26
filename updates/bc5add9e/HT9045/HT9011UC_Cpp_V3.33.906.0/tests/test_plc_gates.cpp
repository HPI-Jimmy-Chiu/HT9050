// =============================================================================
//  test_plc_gates.cpp -- 安全 PLC 閘（RULINGS_20260926 第 20／22 條）能在 ctest 量到的那一部分
//
//  AI(W906-PLC-GATES) 20260926
//    [A] SEAM S4 退役：csystem.cpp 的 bCheckPLCConnet（golden csystem.cpp:24394-24425）讀的是**真的**
//        bSafePLCThread（MyPLC/MyPLC_IO_Modbus.cpp:93，ht9045_comms）。連續 3.5 秒一直翻轉真的旗標（PLC 心跳），
//        每一次都要回 true。對照：舊的 TU 內替身（static false）看不到翻轉 ⇒ 3 秒後回 false，這一段會紅。
//    [B] 心跳停了（旗標不再翻轉）：3 秒內回 true、超過 3 秒回 false（golden「PLC 斷線」那一支）
//    [C] Enable_PLCSafety_IO／InitialOK 還原
//  NOT COVERED：G12／G14／G23（在 #ifndef SOFT_SIMULTE 的 DoSystem／CheckSafeDoorIsClosed 裡，模擬組態不編）；
//  G-PLC-A（IsEMGPressed 觸發時對每支馬達呼叫 MOT[i].Motor->Enable，測試環境的 Motor 可能是 NULL）；
//  G-PLC-B（測試環境的門感測器全部停用，提早 return 與照常掃描的結果一樣是 true，量不出差別）；
//  W7a-I3（InitialHandler 開機序列）。這些用逐字比對 golden＋nm 驗（commit 訊息）。不讀寫任何機台檔。
// =============================================================================
#include <cstdio>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

extern bool Enable_PLCSafety_IO;   // cmydef.cpp
extern bool InitialOK;             // cmydef.h:220
extern bool bSafePLCThread;        // MyPLC/MyPLC_IO_Modbus.cpp:93（ht9045_comms）
bool bCheckPLCConnet();            // csystem.cpp（csystem.h:307）

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

int main()
{
    const bool oldEnable = Enable_PLCSafety_IO, oldInit = InitialOK;
    Enable_PLCSafety_IO = true;
    InitialOK = true;                                   // bCheckPLCConnet 在 InitialOK==false 時一律回 true（golden :24397-24398）

    std::printf("[A] a live heartbeat (the REAL bSafePLCThread toggling) keeps bCheckPLCConnet true past 3 s\n");
    int calls = 0, falses = 0;
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < 3500) {
        bSafePLCThread = !bSafePLCThread;               // golden PLCStatusCheck :134（移植樹 MyPLC_IO_Modbus.cpp:200）每 1 秒翻一次；這裡翻得更勤
        if (!bCheckPLCConnet()) falses++;
        calls++;
        ::Sleep(200);
    }
    std::printf("       %d calls, %d false\n", calls, falses);
    CHECK(calls >= 15);
    CHECK(falses == 0);

    std::printf("[B] the heartbeat stops: true within 3 s, false after\n");
    CHECK(bCheckPLCConnet() == true);                   // 沒翻轉 ⇒ 開始計時（golden :24411-24415），回 true
    ::Sleep(1500);
    CHECK(bCheckPLCConnet() == true);                   // 計時中
    ::Sleep(1800);
    CHECK(bCheckPLCConnet() == false);                  // 滿 3 秒 ⇒ false（golden :24418-24422）

    std::printf("[C] restore\n");
    Enable_PLCSafety_IO = oldEnable;
    InitialOK = oldInit;
    CHECK(Enable_PLCSafety_IO == oldEnable);

    std::printf("%s (%d passed, %d failed)\n", g_fail ? "FAILED" : "ALL PASSED", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
