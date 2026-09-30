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
//  AI(W906-FRW-Q44B) 20260929：Steven 細化 Q44「SOFT_SIMULTE因為馬達不會真的動作, 所以關閉時沒有限制, 機台上只要c++有回復 StopAllMotor()
//    是已經發送, 且加熱io也有off, 就可以當成已停機」⇒ 下面 [3][5][7][8][9][10][11] 改成「送到了就算停」，[14] 新增 SIM 不設限。
//    Base1203 改成出貨建置（simBuild=false），機台規則才測得到。
//    [3] 軸還在動、停止命令已送到卡 ⇒ 立刻確認（不等讀回）；理由照樣寫出軸、狀態、命令速度
//    [4] 停止命令被拒＋軸還在動 ⇒ 立刻擋；停止被拒但讀回都停了 ⇒ 不擋（理由註明被拒）
//    [5] 繼電器讀回 1：寫入已送到卡 ⇒ 確認（註明讀回仍是 1）；DRY RUN／拒寫／廠商回錯 ⇒ 立刻擋；拒寫但讀回 0 ⇒ 不擋
//    [6] 這次開機開過卡、現在失聯 ⇒ 立刻擋（軸與繼電器都擋）
//    [7] 讀回還不新：命令都送到了 ⇒ 照樣確認；命令送不到 ⇒ 等讀回，過了 kSettleMs 擋
//    [8] 繼電器不是 1203 點 ⇒ ok（已照 golden 送 Off）；IO_Table 沒有（Enable=0）⇒ absent
//    [9] 卡片 DO 對應表沒有繼電器那個 byte：寫入送到了 ⇒ ok（註明讀不回）；寫入被拒 ⇒ 擋
//    [10] 軸狀態讀不回（valid=false）：停止命令送到了 ⇒ ok（註明）；送不到 ⇒ 擋
//    [11] golden StopAllMotor（非 1203 馬達卡）：SIM absent、出貨 ok（已發送），永遠不擋
//    [14] SIM 建置：就算卡失聯、DRY RUN、命令被拒也不擋（unverified，理由註明「SIM 建置，關閉不設限」）
//    [12] 強制關閉的等級門檻＝golden Exit 的 LevelSet.AccessLevel[6]（等級 ≥ 門檻才可以）
//    [13] 軸狀態 ERROR_STOP／DISABLE 算停；READY 但命令速度不是 0 算還在動
//  AI(W906-D012) 20260929 [W906] 待辦 D-012（Q44 其餘）：
//    [15] 關站中只收「關」與「停」（A2）：白名單逐條（放行／擋）、motor.access 只放 stop 與 Motor Test 放開、observer.get 只放讀取型 act、
//         理由字串開頭 "closing:"
//    [16] 主控台事件（A3）：第一次 Ctrl-C／Break＝主迴圈停機後正常關站；第二次＝立刻結束；X／登出／關機＝等停機命令送出（<5 秒）再結束
//    [17] golden 關站段補接的步驟（B）：寫檔三步的狀態（noop／done／missing）、條碼條件、Index kit 吸嘴狀態 —— 只測判斷，不呼叫本體
//         （btClearBarcodeCountClick／OutShuttleLog／NewRecordProcess 會寫 D:\HT9045_Log\… 與事件紀錄；Normal() 會寫輸出）
//    NOT COVERED（A2／A3 機台那一半）：wb_serve 兩個分派入口的呼叫、W906_ConsoleQuit 的執行緒互動、Q44ConsoleTick 送命令 —— 要 wb_serve 探針或機邊驗。
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

