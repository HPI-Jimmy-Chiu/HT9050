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
    std::map<std::string, std::string>          str;   // W5-b：params 裡的字串（例 btn＝教導點的 Set／Go 按鈕名）
    std::map<std::string, std::vector<double> > arr;   // W5-b：params 裡的數字陣列（只收全是數字的）
    //AI(W906-W5-b) 20260925: params 裡「值全是數字」的物件（例 fields＝{畫面欄位 id: 值}）。W5B-4：教導點的目標值由 C++ 依 golden
    //  Tag 選出的那一列、按欄位名取，不再收頁面排好的 targets 陣列；值是 null／字串的鍵不收（＝沒給，C++ 會拒絕）。
    std::map<std::string, std::map<std::string, double> > obj;
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
    //AI(W906-MT-E2) 20260925: the rest of what golden Motor Test shows for the selected motor -- the acks of the
    //  parameter actions carry them as "cur" (same keys as /api/struct/motor/runtime "cur", ChanMotorPoints.cpp).
    unsigned range, rate;         // Motor->ReadRange()／ReadRate()（golden UpdateMotorParameter :668；lM00Click :757-758 edtMotorRate/Range）
    int      readSpeed;           // MOT[i].GetSpeed()（golden Timer1Timer :982 lblRealSpeed —— 上一次 SetSpeed 的 s，不是卡片正在跑的速度）
    int      lastHomePos;         // Motor->LastHomePos（golden Timer1Timer :965 edtHomeOffset）
    //AI(W906-MT-E3c) 20260925: what golden TMyEtherCatMotor::SetRate / InitMotor read from the object (Test Range / Test Rate, R4).
    unsigned iSpeed;              // Motor->ReadSpeed() = HTMotor::iSpeed (the last SetSpeed's s; golden SetRate :587/:591)
    int      motorClass;          // golden InitMotor's MotorType arm: 0 Servo_Motor, 1 Rotate_Motor, 2 otherwise (kInitCfgMotor*)
    //AI(W906-MERGE-56bbf785) 20260926: both sides added this field (machine MT-E3c for InitMotor, laptop W5-b for the teach page's
    //  ORG lamp) -- one field, same source. 覆核 W5B-R2 —— golden TMyEtherCatMotor::InitMotor（myEthercatmotor.cpp:252）開軸時
    //  `CFG_AxOrgLogic = bSensorType ? 0 : 1`，ScanMotorStatus（:860／:868）的 Led[iHomeLed] 直接讀 ORG 位元，前提就是這個設定。
    bool     sensorType;          // Motor->bSensorType (golden CFG_AxOrgLogic = bSensorType ? 0 : 1, :252)
    bool     in1Logic;            // Motor->bIn1Logic   (golden SetEtherCatInType AlmLogic)
    // W5-b（uteach）：golden 教導頁依 ActiveMotorIndex 分支的幾類軸
    bool     teachNoServoOff;     // MTrayBracketZ／MMagazine：SetButton140Click（uteach.cpp:3393）手動教導時不關伺服
    bool     indexZ;              // MTestZ1／MTestZ2：SetButton140Click（:3384）不做手動教導
    bool     indexY;              // MTestY1／MTestY2：SetButton064Click（:3683）不關伺服
    int      rotateKit;           // 1 = MInRotateKit、2 = MOutRotateKit：GoButton140Click（:3455）加背隙補償
    //AI(W906-W5-b) 20260925: W5B-6 —— Motor->PServoAlarmOn。golden TMyMotor::ServoOnOff(true)（mymotor.cpp:1904-1940）在這種軸上
    //  開伺服後 MySleep(200)＋PCIL132_ResetPos（命令位置＝編碼器位置）；EastSun 不提供 Acm_AxSetCmdPosition（Pci1203Control.h:222
    //  「MUST STAY ABSENT」）⇒ 這種 1203 軸手動教導結束後，相對移動與 jog 改以編碼器位置為基準（WebMotorAccess.cpp g_encBase）。
    //  [AI(W906-MERGE-56bbf785) 20260926: the machine's EastSun has since opened Acm_AxSetCmdPosition for Reload Motor Data / InitMotor
    //  (kCmdAxSetCmdPos, MT-E1); the teach page still takes the laptop's encoder-base route -- not changed by the merge.]
    bool     servoAlarmOn;
    MotorGolden()
        : valid(false), enable(false), gearRatio(1.0), direction(false), homeDirection(false),
          softP(0), softN(0), initSpeed(0), jogHigh(0), jogLow(0), acc(0.0), dec(0.0),
          homeFlag(0), zStack(false), indexMotor(false), galilIndex(false), armZ(false), scaleMotor(false),
          selectable(true), inShuttle(0), homeHigh(0), homeLow(0), accDb(0.0), decDb(0.0),
          range(0), rate(0), readSpeed(0), lastHomePos(0), iSpeed(0), motorClass(0), sensorType(false), in1Logic(false),
          teachNoServoOff(false), indexZ(false), indexY(false), rotateKit(0), servoAlarmOn(false) {}
};

//AI(W906-MT-E3c) 20260925: golden SW[SwMotorRelay] as the Motor Power button sees it (IMotorAccessBackend::GoldenMotorPower).
struct MotorPowerState {
    bool relayOn;          // SW[SwMotorRelay].OutValue (golden btnMotorPowerClick :1623 / FormShow :1038) -- AFTER the sync below
    bool cardKnown;        // the 1203 monitor read back the relay's DO bit this poll
    bool card;             // that bit
    bool synced;           // OutValue was set from the card bit by this call (the two differed)
    bool motorPowerState;  // global bMotorPowerState (cmydef.cpp:2810)
    MotorPowerState() : relayOn(false), cardKnown(false), card(false), synced(false), motorPowerState(false) {}
};

//AI(W906-MT-E2) 20260925: what Reload Motor Data could put back from Mot_Table.csv (IMotorAccessBackend::GoldenReloadMotorParams).
struct MotorReloadResult {
    bool        read;             // the file was read and its header understood
    std::string why;              // !read: why not (the file path is in it)
    int         applied;          // motors whose table values were re-applied to MOT[i].Motor in place
    int         identityChanged;  // motors whose row identity (No/Alias/CardModel/BoardID/Port/IP/Enable) changed -> nothing applied
    std::string firstChanged;     // the first of those, for the ack ("restart wb_serve")
    MotorReloadResult() : read(false), applied(0), identityChanged(0) {}
};

