// =============================================================================
//  test_mainclose_stop.cpp -- Q44「按 Exit 必須先停下才能關閉」的判斷（FileRW/MainClose.cpp 檔尾 w906q44::Evaluate）
//
//  AI(W906-FRW-S167) 20260928 [W906]  Steven 20260928 裁決（原話）：「Q44 bcb還沒移植的都先列為待辦, 至少馬達跟溫度的要停下來,
//    這個只要兩個命令就可以做到了」；追問三題都選建議（兩個命令＝停全部馬達＋關加熱器繼電器；讀回沒停就不關、給［重試停機］
//    ［強制關閉］；其餘全部待辦、不擋）。
//  本檔先定義 W906_MAINCLOSE_Q44_DECIDE_ONLY 再 #include MainClose.cpp ⇒ 只編檔尾的純判斷（只用標準庫），不 link god-stack、
//  不讀寫任何檔、不碰 1203 卡，秒級。
//    [1] 這台沒有 1203 卡（Steven01）：監看器沒有／沒開過 ⇒ 兩項 absent，確認（不擋）
//    [2] 1203 卡開著、每一軸讀回都停、繼電器讀回 0 ⇒ 確認
//    [3] 軸還在動：kSettleMs 之內等（pending），過了就擋；理由寫出軸、狀態、命令速度
//    [4] 停止命令被拒＋軸還在動 ⇒ 立刻擋；停止被拒但讀回都停了 ⇒ 不擋（讀回說了算，理由註明被拒）
//    [5] 繼電器讀回 1：寫入到卡 ⇒ 先等、過了擋；DRY RUN／拒寫 ⇒ 立刻擋；拒寫但讀回 0 ⇒ 不擋
//    [6] 這次開機開過卡、現在失聯 ⇒ 立刻擋（軸與繼電器都擋）
//    [7] 讀回還不新（送出停止之後監看器輪詢不到 2 次）⇒ 等，即使舊讀回看起來停了
//    [8] 繼電器不是 1203 點 ⇒ unverified（不擋）；IO_Table 沒有（Enable=0）⇒ absent
//    [9] 卡片 DO 對應表沒有繼電器那個 byte ⇒ 擋（讀不回也關不了）
//    [10] 軸狀態讀不回（valid=false）⇒ 等，過了擋
//    [11] golden StopAllMotor（非 1203 馬達卡）：SIM absent、出貨 unverified，永遠不擋
//    [12] 強制關閉的等級門檻＝golden Exit 的 LevelSet.AccessLevel[6]（等級 ≥ 門檻才可以）
//    [13] 軸狀態 ERROR_STOP／DISABLE 算停；READY 但命令速度不是 0 算還在動
//  NOT COVERED：送兩個命令本身（Q44Issue：golden StopAllMotor、W906_Stop1203AllHook、SW[].Off()、命令面 DO 寫 0）、主迴圈的
//    狀態機（Q44QuitTick：晚一圈、等讀回、確認後最多等 2 秒、強制後晚一圈）、act.main.closeProgram 的 op 分派與頁面 ——
//    要 wb_serve 探針或機邊驗（交件報告列了步驟）。
// =============================================================================
#define W906_MAINCLOSE_Q44_DECIDE_ONLY 1
#include "FileRW/MainClose.cpp"

#include <cstdio>
#include <string>

namespace {

int g_fail = 0;
int g_checks = 0;

#define CHECK(c) do { ++g_checks; if (!(c)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

using namespace w906q44;

const Item* Find(const Result& r, const char* key)
{
    for (std::size_t i = 0; i < r.items.size(); ++i)
        if (r.items[i].key == key) return &r.items[i];
    return 0;
}
int StateOf(const Result& r, const char* key) { const Item* it = Find(r, key); return it ? it->state : -1; }
bool Has(const Result& r, const char* key, const char* sub)
{
    const Item* it = Find(r, key);
    return it != 0 && it->reason.find(sub) != std::string::npos;
}

AxisFact Axis(int index, unsigned state, double vel, bool valid = true)
{
    AxisFact a;
    a.index = index; a.station = index + 1; a.state = state; a.cmdVel = vel; a.valid = valid;
    return a;
}

// SIM 建置接真 1203 卡（HT9050）：卡開著、兩軸 READY、繼電器是 1203 點、命令面 LIVE 寫 0 已送到卡、讀回 0、讀回夠新。
Facts Base1203()
{
    Facts f;
    f.simBuild = true;
    f.monPresent = true; f.cardEverOpen = true; f.cardUsable = true;
    f.pollsSinceIssue = 3;
    f.stopHook = true; f.ctlArmed = true; f.ctlDry = false;
    f.stopIssued = 4;
    f.axes.push_back(Axis(0, 1, 0.0));
    f.axes.push_back(Axis(1, 1, 0.0));
    f.relayEnable = true; f.relayIs1203 = true; f.relayIsaBase = 3; f.relayRing = 1; f.relayIp = 3; f.relayPort = 12;
    f.relayWrite.attempted = true; f.relayWrite.accepted = true; f.relayWrite.issued = true; f.relayWrite.ret = 0;
    f.relayWrite.via = "1203 命令面";
    f.relaySlot = 2; f.relayByteValid = true; f.relayBit = 0;
    return f;
}

void T1_NoCard()
{
    Facts f;                                         // 監看器物件都沒有（出貨 HT9045 建置沒有 INSTALL_1203_MONITOR）
    f.simBuild = true;
    f.relayEnable = true; f.relayIs1203 = true; f.relayIsaBase = 3;
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kAbsent);
    CHECK(StateOf(r, "heaterRelay") == kAbsent);