// 出貨建置接真 1203 卡（HT9050）：卡開著、兩軸 READY、繼電器是 1203 點、命令面 LIVE 寫 0 已送到卡、讀回 0、讀回夠新。
//   AI(W906-FRW-Q44B) 20260929：原本 simBuild=true；SIM 現在一律不擋，機台規則要用出貨建置測（SIM 見 T14）。
Facts Base1203()
{
    Facts f;
    f.simBuild = false;
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
    Result r = Evaluate(f, 100, kSettleMs);          // AI(W906-FRW-Q44B) 20260929：停止命令已送到卡 ⇒ 立刻確認，不等減速完
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kOk);
    CHECK(Has(r, "motor1203", "送到了就算停"));
    CHECK(Has(r, "motor1203", "讀回還在減速"));
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
    f.relayBit = 1;                                  // 已送到卡，讀回還是 1 ⇒ AI(W906-FRW-Q44B) 20260929：寫出去了就算關（註明讀回）
    Result r = Evaluate(f, 100, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "heaterRelay") == kOk);
    CHECK(Has(r, "heaterRelay", "加熱 IO 已 off"));
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
    f.pollsSinceIssue = 1;                           // AI(W906-FRW-Q44B) 20260929：命令都送到了 ⇒ 讀回不新也確認
    Result r = Evaluate(f, 100, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(Has(r, "motor1203", "讀回還沒更新"));
    Facts g = f;                                     // 命令送不到＋讀回不新 ⇒ 等；過了 kSettleMs ⇒ 擋
    g.ctlArmed = false;
    g.relayWrite.accepted = false; g.relayWrite.issued = false; g.relayWrite.why = "1203 命令面沒有武裝";
    r = Evaluate(g, 100, kSettleMs);
    CHECK(r.verdict == kPending);
    CHECK(StateOf(r, "motor1203") == kWait);
    CHECK(StateOf(r, "heaterRelay") == kWait);
    r = Evaluate(g, kSettleMs, kSettleMs);
    CHECK(r.verdict == kBlock);
    CHECK(StateOf(r, "motor1203") == kBlocked);
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    g.pollsSinceIssue = kFreshPolls;                 // 讀回來了而且都停／都關 ⇒ 確認（送不到但讀回確定）
    r = Evaluate(g, 400, kSettleMs);
    CHECK(r.verdict == kConfirmed);
}

void T8_RelayNot1203()
{
    Facts f = Base1203();
    f.relayIs1203 = false; f.relayIsaBase = 1;       // 例：MN200
    f.relayBit = 1;                                  // 1203 讀回跟它無關
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kOk);          // AI(W906-FRW-Q44B) 20260929：已照 golden 送 Off＝已 off
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
    f.relaySlot = -1;                                // AI(W906-FRW-Q44B) 20260929：寫入送到了 ⇒ ok（註明讀不回）
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "heaterRelay") == kOk);
    CHECK(Has(r, "heaterRelay", "第 1 個 byte"));   // Port 12 ⇒ 站內第 12/8＝1 個 byte
    CHECK(r.verdict == kConfirmed);
    f.relayWrite.accepted = false; f.relayWrite.issued = false; f.relayWrite.why = "位址不在卡片 DO 對應表";
    r = Evaluate(f, 0, kSettleMs);                   // 寫入被拒、又讀不回 ⇒ 擋
    CHECK(StateOf(r, "heaterRelay") == kBlocked);
    CHECK(r.verdict == kBlock);
}

void T10_AxisInvalid()
{
    Facts f = Base1203();
    f.axes[0].valid = false;                         // AI(W906-FRW-Q44B) 20260929：停止命令送到了 ⇒ ok（註明讀不回）
    Result r = Evaluate(f, 1000, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kOk);
    CHECK(Has(r, "motor1203", "讀不回軸狀態"));
    f.ctlDry = true;                                 // 送不到（DRY RUN）＋讀不回 ⇒ 擋
    r = Evaluate(f, 1000, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kBlocked);

    Facts g = Base1203();                            // 監看器開著卡、但沒開成功任何軸 ⇒ absent
    g.axes.clear();
    r = Evaluate(g, 0, kSettleMs);
    CHECK(StateOf(r, "motor1203") == kAbsent);
}

void T11_GoldenMotors()
{
    Facts f = Base1203();
    f.simBuild = true;
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "motorGolden") == kAbsent);     // SIM
    f.simBuild = false;
    r = Evaluate(f, 0, kSettleMs);
    CHECK(StateOf(r, "motorGolden") == kOk);         // 出貨：已發送＝已停（AI(W906-FRW-Q44B) 20260929），不擋
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