//AI(W906-W5-b) 20260925: W5B-2 —— golden 教導頁登錄表的一列（TechPara／TechTwoPara 的一個 push_back；WebTeachButtons.gen.inc）。
//  TeachRegistry.P／T 是**這台機台上實際登錄的**序列：位置＝golden 的索引（FormShow 設給按鈕的 Tag）。
struct TeachRegRow {
    char        owner;              // 'P' TECH_PARA、'T' TECH_TWOPARA
    int         mot0, mot1;         // golden MotorSelect[0]／[1]（MOT 下標；TECH_PARA 的 mot1 = -1）
    std::string name0, name1;       // golden 列舉名（例 MInShuttle1；Mot_Table 別名可能不同）
    std::string key0, key1;         // ini 鍵
    std::string edit0, edit1;       // golden SetEdit[0]／[1] 的畫面欄位 id（頁面欄位同名）
    std::string setBtn, setHandler, goBtn, goHandler;   // 按鈕名與它的 golden 處理函式（.dfm OnClick＋建構子覆寫）
    //AI(W906-W5-b) 20260925: 覆核 R-W5B-2 —— seq＝這一列在產生表裡的序號（＝golden 建構子的執行順序，TechPara 與 TechTwoPara 交錯）；
    //  vis＝建構子最後一個引數 Visible（golden TECH_PARA 建構子拿它設 funButton／btGo->Visible，:61-74）。同一顆按鈕最後執行的那一列決定它看不看得見。
    int         seq;
    bool        vis;
    TeachRegRow() : owner('P'), mot0(-1), mot1(-1), seq(-1), vis(true) {}
};
//AI(W906-W5-b) 20260925: 覆核 R-W5B-2／R-W5B-3 —— .dfm 上一顆「處理函式是教導處理函式或有登錄」的按鈕（WebTeachButtons.gen.inc 的 W5B_BTN）
struct TeachBtnInfo {
    std::string handler;       // .dfm OnClick 再套建構子覆寫
    int         dfmTag;        // .dfm 的 Tag（沒寫＝0）：沒登錄的按鈕 golden FormShow 不設 Tag，處理函式照這個值讀清單
    bool        dfmVisible;    // .dfm 的 Visible（沒寫＝true）
    std::string selfOther;     // 登錄以外的 Visible 賦值："" 沒有、"hide" 無條件 false、"hideCond" 只有帶條件的 false、"dyn" 可能設 true
    std::string hiddenWhy;     // 非空＝靜態確定按不到（按鈕 Enabled=false 或父物件看不見／停用）的理由
    bool        lateReg;       // golden 只在 InitialFormOncetime（:5998-6014，AOI TopBottomInstall）登錄；移植樹沒有 FrmAOI
    TeachBtnInfo() : dfmTag(0), dfmVisible(true), lateReg(false) {}
};
struct TeachRegistry {
    std::vector<TeachRegRow> P, T;
    // golden FormShow（uteach.cpp:1518-1521）：Set 鈕 Tag 改成軸控鈕的 Tag，處理函式 MotorTrayXClick 讀 TechMotorAxle[Tag] → 只選這顆馬達
    std::map<std::string, std::pair<int, std::string> > selectOnly;   // 按鈕名 → (MOT 下標, golden 列舉名)
    std::map<std::string, TeachBtnInfo> btns;                         // AI(W906-W5-b) 20260925: R-W5B-2／3
};
// 一顆教導鈕在 golden 會做什麼（MotorAccessResolveTeachButton 的結果）
enum TeachHandlerKind { kTkNone = 0, kTkSet140, kTkGo140, kTkSet020, kTkGo020, kTkSet064, kTkSelectOnly, kTkOther };
struct TeachTarget {
    std::string      btn, handler;
    TeachHandlerKind kind;
    char             ownList;       // 設定這顆按鈕 Tag 的那一列在哪張清單（'P'／'T'；最後登錄的勝出）
    int              ownIndex;      // ＝golden 的 Tag
    char             readList;      // 處理函式讀哪張清單（140 → 'P'，020／064 → 'T'；其他 0）
    bool             quirk;         // readList != ownList：golden 拿「另一張清單的同號列」
    bool             outOfRange;    // golden 會讀 readList[Tag] 而 Tag 超出那張清單（未定義行為）
    TeachRegRow      row;           // 處理函式實際要用的列（kTeachQuirkPolicy 決定怪按鈕用哪一列）
    int              selectMot;     // kTkSelectOnly：要選的馬達
    std::string      selectName;
    //AI(W906-W5-b) 20260925: 覆核 R-W5B-2／3
    bool             unregistered;  // 這台機台沒有登錄它（Tag＝.dfm 的值）：golden 讀 readList[dfmTag]
    std::string      hiddenWhy;     // 非空＝golden 按不到這顆按鈕（靜態確定）
    bool             goldenRowKnown;   // golden 會讀的那一列在清單內（goldenRow 有效）
    TeachRegRow      goldenRow;     // 怪按鈕／未登錄按鈕：golden 處理函式實際讀到的那一列
    TeachTarget() : kind(kTkNone), ownList(0), ownIndex(-1), readList(0), quirk(false), outOfRange(false), selectMot(-1),
                    unregistered(false), goldenRowKnown(false) {}
};
// 怪按鈕（W5-b 裁決 2，20260925）：kTeachQuirkRefuse＝拒絕；kTeachQuirkOwnRow＝選項 C（動這顆按鈕自己那一列）＝現行（使用者 20260925 裁決 2C）。
enum { kTeachQuirkRefuse = 0, kTeachQuirkOwnRow = 1 };
extern const int kTeachQuirkPolicy;
// 按 golden Tag 語意解析一顆教導鈕：回 "" = 可以（out 填好）；否則是拒絕理由（out 仍填到能填的部分，給訊息與測試看）。
std::string MotorAccessResolveTeachButton(const TeachRegistry& reg, const std::string& btn, int quirkPolicy, TeachTarget& out);

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
    virtual void GoldenJog(int motIndex, bool positive, int pct) = 0;   // 非 1203：MOT.SetSpeed(pct, true)＋MOT.JogP/JogN（AI(W906-MT-E2) 20260925：pct = 上次捲軸位置，原為 SetSpeed(edtSpeed)）
    //  AI(W906-W5-b) 20260925: pct<0 ＝不設速度（golden uteach 的 JogP(Speed) 不用 Speed，mymotor.cpp:1876）
    //  AI(W906-MERGE-56bbf785) 20260926: machine = SetSpeed(pct, true) for the Motor Test jog, laptop = pct<0 skips the SetSpeed for the
    //  teach jog -- both kept: pct>=0 -> SetSpeed(pct, true) (machine), pct<0 -> no SetSpeed (laptop).
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

    // AI(W906-MT-E1) 20260925: the drive behind a 1203 axis slot, for the home branch (user EastSun 20260925
    //   「歸原點要有分支 ... 看驅動器」): 1 = DS402 servo (the monitor's station identity says CiA profile 402, or
    //   SERVOPACK, or the axis reads as a Yaskawa Sigma-X), 0 = another drive (a profile that is not 402),
    //   -1 = unknown (station not in the scan, or no profile/name) -- the caller refuses, never guesses.
    virtual int  Pci1203DriveKind(int axis) = 0;

    // AI(W906-MT-E1) 20260925: golden btnReloadMotorDataClick runs InitialMotorParameter() first, and that sets
    //   MOT[i].HomeFlag = 0 for every motor (golden cinitial.cpp:3615 / :3773 / :3989). Reload Motor Data zeroes the
    //   coordinates, so the "homed" state must be dropped with them -- otherwise Go / Loop Move interlocks that test
    //   HomeFlag would pass on coordinates that no longer mean anything.
    virtual void GoldenClearAllHomeFlags() = 0;

    // ---- AI(W906-MT-E2) 20260925: the Motor Test functions that needed no ruling ----
    // golden MOT[i].SetSpeed(pct, bSetJog) (TMyMotor::SetSpeed, Motor/mymotor.cpp:320). For a 1203 axis the golden object
    //   never opened its axis here, so TMyEtherCatMotor::SetSpeed only sets iSpeed (myEthercatmotor.cpp:902, return at
    //   :912-913) -- the card is written through Pci1203Execute; this call only keeps golden's ReadSpeed()/GetSpeed() true.   //AI(W906-ENG1203) 20260929: ⚠ true only while NO engine motor route is installed (WB_ENGINE_MOTOR_1203, off by default). With it, SetSpeed's lazy claim opens the golden object's #else arm onto the card (EtherCAT/Pci1203MotorRoute.cpp, kEcSetSpeed) -- the same values a second time; Q7's dedup skips a write equal to the last success that the card reads back
    virtual void GoldenSetSpeed(int motIndex, int pct, bool jog) = 0;
    //  AI(W906-MERGE-56bbf785) 20260926: the laptop's W5-b added GoldenSetSpeed(motIndex, pct) = MOT[i].SetSpeed(pct) (uteach btnHome
    //  release :2195, edtSpeedChange :2453, UpdateMotorTeachMonitor :1286) -- that is this call with jog=false (TMyMotor::SetSpeed's
    //  bSetJog defaults to false, Motor/mymotor.h:157), so the two-argument overload is not kept; the teach page calls (mi, pct, false).
    // golden strngrdMotorSelectCell (uMotorTest.cpp:1198-1218): row 1 SetInitSpeed, 2..5 PJogHigh/PJogLow/PHomeHigh/PHomeLow,
    //   6/7 PSoftLimitP/N, 8/9 SetAccDataBase/SetDecDataBase, 10 SetRange -- memory only (golden same). `value` is already
    //   golden's `ret` (atoi for rows other than 8/9).
    virtual void GoldenSetCell(int motIndex, int row, double value) = 0;
    virtual void GoldenSetLastHomePos(int motIndex, int v) = 0;   // MOT[i].Motor->LastHomePos
    virtual bool Pci1203MotionIO(int axis, unsigned long& io) = 0;  // the monitor sample's Acm_AxGetMotionIO (golden Led[] source)
    //  AI(W906-MERGE-56bbf785) 20260926: the laptop's W5-b declared the same call (W5B-3: valid && opened only; ORG = bit 4, golden
    //  TMyEtherCatMotor::ScanMotorStatus myEthercatmotor.cpp:860 Led[iHomeLed]) -- one declaration, both sides use it.
    // golden UpdateMotorLed's MOT[i].Led[iAlarmLed] for a non-1203 motor (after ScanMotorStatus / Gali_ScanMotStatus).
    virtual bool GoldenAlarmLed(int motIndex, bool& known) = 0;
    // golden btnReloadMotorDataClick's InitialMotorParameter(): re-read Mot_Table.csv (read only) and put the table
    //   values back on the EXISTING MOT[i].Motor objects. Never re-creates an object, never opens an axis, never changes
    //   a row's identity (such a row is skipped and counted). See WebMotorAccessLive.cpp.
    virtual MotorReloadResult GoldenReloadMotorParams() = 0;
    // A monotonic clock in ms (QueryPerformanceCounter) for golden's TQPF_Timer (Loop timing). < 0 = no clock: the loop
    //   then counts the 500 ms beats (MotorAccessTick's elapsedMs) -- what the tests without a clock see.
    virtual double MonotonicMs() = 0;

    // ---- AI(W906-MT-E3c) 20260925: Motor Power, Test Range/Rate (full InitMotor), All-mode loop, Light Scale ----
    // Motor Power (golden btnMotorPowerClick uMotorTest.cpp:1621-1645, FormShow :1038-1051). Every SW[] write below runs
    //   with Pci1203RouteSetSource("web") so the route always executes and records it (the engine's unchanged-value skip,
    //   Pci1203IoRoute.cpp S7, does not apply to an operator's button).
    //   syncFromCard: lead default 20260925 -- IO-page clicks write MyLaneIO directly and never update SW[].OutValue in
    //   this tree, so before golden reads OutValue it is set from the monitor's DO read-back of that bit (when there is one).
    virtual MotorPowerState GoldenMotorPower(bool syncFromCard) = 0;
    virtual void GoldenMotorPowerOnBegin() = 0;                   // W906_DoMotorPowerOnBegin() (csystem.h; golden DoMotorPowerOn, first pass)
    virtual bool GoldenMotorPowerOnStep() = 0;                    // W906_DoMotorPowerOnStep(): true = golden's `break` (1 s over)
    virtual void GoldenServerOn() = 0;                            // SW[SwServerON].On() (golden :1635 / :1042)
    virtual void GoldenMotorServoOff(const std::string& sFunc) = 0;   // fHome->GaliMotorServoOff(sFunc) (golden :1641 / :1048)
    virtual std::string GoldenRouteLastWrite() = 0;               // Pci1203RouteLastWrite() as one line, for the ack
    // Test Range / Test Rate on a 1203 axis (R4): golden SetRange / SetRate MEMORY half (the card half is Pci1203Execute).
    virtual void GoldenSetRangeMemory(int motIndex, unsigned v) = 0;  // Motor->SetRange(v) (TMyEtherCatMotor: Range=min(v,1000), no card)
    virtual void GoldenSetAccMemory(int motIndex, double dAcc) = 0;   // TMyEtherCatMotor::SetAcc = `dAcc=a` (golden SetRate :583's dAcc)
    virtual bool GoldenInitMotorEmgOff() = 0;                     // golden InitMotor :180-181: any of the four EMG sensors IsOff()
    // golden InitMotor's configuration writes for this motor, IN GOLDEN ORDER (:219-361): EastSun's pure
    //   Pci1203GoldenInitCfgPlan(motorClass, sensorType, in1Logic) + Pci1203InitCfgName (EtherCAT/Pci1203Control.h, unit-tested
    //   in test_pci1203_pure). Behind the backend only because Pci1203Control.cpp is not linked into this file's ctest.
    struct InitCfg { int which; double value; std::string name; };
    virtual std::vector<InitCfg> GoldenInitCfgPlan(const MotorGolden& g) = 0;
    virtual void SleepMs(int ms) = 0;                             // golden MotOutputOn's MySleep(100) (:1401) -- only after an alarm
    //AI(W906-BRAKE-AXIS) 20260929: EastSun「只要SERVO ON 就必須要先激磁 0.5秒後 觸發io的煞車」-- the SERVO OFF half: hold this axis'
    //   own brake output (if it has one) BEFORE the servo goes off. true = a brake output was switched to HOLD (note says which);
    //   false = the axis has no brake output / it is not enabled in IO_Table. Default (ctest fakes): no brake.
    virtual bool BrakeHoldBeforeServoOff(const std::string& alias, std::string& note) { (void)alias; note.clear(); return false; }
    // golden btnLoopMoveClick :1314-1315 `for(i<TOTAL_MOTOR) MOT[i].InitMOTParameter();` (fCMD / Gali flags = false, memory only)
    virtual void GoldenInitMOTParameterAll() = 0;
    // golden FormClose :1361 `PauseUT150Polling=false;` (the MotorTest.ini write is the page's, R5)
    virtual void GoldenFormClose() = 0;
    // Light Scale (R7). The MOT[] index -> Mot_Table Alias ("" = no row), for the arm golden's rgAxisClick picks (MOT 0/1/19/20).
    virtual std::string AliasOfMotor(int motIndex) = 0;
    // golden `MOT[MLightScale].ReadEncoderPos()` (uMotorTest.cpp:1887): a PCI1203 MLightScale row -> the monitor's actual
    //   position in user units (src "pci1203"); another row -> the golden object's ReadEncoderPos() (src "MOT"); no object ->
    //   0 (src "none"). `fromMonitor` = the value comes from a monitor sample (the caller then waits for a poll after the delay).
    virtual int  GoldenLightScaleEncoder(std::string& src, bool& fromMonitor) = 0;
    virtual int  GoldenLightScaleDataCount(int k) = 0;            // iLogLightScaleCount_* (k 1..8 = InArmX1..OutArmY2, cmydef.cpp:4902-4909)
    virtual std::vector<std::string> GoldenLightScaleData(int k) = 0;   // fMotorTest->mmo<k>->Lines (empty while fMotorTest is NULL)
    virtual void GoldenLightScaleDataClear(int k) = 0;            // mmo<k>->Clear()
    virtual void GoldenLightScaleCountsReset() = 0;               // the eight iLogLightScaleCount_* = 0 (golden :2084-2091)
    virtual std::string LightScaleRoot() = 0;                     // golden "D:\\LightScale" (uMotorTest.cpp:1752 / :2055); tests: a temp dir
    virtual std::string LocalStampYmdhm() = 0;                    // golden FormatDateTime("yyyymmddhhmm", Now()) (:2065)

    // ---- W5-b（uteach 教導頁的運動鈕）----
    //AI(W906-W5-b) 20260925: W5B-2 —— 這台機台上 golden 登錄表的實際樣子（WebTeachButtons.gen.inc 依機種條件求值，再與
    //  fTeach->TechPara／TechTwoPara 逐列核對 Parameter 指標＋馬達＋Key）。false＋why：表與實際登錄對不上（產生表過期）→ 教導鈕全部拒絕。
    virtual bool GoldenTeachRegistry(TeachRegistry& out, std::string& why) = 0;
    // fTeach->ActiveMotorIndex = mi; CheckCanMove() && IsCanQuickJogMove()（forms/fTeach.cpp；兩支自己會 ShowMyMessage／ShowErrorMessage）
    //   ⚠ 真機組態下 IsCanQuickJogMove → CheckShuttleCanMove 可能 ShowErrorMessage("WAR16435", K_RETRY) —— **阻塞到操作員回答**（W5-b 裁決 1）
    virtual bool GoldenTeachCanMove(int motIndex) = 0;
    virtual bool GoldenMotorPowerOff() = 0;                       // Sen[SnMotorPower].IsOff()（uteach btnHomeClick :2137）
    virtual int  GoldenReadEncoderPos(int motIndex) = 0;          // 非 1203 的 GetTechPos：MOT[i].ReadEncoderPos()
    //AI(W906-W5-b) 20260925: W5B-14 —— golden GetTechPos（uteach.cpp:3487-3490）單軸時 `MOTION_CARD_TYPE==MotionCard_Contec &&
    //  Motor->MotorType==0` 改讀 ReadPos()（兩軸那支沒有這個分支）
    virtual bool GoldenTechPosUsesReadPos(int motIndex) = 0;
    //AI(W906-W5-b) 20260925: W5B-9 —— golden MotorMovePosition（mymotor.cpp:590）每次下移動前記 iLastRotatorDirP；
    //  GetRotatorBacklash（:2012-2045）拿它與目前位置判斷補不補背隙。1203 軸的位置照 W4 讀監看器，所以這三支拆開給分派層算。
    virtual bool GoldenRotatorLastDirP(int motIndex) = 0;         // MOT[i].iLastRotatorDirP
    virtual void GoldenSetRotatorLastDirP(int motIndex, bool p) = 0;
    virtual int  GoldenProdRotatorBacklash(bool inRotator) = 0;   // Prod.iIn_iRotateA_Backlash／iOut_iRotateA_Backlash（畫面值是 0 時 golden 用它）
    virtual bool GoldenIndexArm3Axis() = 0;                       // USE_INDEX_ARM_AXES==IndexArm_3_Axis（GoButton020Click :3578）
    virtual int  GoldenTeachRemap(int motIndex) = 0;              // SetButton140／GoButton140 :3372／:3409：ep1Picker 且 7／26 → −4
    //  AI(W906-MERGE-56bbf785) 20260926: same mapping as the machine's AliasOfMotor above (bool + out parameter instead of "" for no
    //  row); both kept so neither side's call sites change.
    virtual bool AliasOfMotIndex(int motIndex, std::string& alias) = 0;   // Mot_Table 的 No "M%02d" → Alias
    virtual bool Pci1203ActPos(int axis, double& card) = 0;       // 監看器的 actPos（編碼器；golden GetTechPos 讀 ReadEncoderPos）
    //  (Pci1203MotionIO and GoldenSetSpeed: declared once above -- see the AI(W906-MERGE-56bbf785) notes there.)
    //AI(W906-W5-b) 20260925: 覆核 W5B-R3 —— 全域 fAllMotorHome=false。golden 進教導頁（FormShow uteach.cpp:1606）、離開教導頁
    //  （main.cpp:27845）、MotorTest 關閉（uteach.cpp:2439）都清它 ⇒ 下一次 START 先做全部歸零（WebStart.cpp Home by Start）。
    virtual void GoldenClearAllMotorHome() = 0;

    //AI(W906-MT-SAVEMOT) 20260930: Motor Test「回寫 Mot_Table」(action saveMotTable, EastSun 20260930). Defaults = no file, so the
    //  older fakes need nothing; the live backend (WebMotorAccessLive.cpp) answers from MotTablePath / INDEX_MOTION_CARD / HSys.MotTable.
    virtual std::string MotTableFilePath() { return std::string(); }            // MotTablePath (database.cpp LoadMotData: W906_MOTTABLE_PATH, else the golden literal)
    virtual bool        MotTableIndexForced() { return false; }                  // INDEX_MOTION_CARD==0: TMOTDATA forces the four Index rows' Acc/Dec (database.cpp)
    //  after a successful write: the in-memory HSys.MotTable row of that Alias := the file (rowText = its new _CommaText; cells =
    //  (column token, new cell text) of the changed cells only), so a later read in the same run agrees with the file.
    virtual void        MotTableRowSaved(const std::string& alias, const std::string& rowText,
                                         const std::vector<std::pair<std::string, std::string> >& cells) { (void)alias; (void)rowText; (void)cells; }
    virtual bool GoldenSingleHomeBegin(int motIndex) { (void)motIndex; return false; }  virtual bool GoldenSingleHomeStep(int motIndex) { (void)motIndex; return false; }  };   //AI(W906-MT-SMHOME) 20261001: RULINGS_20261001 #8 -- a non-1203 axis homes the golden way. Begin = golden btnHomeClick :1154-1158 `fHome->iHomeStep=1; InitProcessSingleMotorTask(i); MOT[i].HomeFlag=0;` (false = this backend cannot); Step = one golden MainProc pass (csystem.cpp:16954-16960) `ProcessSingleMotorHome(i) && MOT[i].HomeFlag==1` (true = golden drops bSingleHome). Defaults keep the older fakes unchanged.

