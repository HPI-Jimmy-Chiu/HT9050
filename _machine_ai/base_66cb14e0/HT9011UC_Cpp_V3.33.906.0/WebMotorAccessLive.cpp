// =============================================================================
//  WebMotorAccessLive.cpp  --  `motor.access` 的真實後端（只連進 wb_serve）
//
//  AI(W906-W4-MOTOR) 20260925.  介面與分層見 WebMotorAccess.h。
//
//  兩套編號怎麼對起來（都是量過的，不是猜的）：
//    * MOT[] 下標 = Mot_Table 的 No "M%02d"（cinitial.cpp InitialMotorParameter 的對法；
//      JsonBridge/ChanMotorPoints.cpp MotorIndexOf 同一條規則）。
//    * 1203 軸槽 = 監看器樣本的 (station, stationAxis) == Mot_Table 的 (BoardID, Port)。
//      production 以 Acm_AxOpenbyID(dev, BoardID, Port) 開軸（Motor/myEthercatmotor.cpp:384），
//      EastSun 的監看器把每一軸以「擁有它的站＋站內軸號」唯一命名（EtherCAT/Pci1203Monitor.cpp:1440 起），
//      machines/HT9050/Pci1203Axis.ini 的段名 [station<N>.axis<M>] 也是這個鍵。
//      對不到就拒絕並說出站號／軸號 —— 不退回「用 BoardID 當軸槽下標」這種看起來會動的猜法。
// =============================================================================
#include "WebMotorAccess.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "vclcompat/vcl_compat.h"
#include "database.h"                  // HSys.MotTable / TMOTDATA
#include "cmydef.h"                    // MTestY1
#include "Motor/mymotor.h"             // MOT[MAX_TRAY_MOTOR]
#include "Motor/HTMotor.h"             // iServoOn（Led 下標）
#include "EtherCAT/Pci1203Monitor.h"   // Pci1203Monitor() / Pci1203AxisSample
#include "forms/fMotorTest.h"          // fMotorTest->bSingleHome
#include "Motor/myEthercatmotor.h"     // TMyEtherCatMotor::W906_RuntimeAcc/Dec
#include "mysensor.h"                  // Sen[]（IsMotorCanRun）
#include "cprod.h"
#include "csystem.h"                   // CheckSafeDoorIsClosed
#include "canary_support.h"            // ShowMyMessage
#include "mycylin.h"                   // Cylinder[]（飛梭閘門，NB2 R21 W4B-2）
#include "MachineType.h"               // SOFT_SIMULTE
#include "JsonBridge/ChanMotor.h"      // W4-c：MotorRuntimeOverlay

extern void StopAllMotor(bool bIndexCanStop);   // Motor/myGALILmotor.cpp（golden Motor/myGALILmotor.cpp:4712）

namespace ht9045 {

namespace {

const unsigned long kSvOnBit = 0x00004000ul;   // AX_MOTION_IO_SVON（WebBridgeTags.cpp 的 21 個 motionIO 位元表同值）

int MotorIndexOf(const AnsiString& no)
{
    const std::string s(no.c_str());
    if (s.size() < 2 || (s[0] != 'M' && s[0] != 'm')) return -1;
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') return -1;
    return std::atoi(s.c_str() + 1);
}

class LiveBackend : public IMotorAccessBackend {
public:
    bool Resolve(const std::string& motorId, MotorAccessAxis& out) override
    {
        out = MotorAccessAxis();
        const TMOTDATA* row = 0;
        for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
            const TMOTDATA* r = HSys.MotTable[i];
            if (r && motorId == r->Alias.c_str()) { row = r; break; }
        }
        if (!row) return false;
        out.motIndex  = MotorIndexOf(row->No);
        out.cardModel = row->CardModel.c_str();
        out.boardId   = row->iBoardID;
        out.port      = row->iPort;
        out.motorLive = (out.motIndex >= 0 && out.motIndex < MAX_TRAY_MOTOR && MOT[out.motIndex].Motor != 0);
        out.tableEnable = row->iEnable != 0;                                    // NB2 R22（見 WebMotorAccess.h MotorAccessAxis::tableEnable）
        if (!out.Is1203()) return true;

