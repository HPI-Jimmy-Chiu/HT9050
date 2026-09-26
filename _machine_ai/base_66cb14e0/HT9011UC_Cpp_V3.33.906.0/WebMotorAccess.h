// =============================================================================
//  WebMotorAccess.h  --  HW.MotorTest／HW.teach 的馬達按鈕 → C++（WS 命令 `motor.access`）
//
//  AI(W906-W4-MOTOR) 20260925.  NOT in golden（golden 是 VCL 按鈕事件，本檔是它在網頁架構下的落點）。
//
//  ## 為什麼有這個檔
//
//  週末任務 W4（使用者 20260924 晚：「MotorTest畫面功能完善化，確實橋接JSON到C++控制，
//  C++控制可參考Eastsun之前提供的做法」）。量到的起點（NB2 R1 預勘＋20260925 重量）：
//    * `web/page/motor-access.js` 線上模式**根本沒送出命令** —— publish() 只寫 localStorage，
//      ack 去輪詢一個沒有人寫的靜態檔 `JSON/motor-access-ack.json`；
//    * 非 motion 類按鈕（含 STOP）**立刻顯示假成功** `finish('done','no motion')`；
//    * jog 放開只 `finish('aborted')`，**不送停止**。
//    * 移植樹 .cpp/.h 裡 `motor.access` 與 28 個 action 名稱命中 0。
//
//  ## 分層（使用者 20260924 晚的通則，逐字見 docs/WEEKEND_PLAN_20260925.md §0.6）
//
//      畫面、JSON 形狀            = Steven（motor-access.json 的 38 個命令、request／ack 欄位）
//      按鈕的邏輯與互鎖           = golden uMotorTest.cpp／uteach.cpp（逐行號對帳）
//      命令語意與底層（1203 軸）  = EastSun 的 TPci1203Control（實機驗證過）
//      非 1203 軸（MN200 等）     = golden 的 MOT[] 馬達物件（那是它唯一的路）
//
//  「衝突時跟 EastSun，落差寫下來」—— 落差清單在 WebMotorAccess.cpp 檔尾與 docs/W4_PROGRESS.md。
//
//  ## 傳輸
//
//      瀏覽器 {type:"cmd", cmd:"motor.access", tag:<motorId>, value:"<request JSON 字串>"}
//        -> wb_serve 主迴圈（單執行緒，與 1203 監看器同一條）
//        -> MotorAccessParse() -> MotorAccessDispatch(req, backend)
//        -> server.CompleteCommand(id, ok, ok ? ackJson : 拒絕理由)
//
//  request 就是 motor-access.js 原本組好的那一包（seq/id/source/button/action/kind/motors/params），
//  不另外發明欄位。value 走字串是因為 WebCommand 只帶 {id, cmd, tag, value}（CommandQueue.h）。
//
//  ## 後端抽成介面的理由
//
//  這台筆電沒有 1203 卡，`Pci1203Control()` 是 NULL；分派邏輯（哪個按鈕 → 哪個 golden 互鎖 → 哪個
//  1203 命令）仍然要能驗。所以分派只認 IMotorAccessBackend，wb_serve 用 MotorAccessLiveBackend()
//  （HSys.MotTable＋MOT[]＋Pci1203Control／Pci1203Monitor），ctest 用假後端逐條記錄它送了什麼。
//
//  ⚠ 誠實規則：沒有接上的 action 一律回 ok=false 並說出原因（頁面顯示成 error），**不回假成功**。
//    唯一回 done 而不碰任何硬體的，是 golden 本來就只動畫面的按鈕（setPos1/setPos2/refreshParameter），
//    ack 的 layer 會寫 "ui" 讓人看得出來。
// =============================================================================
#ifndef HT9045_WEBMOTORACCESS_H
#define HT9045_WEBMOTORACCESS_H

#include <map>
#include <string>
#include <vector>

#include "EtherCAT/Pci1203Control.h"   // Pci1203Cmd / Pci1203CmdResult（純結構，無連結相依）