struct MotorAccessOutcome {
    bool        ok;
    std::string ackJson;   // ok：JSON 物件（併進 WS ack）；!ok：拒絕理由（純文字）
    MotorAccessOutcome() : ok(false) {}
};

// 分派。wireId = WS 命令 id（寫進 1203 的稽核線，讓畫面分得出是哪一次按鈕）。
MotorAccessOutcome MotorAccessDispatch(const MotorAccessReq& r, long long wireId,
                                       IMotorAccessBackend& be);

// 38 個 catalog 命令的 action 名稱各自接到哪裡（給 /api 與文件用；也讓測試能逐條比對 catalog）。
//   AI(W906-MT-E3c) 20260925：現在 48 個命令、37 個 action；"blocked" 已經沒有任何一列（motorPowerToggle 接上了）。
//   AI(W906-MERGE-56bbf785) 20260926：W5-b 接上教導頁之後 "queued" 也沒有任何一列了（teachSet／teachGo live、setTeachFromCurrent／
//   setTeachFromOffset ui）：live 32、ui 5。
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
//   AI(W906-W5-b) 20260925: 覆核 W5B-R5／R6 —— 手動教導（伺服已關）**不再**在這裡結束：golden 的 fTeachShow 不會自己關，伺服只在
//   操作員按確定／取消後才開（uteach.cpp:3393-3396）；連線斷掉時自動開伺服，操作員的手可能還在軸上（Q3 未量）。
//   重新整理後頁面用 teachSet {query:true} 把對話框叫回來。
struct MotorAccessJobState {
    int           homesActive;
    bool          loopActive;
    std::string   loopMotor;
    int           loopTask;        // golden DoLoopMove 的 Task：1 去 pos1、10 等、50 去 pos2、60 等
    unsigned long loopCount;       // golden dwLoopCount
    std::string   lastNote;        // 最近一次工作結束的原因（給畫面與日誌）
    int           jogsActive;      // W4B-4：正在 jog 的軸數
    // AI(W906-MT-E1) 20260925: which motors (Alias) have a HOME job running -- the page's btnHome pops up when its
    //   motor's job is gone (golden Timer1Timer :962-966 `else if(bSingleHome==false) btnHome->Down=false;`).
    std::vector<std::string> homeMotors;
    //AI(W906-MT-E2) 20260925: golden TfMotorTest page state.
    std::string   selectedMotor;   // golden ActiveIndex as an Alias (selectMotor = lM00Click / lpA00Click); empty = none
    //  golden lblJogPTime / lblJogNTime / lblAvgTime (uMotorTest.cpp:425/452, :524/555, :529/561). They are ONE set of
    //  labels on the form (not per motor), they keep their value after the loop stops, and a new loop does not clear
    //  lblJogPTime/lblJogNTime (only the homed start branch zeroes lblAvgTime, :1334). has* = false: never measured yet.
    bool          hasJogPTime, hasJogNTime, hasAvgTime;
    int           jogPTime, jogNTime;   // ms (golden LatchCycleTime: (int)(float) of the QPC ms since the leg's latch)
    double        avgTime;              // golden ChangeToFloatNonPcnt((double)Average, (double)dwLoopCount) -- a float
    //AI(W906-MT-E3c) 20260925
    bool          loopAll;              // golden select->ItemIndex==1 (All mode); loopMotor is then the selected motor (golden ActiveIndex)
    std::vector<std::string> loopMotors;   // All mode: the cbUsing-checked motors the loop drives (single mode: loopMotor alone)
    bool          pageShown;            // golden TfMotorTest::fShow as C++ last heard it (formShow / selectMotor true, formClose false)
    bool          powerPending;         // Motor Power On: golden DoMotorPowerOn's 1 s wait is still running (cross-tick job)
    bool          singleHome;           // golden bSingleHome (a HOME job, or Light Scale's single home not yet released)
    bool          teachActive;     // AI(W906-W5-b) 20260925：手動教導（伺服已關）進行中
    std::string   teachBtn;        //   是哪一顆 Set 鈕
    int           encBaseAxes;     //   W5B-6：以編碼器位置為基準的 1203 軸數
    //AI(W906-MERGE-56bbf785) 20260926: machine MT-E1..E3c fields + laptop W5-b fields (teachActive/teachBtn/encBaseAxes), both kept;
    //  the machine's constructor now also zeroes the laptop's two scalars.
    MotorAccessJobState() : homesActive(0), loopActive(false), loopTask(1), loopCount(0), jogsActive(0),
                            hasJogPTime(false), hasJogNTime(false), hasAvgTime(false), jogPTime(0), jogNTime(0), avgTime(0.0),
                            loopAll(false), pageShown(false), powerPending(false), singleHome(false),
                            teachActive(false), encBaseAxes(0) {}
};
MotorAccessJobState MotorAccessJobs();
void MotorAccessResetJobs();       // 測試用