        TPci1203Monitor* m = Pci1203Monitor();
        if (m == 0) { out.why = "1203 監看器沒有開（這個執行檔沒有 1203 SDK，或開卡失敗）"; return true; }
        int hit = -1, hits = 0;
        for (int ax = 0; ax < m->axisCount(); ++ax) {
            const Pci1203AxisSample& s = m->axis(ax);
            if (!s.opened) continue;
            if (s.station == out.boardId && s.stationAxis == out.port) { if (hit < 0) hit = ax; ++hits; }
        }
        char b[200];
        if (hits == 1) out.axis = hit;
        else if (hits == 0) {
            std::snprintf(b, sizeof(b), "1203 監看器找不到 站 %d 軸 %d（Mot_Table 的 BoardID／Port）的開成功軸",
                          out.boardId, out.port);
            out.why = b;
        } else {
            std::snprintf(b, sizeof(b), "站 %d 軸 %d 對到 %d 個軸槽 —— 不挑", out.boardId, out.port, hits);
            out.why = b;
        }
        return true;
    }

    bool Pci1203Ready(std::string& why) override
    {
        if (Pci1203Control() == 0) {
            why = "1203 control is not armed in this binary (built without HAVE_PCI1203, or Open() refused)";
            return false;
        }
        return true;
    }
    int Pci1203AxisCount() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m ? m->axisCount() : 0;
    }
    bool Pci1203AxisOpened(int axis) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m && axis >= 0 && axis < m->axisCount() && m->axis(axis).opened;
    }
    bool Pci1203ServoOn(int axis, bool& known) override
    {
        known = false;
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        known = true;
        return (s.motionIO & kSvOnBit) != 0;
    }
    Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) override
    {
        TPci1203Control* ctl = Pci1203Control();
        if (ctl == 0) {
            Pci1203CmdResult r;
            r.accepted = false; r.issued = false; r.ret = 0;
            r.why = "1203 control is not armed";
            return r;
        }
        const Pci1203CmdResult r = ctl->Execute(c);
        std::printf("motor.access -> 1203 kind=%d axis=%d: accepted=%d issued=%d ret=0x%08lX %s%s%s\n",
                    (int)c.kind, c.axis, (int)r.accepted, (int)r.issued, r.ret, r.wouldCall.c_str(),
                    r.why.empty() ? "" : "  -- ", r.why.c_str());
        return r;
    }
    void Pci1203NoteRefusal(long long wireId, const std::string& name, const std::string& why) override
    {
        TPci1203Control* ctl = Pci1203Control();
        if (ctl) ctl->NoteRefusal(wireId, name, why);
    }

    void GoldenStopAll(const std::string& source) override
    {
        StopAllMotor(true);                                                     // golden uMotorTest.cpp:1650／uteach btnStopClick（StopAllMotor() 預設參數即 true）
        if (source == "uMotorTest") {
            if (fMotorTest != 0) fMotorTest->bSingleHome = false;               // golden uMotorTest.cpp:1651（wb_serve 目前不建立 fMotorTest ⇒ 守衛）
        } else {
            // golden uteach btnStopClick 的 `Tech_Part=0;` 不做：TfTeach facade 沒有這個成員（forms/fTeachPara.h:207 記著
            //   「等消費者落地再帶進來」）—— 缺相依，不是選擇。
            MOT[MTestY1].Gali_Command("ST", "TfTeach::btnStopClick");           // golden uteach btnStopClick（Steven 20230721）
        }
        std::printf("motor.access stop (%s): StopAllMotor(true) (golden btnStopClick)\n", source.c_str());
    }
    void GoldenStopMotor(int motIndex) override
    {
        if (motIndex < 0 || motIndex >= MAX_TRAY_MOTOR) return;
        if (INDEX_MOTION_CARD == 0 && (motIndex == MTestY1 || motIndex == MTestZ1 ||
                                       motIndex == MTestZ2 || motIndex == MTestY2))   // golden uMotorTest.cpp:904-906／:1655-1656（Steven 20210623 : Index使用Galil）
            MOT[motIndex].Gali_Command("ST", "motor.access stop");
        else
            MOT[motIndex].PCIL132_StopMotor();                                  // golden uMotorTest.cpp:908／:1658
    }
    bool GoldenServoOn(int motIndex, bool& known) override
    {
        known = false;
        if (motIndex < 0 || motIndex >= MAX_TRAY_MOTOR || MOT[motIndex].Motor == 0) return false;
        MOT[motIndex].ScanMotorStatus();                                        // golden uMotorTest.cpp:1665
        known = true;
        return MOT[motIndex].Led[iServoOn];
    }
    void GoldenServoOnOff(int motIndex, bool on) override
    {
        if (motIndex >= 0 && motIndex < MAX_TRAY_MOTOR) MOT[motIndex].ServoOnOff(on);
    }

    // ---- W4-b ----
    bool Pci1203CmdPos(int axis, double& card) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        card = s.cmdPos;
        return true;
    }
    bool Pci1203AxisReady(int axis) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        return s.valid && s.opened && (s.state & 0xFFu) == 1u;                 // STA_AX_READY（EtherCAT/vendor/AdvMotDrv.h:793）
    }
    bool GoldenMotor(int mi, MotorGolden& g) override
    {
        g = MotorGolden();
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return false;
        HTMotor* M = MOT[mi].Motor;
        g.valid         = true;
        g.enable        = M->Enable;
        g.gearRatio     = M->GearRatio;
        g.direction     = M->Direction;
        g.homeDirection = M->HomeDirection;
        g.softP         = M->PSoftLimitP;
        g.softN         = M->PSoftLimitN;
        g.initSpeed     = M->InitSpeed;
        g.jogHigh       = M->PJogHighSpeed;
        g.jogLow        = M->PJogLowSpeed;
        TMyEtherCatMotor* E = dynamic_cast<TMyEtherCatMotor*>(M);
        if (E) { g.acc = E->W906_RuntimeAcc(); g.dec = E->W906_RuntimeDec(); }  // TMyEtherCatMotor::SetSpeed 寫進卡的那兩個
        else   { g.acc = M->ReadAcc();         g.dec = M->ReadDec(); }          // 非 EtherCAT 軸不會走 1203 路徑，只是填值
        g.homeFlag      = MOT[mi].HomeFlag;
        const int n     = MOT[mi].Mot_Name;                                     // golden TMyMotor::SetSpeed 用 Mot_Name 判斷
        g.zStack        = (n == MLoaderZ || n == MEmptyZ || n == MColorZ || n == MAuto1Z || n == MAuto2Z ||
                           n == MAuto3Z  || n == MAuto4Z || n == MAuto5Z || n == MAuto6Z);
        g.indexMotor    = (n == MTestY1 || n == MTestZ1 || n == MTestZ2 || n == MTestY2);
        g.galilIndex    = g.indexMotor && INDEX_MOTION_CARD == 0;
        g.armZ          = (n == MInArmZA  || n == MInArmZB  || n == MInArmZC  || n == MInArmZD  ||   // golden uhome.cpp ProcessSingleMotorHome
                           n == MInArmZE  || n == MInArmZF  || n == MInArmZG  || n == MInArmZH  ||   //   case 300 的清單（逐字同序）
                           n == MOutArmZA || n == MOutArmZB || n == MOutArmZC || n == MOutArmZD ||
                           n == MOutArmZE || n == MOutArmZF || n == MOutArmZG || n == MOutArmZH ||
                           n == MInArmZAe || n == MInArmZAf || n == MInArmZAg || n == MInArmZAh ||
                           n == MInArmZBe || n == MInArmZBf || n == MInArmZBg || n == MInArmZBh ||
                           n == MOutArmZAe || n == MOutArmZAf || n == MOutArmZAg || n == MOutArmZAh ||
                           n == MOutArmZBe || n == MOutArmZBf || n == MOutArmZBg || n == MOutArmZBh ||
                           n == MOutSortAa || n == MOutSortAb);
        g.scaleMotor    = (n == MInArmXScale || n == MInArmYScale || n == MOutArmXScale || n == MOutArmYScale);   // golden :1122-1123
#ifdef SOFT_SIMULTE
        g.selectable    = true;                                                 // golden lM00Click 的 Enable 擋在 #ifndef SOFT_SIMULTE 裡
#else
        g.selectable    = M->Enable;                                            // golden uMotorTest.cpp:740-743
#endif
        g.inShuttle     = (n == MInShuttle1) ? 1 : (n == MInShuttle2) ? 2 : 0;
        g.homeHigh      = M->PHomeHighSpeed;                                    // NB2 R23 W4C-3：golden SetHomeSpeed（myEthercatmotor.cpp:1703）
        g.homeLow       = M->PHomeLowSpeed;
        g.accDb         = M->GetAccDataBase();                                  // golden TMyMotor::Home SetADCRate(100) ⇒ dAcc = 資料庫值（mymotor.cpp:296-300）
        g.decDb         = M->GetDecDataBase();
        return true;
    }
    bool GoldenSafeDoorOpen(int mi) override
    {
        return mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0 && MOT[mi].Motor->CheckIsSafeDoorOpen();
    }
    bool GoldenSafeDoorClosed() override { return CheckSafeDoorIsClosed(); }   // csystem.h:228（golden 同名全域）
    bool GoldenMotorCanRun() override
    {
        // golden TfMotorTest::IsMotorCanRun(true)（uMotorTest.cpp:1280-1298；移植 forms/fMotorTest.cpp:699 是成員，
        //   而 wb_serve 不建立 fMotorTest）—— 同一段本體逐行抄過來。
        bool flag = true;
        if (Sen[SnFrontLeftEMG].IsOff())  flag = false;
        if (Sen[SnFrontRightEMG].IsOff()) flag = false;
        if (Sen[SnRearLeftEMG].IsOff())   flag = false;
        if (Sen[SnRearRightEMG].IsOff())  flag = false;
        if (Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()) flag = false;         // KenHsieh 20250212
        if (Sen[SnServo].Enable && Sen[SnServo].IsOff()) flag = false;          // kevin 20140121
        if (flag == false) ShowMyMessage("EMG Stop", "");
        return flag;
    }
    bool GoldenMoveLocked(int mi) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return true;
        const TTrayMotor& M = MOT[mi];
        return M.fCanMove == false || M.fCanMoveR == false || M.fCanMoveM == false || M.fCanMoveL == false ||
               M.W906_HasMotorLocks();                                       // TMyMotor::MotorMove（Klutter 20210817／Steven 20210825）
    }
    int GoldenReadPos(int mi) override
    {
        return (mi >= 0 && mi < MAX_TRAY_MOTOR) ? MOT[mi].ReadPos() : 0;
    }
    void GoldenJog(int mi, bool positive, int pct) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return;
        MOT[mi].SetSpeed(pct);                                                  // golden edtSpeedChange → MOT.SetSpeed(edtSpeed)
        if (positive) MOT[mi].JogP(pct); else MOT[mi].JogN(pct);                // golden :895／:845
    }
    int GoldenMotorMove(int mi, int target, int pct, bool setSpeed) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) return -1;
        if (setSpeed) { MOT[mi].SetSpeed(pct); MOT[mi].InitMOTParameter(); }   // golden MoveP/MoveN :1246-1247
        return MOT[mi].MotorMove(target);
    }
    void GoldenSetParam(int mi, MotorParamWhich which, int value) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        HTMotor* M = MOT[mi].Motor;
        switch (which) {
            case kParamJogHigh:  M->PJogHighSpeed  = (unsigned)value; break;    // golden :1407
            case kParamJogLow:   M->PJogLowSpeed   = (unsigned)value; break;    // :1415
            case kParamHomeHigh: M->PHomeHighSpeed = (unsigned)value; break;    // :1423
            case kParamHomeLow:  M->PHomeLowSpeed  = (unsigned)value; break;    // :1431
            case kParamSoftP:    M->PSoftLimitP    = value;           break;    // :1439
            case kParamSoftN:    M->PSoftLimitN    = value;           break;    // :1447
        }
    }

    // ---- NB2 R21 ----
    unsigned long Pci1203PollCount() override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        return m ? m->card().pollCount : 0ul;
    }
    bool GoldenShuttleFloodgateReady(int which) override
    {
        if (SHUTTLE_FLOODGATE != 1) return true;                                // golden 只在 SHUTTLE_FLOODGATE==1 時檢查
        if (which == 1) {                                                       // golden mymotor.cpp:659-661
            Cylinder[C_Shuttle1Floodgate].Off();
            Cylinder[C_OutShuttle1Floodgate].Off();
            return Cylinder[C_Shuttle1Floodgate].OffSensor() && Cylinder[C_OutShuttle1Floodgate].OffSensor();
        }
        Cylinder[C_Shuttle2Floodgate].Off();                                    // golden mymotor.cpp:681-683
        Cylinder[C_OutShuttle2Floodgate].Off();
        return Cylinder[C_Shuttle2Floodgate].OffSensor() && Cylinder[C_OutShuttle2Floodgate].OffSensor();
    }
    unsigned GoldenReadSpeed(int mi) override
    {
        return (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0) ? MOT[mi].Motor->ReadSpeed() : 0u;
    }
    // ---- NB2 R23 ----
    bool GoldenSafeLockActive() override { return IsSafeLockCheck(); }         // csystem.cpp:478（golden 同名全域；會 On 安全鎖開關，golden Timer1Timer 每拍同樣呼叫）

    // ---- W4-b2 ----
    bool Pci1203AxisState(int axis, unsigned& state) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        state = s.state;
        return true;
    }
    void GoldenSetHomeFlag(int mi, int v) override
    {
        if (mi >= 0 && mi < MAX_TRAY_MOTOR) MOT[mi].HomeFlag = v;
        std::printf("motor.access: MOT[%d].HomeFlag = %d\n", mi, v);
    }
    int  GoldenZSafePos() override { return ZSafePos; }
    bool GoldenSystemStart() override { return SystemStart; }
    void GoldenResetMNet() override
    {
        ResetMNet(0, "MNet斷電", "Power Off", false);                            // golden uMotorTest.cpp:1723
        std::printf("motor.access: ResetMNet(0) (golden btResetMNetClick)\n");
    }
    void GoldenSetRangeRate(int mi, bool range, int v) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        if (range) MOT[mi].Motor->SetRange((unsigned)v);                        // golden :1378
        else       MOT[mi].Motor->SetRate((unsigned)v);                         // golden :1368
        MOT[mi].Motor->InitMotor(MOT[mi].Motor->Address);                       // golden :1379／:1369
        MOT[mi].HomeFlag = 0;                                                   // golden :1380／:1370
    }
};

}  // namespace