    Facts g = f;                                     // 監看器物件在，但沒 link／沒開過（Steven01：Enable() 在 "not linked" 時照樣掛上物件）
    g.monPresent = true; g.cardEverOpen = false; g.cardUsable = false;
    r = Evaluate(g, 0, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kAbsent);
    CHECK(StateOf(r, "heaterRelay") == kAbsent);
    CHECK(Has(r, "motor1203", "沒有開過 1203 卡"));
}

void T2_AllStopped()
{
    Result r = Evaluate(Base1203(), 600, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kOk);
    CHECK(StateOf(r, "heaterRelay") == kOk);
    CHECK(Has(r, "motor1203", "2 軸讀回都已停"));
    CHECK(Has(r, "heaterRelay", "讀回 0（已關）"));
}

void T3_AxisMoving()
{
    Facts f = Base1203();
    f.axes[1] = Axis(1, 5, 1200.0);                  // STA_AX_PTP_MOT
    Result r = Evaluate(f, 1000, kSettleMs);
    CHECK(r.verdict == kPending);
    CHECK(StateOf(r, "motor1203") == kWait);
    CHECK(Has(r, "motor1203", "減速中"));
    r = Evaluate(f, kSettleMs, kSettleMs);
    CHECK(r.verdict == kBlock);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(Has(r, "motor1203", "還在動"));
    CHECK(Has(r, "motor1203", "PTP_MOT"));
    CHECK(Has(r, "motor1203", "軸 1（站 2）"));
    CHECK(StateOf(r, "heaterRelay") == kOk);         // 另一項照樣判
}

void T4_StopRefused()
{
    Facts f = Base1203();
    f.stopRefused = 2; f.stopIssued = 2;
    f.axes[0] = Axis(0, 6, -300.0);                  // STA_AX_CONTI_MOT
    Result r = Evaluate(f, 100, kSettleMs);          // 還沒到 kSettleMs 也不等：停止命令到不了
    CHECK(r.verdict == kBlock);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(Has(r, "motor1203", "停止命令被拒 2 筆"));

    Facts g = Base1203();                            // 停止被拒，但每一軸讀回都停了 ⇒ 不擋（註明）
    g.stopVendorErr = 1;
    r = Evaluate(g, 100, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kOk);
    CHECK(Has(r, "motor1203", "廠商回錯 1 筆"));

    Facts h = Base1203();                            // 命令面沒武裝、軸在動 ⇒ 立刻擋
    h.ctlArmed = false;
    h.axes[0] = Axis(0, 8, 50.0);                    // STA_AX_EXT_JOG
    r = Evaluate(h, 100, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(Has(r, "motor1203", "沒有武裝"));

    Facts d = Base1203();                            // DRY RUN、軸都停 ⇒ 不擋
    d.ctlDry = true;
    r = Evaluate(d, 100, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kOk);
    CHECK(Has(r, "motor1203", "DRY RUN"));
}

void T5_RelayStillOn()
{
    Facts f = Base1203();
    f.relayBit = 1;                                  // 已送到卡，讀回還是 1
    Result r = Evaluate(f, 800, kSettleMs);
    CHECK(r.verdict == kPending);
    CHECK(StateOf(r, "heaterRelay") == kWait);
    r = Evaluate(f, kSettleMs + 1, kSettleMs);
    CHECK(r.verdict == kBlock);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "讀回仍是 1"));
    CHECK(Has(r, "heaterRelay", "Lane 1、IP 3、Port 12"));

    Facts d = Base1203();                            // DRY RUN：不會變 ⇒ 立刻擋
    d.relayBit = 1; d.relayWrite.issued = false; d.relayWrite.dry = true;
    r = Evaluate(d, 100, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "DRY RUN"));

    Facts x = Base1203();                            // 拒寫（例：路由說是驅動器站）⇒ 立刻擋
    x.relayBit = 1; x.relayWrite.accepted = false; x.relayWrite.issued = false; x.relayWrite.why = "ring 1 站 3 是伺服／馬達驅動器";
    r = Evaluate(x, 100, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "拒寫"));
    CHECK(Has(r, "heaterRelay", "驅動器"));

    Facts v = Base1203();                            // 廠商回錯
    v.relayBit = 1; v.relayWrite.ret = 0x83100000UL;
    r = Evaluate(v, 100, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "0x83100000"));

    Facts z = Base1203();                            // 拒寫，但讀回已是 0 ⇒ 不擋
    z.relayWrite.accepted = false; z.relayWrite.issued = false; z.relayWrite.why = "1203 命令面沒有武裝";
    r = Evaluate(z, 100, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kOk);
    CHECK(r.verdict == kConfirmed);

    Facts n = Base1203();                            // 沒有送出（出貨建置：MyLaneIO 在後端之前 return）且讀回 1 ⇒ 擋
    n.relayBit = 1; n.relayWrite = WriteFact(); n.relayWrite.why = "MyLaneIO 在後端之前就 return";
    r = Evaluate(n, 100, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "沒有送出"));
}