//AI(W906-MT-E3c) 20260925: golden TfMotorTest's Light Scale page (BitBtn2 / LightScale / Timer2Timer, uMotorTest.cpp:1749-2133)
//  as C++ holds it -- for /api/struct/motor/runtime "lightScale" and the tests.
struct MotorLightScaleState {
    bool          active;           // golden Timer2->Enabled
    int           task;             // golden LightScale's `static int Task` (-1 = never reset)
    bool          editsEnabled;     // golden edPitech / edDelayTime ->Enabled
    bool          homePending;      // golden bSingleHome as LightScale's case 0 set it (case 1 waits for it to drop)
    int           useAxis;          // golden iUseAxis (rgAxisClick: 1->0, 2->1, 3->19, 4->20, else -1; VCL zero-init = 0)
    int           axisItem;         // rgAxis->ItemIndex as the page last sent it (-1 = never clicked)
    int           moveType;         // rgMoveType->ItemIndex as the page last sent it
    int           needMovePos;      // golden iNeedMovePos (the current target)
    unsigned long memoCount;        // golden Memo1->Lines->Count
    std::vector<std::string> memoTail;   // the last `tail` lines of Memo1 (golden format, byte for byte)
    std::string   lastSaved, lastNote, encoderSrc;
    MotorLightScaleState() : active(false), task(-1), editsEnabled(true), homePending(false), useAxis(0), axisItem(-1),
                             moveType(-1), needMovePos(0), memoCount(0) {}
};
MotorLightScaleState MotorAccessLightScale(std::size_t tail);