IMotorAccessBackend& MotorAccessLiveBackend()
{
    static LiveBackend s_backend;
    return s_backend;
}

}  // namespace ht9045

// wb_serve 的 `motor.access`／`motor.stop` 臂呼叫這一支（全域函式、區塊內 extern 宣告，同 W906_AlarmStopLikeGolden 的做法 ——
//   wb_serve.cpp 不加 include 行，既有的行號引用不位移）。
//   stopOnly = 命令是 `motor.stop`：WebBridgeServer 讓它**免操作權杖**（任何連上的頁面都要停得了機台，
//   同告警回答的豁免），所以這裡必須把它鎖死在 action=="stop" —— 否則豁免就變成「不拿權杖也能動馬達」的洞。
bool W906_MotorAccessWire(const std::string& valueJson, long long wireId, std::string& ackOut, bool stopOnly)
{
    ht9045::MotorAccessReq req;
    std::string why;
    if (!ht9045::MotorAccessParse(valueJson, req, why)) {
        std::printf("motor.access REFUSED: %s\n", why.c_str());
        ackOut = why;
        return false;
    }
    if (stopOnly && req.action != "stop") {
        ackOut = "motor.stop 只收 action=stop（它免操作權杖；其他動作請走 motor.access）";
        std::printf("motor.stop REFUSED: action=%s\n", req.action.c_str());
        return false;
    }
    const ht9045::MotorAccessOutcome o =
        ht9045::MotorAccessDispatch(req, wireId, ht9045::MotorAccessLiveBackend());
    std::printf("motor.access %s/%s %s: %s\n", req.source.c_str(), req.action.c_str(),
                req.motors.empty() ? "-" : req.motors[0].c_str(),
                o.ok ? "ok" : o.ackJson.c_str());
    ackOut = o.ackJson;
    return o.ok;
}