void T6_CardLost()
{
    Facts f = Base1203();
    f.cardUsable = false; f.cardWhy = "auto-disabled after 10 consecutive polls";
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(r.verdict == kBlock);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "motor1203", "1203 卡失聯"));
    CHECK(Has(r, "heaterRelay", "auto-disabled"));
}

void T7_NotFresh()
{
    Facts f = Base1203();
    f.pollsSinceIssue = 1;
    Result r = Evaluate(f, 100, kSettleMs);
    CHECK(r.verdict == kPending);
    CHECK(StateOf(r, "motor1203") == kWait);
    CHECK(StateOf(r, "heaterRelay") == kWait);
    f.pollsSinceIssue = kFreshPolls;
    r = Evaluate(f, 400, kSettleMs);
    CHECK(r.verdict == kConfirmed);
}

void T8_RelayNot1203()
{
    Facts f = Base1203();
    f.relayIs1203 = false; f.relayIsaBase = 1;       // 例：MN200
    f.relayBit = 1;                                  // 1203 讀回跟它無關
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kUnverified);
    CHECK(!Holds(StateOf(r, "heaterRelay")));
    CHECK(r.verdict == kConfirmed);
    CHECK(Has(r, "heaterRelay", "ISABase=1"));

    Facts g = Base1203();
    g.relayEnable = false;
    r = Evaluate(g, 0, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kAbsent);
    CHECK(r.verdict == kConfirmed);
}

void T9_NoByte()
{
    Facts f = Base1203();
    f.relaySlot = -1;
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(Has(r, "heaterRelay", "第 1 個 byte"));   // Port 12 ⇒ 站內第 12/8＝1 個 byte
    CHECK(r.verdict == kBlock);
}

void T10_AxisInvalid()
{
    Facts f = Base1203();
    f.axes[0].valid = false;
    Result r = Evaluate(f, 1000, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kWait);
    r = Evaluate(f, kSettleMs, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(Has(r, "motor1203", "讀不回軸狀態"));

    Facts g = Base1203();                            // 監看器開著卡、但沒開成功任何軸 ⇒ absent
    g.axes.clear();
    r = Evaluate(g, 0, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kAbsent);
}

void T11_GoldenMotors()
{
    Facts f = Base1203();
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "motorGolden") == kAbsent);     // SIM
    f.simBuild = false;
    r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "motorGolden") == kUnverified); // 出貨：送了、讀不回，不擋
    CHECK(r.verdict == kConfirmed);
    CHECK(r.items.size() == 3u);
}

void T12_ForceLevel()
{
    CHECK(ForceAllowed(5, 5));
    CHECK(ForceAllowed(9, 5));
    CHECK(!ForceAllowed(4, 5));
    CHECK(ForceAllowed(0, 0));
}

void T13_AxisStates()
{
    CHECK(AxisStopped(1, 0.0));                      // READY
    CHECK(AxisStopped(0, 0.0));                      // DISABLE（伺服沒開）
    CHECK(AxisStopped(3, 0.0));                      // ERROR_STOP
    CHECK(!AxisStopped(2, 0.0));                     // STOPPING
    CHECK(!AxisStopped(4, 0.0));                     // HOMING
    CHECK(!AxisStopped(15, 0.0));                    // EXT_JOG_READY（ExtDrive 還開著）
    CHECK(!AxisStopped(1, 10.0));                    // READY 但命令速度不是 0
    CHECK(AxisStopped(0x0101u, 0.0));                // 高位元不看（同 WebMotorAccess.cpp IsReadyState 的遮罩）
    CHECK(std::string(AxisStateName(5)) == "PTP_MOT");
    CHECK(std::string(StateName(kBlocked)) == "blocked");
    CHECK(Holds(kWait) && Holds(kBlocked) && !Holds(kOk) && !Holds(kAbsent) && !Holds(kUnverified));
}

}  // namespace

int main()
{
    T1_NoCard();
    T2_AllStopped();
    T3_AxisMoving();
    T4_StopRefused();
    T5_RelayStillOn();
    T6_CardLost();
    T7_NotFresh();
    T8_RelayNot1203();
    T9_NoByte();
    T10_AxisInvalid();
    T11_GoldenMotors();
    T12_ForceLevel();
    T13_AxisStates();
    std::printf("test_mainclose_stop: %s (%d check(s), %d failure(s))\n", g_fail ? "FAIL" : "PASS", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