// ---- AI(W906-MT-E3c) 20260925: the engine's VerifyMotorAction hooks (csystem.h W906_*Hook; registered by wb_serve) ----
// W906_MotorMovingHook: -1 = the row is not PCI1203 (golden MotionDone runs), 0 = not moving, 1 = moving. A 1203 row is
//   "moving" when its monitor sample is not STA_AX_READY (golden TMyEtherCatMotor::MotionDone `state==STA_AX_READY`), when
//   a command of this file went out and no fresh sample has come back yet, or while it jogs. EastSun R2: M14 included.
int  MotorAccessMovingOf(IMotorAccessBackend& be, int motIndex);   //AI(W906-MT-AXISLOCK) 20260929: ERROR_STOP / DISABLE are stopped states -> 0 (EastSun: one alarmed axis must not lock the page); a HOME job on the axis -> 1
//AI(W906-MT-AXISLOCK) 20260929: Motor Test's per-axis lock (EastSun 20260929「每個軸都是獨立可控的」): -1 = not a 1203 row (the page keeps golden's
//   page-wide lock for it), 0 = this axis' MoveN/MoveP/LoopMove are free, 1 = locked by this axis' OWN motion (why = JOG / HOME / command sent / state).
int  MotorAccessAxisLock(IMotorAccessBackend& be, int motIndex, std::string& why);
// W906_MotorHomingHook: a HOME job started by btnHome runs (golden btnHome->Down; Light Scale's single home is not btnHome).
bool MotorAccessHomingActive();
// W906_MotorAllBtnUpHook: golden fMotorTest->AllBtnUp() + bSingleHome=false (the Light Scale timer is NOT touched, R7).
void MotorAccessAllBtnUp(IMotorAccessBackend& be, const std::string& why);
// W906_Stop1203AllHook: Stop1203 (StopDec + ExtDrive 0) on every axis the monitor opened.
void MotorAccessStop1203All(IMotorAccessBackend& be, const std::string& why);