namespace ht9045 {

struct MotorAccessReq {
    long long                     seq;       // motor-access.js 的遞增序號（ack 帶回去對號）
    std::string                   id;        // "cmd-<seq>"
    std::string                   source;    // "uMotorTest" | "uteach"
    std::string                   button;    // golden 元件名，例 "sbMotorTest_JogP"
    std::string                   action;    // 例 "jogP"
    std::string                   kind;      // "motion" | "control" | "edit"
    std::vector<std::string>      motors;    // Mot_Table 的 Alias（例 "MInArmX"）
    std::map<std::string, double> num;       // params 裡的數字
    std::map<std::string, bool>   flag;      // params 裡的布林
    MotorAccessReq() : seq(-1) {}
};

// request JSON（字串）→ MotorAccessReq。false＋why：不是物件、缺 action／source、型別不對。
// 不做語意檢查（那是 Dispatch 的事）。
bool MotorAccessParse(const std::string& json, MotorAccessReq& out, std::string& why);

// 一軸在兩套編號裡的身分。
struct MotorAccessAxis {
    int         motIndex;    // MOT[] 下標（Mot_Table 的 No "M%02d"），-1 = 表上不是 M+數字
    std::string cardModel;   // Mot_Table 的 CardModel（"PCI1203" / "MN200" / ...）
    int         boardId;     // Mot_Table BoardID（1203 = EtherCAT 站號）
    int         port;        // Mot_Table Port（1203 = 站內軸號）
    int         axis;        // EastSun 1203 監看器的軸槽，-1 = 對不到
    std::string why;         // axis == -1 的原因（給操作員看）
    bool        motorLive;   // MOT[motIndex].Motor != NULL（golden 馬達物件存在）
    // Mot_Table 的 Enable 欄（這台機台有沒有裝這一軸）。⚠ 1203 路徑用這個，不用 MotorGolden.enable：golden 在 SOFT_SIMULTE 建置把
    //   每一軸的 Motor->Enable 強制設 false（cinitial.cpp:4011-4034，golden 的「沒有硬體」模擬），而 EastSun 的 1203 層在那個建置
    //   照樣驅動真的軸（0918 裁決甲）；真機建置 golden 的 Motor->Enable 正是這個欄位（bHasMotor 時）⇒ 兩種建置行為一致。
    bool        tableEnable;
    MotorAccessAxis() : motIndex(-1), boardId(-1), port(-1), axis(-1), motorLive(false), tableEnable(true) {}
    bool Is1203() const { return cardModel == "PCI1203"; }
};

// golden 馬達物件（MOT[i].Motor）的即時參數 —— 運動命令的互鎖與單位換算都照這些值（W4-b）。
struct MotorGolden {
    bool     valid;          // MOT[i].Motor 存在
    bool     enable;         // Motor->Enable（真機建置 = Mot_Table Enable；⚠ SOFT_SIMULTE 建置 golden 一律設 false ⇒ 1203 路徑改看 MotorAccessAxis.tableEnable）
    double   gearRatio;      // Motor->GearRatio（使用者單位 = 卡片脈波 × GearRatio）
    bool     direction;      // Motor->Direction（⚠ 1203 軸的慣例待使用者決定，見夜間報告 §0 第 5 件）
    bool     homeDirection;  // Motor->HomeDirection
    int      softP, softN;   // Motor->PSoftLimitP／N（使用者單位）
    unsigned initSpeed, jogHigh, jogLow;   // Motor->InitSpeed／PJogHighSpeed／PJogLowSpeed
    double   acc, dec;       // Mot_Table 的 Acc／Dec（TMyEtherCatMotor::SetSpeed 寫進 PAR_AxAcc／PAR_AxDec 的值）
    int      homeFlag;       // MOT[i].HomeFlag：0 未歸零、1 完成、2 失敗
    bool     zStack;         // MLoaderZ／MEmptyZ／MColorZ／MAuto1Z..6Z：golden SetSpeed 用 low..high 內插
    bool     indexMotor;     // MTestY1／Z1／Z2／Y2：golden TMyMotor::SetSpeed 對這四軸整段空白（不設速度）
    bool     galilIndex;     // indexMotor 且 INDEX_MOTION_CARD==0（golden 走 Galil 分支）
    bool     armZ;           // MInArmZA..ZH／MOutArmZA..ZH／…／MOutSortAa,Ab：golden 單軸歸零後再去 ZSafePos（uhome.cpp case 500）
    bool     scaleMotor;     // MInArmXScale／MInArmYScale／MOutArmXScale／MOutArmYScale：golden btnHome 直接 ResetPos(0)
    // NB2 R21 W4B-1：golden lM00Click（uMotorTest.cpp:740-743）`#ifndef SOFT_SIMULTE if(Motor->Enable==false) return;` ——
    //   真機組態下 Enable=0 的軸根本選不到，所有按鈕都碰不到它；模擬組態可選。後端依建置組態算好。
    bool     selectable;
    int      inShuttle;      // 1 = MInShuttle1、2 = MInShuttle2、0 = 其他（golden MotorMovePosition 的閘門互鎖只對這兩軸）
    // NB2 R23 W4C-3：golden TMyMotor::Home（mymotor.cpp:1647）SetADCRate(100) ⇒ dAcc/dDec = 資料庫值；
    //   TMyEtherCatMotor::SetHomeSpeed（myEthercatmotor.cpp:1703）寫 PAR_AxHomeVelLow/High = PHomeLowSpeed/PHomeHighSpeed、HomeAcc/Dec = dAcc/dDec
    unsigned homeHigh, homeLow;   // Motor->PHomeHighSpeed／PHomeLowSpeed（卡片單位）
    double   accDb, decDb;        // Motor->GetAccDataBase()／GetDecDataBase()
    MotorGolden()
        : valid(false), enable(false), gearRatio(1.0), direction(false), homeDirection(false),
          softP(0), softN(0), initSpeed(0), jogHigh(0), jogLow(0), acc(0.0), dec(0.0),
          homeFlag(0), zStack(false), indexMotor(false), galilIndex(false), armZ(false), scaleMotor(false),
          selectable(true), inShuttle(0), homeHigh(0), homeLow(0), accDb(0.0), decDb(0.0) {}
};

// 速度參數的哪一個（setJogHighSpeed 等四顆鈕、兩個軟體極限鈕）
enum MotorParamWhich {
    kParamJogHigh = 0, kParamJogLow, kParamHomeHigh, kParamHomeLow, kParamSoftP, kParamSoftN
};

class IMotorAccessBackend {
public:
    virtual ~IMotorAccessBackend() {}

