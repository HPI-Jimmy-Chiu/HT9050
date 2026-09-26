/* ht9045_wire_statussecurity.js -- Status.Security.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260916
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-LEVELSET) 20260916。這個檔是手寫的，不是 gen_wire.py 產生的 ——
 * 理由見下面「為什麼沒有靜態對照表」。
 *
 * 來源表單    cSecurity.dfm / cSecurity.cpp
 *
 * ---------------------------------------------------------------------------
 * 這一頁橫跨三套不同的持久化機制，這次只接第一套
 * ---------------------------------------------------------------------------
 *   (1) 179 組 radio（權限表）   -> system\levelset.dat
 *       cSecurity.cpp:1620 GetLevelSet() / :1682 SetLevelSet()，
 *       整塊 ReadData/WriteData 一個 LAST_LEVEL_SET（cprod.h:1148，
 *       `int AccessLevel[256]`，1024 bytes）。**本檔接的就是這一套。**
 *
 *   (2) 13 個 checkbox ＋ rgJamLevel / rgMachineStatusBit8
 *                                -> Error\English\JAM0000.dat（ini）
 *       cSecurity.cpp:272 定義 FileNameJam000，寫入集中在 :1269-1324。
 *       **未接**：那個檔不在 wb_serve 的 40 支 sysfile 表裡（Error 目錄只列了
 *       AlarmDescription.ini 與 AlarmCodeList.txt）。要接得先補表。
 *
 *   (3) config\Security_new.def -> **與這一頁無關**。
 *       它是 cAuthority.cpp 讀的各表單 Enable/Disable 旗標；cSecurity.cpp 全檔
 *       0 次提及它。20260915/16 的日誌寫「後端 securityNew 已在服務中，可以直接
 *       接」是把兩個檔搞混了，那個推論是斷的。
 *
 * ---------------------------------------------------------------------------
 * 為什麼沒有靜態對照表（id -> 檔/區段/鍵）
 * ---------------------------------------------------------------------------
 * 這一頁的 722 個 radio **沒有任何一個帶 id**，包住它們的 fieldset 也沒有。
 * 引擎的 sysEnums 是靠 `<fieldset id=X>` 取容器，這條路在這裡走不通。
 * 而索引本身就在 DOM 裡：
 *     title="MySecurity_Panel_N : TMySecurity（[NN] Main - Tools）"
 * 所以引擎的 sysLevels 模式改成執行期從 title 掃出 [NN] 建表，並自我驗證。
 *
 * ⚠ N 與 NN 是兩個不同的數，這是這一頁最容易靜默寫錯的地方：
 *     MySecurity_Panel_14  ->  [32] Main - Temperature Deg Setup
 *     sec_sbTools_0        ->  [14] Tools - Tray Form
 *     sec_sbTools_29       ->  [178] Tools - Tray Function
 *   `_<n>` 是「該 scrollbox 內的第幾個」，`[NN]` 才是 AccessLevel 的下標。
 *   拿 `_<n>` 當索引會把 179 個權限全部寫到錯的格子；因為值域都是 0..3，
 *   寫進去不會報錯，只會在下次有人登入時發現權限全亂。
 *
 * 期望 179 組：cSecurity.cpp:71-260 的 mySecurityPal.push_back 實測 179 筆
 * （[00] 到 [178]）。該檔 :65 的註解寫「178-entry」是錯的，forms/fSecurity.h:188
 * 的 179 才對。掃到的組數與這個數字對不上，引擎會整頁拒接。
 *
 * ---------------------------------------------------------------------------
 * ⚠ 已知落差：寫進去的值，這個行程不會讀到
 * ---------------------------------------------------------------------------
 * C++ 端 GATE SEC1（cSecurity.cpp:71-260，mySecurityPal 全部建構）與
 * GATE SEC-W1/SEC-W2（:1642 / :1690，兩處 WriteData）都還關著，iMaxLevelItem
 * 恆為 0。所以：
 *   * handler 行程自己不會寫 levelset.dat，也不會在執行中重讀；
 *   * web 寫進去的值要等 handler 下次啟動 GetLevelSet() 才生效。
 * 這是使用者 20260916 明確定的範圍：「讀檔跟寫檔要先做起來，實際使用先不用管」。
 *
 * 另一個落差：畫面只有 4 個選項（Operator/Engineer/Supervisor/HonPrec），但
 * golden 在 CosFunction.bSecurityHave5Level 時是 5 階（0..4）。伺服器收 0..4，
 * 所以 5 階機台的值不會被夾壞；但這一頁表達不了第 5 階 —— 讀到 4 的那一格會
 * 被記成 UNFILLABLE 並讓整頁拒寫，而不是靜靜挑一顆選起來。
 */
HT9045Wire.register({
  page: 'Status.Security.html',
  slug: 'statussecurity',
  // Steven 20260916：頁面自己的存檔鈕（引擎不再注入浮動的 Save to recipe / Reload）
  // 依據：golden cSecurity.cpp:510 FormClose() -> SetLevelSet()。這一頁沒有存檔鈕，golden 是「關閉表單時才寫檔」，所以 Exit 就是存檔動作 —— 這是忠實移植，不是自訂
  saveBtn: 'SecurityExit',
  // ⚠ 引擎在捕獲階段攔截這顆鈕，所以按下去會**存檔但不會離開頁面**。
  //   golden 的 Exit 是「關閉表單 -> FormClose -> SetLevelSet 寫檔」，
  //   web 這邊沒有表單關閉的語意，而存檔流程是非同步的（預演->確認->寫入->重讀），
  //   放行關閉會把流程中斷。這行說明會顯示在狀態列，讓操作員知道行為不一樣。
  saveBtnNote: 'golden 是關閉表單時才寫檔，所以這裡按 Exit＝存檔；存完不會離開頁面',
  fields: {
  },
  optional: {
  },
  // i32 投影：/api/system/levelset ＋ system.levels.put
  //   file   伺服器那側的名字（wb_serve.cpp 的 SysBinTable）
  //   expect 期望掃到的組數；對不上就整頁拒接
  sysLevels: {
    file: 'levelset',
    expect: 180   // Steven 20260924（審查第 8 輪 M-3）：golden V912 180 組（[00]..[179]）；W906_SecurityBoot iMaxLevelItem=180
  },
  kb: {
  },
  /* PENDING —— 這一頁還沒接的部分，不是漏掉，是各有原因
   *
   * cbJamNeedRed / cbSilentMode / cbUnlockPassWord / cbIncludeMTBA /
   * chkCheckContAlarm / chkO17 / cbAddAlarmLog / cbN27AlarmSel /
   * cbN27AlarmSelByArea / cbN27AddBoard / chkTCPAlarm / cbContAlarmNotUpload /
   * chkAlarmAfterFullTray / rgJamLevel / rgMachineStatusBit8
   *   -> Error\English\JAM0000.dat，不在 wb_serve 的 sysfile 表裡（見檔頭 (2)）。
   *
   * sbSupervisor / sbEngineer / btnHonPrec / btnOperator / ChangePassword 那組
   *   -> 密碼與登入，走 auth.login 指令不是檔案讀寫，另案。
   */
  pending: {
  }
});