// ---- AI(W906-MT-E3c) 20260925: golden pure arithmetic / text, tested with hand values ----
// golden TMyEtherCatMotor::SetRate (Motor/myEthercatmotor.cpp:576-616), unsigned 32-bit like golden:
//   dAcc = (unsigned)(PJogHighSpeed-InitSpeed) * a / 8e6; dAcc==0 -> skip (golden returns before the card writes);
//   Rate = (double)(unsigned)(iSpeed*Range - InitSpeed*Range) / dAcc, clamped to [iAccPersent*2001, iAccPersent*8192000]
//   with iAccPersent = (iSpeed*Range)/65535 (0 -> 1) -- the value golden writes to PAR_AxAcc and PAR_AxDec.
struct MotorGoldenRate {
    double   dAcc;
    bool     skip;
    double   rate;
    unsigned accPersent, accMin, accMax;
};
MotorGoldenRate MotorRateFromGolden(unsigned a, unsigned jogHigh, unsigned initSpeed, unsigned iSpeed, unsigned range);
// golden LightScale case 170 (uMotorTest.cpp:1892): "ArmPosition, %d, LightScalePos, %d ,[ %d ]"
std::string MotorLightScaleLine(int needMovePos, int lightScalePos);
// golden TStrings::SaveToFile body (BCB6 TStrings::GetTextStr): every line followed by CR LF, the last one too.
std::string MotorStringsFileText(const std::vector<std::string>& lines);