    // motorId（Alias）→ 身分。false = 馬達表上沒有這個 Alias。
    virtual bool Resolve(const std::string& motorId, MotorAccessAxis& out) = 0;

    // ---- EastSun 1203 層 ----
    // TPci1203Control 可用嗎（這台沒有 SDK／Open() 拒絕 → false＋why）。
    virtual bool Pci1203Ready(std::string& why) = 0;
    virtual int  Pci1203AxisCount() = 0;              // 監看器軸槽數
    virtual bool Pci1203AxisOpened(int axis) = 0;     // 這一槽有開成功的軸
    virtual bool Pci1203ServoOn(int axis, bool& known) = 0;   // motionIO 的 SVON 位元
    virtual Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) = 0;
    virtual void Pci1203NoteRefusal(long long wireId, const std::string& name,
                                    const std::string& why) = 0;

    // ---- golden 層（MOT[]、表單狀態）----
    // golden btnStopClick 除了停「這一軸」以外的部分：
    //   uMotorTest  StopAllMotor(); bSingleHome=false;          （uMotorTest.cpp:1650-1651）
    //   uteach      StopAllMotor(); Tech_Part=0; Gali "ST";      （uteach.cpp btnStopClick）
    virtual void GoldenStopAll(const std::string& source) = 0;
    // golden 停「這一軸」（btnStopClick 尾段、jog 的 MouseUp）：INDEX_MOTION_CARD==0 的 Index 四軸 → Gali "ST"，
    //   其他 → MOT[i].PCIL132_StopMotor()。
    virtual void GoldenStopMotor(int motIndex) = 0;
    virtual bool GoldenServoOn(int motIndex, bool& known) = 0;    // MOT[i].Led[iServoOn]（先 ScanMotorStatus）
    virtual void GoldenServoOnOff(int motIndex, bool on) = 0;     // MOT[i].ServoOnOff(on)