void T14_SimNoRestriction()                          // AI(W906-FRW-Q44B) 20260929：SIM 建置關閉不設限
{
    Facts f = Base1203();
    f.simBuild = true;
    f.cardUsable = false; f.cardWhy = "auto-disabled after 10 consecutive polls";
    Result r = Evaluate(f, 0, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(StateOf(r, "motor1203") == kUnverified);
    CHECK(StateOf(r, "heaterRelay") == kUnverified);
    CHECK(Has(r, "motor1203", "SIM 建置，關閉不設限"));
    CHECK(Has(r, "motor1203", "1203 卡失聯"));     // 原因照樣列出來
    Facts g = Base1203();
    g.simBuild = true;
    g.ctlDry = true; g.axes[0] = Axis(0, 5, 800.0);
    g.relayBit = 1; g.relayWrite.issued = false; g.relayWrite.dry = true;
    r = Evaluate(g, 100, kSettleMs);
    CHECK(r.verdict == kConfirmed);
    CHECK(!Holds(StateOf(r, "motor1203")) && !Holds(StateOf(r, "heaterRelay")));
    g.pollsSinceIssue = 0;                           // 讀回還沒來也不等
    r = Evaluate(g, 0, kSettleMs);
    CHECK(r.verdict == kConfirmed);
}

// ---- AI(W906-D012) 20260929 [W906] Q44 A2：關站中只收「關」與「停」（w906q44::CmdAllowedWhileClosing）-----------------------------
void T15_ClosingAllowList()
{
    // 放行：關站本身、停止、基礎設施、唯讀、回答框、登入
    const char* allow[] = { "act.main.closeProgram", "motor.stop", "pci1203.ax.stop", "pci1203.ax.emgStop", "pause.run",
                            "sys.ping", "cfg.resync", "log.event", "ui.windows.put", "stream.resync",
                            "contactct.get", "counterclear.get", "modal.answer", "dialog.response", "dialog.notifyAck",   /* AI(W906-J5-ACK) 20260930 */
                            "auth.login", "auth.logout", "auth.mode", "auth.select", "dialog.auth", 0 };
    for (int i = 0; allow[i]; ++i) CHECK(CmdAllowedWhileClosing(allow[i], "", -1));
    // 擋：開輸出、動馬達、寫檔、開始生產、其他 act.*、模擬 DI、echo 框、未來的新指令
    const char* deny[] = { "io.btnPanelClick", "pci1203.do.setBit", "pci1203.do.setByte", "pci1203.ax.svOn", "pci1203.ax.jog",
                           "pci1203.ax.moveRel", "pci1203.ax.home", "pci1203.card.rescan", "pci1203.ax.resetError",
                           "form.event", "form.save", "editlist.save", "editlist.get", "struct.put", "recipe.change", "recipe.doc.put",
                           "system.file.put", "system.csv.rows", "system.levels.put", "counter.clear", "counterclear.click", "counterclear.exe",
                           "act.main.light", "act.main.fan", "act.main.cleanOut", "act.main.autoSkip", "act.showBinSelect.clearCount",
                           "act.sortCT.clearCount", "start.run", "lot.start", "main.home", "main.runStartMode", "act.main.ctlButton",
                           "security.jam", "security.passwd", "sim.di.set", "sys.echoModal", "sys.echoYesNo", "sys.echoErrorModal",
                           "olp.anything", "builder.op", "lotinfo.op", "smartdiag.op", "towerlight.op", "ttlcfg.op", "hw.access",
                           "control.acquire", "", "totally.new.cmd", 0 };
    for (int i = 0; deny[i]; ++i) CHECK(!CmdAllowedWhileClosing(deny[i], "", -1));
    // motor.access：只放 stop 與 Motor Test 的放開（loopMove／home 帶 params.start=false）
    CHECK(CmdAllowedWhileClosing("motor.access", "stop", -1));
    CHECK(CmdAllowedWhileClosing("motor.access", "loopMove", 0));
    CHECK(CmdAllowedWhileClosing("motor.access", "home", 0));
    CHECK(!CmdAllowedWhileClosing("motor.access", "home", 1));
    CHECK(!CmdAllowedWhileClosing("motor.access", "home", -1));
    CHECK(!CmdAllowedWhileClosing("motor.access", "loopMove", 1));
    // AI(W906-D012-M2) 20260929 St02-E2 review M2: Light Scale -- only its stop (toggle while scanning) passes the closing gate
    CHECK(!CmdAllowedWhileClosing("motor.access", "lightScale", -1));          // default: not scanning -> a start -> refused
    CHECK(!CmdAllowedWhileClosing("motor.access", "lightScale", -1, false));
    CHECK(CmdAllowedWhileClosing("motor.access", "lightScale", -1, true));      // scanning -> it stops the scan -> allowed
    CHECK(!CmdAllowedWhileClosing("motor.access", "lightScaleSave", -1, true));  // saves are not stops
    CHECK(!CmdAllowedWhileClosing("motor.access", "lightScaleDataSave", -1, true));
    CHECK(!CmdAllowedWhileClosing("motor.access", "jogP", -1, true));           // the flag only opens lightScale
    const char* maDeny[] = { "jogP", "jogN", "moveRelative", "moveAbsolute", "moveSoftLimitP", "servoToggle", "motorPowerToggle",
                             "teachGo", "teachSet", "lightScale", "reloadMotorData", "setRangeAndInit", "setSpeed", "formShow",
                             "formClose", "resetMNet", "", "(unparsable value)", 0 };
    for (int i = 0; maDeny[i]; ++i) { CHECK(!CmdAllowedWhileClosing("motor.access", maDeny[i], -1)); CHECK(!CmdAllowedWhileClosing("motor.access", maDeny[i], 0)); }
    // observer.get：讀取型 act 放行（缺 act＝open），Yield 分頁會改記憶體的擋
    CHECK(CmdAllowedWhileClosing("observer.get", "", -1));
    CHECK(CmdAllowedWhileClosing("observer.get", "timer", -1));
    CHECK(CmdAllowedWhileClosing("observer.get", "ccHistoryForm", -1));
    CHECK(!CmdAllowedWhileClosing("observer.get", "yieldClear", -1));
    CHECK(!CmdAllowedWhileClosing("observer.get", "yieldSite", -1));
    CHECK(!CmdAllowedWhileClosing("observer.get", "(unparsable value)", -1));
    // 理由字串：開頭 "closing:"、帶指令名、分 Exit／主控台
    const std::string e = ClosingRefusal("io.btnPanelClick", false);
    CHECK(e.compare(0, 8, "closing:") == 0);
    CHECK(e.find("Exit is stopping the machine") != std::string::npos);
    CHECK(e.find("'io.btnPanelClick' not run") != std::string::npos);
    const std::string c = ClosingRefusal("motor.access", true);
    CHECK(c.compare(0, 8, "closing:") == 0);
    CHECK(c.find("console Ctrl-C / window close") != std::string::npos);
}

// ---- AI(W906-D012) 20260929 [W906] Q44 A3：主控台事件（w906q44::ConsoleDecide／ConsoleHardWaitDone）-----------------------------
void T16_ConsoleEvents()
{
    // wincon.h：CTRL_C_EVENT 0、CTRL_BREAK_EVENT 1、CTRL_CLOSE_EVENT 2、CTRL_LOGOFF_EVENT 5、CTRL_SHUTDOWN_EVENT 6
    CHECK(ConsoleDecide(0, 1) == kConLetMainClose);   // 第一次 Ctrl-C：主迴圈停機後走正常關站路
    CHECK(ConsoleDecide(1, 1) == kConLetMainClose);
    CHECK(ConsoleDecide(0, 2) == kConExitNow);        // 第二次：立刻結束（今天的行為）
    CHECK(ConsoleDecide(1, 3) == kConExitNow);
    CHECK(ConsoleDecide(2, 1) == kConStopThenExit);   // X／登出／關機：等停機命令送出再結束
    CHECK(ConsoleDecide(5, 1) == kConStopThenExit);
    CHECK(ConsoleDecide(6, 1) == kConStopThenExit);
    CHECK(ConsoleDecide(2, 2) == kConStopThenExit);   // 先 Ctrl-C 再 X
    CHECK(ConsoleDecide(3, 1) == kConIgnore);
    CHECK(ConsoleDecide(4, 1) == kConIgnore);
    CHECK(std::string(ConsoleEventName(0)) == "Ctrl-C");
    CHECK(std::string(ConsoleEventName(2)) == "console window closed");
    // 等待：送出了就結束；主執行緒已在正常關站路就等到期限；期限一定結束（CTRL_CLOSE 系統 5 秒就砍）
    CHECK(kConsoleHardWaitMs < 5000);
    CHECK(!ConsoleHardWaitDone(false, false, 0, kConsoleHardWaitMs));
    CHECK(ConsoleHardWaitDone(true, false, 10, kConsoleHardWaitMs));
    CHECK(!ConsoleHardWaitDone(true, true, 10, kConsoleHardWaitMs));
    CHECK(!ConsoleHardWaitDone(false, true, kConsoleHardWaitMs - 1, kConsoleHardWaitMs));
    CHECK(ConsoleHardWaitDone(false, false, kConsoleHardWaitMs, kConsoleHardWaitMs));
    CHECK(ConsoleHardWaitDone(false, true, kConsoleHardWaitMs + 5, kConsoleHardWaitMs));
}

// ---- AI(W906-D012) 20260929 [W906] Q44 B：golden 關站段補接的步驟的狀態（純判斷；呼叫本身不在 ctest —— 會寫 D:\HT9045_Log\… 與事件紀錄）----
void T17_ShutdownStepStatus()
{
    // 寫檔三步（條碼顆數清除、OutShuttleLog、NewRecordProcess MES2109）
    CHECK(std::string(FileStepStatus(false, true,  true))  == "noop");      // golden 條件不成立 ⇒ golden 同樣不做
    CHECK(std::string(FileStepStatus(false, false, true))  == "noop");
    CHECK(std::string(FileStepStatus(true,  true,  true))  == "done");      // 照做
    CHECK(std::string(FileStepStatus(true,  false, true))  == "missing");   // bHandlerModel=false：關站時不寫
    CHECK(std::string(FileStepStatus(true,  true,  false)) == "done");      // 預估
    CHECK(std::string(FileStepStatus(true,  false, false)) == "done");      // 預估（Exit 在 bHandlerModel=false 時 step 0 就擋，走不到）
    // golden :12002-12005 條碼條件：三種之一、而且 bEnableBarCode（值同 MachineType.h:639-642：ebctInShtIntel=2、ebctEtherNetCCD=4、ebcUseOCR=5）
    CHECK(BarcodeClearApplies(2, 2, 4, 5, true));
    CHECK(BarcodeClearApplies(4, 2, 4, 5, true));
    CHECK(BarcodeClearApplies(5, 2, 4, 5, true));
    CHECK(!BarcodeClearApplies(2, 2, 4, 5, false));
    CHECK(!BarcodeClearApplies(0, 2, 4, 5, true));
    CHECK(!BarcodeClearApplies(1, 2, 4, 5, true));
    CHECK(!BarcodeClearApplies(3, 2, 4, 5, true));
    // :12047-12048 Index kit 吸嘴：沒有點＝noop；全 1203 照快照；有非 1203 點而快照 done ⇒ unverified
    CHECK(KitSuckStepStatus(0, 0, "done") == "noop");
    CHECK(KitSuckStepStatus(0, 0, "") == "noop");
    CHECK(KitSuckStepStatus(4, 4, "done") == "done");
    CHECK(KitSuckStepStatus(2, 2, "stub") == "stub");
    CHECK(KitSuckStepStatus(2, 2, "failed") == "failed");
    CHECK(KitSuckStepStatus(2, 1, "done") == "unverified");
    CHECK(KitSuckStepStatus(2, 0, "failed") == "failed");
    CHECK(KitSuckStepStatus(2, 0, "unverified") == "unverified");
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
    T14_SimNoRestriction();
    T15_ClosingAllowList();    // AI(W906-D012) 20260929
    T16_ConsoleEvents();       // AI(W906-D012) 20260929
    T17_ShutdownStepStatus();  // AI(W906-D012) 20260929 B
    std::printf("test_mainclose_stop: %s (%d check(s), %d failure(s))\n", g_fail ? "FAIL" : "PASS", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