//AI(W906-W5-b) 20260925: 覆核 W5B-R5 —— START（start.run 與告警框按 START）在呼叫 StartFromWeb 之前問這一支：手動教導中回 true＋理由，
//  呼叫端拒絕啟動。golden 的 fTeachShow 是 ShowModal：它開著時畫面上的 START 按不到；網頁上擋不住別的分頁，改成「手動教導中不准啟動」。
//  （上一版是「啟動前先結束手動教導並自動開伺服」—— 操作員的手可能還在軸上，W5B-R5 指出後改掉。）
bool MotorAccessStartBlocked(std::string& why);
//AI(W906-W5-b) 20260925: W5B-3 —— forms/fTeach.cpp IsCanQuickJogMove 本體讀「Z 在原點」的來源（W906_TeachHomeLedHook 由
//  WebMotorAccessLive.cpp 安裝成呼叫這兩支）。
//    MotorAccessTeachHomeLed：-2 = 不是 1203 軸（走 golden MOT[mi].Led[iHomeLed]）；1 = 在原點；0 = 不在；-1 = 不明（沒樣本、
//      監看器沒開這一軸、命令後還沒新樣本）→ 呼叫端當「不在原點」（fail-closed）。Mot_Table Enable=0 → 1（golden Enable=false
//      的 ScanMotorStatus 把 Led[iHomeLed] 設 true，golden myEthercatmotor.cpp:888）。
//    MotorAccessTeachLive1203：這一軸是 1203 而且 1203 控制層可用（真的會動）→ SOFT_SIMULTE 組態也要跑 IsCanQuickJogMove 本體。
//  AI(W906-W5-b) 20260925: 覆核 W5B-R2 —— EastSun 監看器開軸**沒有設** CFG_AxOrgLogic（golden InitMotor :252 依 SensorType 設），
//    所以 ORG 位元的極性是卡片當下的設定，不一定是 golden 的「在原點」。kPci1203CardOrgLogic＝在機台上量到的卡片 CFG_AxOrgLogic 值
//    （-1 未量、0 ORG_ACT_LOW、1 ORG_ACT_HIGH）；未量 ⇒ 1203 軸的「在原點」一律 -1（不明 ⇒ 不在原點，fail-closed）。
//    量到之後：卡片值 == golden 的 (SensorType ? 0 : 1) ⇒ 位元照讀；不同 ⇒ 反相。
extern const int kPci1203CardOrgLogic;
// 純函式（測試直接驗）：orgBit＝motionIO bit 4；sensorType＝Motor->bSensorType；cardOrgLogic＝卡片的 CFG_AxOrgLogic（不是 0／1 ⇒ -1 不明）
int  MotorAccessOrgHomeLed(bool orgBit, bool sensorType, int cardOrgLogic);
int  MotorAccessTeachHomeLed(IMotorAccessBackend& be, int motIndex);
bool MotorAccessTeachLive1203(IMotorAccessBackend& be, int motIndex);
// 最近一次 MotorAccessTeachHomeLed 回 -1 的原因（TeachPrelude 用來把「不明」講清楚；空＝沒有）
std::string MotorAccessTeachHomeUnknownWhy();
//AI(W906-W5-b) 20260925: W5B-6 —— 這個 1203 軸槽在手動教導後以編碼器為基準（/api/struct/motor/runtime 的 cmdPos 也改給編碼器，
//  golden ResetPos 之後 ReadPos＝編碼器；W906_MotorAccessTick 的主執行緒抄表用）
bool MotorAccessEncoderBase(int axis1203);
//AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— golden MOT[mi] 目前的 SetSpeed 百分比（1203 軸由這裡記；-1＝這個行程還沒設過）。測試與日誌用。
int  MotorAccessGoldenSpeedPct(int motIndex);
// NB2 R23 W4C-4：golden Timer1Timer（uMotorTest.cpp:921-926）`if(fNote->fShow || MyMessageBox->fShow) { btnLoopMove->Down=false;
//   btnHome->Down=false; bSingleHome=false; }` —— 告警一出現就放棄 HOME／LoopMove。wb_serve 的 ShowErrorMessage 入口
//   （W906_AlarmStopLikeGolden，golden note.cpp:795 StopAllMotor）呼叫這支。golden 的 StopAllMotor 是「全部馬達」，
//   而 EastSun 監看器開的 1203 軸走不到 golden 的停止（m_Axishand 不是它開的）⇒ 這裡照 DoStop 對每一個開成功的 1203 軸補送停止。
void MotorAccessOnAlarm(IMotorAccessBackend& be, const std::string& what);
//AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- true while MotorAccessOnAlarm's Stop1203 sweep of every opened axis
//  runs (the port's 1203 half of golden StopAllMotor). The Index Z1 Gali route keeps a HALTED Z1 job halted on that sweep's
//  stop (golden StopAllMotor's per-axis PCIL132_StopMotor skips the index axes, golden Motor/mymotor.cpp:1864-1867) and
//  treats every other stop outside it as an operator's golden "ST" (EtherCAT/Pci1203GaliRoute.cpp W906_EngineRouteForeignStop).
//  Tick thread only (the counter is plain). Body: WebMotorAccess.cpp EOF.
bool MotorAccessInAlarmSweep();
struct MotorAccessAlarmSweep { MotorAccessAlarmSweep(); ~MotorAccessAlarmSweep(); };   // the scope MotorAccessOnAlarm opens around its sweep
//AI(W906-MT-E2) 20260925: called right after every 1203 monitor Poll() (wb_serve, W906_MotorAccessPollTick), on the same
//  thread: a 1203 LoopMove sees its arrival on the fresh sample and the leg time is taken there (golden Timer1Timer checks
//  every 5 ms; the beat alone would round every leg up to 500 ms). Everything else keeps the 500 ms beat.
//AI(W906-MT-FIX1) 20260926: operatorConnected = the LIVE WebBridgeServer::ControlOwner()!=0 (wb_serve passes it). The step
//  after Poll is gated exactly like the beat (operator gone / last beat's safe lock -> CancelAllJobs before stepping): it used
//  to issue a new LoopMove leg between beats with nobody connected (review medium).
void MotorAccessPollTick(IMotorAccessBackend& be, bool operatorConnected = true);