// NB2 R23 W4C-4：wb_serve 的 ShowErrorMessage 入口（W906_AlarmStopLikeGolden，golden note.cpp:795 StopAllMotor）同行呼叫。
void W906_MotorAccessOnAlarm(const char* code)
{
    ht9045::MotorAccessOnAlarm(ht9045::MotorAccessLiveBackend(), code ? code : "");
}

// wb_serve 主迴圈每個 500 ms 拍子呼叫（tools/wb_serve.cpp:4048，同行附加在 W906_StateRecordTimer2Pump 之後）。
//   用實際經過的毫秒數推進，拍子被拖慢（1203 Poll 約 140 ms）時等待時間不會被算短。
#include <windows.h>
#include <map>
// W4-c：/api/struct/motor/runtime 的 1203 軸改讀監看器樣本（ChanMotor.h 檔尾的掛鉤）。
// ⚠ NB2 R23 §2 更正：這個掛鉤在 **HTTP socket 執行緒**上被呼叫（wb_serve 的 /api/struct/* 路由），而 Pci1203Monitor 標明
//   NOT THREAD-SAFE —— 主執行緒的 Poll 同時會改樣本（含 std::string driveErrText）。原本直接讀 m->axis() 是資料競爭（可能讀到半寫的字串）。
//   ⇒ 主執行緒每個拍子（W906_MotorAccessTick）把需要的欄位抄一份，掛鉤只在鎖內讀這份抄本。
//   Resolve（查監看器的軸槽）與 GoldenMotor（MOT[]）也都在主執行緒做完 —— socket 執行緒只在鎖內查這張以 Alias 為鍵的表。
namespace {
CRITICAL_SECTION                                            g_ovCs;
bool                                                        g_ovInit = false;
std::map<std::string, ht9045::sjson::MotorRuntimeOverlay>   g_ov;
void OverlaySnapshot()                                                          // 主執行緒
{
    if (!g_ovInit) { ::InitializeCriticalSection(&g_ovCs); g_ovInit = true; }
    std::map<std::string, ht9045::sjson::MotorRuntimeOverlay> v;
    ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
    ht9045::IMotorAccessBackend& be = ht9045::MotorAccessLiveBackend();
    for (std::size_t i = 0; m && i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* row = HSys.MotTable[i];
        if (!row) continue;
        ht9045::MotorAccessAxis a;
        if (!be.Resolve(row->Alias.c_str(), a) || !a.Is1203() || a.axis < 0 || a.axis >= m->axisCount()) continue;
        const ht9045::Pci1203AxisSample& s = m->axis(a.axis);
        if (!s.valid || !s.opened) continue;
        ht9045::sjson::MotorRuntimeOverlay out;
        ht9045::MotorGolden g;
        const bool hasG = be.GoldenMotor(a.motIndex, g);
        if (hasG && g.direction) {
            out.posKnown = false;
            out.why = "Direction=1：方向慣例待使用者決定（夜間報告 §0 第 5 件），位置先不給";
        } else {
            out.posKnown = true;
            out.cmdPos = ht9045::MotorCardToUser(s.cmdPos, hasG ? g.gearRatio : 1.0);
            out.encPos = ht9045::MotorCardToUser(s.actPos, hasG ? g.gearRatio : 1.0);
        }
        out.state   = s.state;
        out.servoOn = (s.motionIO & 0x00004000ul) != 0;                         // AX_MOTION_IO_SVON
        out.alarm   = (s.motionIO & 0x00000002ul) != 0 || (s.state & 0xFFu) == 3u;  // AX_MOTION_IO_ALM 或 ERROR_STOP
        out.inPos   = (s.motionIO & 0x00002000ul) != 0;                         // AX_MOTION_IO_INP
        out.busy    = (s.state & 0xFFu) != 1u;                                  // 不在 READY
        out.driveErr = s.driveErrText;
        v[row->Alias.c_str()] = out;
    }
    ::EnterCriticalSection(&g_ovCs);
    g_ov.swap(v);
    ::LeaveCriticalSection(&g_ovCs);
}
}  // namespace
static bool W906_MotorOverlay(const std::string& alias, ht9045::sjson::MotorRuntimeOverlay& out)   // socket 執行緒
{
    if (!g_ovInit) return false;                                                // 主執行緒還沒抄過（掛鉤在第一次抄完之後才註冊，這行只是保險）
    ::EnterCriticalSection(&g_ovCs);
    std::map<std::string, ht9045::sjson::MotorRuntimeOverlay>::const_iterator it = g_ov.find(alias);
    const bool ok = it != g_ov.end();
    if (ok) out = it->second;
    ::LeaveCriticalSection(&g_ovCs);
    return ok;
}

void W906_MotorAccessTick(bool operatorConnected)
{
    OverlaySnapshot();                                                          // NB2 R23 §2：先抄樣本，再（第一次）註冊掛鉤
    static bool s_overlay = false;
    if (!s_overlay) { ht9045::sjson::SetMotorRuntimeOverlay(&W906_MotorOverlay); s_overlay = true; }
    static DWORD s_last = 0;
    const DWORD now = ::GetTickCount();
    const int elapsed = (s_last == 0) ? 0 : (int)(now - s_last);
    s_last = now;
    if (elapsed > 0) ht9045::MotorAccessTick(ht9045::MotorAccessLiveBackend(), elapsed, operatorConnected);
}
