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
#include "forms/fTeach.h"              // W5-b：fTeach->CheckCanMove／IsCanQuickJogMove（uteach 的互鎖）
#include "Motor/myEthercatmotor.h"     // TMyEtherCatMotor::W906_RuntimeAcc/Dec
#include "mysensor.h"                  // Sen[]（IsMotorCanRun）
#include "cprod.h"
#include "csystem.h"                   // CheckSafeDoorIsClosed
#include "canary_support.h"            // ShowMyMessage
#include "mycylin.h"                   // Cylinder[]（飛梭閘門，NB2 R21 W4B-2）
#include "MachineType.h"               // SOFT_SIMULTE
#include "JsonBridge/ChanMotor.h"      // W4-c：MotorRuntimeOverlay
#include "LastSet.h"                   // AI(W906-W5-b) 20260925: Tech（WebTeachButtons.gen.inc 的 Parameter 指標，與 fTeach 登錄表核對）
#include "cpublic.h"                   // AI(W906-W5-b) 20260925: 登錄條件用的組態旗標（與 forms/fTeachRegistry.cpp 同一組 include）

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

//AI(W906-W5-b) 20260925: W5B-3 —— forms/fTeach.cpp IsCanQuickJogMove 本體的兩個掛鉤（GoldenTeachCanMove 呼叫本體之前裝上）：
//  1203 軸的「Z 在原點」讀監看器 ORG、SOFT_SIMULTE 下被移動的是真的會動的 1203 軸就跑本體。邏輯在 WebMotorAccess.cpp（有單元測試）。
bool TeachLive1203Thunk(int mi) { return MotorAccessTeachLive1203(MotorAccessLiveBackend(), mi); }
int  TeachHomeLedThunk(int mi)  { return MotorAccessTeachHomeLed(MotorAccessLiveBackend(), mi); }

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
        } else if (source == "uteach.home") {
            // AI(W906-W5-b) 20260925: 覆核 R-W5B-6 —— golden uteach btnHomeClick 抬起（uteach.cpp:2193-2194）只有 StopAllMotor()，沒有 MTestY1 的 Galil "ST"
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
        g.teachNoServoOff = (n == MTrayBracketZ || n == MMagazine);              // W5-b：uteach.cpp:3393
        g.indexZ        = (n == MTestZ1 || n == MTestZ2);                       // :3384
        g.indexY        = (n == MTestY1 || n == MTestY2);                       // :3683
        g.rotateKit     = (n == MInRotateKit) ? 1 : (n == MOutRotateKit) ? 2 : 0;   // :3455
        g.servoAlarmOn  = M->PServoAlarmOn;                                     // AI(W906-W5-b) 20260925: W5B-6（golden mymotor.cpp:1931 ServoOnOff）
        g.sensorType    = M->bSensorType;                                       // AI(W906-W5-b) 20260925: W5B-R2（golden InitMotor :252 CFG_AxOrgLogic）
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
        if (pct >= 0) MOT[mi].SetSpeed(pct);                                    // golden edtSpeedChange → MOT.SetSpeed(edtSpeed)；AI(W906-W5-b) 20260925: pct<0＝教導頁，不設速度（R-W5B-4）
        if (positive) MOT[mi].JogP(pct < 0 ? 0 : pct); else MOT[mi].JogN(pct < 0 ? 0 : pct);   // golden :895／:845（TMyMotor::JogP 不用這個引數）
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

    // ---- W5-b（uteach）----
    //AI(W906-W5-b) 20260925: W5B-2 —— 產生表（golden 建構子登錄表的每一列＋條件）在這台機台上求值，再與 fTeach 實際登錄的
    //  TechPara／TechTwoPara 逐列核對（Parameter 指標、MotorSelect、Key）。對不上＝產生表過期或登錄表被改過 ⇒ 回 false（fail-closed）。
    //  每次按鈕都重建（312 列，微秒級）：不快取，免得「開機後改了 Gerneral.ini」這種情況拿到舊表。
    bool GoldenTeachRegistry(TeachRegistry& out, std::string& why) override
    {
        out = TeachRegistry();
        if (!fTeach) { why = "教導頁的 facade（fTeach）沒建（wb_serve 開機會建）"; return false; }
        struct Row { char owner; bool live; const int* p0; const int* p1; int m0; const char* n0; int m1; const char* n1;
                     const char* k0; const char* k1; const char* e0; const char* e1; const char* sb; const char* sh; const char* gb; const char* gh; int vis; };
        struct Ovr { const char* btn; const char* axleBtn; int m; const char* n; };
        struct Btn { const char* btn; const char* handler; int tag; int dfmVis; const char* selfOther; const char* hiddenWhy; int lateReg; };   // AI(W906-W5-b) 20260925: R-W5B-2／3
        // 非 static：條件式與 extern const int 的馬達下標每次在執行期取值（放檔案層會有跨檔靜態初始化順序問題）
#define W5B_COND(e)  (e)
#define W5B_PTR(x)   (&(x))
#define W5B_NOPTR    (const int*)0
#define W5B_MOT(x)   (int)(x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis) { o, live, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis },
#define W5B_TAGOVR(b, a, m)
#define W5B_BTN(b, h, tag, dv, so, hw, lr)
        const Row rows[] = {
#include "WebTeachButtons.gen.inc"
        };
#define W5B_COND(e)  false
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   (int)(x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis)
#define W5B_TAGOVR(b, a, m) { b, a, m },
#define W5B_BTN(b, h, tag, dv, so, hw, lr)
        const Ovr ovr[] = {
#include "WebTeachButtons.gen.inc"
            { 0, 0, -1, 0 }                                                     // 哨兵（表可能是空的）
        };
#define W5B_COND(e)  false
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   (int)(x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis)
#define W5B_TAGOVR(b, a, m)
#define W5B_BTN(b, h, tag, dv, so, hw, lr) { b, h, tag, dv, so, hw, lr },
        const Btn btns[] = {
#include "WebTeachButtons.gen.inc"
            { 0, 0, 0, 0, 0, 0, 0 }                                             // 哨兵
        };
        std::size_t ip = 0, it = 0;
        char b[256];
        for (std::size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i) {
            const Row& w = rows[i];
            if (!w.live) continue;
            TeachRegRow t;
            t.owner = w.owner; t.mot0 = w.m0; t.mot1 = w.m1; t.name0 = w.n0; t.name1 = w.n1; t.key0 = w.k0; t.key1 = w.k1;
            t.edit0 = w.e0; t.edit1 = w.e1; t.setBtn = w.sb; t.setHandler = w.sh; t.goBtn = w.gb; t.goHandler = w.gh;
            t.seq = (int)i; t.vis = w.vis != 0;                                  // AI(W906-W5-b) 20260925: R-W5B-2（golden 建構子的執行順序與 Visible 引數）
            if (w.owner == 'P') {
                const TECH_PARA* q = ip < fTeach->TechPara.size() ? fTeach->TechPara[ip] : 0;
                if (!q || q->Parameter != w.p0 || q->MotorSelect != w.m0 || std::string(q->Key.c_str()) != w.k0) {
                    std::snprintf(b, sizeof(b), "TechPara[%u] 與產生表第 %u 列（%s／%s）對不上", (unsigned)ip, (unsigned)i + 1, w.k0, w.n0);
                    why = b; return false;
                }
                out.P.push_back(t); ++ip;
            } else {
                const TECH_TWOPARA* q = it < fTeach->TechTwoPara.size() ? fTeach->TechTwoPara[it] : 0;
                if (!q || q->Parameter[0] != w.p0 || q->Parameter[1] != w.p1 || q->MotorSelect[0] != w.m0 || q->MotorSelect[1] != w.m1 ||
                    std::string(q->Key[0].c_str()) != w.k0 || std::string(q->Key[1].c_str()) != w.k1) {
                    std::snprintf(b, sizeof(b), "TechTwoPara[%u] 與產生表第 %u 列（%s／%s）對不上", (unsigned)it, (unsigned)i + 1, w.k0, w.k1);
                    why = b; return false;
                }
                out.T.push_back(t); ++it;
            }
        }
        if (ip != fTeach->TechPara.size() || it != fTeach->TechTwoPara.size()) {
            std::snprintf(b, sizeof(b), "實際登錄 TechPara %u／TechTwoPara %u 列，產生表在這台機台的條件下是 %u／%u 列",
                          (unsigned)fTeach->TechPara.size(), (unsigned)fTeach->TechTwoPara.size(), (unsigned)ip, (unsigned)it);
            why = b; return false;
        }
        for (std::size_t i = 0; ovr[i].btn; ++i) out.selectOnly[ovr[i].btn] = std::make_pair(ovr[i].m, std::string(ovr[i].n));
        for (std::size_t i = 0; btns[i].btn; ++i) {                             // AI(W906-W5-b) 20260925: R-W5B-2／3
            TeachBtnInfo b2;
            b2.handler = btns[i].handler; b2.dfmTag = btns[i].tag; b2.dfmVisible = btns[i].dfmVis != 0;
            b2.selfOther = btns[i].selfOther; b2.hiddenWhy = btns[i].hiddenWhy; b2.lateReg = btns[i].lateReg != 0;
            out.btns[btns[i].btn] = b2;
        }
        return true;
    }
    bool GoldenTeachCanMove(int mi) override
    {
        if (!fTeach) return false;                                              // 教導頁的 facade 沒建（wb_serve 開機會建）
        W906_TeachLive1203Hook = &TeachLive1203Thunk;                            // AI(W906-W5-b) 20260925: W5B-3（forms/fTeach.h 檔尾）
        W906_TeachHomeLedHook  = &TeachHomeLedThunk;
        fTeach->ActiveMotorIndex = mi;                                          // golden：按運動鈕時的選取軸
        return fTeach->CheckCanMove() && fTeach->IsCanQuickJogMove();          // forms/fTeach.cpp:330／:135
    }
    bool GoldenMotorPowerOff() override { return Sen[SnMotorPower].IsOff(); }  // uteach.cpp:2137
    int  GoldenReadEncoderPos(int mi) override
    {
        return (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0) ? MOT[mi].ReadEncoderPos() : 0;
    }
    bool GoldenTechPosUsesReadPos(int mi) override                              // AI(W906-W5-b) 20260925: W5B-14（uteach.cpp:3487-3490）
    {
        return MOTION_CARD_TYPE == MotionCard_Contec && mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0 && MOT[mi].Motor->MotorType == 0;
    }
    bool GoldenRotatorLastDirP(int mi) override                                 // AI(W906-W5-b) 20260925: W5B-9
    {
        return (mi >= 0 && mi < MAX_TRAY_MOTOR) ? MOT[mi].iLastRotatorDirP : true;
    }
    void GoldenSetRotatorLastDirP(int mi, bool p) override
    {
        if (mi >= 0 && mi < MAX_TRAY_MOTOR) MOT[mi].iLastRotatorDirP = p;
    }
    int  GoldenProdRotatorBacklash(bool inRot) override { return inRot ? Prod.iIn_iRotateA_Backlash : Prod.iOut_iRotateA_Backlash; }   // golden mymotor.cpp:2017／:2031
    bool Pci1203MotionIO(int axis, unsigned long& io) override                  // AI(W906-W5-b) 20260925: W5B-3
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        io = s.motionIO;
        return true;
    }
    void GoldenSetSpeed(int mi, int pct) override                               // AI(W906-W5-b) 20260925: uteach btnHome 抬起 :2195
    {
        if (mi >= 0 && mi < MAX_TRAY_MOTOR) MOT[mi].SetSpeed(pct);
    }
    void GoldenClearAllMotorHome() override { fAllMotorHome = false; }         // AI(W906-W5-b) 20260925: W5B-R3（golden uteach.cpp:1606／:2439、main.cpp:27845）
    bool GoldenIndexArm3Axis() override { return USE_INDEX_ARM_AXES == IndexArm_3_Axis; }
    int  GoldenTeachRemap(int mi) override
    {
        if (USE_PICKER_COUNT == ep1Picker && (mi == 7 || mi == 26)) return mi - 4;   // Ifor 20260109 fix 單一吸嘴模組顯示異常問題（uteach.cpp:3372／:3409）
        return mi;
    }
    bool AliasOfMotIndex(int mi, std::string& alias) override
    {
        for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
            const TMOTDATA* r = HSys.MotTable[i];
            if (r && MotorIndexOf(r->No) == mi) { alias = r->Alias.c_str(); return !alias.empty(); }
        }
        return false;
    }
    bool Pci1203ActPos(int axis, double& card) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        card = s.actPos;
        return true;
    }

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

// AI(W906-W5-b) 20260925: 覆核 W5B-R5 —— wb_serve 的 start.run 與告警框按 START（W906_AlarmAnswerStartLikeGolden）在呼叫 StartFromWeb 之前問：
//   true＝手動教導中（伺服關、操作員可能正推著軸），不准啟動（why 是給畫面的理由）。
bool W906_MotorAccessStartBlocked(std::string& why)
{
    return ht9045::MotorAccessStartBlocked(why);
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
        if (false && hasG && g.direction) {   //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕；位置照卡片給（使用者單位＝卡片／GearRatio，不翻號）
            out.posKnown = false;
            out.why = "Direction=1：方向慣例待使用者決定（夜間報告 §0 第 5 件），位置先不給";
        } else {
            out.posKnown = true;
            // AI(W906-W5-b) 20260925: W5B-6 —— 手動教導後以編碼器為基準的軸，「目前位置」給編碼器（golden ResetPos 之後 ReadPos＝編碼器；
            //   教導頁 btnSetTo 就是把這個值寫進教導欄位）
            out.cmdPos = ht9045::MotorCardToUser(ht9045::MotorAccessEncoderBase(a.axis) ? s.actPos : s.cmdPos, hasG ? g.gearRatio : 1.0);
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