    // ---- W4-b：運動命令用 ----
    virtual bool Pci1203CmdPos(int axis, double& card) = 0;       // 監看器的 cmdPos（卡片單位）；false = 讀不到
    virtual bool Pci1203AxisReady(int axis) = 0;                  // STA_AX_READY（golden MotionDone 的等價）
    virtual bool GoldenMotor(int motIndex, MotorGolden& g) = 0;   // false = MOT[i].Motor 為 NULL
    virtual bool GoldenSafeDoorOpen(int motIndex) = 0;            // MOT[i].Motor->CheckIsSafeDoorOpen()（TMyMotor::JogP／MotorMove 第一行）
    virtual bool GoldenSafeDoorClosed() = 0;                      // CheckSafeDoorIsClosed()（jog MouseDown，uMotorTest.cpp:826／:876）
    virtual bool GoldenMotorCanRun() = 0;                         // TfMotorTest::IsMotorCanRun(true)（uMotorTest.cpp:1280）的本體
    virtual bool GoldenMoveLocked(int motIndex) = 0;              // fCanMove*==false || mapLockList.size()!=0（TMyMotor::MotorMove）
    virtual int  GoldenReadPos(int motIndex) = 0;                 // MOT[i].ReadPos()（非 1203 軸）
    virtual void GoldenJog(int motIndex, bool positive, int pct) = 0;   // 非 1203：MOT.SetSpeed(pct)＋MOT.JogP/JogN
    virtual int  GoldenMotorMove(int motIndex, int target, int pct, bool setSpeed) = 0;  // 非 1203：MOT.SetSpeed＋InitMOTParameter＋MotorMove
    virtual void GoldenSetParam(int motIndex, MotorParamWhich which, int value) = 0;     // MOT[i].Motor->P*（只改記憶體，golden 同）

    // ---- W4-b2：跨拍工作（HOME／LoopMove）與其餘按鈕 ----
    virtual bool Pci1203AxisState(int axis, unsigned& state) = 0; // 監看器樣本的 Acm_AxGetState 原值（STA_AX_*）
    virtual void GoldenSetHomeFlag(int motIndex, int v) = 0;      // MOT[i].HomeFlag
    virtual int  GoldenZSafePos() = 0;                            // 全域 ZSafePos（Motor/mymotor.cpp:81）
    virtual bool GoldenSystemStart() = 0;                         // 全域 SystemStart
    virtual void GoldenResetMNet() = 0;                           // ResetMNet(0,"MNet斷電","Power Off",false)（golden uMotorTest.cpp:1723）
    virtual void GoldenSetRangeRate(int motIndex, bool range, int v) = 0;  // SetRange/SetRate＋InitMotor(Address)＋HomeFlag=0

    // ---- NB2 R21（W4-b1 覆核）----
    virtual unsigned long Pci1203PollCount() = 0;                 // 監看器 card().pollCount（W4B-5：命令後要等一次新的輪詢）
    // W4B-2：golden MotorMovePosition（mymotor.cpp:652-697）—— SHUTTLE_FLOODGATE==1 時對 InShuttle1／2 先下開閘
    //   （Cylinder[..Floodgate].Off()），兩顆 OffSensor 都到位才准動。回 true = 可以動（或沒裝閘門）。
    virtual bool GoldenShuttleFloodgateReady(int which) = 0;
    virtual unsigned GoldenReadSpeed(int motIndex) = 0;           // W4B-6：MOT[i].Motor->ReadSpeed()（Index 軸 golden SetSpeed 不動它）

    // ---- NB2 R23（W4-b2 覆核）----
    // W4C-4：golden uMotorTest Timer1Timer（:919）`if(IsSafeLockCheck()) Close();` —— csystem.cpp:478（會 On 安全鎖開關，golden 同）
    virtual bool GoldenSafeLockActive() = 0;
};

struct MotorAccessOutcome {
    bool        ok;
    std::string ackJson;   // ok：JSON 物件（併進 WS ack）；!ok：拒絕理由（純文字）
    MotorAccessOutcome() : ok(false) {}
};