// wb_serve 用的真實後端（WebMotorAccessLive.cpp；只連進 wb_serve）。
IMotorAccessBackend& MotorAccessLiveBackend();

//AI(W906-WSLINK-B) 20260929: the page-table close edge of fMotorTest / fTeach (St01 13:32 (b)): the C++ half of that window's golden
//  FormClose -- fMotorTest: DoFormClose + its own jogs stopped; fTeach: btnStop->Click() = DoStop("uteach") (while SystemStart
//  only Teach's jogs). Rules and golden cites: WebMotorAccess.cpp EOF.
void MotorAccessPageClosed(IMotorAccessBackend& be, const char* form);

//AI(W906-MT-SAVEMOT) 20260930: EastSun 20260930「圖片上也幫我新增回寫至 MOT_table的按鈕」-- Motor Test's "<Alias> Settings" grid
//  (golden strngrdMotor rows 1..10 = UpdateMotorParameter, uMotorTest.cpp:659-668) written back into THAT motor's row of
//  Mot_Table.csv (action saveMotTable). NOT golden: golden's Settings grid is memory only (strngrdMotorSelectCell); golden's only
//  Mot_Table writer is the Motor Database tab's sbUpdateClick (uMotorTest.cpp:2240-2268), which rewrites the WHOLE file from its
//  text grid (TStringList CommaText + SaveToFile). This one changes only the cells whose value differs, in one row; every other
//  byte of the file stays. Column names = TMOTNO::SetMOTTableNo's tokens (database.cpp); rules at WebMotorAccess.cpp EOF.
enum MotTableFieldKind { kMtfInt = 0, kMtfUInt, kMtfReal };   // how TMOTDATA reads the cell: atoi into an int / an unsigned member, atof
struct MotTableSaveField {
    std::string       column;   // SetMOTTableNo token: InitSpeed, JogHighSpeed, JogLowSpeed, HomeHighSpeed, HomeLowSpeed, SoftLimitP, SoftLimitN, Acc, Dec, Range
    MotTableFieldKind kind;
    double            value;    // what MOT[mi] holds = what the grid shows
};
struct MotTableCellChange {
    std::string column;
    std::string oldText, newText;   // the cell's text as TMOTDATA reads it (quotes stripped); newText = oldText when unchanged
    bool        changed;
    std::string note;               // an unchanged cell whose text is not the grid's number (the boot's clamp / MN200 /100), or ""
    MotTableCellChange() : changed(false) {}
};
struct MotTableEditResult {
    bool        ok;
    std::string why;                // !ok: why nothing may be written
    int         lineNo;             // 1-based line of the row in the file
    std::string motorName;          // the row's Motorname cell ("M03")
    std::string cardModel;          // the row's CardModel as the boot reads it (the Index override -> "SMC")
    std::string newRowText;         // the row after the edit, no line ending (= TMOTDATA::_CommaText)
    std::vector<MotTableCellChange> cells;   // one per field, in the fields' order
    int         changedCount;
    MotTableEditResult() : ok(false), lineNo(0), changedCount(0) {}
};
// the ten grid rows in golden order (UpdateMotorParameter :659-668): InitSpeed .. Range, from the golden object
std::vector<MotTableSaveField> MotTableFieldsOf(const MotorGolden& g);
// pure: `text` = the file's bytes; `outText` = the same bytes with only the changed cells of the row replaced.
//   expectCard: the CardModel this run loaded for the motor ("" = not checked) -- a different one = the file changed since boot.
MotTableEditResult MotTableEditRow(const std::string& text, const std::string& alias, int motIndex,
                                   const std::vector<MotTableSaveField>& fields, bool indexMotionCard0,
                                   const std::string& expectCard, std::string& outText);
struct MotTableSaveResult {
    bool        ok;
    std::string why;
    std::string path, backup;       // backup = "" when nothing was written
    bool        wrote;
    MotTableEditResult edit;
    MotTableSaveResult() : ok(false), wrote(false) {}
};
// read `path`, MotTableEditRow, and when a cell changed: backup (path + ".bak_" + stamp; stamp "" = local yyyymmdd_hhmmss) ->
//   temp file -> MoveFileEx replace -> read back. Refuses (nothing written) on any failure before the replace.
MotTableSaveResult MotTableSaveRow(const std::string& path, const std::string& stamp, const std::string& alias, int motIndex,
                                   const std::vector<MotTableSaveField>& fields, bool indexMotionCard0, const std::string& expectCard);

}  // namespace ht9045

#endif  // HT9045_WEBMOTORACCESS_H