// 分派。wireId = WS 命令 id（寫進 1203 的稽核線，讓畫面分得出是哪一次按鈕）。
MotorAccessOutcome MotorAccessDispatch(const MotorAccessReq& r, long long wireId,
                                       IMotorAccessBackend& be);

// 38 個 catalog 命令的 action 名稱各自接到哪裡（給 /api 與文件用；也讓測試能逐條比對 catalog）。
//   "live"   = 已接上（W4-a：stop／servoToggle）
//   "ui"     = golden 只動畫面，C++ 回 done 不碰硬體
//   "queued" = 尚未接上，誠實拒絕（W4-b 運動／參數、W5 教導）
const char* MotorAccessActionStatus(const std::string& action);

// ---- golden 的單位與速度換算（純函式，W4-b；測試直接驗手算值）----
// 使用者目標 → 卡片位置：TMyMotor::GetRealPos（Motor/mymotor.cpp:1381）的 ChangeToFloatNonPcnt＋上下逼近迴圈。
//   EtherCAT 卡不翻號（只有 Contec 卡依 Direction 翻）；呼叫端保證 Direction==0（§0 第 5 件）。
int MotorUserToCard(int user, double gearRatio);
// 卡片 cmdPos → 使用者位置：TMyEtherCatMotor::ReadPos = (int)ReadRealPos() × GearRatio（ReadRealPos 回 int）。
int MotorCardToUser(double card, double gearRatio);
// TMyMotor::SetSpeed(pct)（mymotor.cpp）→ TMyEtherCatMotor::SetSpeed(s)（myEthercatmotor.cpp:898）的卡片速度。
struct Motor1203Speed {
    unsigned s;              // golden 的 iSpeed（= Motor->ReadSpeed() 之後會回的值）
    double   velLow, velHigh, acc, dec;   // PAR_AxVelLow／VelHigh／Acc／Dec
    bool     skip;           // golden 在 PJogHighSpeed==0 時直接 return（不設任何速度）
};
Motor1203Speed MotorSpeedFromPct(int pct, const MotorGolden& g);

// W4-b2：跨拍工作（HOME 完成偵測、LoopMove 狀態機）。wb_serve 每個 500 ms 拍子呼叫一次（golden Timer1Timer 的角色）。
void MotorAccessTick(IMotorAccessBackend& be, int elapsedMs, bool operatorConnected = true);
//   operatorConnected = WebBridgeServer::ControlOwner()!=0。W4B-4：jog（moveVel）在操作員連線消失（關分頁、斷網、
//   權杖閒置 10 分鐘被收回）時由這裡停下 —— golden 的放開是本機滑鼠事件，網頁端要自己補這個死人開關。
struct MotorAccessJobState {
    int           homesActive;
    bool          loopActive;
    std::string   loopMotor;
    int           loopTask;        // golden DoLoopMove 的 Task：1 去 pos1、10 等、50 去 pos2、60 等
    unsigned long loopCount;       // golden dwLoopCount
    std::string   lastNote;        // 最近一次工作結束的原因（給畫面與日誌）
    int           jogsActive;      // W4B-4：正在 jog 的軸數
};
MotorAccessJobState MotorAccessJobs();
void MotorAccessResetJobs();       // 測試用
// NB2 R23 W4C-4：golden Timer1Timer（uMotorTest.cpp:921-926）`if(fNote->fShow || MyMessageBox->fShow) { btnLoopMove->Down=false;
//   btnHome->Down=false; bSingleHome=false; }` —— 告警一出現就放棄 HOME／LoopMove。wb_serve 的 ShowErrorMessage 入口
//   （W906_AlarmStopLikeGolden，golden note.cpp:795 StopAllMotor）呼叫這支。golden 的 StopAllMotor 是「全部馬達」，
//   而 EastSun 監看器開的 1203 軸走不到 golden 的停止（m_Axishand 不是它開的）⇒ 這裡照 DoStop 對每一個開成功的 1203 軸補送停止。
void MotorAccessOnAlarm(IMotorAccessBackend& be, const std::string& what);

// wb_serve 用的真實後端（WebMotorAccessLive.cpp；只連進 wb_serve）。
IMotorAccessBackend& MotorAccessLiveBackend();

}  // namespace ht9045

#endif  // HT9045_WEBMOTORACCESS_H
