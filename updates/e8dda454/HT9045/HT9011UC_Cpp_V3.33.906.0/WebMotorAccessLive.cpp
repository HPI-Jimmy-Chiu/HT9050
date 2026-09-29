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
#include <map>                         //AI(W906-MT-E2) 20260925: GoldenReloadMotorParams
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "database.h"                  // HSys.MotTable / TMOTDATA
#include "common.h"                    //AI(W906-MT-E2) 20260925: MotTablePath (Reload Motor Data re-reads it)
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
#include "JsonBridge/ChanIo.h"         //AI(W906-MT-E3c) 20260925: IoByteSample / PickIoSample (the relay's DO read-back)
#include "EtherCAT/Pci1203IoRoute.h"   //AI(W906-MT-E3c) 20260925: Pci1203RouteSetSource / Pci1203RouteLastWrite (Motor Power's SW[] writes)
#include "WebWindowRegistry.h"         //AI(W906-MT-E3c) 20260925: WebWindowRegistryFShowPolicy (the engine's W906_FormFShowHook)
#include "myswitch.h"                  //AI(W906-MT-E3c) 20260925: SW[] (SwMotorRelay / SwServerON)
#include "forms/fHome.h"               //AI(W906-MT-E3c) 20260925: fHome->GaliMotorServoOff (MT-E3b)
#include "LastSet.h"                   // AI(W906-W5-b) 20260925: Tech（WebTeachButtons.gen.inc 的 Parameter 指標，與 fTeach 登錄表核對）
#include "cpublic.h"                   // AI(W906-W5-b) 20260925: 登錄條件用的組態旗標（與 forms/fTeachRegistry.cpp 同一組 include）
//AI(W906-MERGE-56bbf785) 20260926: the include lists of both sides (machine MT-E3c + laptop W5-b), nothing dropped.
#include "EtherCAT/Pci1203MotorRoute.h"   //AI(W906-ENG1203) 20260929: review HIGH-1 / MEDIUM-1 -- W906_EngineRouteForeignStop (EOF) and the route's per-axis fault latch (OverlaySnapshot). Was a blank line: no line below moves
extern void StopAllMotor(bool bIndexCanStop);   // Motor/myGALILmotor.cpp（golden Motor/myGALILmotor.cpp:4712）
void SetMotorAccelSpeed(int Index, int ADCSpeed);   //AI(W906-MT-E2) 20260925: cinitial.cpp:16730 (golden cinitial.h:51) -- Reload Motor Data

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

//AI(W906-MT-E3c) 20260925: the Mot_Table row behind MOT[mi] (first row whose No is "M%02d"(mi), cinitial.cpp's rule) and
//  whether it is a PCI1203 row -- EastSun R2: such a row is a 1203 axis whatever INDEX_MOTION_CARD says (M14 MTestZ1).
const TMOTDATA* RowOfIndex(int mi)
{
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (r && MotorIndexOf(r->No) == mi) return r;
    }
    return 0;
}
bool RowIs1203(int mi)
{
    const TMOTDATA* r = RowOfIndex(mi);
    return r && std::string(r->CardModel.c_str()) == "PCI1203";
}

//AI(W906-MT-E3c) 20260925: the monitor's DO read-back of SW[sw]'s bit, the way the route addresses it
//  (EtherCAT/Pci1203IoRoute.cpp RouteWriteBit/CheckWrite_: ring = Lane, station = IP, byte = Port/8, bit = Port%8;
//  JsonBridge/ChanIo.h PickIoSample). false = no reading: the point is disabled, the engine's IO is not routed to the 1203,
//  no open / polling monitor, the byte is not in the card map, or it did not read back.
bool RelayCardBit(int sw, bool& bit)
{
    bit = false;
    const TMySwitch& s = SW[sw];
    if (!s.Enable || s.Port < 0 || !ht9045::Pci1203RouteInstalled()) return false;
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    if (mon == 0 || !mon->Open_() || mon->Disabled() || !mon->card().open) return false;
    std::vector<ht9045::sjson::IoByteSample> v;
    for (int i = 0; i < mon->doCount(); ++i) {
        const ht9045::Pci1203DoSample& d = mon->do_(i);
        ht9045::sjson::IoByteSample b;
        b.valid = d.valid; b.ring = d.ring; b.station = d.station; b.stationChan = d.stationChan; b.byteData = d.byteData;
        v.push_back(b);
    }
    const int k = ht9045::sjson::PickIoSample(v, s.Ring, s.IP, s.Port / 8, 0);
    if (k < 0 || !v[k].valid) return false;
    bit = ((v[k].byteData >> (s.Port % 8)) & 1) != 0;
    return true;
}

//AI(W906-MERGE-56bbf785) 20260926: machine helpers above (MT-E3c) + the laptop's two teach-page thunks below (W5-b), both kept.
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
    // AI(W906-MT-E1) 20260925: see WebMotorAccess.h. Same identity rule as EtherCAT/Pci1203IoRoute.cpp
    //   StationIsDrive (and web/js/pci1203/view.js stationIsDrive): a present slave at the axis's station on
    //   ring 0 (drive SDO reads use ring 0) that says CiA 402 or SERVOPACK; the axis's own driveIsSigmaX too.
    int Pci1203DriveKind(int axis) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return -1;
        const Pci1203AxisSample& a = m->axis(axis);
        if (!a.opened || a.station < 0) return -1;
        if (a.driveIsSigmaX) return 1;
        bool otherProfile = false;
        for (int i = 0; i < m->slaveCount(); ++i) {
            const Pci1203SlaveSample& s = m->slave(i);
            if (!s.present || s.addr != a.station) continue;
            if (s.ring >= 0 && s.ring != 0) continue;
            if (s.profileValid && s.profile == 402) return 1;
            std::string u(s.name);
            for (std::size_t k = 0; k < u.size(); ++k) if (u[k] >= 'a' && u[k] <= 'z') u[k] = (char)(u[k] - 'a' + 'A');
            if (u.find("SERVOPACK") != std::string::npos) return 1;
            if (s.profileValid) otherProfile = true;
        }
        return otherProfile ? 0 : -1;
    }

    void GoldenClearAllHomeFlags() override                                     // AI(W906-MT-E1) 20260925: golden InitialMotorParameter's HomeFlag=0 (cinitial.cpp:3615/:3773/:3989)
    {
        for (int i = 0; i < MAX_TRAY_MOTOR; ++i) MOT[i].HomeFlag = 0;
        std::printf("motor.access: MOT[0..%d].HomeFlag = 0 (Reload Motor Data)\n", MAX_TRAY_MOTOR - 1);
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
        const Pci1203CmdResult r = ctl->Execute(c);  ::W906_EngineRouteForeignStop(c, r);   //AI(W906-ENG1203) 20260929: review HIGH-1 -- EVERY WebMotorAccess 1203 command (Stop1203 / Stop1203All: alarm, STOP, jog release, dead-man, cancels) passes here; a stop of an engine-claimed axis cancels its route home job, marks it pending and resets golden fCMD on its MOT rows. No engine route installed = nothing (WB_ENGINE_MOTOR_1203 is off)
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
        //AI(W906-MT-E3c) 20260925: EastSun R2 -- a PCI1203 row is never stopped through golden's Galil "ST" (its stop is the
        //  1203 Stop1203 the dispatcher sends); the dispatcher does not call this for a 1203 row, this is the second lock.
        if (RowIs1203(motIndex)) return;
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
        //AI(W906-MT-E3c) 20260925: EASTSUN R2 -- a Mot_Table row whose CardModel is PCI1203 is a 1203 axis (M14 MTestZ1 on HT9050;
        //  INDEX_MOTION_CARD stays 0, Gerneral.ini is not edited). So neither golden Index rule applies to it:
        //    galilIndex=false -- golden's Galil branch (Gali_MotMove / "ST") is not its path;
        //    indexMotor=false -- HT9050 DEVIATION: golden TMyMotor::SetSpeed is EMPTY for the Index four (Motor/mymotor.cpp
        //      :485-487), which leaves a 1203 axis at whatever speed the card holds -- undefined for an axis golden never drove
        //      through a card. It gets the same SetSpeed(pct) -> PTP/jog family as every other 1203 axis (from its Mot_Table
        //      speeds: M14 JogHigh 900000 / Acc 9000000 -- check them before the first move).
        if (RowIs1203(mi)) { g.indexMotor = false; g.galilIndex = false; }
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
        g.range         = M->ReadRange();                                       //AI(W906-MT-E2) 20260925: golden UpdateMotorParameter :668 / lM00Click :757-758
        g.rate          = M->ReadRate();
        g.readSpeed     = MOT[mi].GetSpeed();                                   //   golden Timer1Timer :982 lblRealSpeed (cached iSpeed / speed, no driver call)
        g.lastHomePos   = M->LastHomePos;                                       //   golden Timer1Timer :965 edtHomeOffset
        //AI(W906-MT-E3c) 20260925: what golden SetRate / InitMotor read (Test Range / Rate, R4)
        g.iSpeed        = M->ReadSpeed();                                       // HTMotor::iSpeed
        g.motorClass    = (M->MotorType == Servo_Motor) ? kInitCfgMotorServo : (M->MotorType == Rotate_Motor) ? kInitCfgMotorRotate : kInitCfgMotorOther;
        g.sensorType    = M->bSensorType;                                       // (also W5-b W5B-R2: golden InitMotor :252 CFG_AxOrgLogic)
        g.in1Logic      = M->bIn1Logic;
        //AI(W906-MERGE-56bbf785) 20260926: machine MT-E2/E3c fields above + the laptop's W5-b fields below; sensorType set once.
        //  indexZ / indexY are golden's teach-page rules on the motor's identity (uteach ActiveMotorIndex==MTestZ1...), so they stay
        //  set on a PCI1203 Index row too (unlike indexMotor / galilIndex above, which EastSun R2 clears for such a row).
        g.teachNoServoOff = (n == MTrayBracketZ || n == MMagazine);              // W5-b：uteach.cpp:3393
        g.indexZ        = (n == MTestZ1 || n == MTestZ2);                       // :3384
        g.indexY        = (n == MTestY1 || n == MTestY2);                       // :3683
        g.rotateKit     = (n == MInRotateKit) ? 1 : (n == MOutRotateKit) ? 2 : 0;   // :3455
        g.servoAlarmOn  = M->PServoAlarmOn;                                     // AI(W906-W5-b) 20260925: W5B-6（golden mymotor.cpp:1931 ServoOnOff）
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
        //AI(W906-MT-E2) 20260925: pct = the last scroll-bar position (DoJog, user ruling 「Jog = golden」), so this re-applies
        //  golden's SetSpeed(pos, true) (:801) -- was MOT.SetSpeed(edtSpeed) (the PTP family) as a stand-in. golden JogP/JogN
        //  ignore their argument (Motor/mymotor.cpp:1322-1335).
        //AI(W906-W5-b) 20260925: pct<0＝教導頁，不設速度（R-W5B-4）
        //AI(W906-MERGE-56bbf785) 20260926: machine = SetSpeed(pct, true) (Motor Test), laptop = no SetSpeed when pct<0 (teach page):
        //  both -- the laptop's `SetSpeed(pct)` arm is superseded by the machine's (pct, true) for every pct>=0 caller (only DoJog).
        if (pct >= 0) MOT[mi].SetSpeed(pct, true);
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
    //AI(W906-MERGE-56bbf785) 20260926: the laptop's Pci1203MotionIO (W5B-3) and GoldenSetSpeed(mi, pct) (uteach btnHome 抬起 :2195)
    //  were dropped here: the machine's MT-E2 block below has the identical Pci1203MotionIO, and GoldenSetSpeed(mi, pct, jog=false)
    //  is golden SetSpeed(pct) (the teach page now calls that; see WebMotorAccess.h).
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

    // ---- AI(W906-MT-E2) 20260925 ----
    void GoldenSetSpeed(int mi, int pct, bool jog) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        MOT[mi].SetSpeed(pct, jog);                                             // TMyMotor::SetSpeed(double p, bool bSetJog) (Motor/mymotor.cpp:320)   //AI(W906-ENG1203) 20260929: ⚠ with the ENGINE MOTOR ROUTE installed (WB_ENGINE_MOTOR_1203, off by default) this also WRITES the card for a 1203 row (TMyEtherCatMotor::SetSpeed -> route kEcSetSpeed, the same values Motor Test sends -- Q7's dedup skips an equal write the card reads back); without the route it only sets iSpeed, as before
    }
    void GoldenSetCell(int mi, int row, double v) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        HTMotor* M = MOT[mi].Motor;
        // golden assigns `double ret` to the unsigned fields; BCB converts through a 64-bit integer (__ftol), so an
        //   integral ret below 0 wraps modulo 2^32 -- (unsigned)(int) is that, without C++'s undefined double->unsigned.
        //   The dispatcher already made ret golden's atoi value (integral, in int range) for every row but 8/9.
        const int iv = (row == 8 || row == 9) ? 0 : (int)v;
        switch (row) {
            case 1:  M->SetInitSpeed((unsigned)iv);    break;                  // golden :1200 (TMyEtherCatMotor: InitSpeed=x)
            case 2:  M->PJogHighSpeed  = (unsigned)iv; break;                  // :1202
            case 3:  M->PJogLowSpeed   = (unsigned)iv; break;                  // :1204
            case 4:  M->PHomeHighSpeed = (unsigned)iv; break;                  // :1206
            case 5:  M->PHomeLowSpeed  = (unsigned)iv; break;                  // :1208
            case 6:  M->PSoftLimitP    = iv;           break;                  // :1210
            case 7:  M->PSoftLimitN    = iv;           break;                  // :1212
            case 8:  M->SetAccDataBase(v);             break;                  // :1214
            case 9:  M->SetDecDataBase(v);             break;                  // :1216
            case 10: M->SetRange((unsigned)iv);        break;                  // :1218 (TMyEtherCatMotor: Range=min(a,1000))
            default: return;
        }
        std::printf("motor.access: MOT[%d] strngrdMotor row %d = %g (golden SelectCell, memory only)\n", mi, row, v);
    }
    void GoldenSetLastHomePos(int mi, int v) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        MOT[mi].Motor->LastHomePos = v;
        std::printf("motor.access: MOT[%d].Motor->LastHomePos = %d\n", mi, v);
    }
    bool Pci1203MotionIO(int axis, unsigned long& io) override
    {
        TPci1203Monitor* m = Pci1203Monitor();
        if (!m || axis < 0 || axis >= m->axisCount()) return false;
        const Pci1203AxisSample& s = m->axis(axis);
        if (!s.valid || !s.opened) return false;
        io = s.motionIO;
        return true;
    }
    bool GoldenAlarmLed(int mi, bool& known) override
    {
        known = false;
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return false;
        if (RowIs1203(mi)) return false;                                        //AI(W906-MT-E3c) 20260925: R2 -- a 1203 row's lamp is the monitor's (the dispatcher reads it there)
        if (INDEX_MOTION_CARD == 0 && (mi == MTestY1 || mi == MTestZ1 || mi == MTestZ2 || mi == MTestY2))
            MOT[mi].Gali_ScanMotStatus();                                       // golden UpdateMotorLed :636-638
        else
            MOT[mi].ScanMotorStatus();                                          // :640
        known = true;
        return MOT[mi].Led[iAlarmLed];
    }
    MotorReloadResult GoldenReloadMotorParams() override;                       // below the class
    double MonotonicMs() override
    {
        LARGE_INTEGER f, c;                                                     // golden TQPF_Timer's clock (myTimer.cpp:128-134)
        if (!::QueryPerformanceFrequency(&f) || f.QuadPart == 0 || !::QueryPerformanceCounter(&c)) return -1.0;
        return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
    }

    // ---- AI(W906-MT-E3c) 20260925 ----
    //  Motor Power: SW[] writes with the route source "web" (always executed and recorded, Pci1203IoRoute.h), restored to
    //  "engine" right after -- the same bracket JsonBridge/IoBtnPanelClick.cpp puts around its IOBitOn.
    MotorPowerState GoldenMotorPower(bool syncFromCard) override
    {
        MotorPowerState s;
        bool card = false;
        s.cardKnown = RelayCardBit(SwMotorRelay, card);
        s.card = card;
        if (syncFromCard && s.cardKnown && SW[SwMotorRelay].OutValue != card) {
            SW[SwMotorRelay].OutValue = card;                                   // lead default: the card is the truth (IO-page clicks bypass OutValue)
            s.synced = true;
            std::printf("motor.access: SW[SwMotorRelay].OutValue := %d from the 1203 DO read-back\n", (int)card);
        }
        s.relayOn = SW[SwMotorRelay].OutValue;
        s.motorPowerState = bMotorPowerState;
        return s;
    }
    void GoldenMotorPowerOnBegin() override
    {
        Pci1203RouteSetSource("web");
        W906_DoMotorPowerOnBegin();                                             // csystem.h (MT-E3b): golden DoMotorPowerOn, first pass
        Pci1203RouteSetSource("engine");
        std::printf("motor.access: DoMotorPowerOn begin (relay On + IndexMotorBreakerOFF/MagazineBreakerOFF/CassetteBreakerOFF)\n");
    }
    bool GoldenMotorPowerOnStep() override
    {
        Pci1203RouteSetSource("web");
        const bool done = W906_DoMotorPowerOnStep();
        Pci1203RouteSetSource("engine");
        return done;
    }
    void GoldenServerOn() override
    {
        Pci1203RouteSetSource("web");
        SW[SwServerON].On();                                                    // golden :1635 / :1042
        Pci1203RouteSetSource("engine");
        std::printf("motor.access: SW[SwServerON].On()\n");
    }
    void GoldenMotorServoOff(const std::string& sFunc) override
    {
        if (fHome == 0) { std::printf("motor.access: fHome is NULL -- GaliMotorServoOff(%s) not run\n", sFunc.c_str()); return; }
        Pci1203RouteSetSource("web");
        fHome->GaliMotorServoOff(AnsiString(sFunc.c_str()));                    // uhome.cpp (MT-E3b): golden uhome.cpp:4988-5016
        Pci1203RouteSetSource("engine");
        std::printf("motor.access: fHome->GaliMotorServoOff(\"%s\")\n", sFunc.c_str());
    }
    std::string GoldenRouteLastWrite() override
    {
        const Pci1203RouteWrite& w = Pci1203RouteLastWrite();
        if (w.seq == 0) return "no write through the 1203 route yet";
        char b[320];
        std::snprintf(b, sizeof(b), "#%lu ring %d st %d %s %d = %d: %s%s%s", w.seq, w.ring, w.station, w.byte ? "byte" : "chan", w.port, w.value,
                      !w.reached || !w.accepted ? "REFUSED" : w.issued ? (w.ret == 0 ? "ISSUED OK" : "ISSUED, VENDOR ERROR") : "DRY RUN",
                      w.why.empty() ? "" : " -- ", w.why.c_str());
        return b;
    }
    void GoldenSetRangeMemory(int mi, unsigned v) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        MOT[mi].Motor->SetRange(v);                                             // TMyEtherCatMotor::SetRange: Range=min(a,1000), no card
        std::printf("motor.access: MOT[%d].Motor->SetRange(%u) (memory)\n", mi, v);
    }
    void GoldenSetAccMemory(int mi, double a) override
    {
        if (mi < 0 || mi >= MAX_TRAY_MOTOR || MOT[mi].Motor == 0) return;
        TMyEtherCatMotor* E = dynamic_cast<TMyEtherCatMotor*>(MOT[mi].Motor);
        if (E) E->SetAcc(a);                                                    // `dAcc=a` (myEthercatmotor.cpp:1563) -- the member SetRate assigns
        else   MOT[mi].Motor->HTMotor::SetAcc(a);                               // another object on a PCI1203 row (M14): the base member, no card
        std::printf("motor.access: MOT[%d] dAcc = %.9g (golden SetRate, memory)\n", mi, a);
    }
    std::vector<InitCfg> GoldenInitCfgPlan(const MotorGolden& g) override
    {
        Pci1203InitCfgStep plan[16];
        const int n = Pci1203GoldenInitCfgPlan(g.motorClass, g.sensorType, g.in1Logic, plan, 16);   // EastSun MT-E3a (golden order)
        std::vector<InitCfg> v;
        for (int i = 0; i < n && i < 16; ++i) { InitCfg c; c.which = plan[i].which; c.value = plan[i].value; c.name = Pci1203InitCfgName(plan[i].which); v.push_back(c); }
        return v;
    }
    bool GoldenInitMotorEmgOff() override
    {
        return Sen[SnFrontLeftEMG].IsOff() || Sen[SnFrontRightEMG].IsOff() ||   // golden myEthercatmotor.cpp:180-181
               Sen[SnRearLeftEMG].IsOff()  || Sen[SnRearRightEMG].IsOff();
    }
    void SleepMs(int ms) override { if (ms > 0) ::Sleep((DWORD)ms); }
    void GoldenInitMOTParameterAll() override
    {
        for (int i = 0; i < TOTAL_MOTOR && i < MAX_TRAY_MOTOR; ++i) MOT[i].InitMOTParameter();   // golden uMotorTest.cpp:1314-1315
    }
    void GoldenFormClose() override { PauseUT150Polling = false; }              // golden uMotorTest.cpp:1361
    std::string AliasOfMotor(int mi) override
    {
        const TMOTDATA* r = RowOfIndex(mi);
        return r ? std::string(r->Alias.c_str()) : std::string();
    }
    int GoldenLightScaleEncoder(std::string& src, bool& fromMonitor) override
    {
        fromMonitor = false;
        const TMOTDATA* row = RowOfIndex(MLightScale);
        if (row && std::string(row->CardModel.c_str()) == "PCI1203") {        // a 1203 scale axis: its actual position (golden ReadEncoderPos)
            MotorAccessAxis a;
            TPci1203Monitor* m = Pci1203Monitor();
            if (Resolve(row->Alias.c_str(), a) && a.axis >= 0 && m && a.axis < m->axisCount() && m->axis(a.axis).valid) {
                const bool hasG = MOT[MLightScale].Motor != 0;
                //AI(W906-MERGE-56bbf785) 20260926: `if (Direction) { "Direction=1 withheld (§0 #5)"; return 0; }` removed -- ruling 6B
                //  (RULINGS_20260925 #13: a 1203 axis ignores Direction), as the laptop did for the runtime overlay below.
                src = "pci1203";
                fromMonitor = true;
                return MotorCardToUser(m->axis(a.axis).actPos, hasG ? MOT[MLightScale].Motor->GearRatio : 1.0);
            }
            src = "pci1203: axis not readable (" + a.why + ") -> 0";
            return 0;
        }
        if (MOT[MLightScale].Motor != 0) { src = row ? "MOT" : "MOT (no Mot_Table row)"; return MOT[MLightScale].ReadEncoderPos(); }   // golden, verbatim
        src = "none";
        return 0;
    }
    int GoldenLightScaleDataCount(int k) override
    {
        switch (k) {
            case 1: return iLogLightScaleCount_InArmX1;   case 2: return iLogLightScaleCount_InArmX2;
            case 3: return iLogLightScaleCount_InArmY1;   case 4: return iLogLightScaleCount_InArmY2;
            case 5: return iLogLightScaleCount_OutArmX1;  case 6: return iLogLightScaleCount_OutArmX2;
            case 7: return iLogLightScaleCount_OutArmY1;  case 8: return iLogLightScaleCount_OutArmY2;
        }
        return 0;
    }
    static TMemo* LsMemo(int k)
    {
        if (fMotorTest == 0) return 0;                                          // wb_serve does not build fMotorTest (forms/fMotorTest.cpp:99)
        TMemo* const p[9] = { 0, fMotorTest->mmo1, fMotorTest->mmo2, fMotorTest->mmo3, fMotorTest->mmo4,
                              fMotorTest->mmo5, fMotorTest->mmo6, fMotorTest->mmo7, fMotorTest->mmo8 };
        return (k >= 1 && k <= 8) ? p[k] : 0;
    }
    std::vector<std::string> GoldenLightScaleData(int k) override
    {
        std::vector<std::string> v;
        TMemo* m = LsMemo(k);
        if (m && m->Lines) for (int i = 0; i < m->Lines->Count; ++i) v.push_back(m->Lines->Strings[i].c_str());
        return v;
    }
    void GoldenLightScaleDataClear(int k) override { TMemo* m = LsMemo(k); if (m) m->Clear(); }
    void GoldenLightScaleCountsReset() override
    {
        iLogLightScaleCount_InArmX1 = 0; iLogLightScaleCount_InArmX2 = 0; iLogLightScaleCount_InArmY1 = 0; iLogLightScaleCount_InArmY2 = 0;
        iLogLightScaleCount_OutArmX1 = 0; iLogLightScaleCount_OutArmX2 = 0; iLogLightScaleCount_OutArmY1 = 0; iLogLightScaleCount_OutArmY2 = 0;
    }
    std::string LightScaleRoot() override { return "D:\\LightScale"; }          // golden uMotorTest.cpp:1752 / :2055
    std::string LocalStampYmdhm() override
    {
        SYSTEMTIME t;
        ::GetLocalTime(&t);                                                     // golden FormatDateTime("yyyymmddhhmm", Now())
        char b[32];
        std::snprintf(b, sizeof(b), "%04u%02u%02u%02u%02u", (unsigned)t.wYear, (unsigned)t.wMonth, (unsigned)t.wDay, (unsigned)t.wHour, (unsigned)t.wMinute);
        return b;
    }
};

//AI(W906-MT-E2) 20260925: golden btnReloadMotorDataClick's InitialMotorParameter() (golden cinitial.cpp:3392 on; port
//  cinitial.cpp:3835-4107), WITHOUT the parts that would fight the 1203 monitor or re-shape the process:
//    * the file is read the port's own way (TStringList + TMOTNO::SetMOTTableNo + the TMOTDATA row parser = the pieces of
//      SYSTEM_MODULAR::LoadMotData, database.cpp:1774) into a PRIVATE list -- HSys.MotTable / mapMotTable (what Resolve,
//      /api/struct/motor/config and the monitor's axis names are keyed on) are not replaced; HSys.MotNo is put back after.
//      READ ONLY: Mot_Table.csv is never written.
//    * a motor whose row identity changed (Motorname / Alias / CardModel / BoardID / Port / IP / Enable, or the row is new
//      or gone) gets NOTHING -- its object was built for the old row; the ack says restart wb_serve.
//    * for the others, exactly the table assignments of the bHasMotor branch (cinitial.cpp:4039-4094) onto the EXISTING
//      MOT[i].Motor, then golden's SetMotorAccelSpeed(i, 100) (dAcc/dDec back to the database value -- memory only for the
//      card types here, TMyEtherCatMotor::SetAcc is `dAcc=a`).
//    * NOT re-run: InitialMotorName, `new`/`delete` of motor objects, Motor->Enable and MOT[i].CardType (identity),
//      SetArmMaxSpeed (a SYNTEK card write), InitMotor(iAdder) (opens the axis -- the monitor owns every 1203 axis), and
//      MC88X1's SetRate (no MC88X1 driver in this port, cinitial.cpp:3979).
//  HomeFlag=0 for every motor is done by the caller first (GoldenClearAllHomeFlags), as golden's loop does for every i.
//  (Still inside this file's anonymous namespace.)
bool SameMotRowIdentity(const TMOTDATA& a, const TMOTDATA& b)
{
    return a.No == b.No && a.Alias == b.Alias && a.CardModel == b.CardModel && a.iBoardID == b.iBoardID &&
           a.iPort == b.iPort && a.iIP == b.iIP && a.iEnable == b.iEnable;
}
void ReloadApplyRow(int i, const TMOTDATA& t)                                   // cinitial.cpp:4039-4094, line for line
{
    HTMotor* M = MOT[i].Motor;
    const AnsiString sModel = t.CardModel;
    M->GearRatio         = t.dGearRatio;
    M->Direction         = (t.iDirection == 1) ? true : false;
    M->HomeDirection     = (t.iHomeDirectior == 1) ? true : false;
    double dAcc = t.dAcc;
    double dDec = t.dDec;
    if (sModel == "MN200") {                                                    // Steven 20230616 : MN200的加減速單位是秒
        if (dAcc > 1) dAcc = t.dAcc / 100.0;
        if (dDec > 1) dDec = t.dDec / 100.0;
    } else if (sModel == "MC88X1") {                                            // golden also calls SetRate(iRate) -- not re-run (see above)
        dAcc = t.iRate;
        dDec = t.iRate;
    }
    M->SetAccDataBase(dAcc);
    M->SetDecDataBase(dDec);
    M->SetRange(t.iRange);
    M->PHomeHighSpeed    = t.iHomeHighSpeed;
    M->PHomeLowSpeed     = t.iHomeLowSpeed;
    M->PJogHighSpeed     = t.iJogHighSpeed;
    M->PJogLowSpeed      = t.iJogLowSpeed;
    M->SetInitSpeed(t.iInitSpeed);
    M->InitSpeed         = t.iInitSpeed;
    M->PServoAlarmOn     = (t.iServoAlarmOn == 1) ? true : false;
    M->MotorType         = t.i1P2P;
    M->bSensorType       = t.iSensorType;
    M->bLimitLogic       = (t.iLimitLogic == 1) ? true : false;
    M->bIn1Logic         = (t.iIn1Logic == 1) ? true : false;
    M->PSoftLimitN       = t.iSoftLimitN;
    M->PSoftLimitP       = t.iSoftLimitP;
    MOT[i].SimulateSpeed = t.iSimulateSpeed;
    MOT[i].HomeFlag      = 0;                                                   // cinitial.cpp:4078
    SetMotorAccelSpeed(i, 100);                                                 // :4079
    if (MOT[i].Mot_Name == MInShuttle1) { iInitSpeedSh1 = M->InitSpeed; iPJogHighSpeedSh1 = M->PJogHighSpeed; }   // :4081-4085
    if (MOT[i].Mot_Name == MInShuttle2) { iInitSpeedSh2 = M->InitSpeed; iPJogHighSpeedSh2 = M->PJogHighSpeed; }   // :4087-4091
    if (i == MTestZ1 || i == MTestZ2) MOT[i].IndexPickLimit = t.iPickLimit;    // :4093-4094
}

MotorReloadResult LiveBackend::GoldenReloadMotorParams()
{
    MotorReloadResult R;
    const AnsiString path = MotTablePath;                                       // what the boot LoadMotData read (database.cpp:1778)
    if (!FileExists(path)) { R.why = std::string("file not found: ") + path.c_str(); return R; }
    std::vector<TMOTDATA*> rows;
    const TMOTNO bootCols = HSys.MotNo;
    TStringList* SL = new TStringList();
    try {
        SL->LoadFromFile(path);
        if (SL->Count <= 1)
            R.why = std::string("no data rows in ") + path.c_str();
        else if (HSys.MotNo.SetMOTTableNo(SL->Strings[0]) < HSys.MotNo.emotTotal - 1)   // LoadMotData's own gate (database.cpp:1799)
            R.why = std::string("header not understood (LoadMotData gate) in ") + path.c_str();
        else {
            for (int k = 1; k < SL->Count; ++k) rows.push_back(new TMOTDATA(SL->Strings[k]));
            R.read = true;
        }
    } catch (...) {
        R.why = std::string("could not read (opened by other software?) ") + path.c_str();
    }
    HSys.MotNo = bootCols;                                                      // HSys.MotTable was parsed with this map
    delete SL;
    if (R.read) {
        std::map<std::string, const TMOTDATA*> byNo;                            // LoadMotData's rule: the first row of a Motorname wins
        for (std::size_t k = 0; k < rows.size(); ++k) {
            const std::string no = rows[k]->No.c_str();
            if (!no.empty() && byNo.find(no) == byNo.end()) byNo[no] = rows[k];
        }
        for (int i = 0; i < TOTAL_MOTOR && i < MAX_TRAY_MOTOR; ++i) {
            AnsiString key;
            key.sprintf("M%02d", i);
            const TMOTDATA* o = 0;
            std::map<AnsiString, AnsiString>::iterator it = HSys.mapMotTable.find(key);   // the boot row (cinitial.cpp:3880-3883)
            if (it != HSys.mapMotTable.end()) {
                const int k = std::atoi(it->second.c_str());
                if (k >= 0 && k < (int)HSys.MotTable.size()) o = HSys.MotTable[k];
            }
            std::map<std::string, const TMOTDATA*>::const_iterator nt = byNo.find(key.c_str());
            const TMOTDATA* n = (nt == byNo.end()) ? 0 : nt->second;
            if (!o && !n) continue;
            if (!o || !n || !SameMotRowIdentity(*o, *n)) {
                ++R.identityChanged;
                if (R.firstChanged.empty())
                    R.firstChanged = std::string(key.c_str()) + (!o ? " (row added)" : !n ? " (row removed)" : " (No/Alias/CardModel/BoardID/Port/IP/Enable changed)");
                continue;
            }
            if (MOT[i].Motor == 0) continue;
            ReloadApplyRow(i, *n);
            ++R.applied;
        }
    }
    for (std::size_t k = 0; k < rows.size(); ++k) delete rows[k];
    std::printf("motor.access reloadMotorData: %s: %s, applied %d, identity changed %d%s%s\n", path.c_str(),
                R.read ? "re-read" : "NOT re-read", R.applied, R.identityChanged,
                R.firstChanged.empty() ? "" : " first ", R.firstChanged.c_str());
    if (!R.read) std::printf("  why: %s\n", R.why.c_str());
    return R;
}

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
ht9045::sjson::MotorTestPageState                           g_page;   //AI(W906-MT-E2) 20260925: under g_ovCs too
void OverlaySnapshot()                                                          // 主執行緒
{
    if (!g_ovInit) { ::InitializeCriticalSection(&g_ovCs); g_ovInit = true; }
    std::map<std::string, ht9045::sjson::MotorRuntimeOverlay> v;
    ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
    ht9045::IMotorAccessBackend& be = ht9045::MotorAccessLiveBackend();
    const ht9045::MotorAccessJobState js = ht9045::MotorAccessJobs();          // AI(W906-MT-E1) 20260925: btnHome / btnLoopMove state
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
        out.driveErr = s.driveErrText;  { ht9045::Pci1203MotorRouteFault f; if (ht9045::Pci1203MotorRouteAxisFault(s.station, s.stationAxis, f)) out.why = ht9045::Pci1203MotorRouteFaultText(f); }   //AI(W906-ENG1203) 20260929: review MEDIUM-1 -- the engine route's latched failed stops / motions of this axis -> /api/struct/motor/... diag.why ("" when none, as before; the route never saw the axis = untouched)
        out.ioKnown  = true;                                                    // AI(W906-MT-E1) 20260925: Motor Test ALed1..10
        out.motionIO = s.motionIO;
        const std::string alias = row->Alias.c_str();
        for (std::size_t k = 0; k < js.homeMotors.size(); ++k) if (js.homeMotors[k] == alias) { out.homeJob = true; break; }
        out.loopJob   = js.loopActive && js.loopMotor == alias;
        for (std::size_t k = 0; !out.loopJob && js.loopActive && k < js.loopMotors.size(); ++k) out.loopJob = (js.loopMotors[k] == alias);   //AI(W906-MT-E3c) 20260925: every motor of an All-mode loop
        out.loopCount = out.loopJob ? js.loopCount : 0;
        //AI(W906-MT-E3c) 20260925: actual torque (MT-E3a sample fields; valid for this poll only -- null when not)
        out.torqueRawValid     = s.torqueValid;
        out.torqueRaw          = s.torqueRaw;
        out.torqueSrc          = s.torqueSrc;
        out.torquePctValid     = s.torquePctValid;
        out.torquePct          = s.torquePct;
        out.torqueNmValid      = s.torqueNmValid;
        out.torqueNm           = s.torqueNm;
        out.torqueUnitVerified = s.torqueUnitVerified;
        v[row->Alias.c_str()] = out;
    }
    //AI(W906-MT-E2) 20260925: the page-wide half (golden ActiveIndex, lblJogPTime / lblJogNTime / lblAvgTime), same copy rule.
    ht9045::sjson::MotorTestPageState pg;
    pg.selectedMotor = js.selectedMotor;
    pg.hasJogPTime = js.hasJogPTime;  pg.jogPTime = js.jogPTime;
    pg.hasJogNTime = js.hasJogNTime;  pg.jogNTime = js.jogNTime;
    pg.hasAvgTime  = js.hasAvgTime;   pg.avgTime  = js.avgTime;
    //AI(W906-MT-E3c) 20260925: motorPower / lock / lightScale (the contract's three top-level blocks), copied on this thread --
    //  W906_GetMotorLockState() and SW[] may only be read here (csystem.h THREADING).
    {
        const ht9045::MotorPowerState ps = be.GoldenMotorPower(false);         // read only: no sync from the runtime producer
        pg.hasPower = true;
        pg.relayOn = ps.relayOn; pg.relayCardKnown = ps.cardKnown; pg.relayCard = ps.card;
        pg.motorPowerState = ps.motorPowerState; pg.powerPending = js.powerPending;
        const W906MotorLockState& lk = W906_GetMotorLockState();
        pg.hasLock = true;
        pg.locked = lk.locked; pg.labLockVisible = lk.labLockVisible; pg.lockText = lk.labLockCaption.c_str();
        pg.unlockCount = lk.unlockCount; pg.lastUnlockWhy = lk.lastUnlockWhy.c_str();
        ht9045::MotorLightScaleState ls = ht9045::MotorAccessLightScale(500);  // contract: the last 500 lines of Memo1
        pg.hasLightScale = true;
        pg.lsActive = ls.active; pg.lsTask = ls.task; pg.lsEditsEnabled = ls.editsEnabled; pg.lsHomePending = ls.homePending;
        pg.lsUseAxis = ls.useAxis; pg.lsAxisItem = ls.axisItem; pg.lsMoveType = ls.moveType; pg.lsNeedMovePos = ls.needMovePos;
        pg.lsMemoCount = ls.memoCount; pg.lsMemoTail.swap(ls.memoTail);
        for (int k = 0; k < 8; ++k) { pg.lsData[k] = be.GoldenLightScaleData(k + 1); pg.lsDataCounts[k] = be.GoldenLightScaleDataCount(k + 1); }
        pg.lsLastSaved = ls.lastSaved; pg.lsLastNote = ls.lastNote; pg.lsEncoderSrc = ls.encoderSrc;
        pg.loopMotors = js.loopMotors;
        pg.pageShown = js.pageShown;
    }
    ::EnterCriticalSection(&g_ovCs);
    g_ov.swap(v);
    g_page = pg;
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
static bool W906_MotorTestPage(ht9045::sjson::MotorTestPageState& out)          //AI(W906-MT-E2) 20260925: any thread, same lock
{
    if (!g_ovInit) return false;
    ::EnterCriticalSection(&g_ovCs);
    out = g_page;
    ::LeaveCriticalSection(&g_ovCs);
    return true;
}
static void W906_MotorAccessHooks()                                            // after the first copy (both hooks read it)
{
    static bool s_hooks = false;
    if (s_hooks) return;
    ht9045::sjson::SetMotorRuntimeOverlay(&W906_MotorOverlay);
    ht9045::sjson::SetMotorTestPageState(&W906_MotorTestPage);                  //AI(W906-MT-E2) 20260925
    s_hooks = true;
}

//AI(W906-MT-E3c) 20260925: the engine's hooks (csystem.h, MT-E3b) -- EastSun R8: VerifyMotorAction completely golden and
//  MainProc paused while Motor Test / Teach is open. Registered TOGETHER (MT-E3b api_for_next 2: the fShow hook alone would
//  lock the page for ever, a TMyEtherCatMotor's MotionDone() is always false here). All run on the tick thread.
static bool W906_HookFShow(const char* goldenForm)                             // golden fMotorTest / fTeach ->fShow
{
    { extern bool W906_PageFormAnswer(const char*); return W906_PageFormAnswer(goldenForm ? goldenForm : ""); }   //AI(W906-PAGETAB-Q51) 20260928 [W906] 改問頁面表（WebPageTable.cpp 規則 1～7；筆電 NIGHT_REPORT 第 36 題 A 案那一行）：瀏覽器全關 ⇒ 不再當成 Teach／Motor Test 開著，MainProc 不會無聲暫停（Steven Q-P1 20260928）；原本 ht9045::WebWindowRegistryFShowPolicy（過期＝開著）
}
static int  W906_HookMoving(int motIndex) { return ht9045::MotorAccessMovingOf(ht9045::MotorAccessLiveBackend(), motIndex); }
static bool W906_HookHoming() { return ht9045::MotorAccessHomingActive(); }
static void W906_HookAllBtnUp(const char* why) { ht9045::MotorAccessAllBtnUp(ht9045::MotorAccessLiveBackend(), why ? why : ""); }
static void W906_HookStop1203All(const char* why) { ht9045::MotorAccessStop1203All(ht9045::MotorAccessLiveBackend(), why ? why : ""); }
//AI(W906-MT-FIX1) 20260926: EastSun ruling 20260926 on G16 -- "對應軸 Servo On 才放" (csystem.h W906_BrakeServoOnHook).
//  Which axes each golden brake group holds on HT9050 (from the output names in IO_Table.csv and Mot_Table.csv; to be
//  confirmed by EastSun on the machine):
//    Index        SwFMotorBreaker / SwBMotorBreaker                 -> MTestZ1 (M14), MTestZ2 (not fitted)
//    InOutArmZ    SwInArmZBreaker / SwOutArmZBreaker                -> MInArmZ*, MOutArmZ* (M03 MInArmZA, M22 MOutArmZA)
//    Cassette     SwCassetteLD/Auto1/Auto2MotBreaker                -> MLoaderZ (M35), MAuto1Z (M38), MAuto2Z (M39)
//    Magazine     SwMagazineMotorBreaker (Enable=0 on HT9050)       -> no axis
//    LDCarRotArmZ SwLoadCarRFIDZBreaker (not in IO_Table on HT9050) -> no axis
//  Only enabled PCI1203 rows count; each must be opened by the monitor with a valid sample and its SVON bit on. A group with
//  no such row answers true (nothing of this tree drives it; golden releases).
static bool W906_BrakeGroupHasAlias(const char* group, const std::string& alias)
{
    const std::string g = group ? group : "";
    if (g == "Index")     return alias == "MTestZ1" || alias == "MTestZ2";
    if (g == "InOutArmZ") return alias.compare(0, 7, "MInArmZ") == 0 || alias.compare(0, 8, "MOutArmZ") == 0;
    if (g == "Cassette")  return alias == "MLoaderZ" || alias == "MAuto1Z" || alias == "MAuto2Z";
    return false;                                                               // Magazine / LDCarRotArmZ: no axis on HT9050
}
static bool W906_HookBrakeServoOn(const char* group, AnsiString* why)
{
    ht9045::IMotorAccessBackend& be = ht9045::MotorAccessLiveBackend();
    std::string off;
    int n = 0;
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* row = HSys.MotTable[i];
        if (!row || row->iEnable != 1) continue;
        const std::string alias = row->Alias.c_str();
        if (!W906_BrakeGroupHasAlias(group, alias)) continue;
        ht9045::MotorAccessAxis a;
        if (!be.Resolve(alias, a) || !a.Is1203()) continue;                     // not a 1203 row: not this tree's to judge
        ++n;
        bool known = false;
        const bool on = (a.axis >= 0) && be.Pci1203ServoOn(a.axis, known);
        if (!known || !on) off += (off.empty() ? "" : ", ") + alias + (a.axis < 0 ? "（監看器沒開這一軸）" : (!known ? "（讀不到狀態）" : "（沒激磁）"));
    }
    if (off.empty()) return true;
    if (why) *why = AnsiString(("servo not ON: " + off + " -- 煞車維持鎖住，Servo On 之後下一拍才放（EastSun 裁決 20260926）").c_str());
    (void)n;
    return false;
}
// Called by wb_serve once before the pump loop (tools/wb_serve.cpp, the "spine pump: ARMED" line) and again from both
//   ticks below (idempotent) -- so the hooks are in place before the first MainProc of the pump.
void W906_MotorAccessEngineHooks()
{
    static bool s_done = false;
    if (s_done) return;
    W906_Stop1203AllHook   = &W906_HookStop1203All;
    W906_MotorMovingHook   = &W906_HookMoving;
    W906_FormFShowHook     = &W906_HookFShow;
    W906_MotorHomingHook   = &W906_HookHoming;
    W906_MotorAllBtnUpHook = &W906_HookAllBtnUp;
    W906_BrakeServoOnHook  = &W906_HookBrakeServoOn;                            //AI(W906-MT-FIX1) 20260926
    s_done = true;  { extern void W906_MotorAccessPageEdges(); W906_MotorAccessPageEdges(); }   //AI(W906-WSLINK-B) 20260929: page-table close edges of fMotorTest / fTeach (EOF)
    std::printf("motor.access: engine hooks registered (Motor Test / Teach fShow, 1203 motion, HOME, AllBtnUp, Stop1203All)\n");
}

//AI(W906-MT-E3c) 20260925: the torque SDO focus (MT-E3a TPci1203Monitor::SetTorqueFocusAxis) for the Motor Test's selected
//  axis -- EastSun R6 shows its torque next to Real Speed. Refreshed on every tick while the page is shown (C++'s record of
//  golden fShow: formShow / selectMotor / formClose) and the operator is connected; it expires by itself 3 s after the last
//  call (kPci1203TorqueFocusMs), so a closed page stops the mailbox traffic. Not a vendor call.
static void W906_TorqueFocus(bool operatorConnected)
{
    ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
    if (m == 0 || !operatorConnected) return;
    const ht9045::MotorAccessJobState js = ht9045::MotorAccessJobs();
    if (!js.pageShown || js.selectedMotor.empty()) return;
    ht9045::MotorAccessAxis a;
    if (!ht9045::MotorAccessLiveBackend().Resolve(js.selectedMotor, a) || !a.Is1203() || a.axis < 0) return;
    m->SetTorqueFocusAxis(a.axis);
}

static bool g_opConnected = true;                                               // the last beat's WebBridgeServer::ControlOwner()!=0

void W906_MotorAccessTick(bool operatorConnected)
{
    W906_MotorAccessEngineHooks();                                              //AI(W906-MT-E3c) 20260925: idempotent (wb_serve registers them before the loop)
    g_opConnected = operatorConnected;
    OverlaySnapshot();                                                          // NB2 R23 §2：先抄樣本，再（第一次）註冊掛鉤
    W906_MotorAccessHooks();                                                    //AI(W906-MT-E2) 20260925: was the overlay hook alone, registered here
    static DWORD s_last = 0;
    const DWORD now = ::GetTickCount();
    const int elapsed = (s_last == 0) ? 0 : (int)(now - s_last);
    s_last = now;
    if (elapsed > 0) ht9045::MotorAccessTick(ht9045::MotorAccessLiveBackend(), elapsed, operatorConnected);
    W906_TorqueFocus(operatorConnected);                                        //AI(W906-MT-E3c) 20260925
}

//AI(W906-MT-E2) 20260925: wb_serve calls this right after every Pci1203Monitor()->Poll() (tools/wb_serve.cpp, the Poll line of
//  the IO clock, same thread, about every 200 ms). Order: first the loop step on the fresh sample -- its arrival time is
//  taken as close to the Poll as this thread gets (QueryPerformanceCounter, LiveBackend::MonotonicMs) -- then the copy for
//  /api/struct/motor/runtime, so the copy already holds the new leg time / count and this Poll's motor IO lamps (they used to
//  wait for the 500 ms beat). The beat (W906_MotorAccessTick) is unchanged.
void W906_MotorAccessPollTick(bool operatorConnected)                           //AI(W906-MT-FIX1) 20260926: live ControlOwner()!=0 from wb_serve
{
    W906_MotorAccessEngineHooks();                                              //AI(W906-MT-E3c) 20260925: idempotent
    ht9045::MotorAccessPollTick(ht9045::MotorAccessLiveBackend(), operatorConnected);
    OverlaySnapshot();
    W906_MotorAccessHooks();
    W906_TorqueFocus(operatorConnected);                                        //AI(W906-MT-E3c) 20260925: every Poll while the page shows the axis
}

//AI(W906-WSLINK-B) 20260929: St01 13:32 safety point (b): the page-table close edge of fMotorTest / fTeach
//  -> the C++ half of that window's golden FormClose (ht9045::MotorAccessPageClosed, rules in WebMotorAccess.cpp EOF; the two
//  teach-pitch globals are cleared here). Registered once, from W906_MotorAccessEngineHooks (wb_serve
//  calls it before the pump loop). skipWhileRunning=false: a close edge always stops (the safe direction). motor.access refuses
//  new motion while SystemStart (WebMotorAccess.cpp dispatch gate, only STOP passes), so while running this can only end what
//  was already going before START. onOpen is 0 (an opening page starts nothing).
#include "WebTeachLeave.h"
static void W906_MtPageClosed()    { ht9045::MotorAccessPageClosed(ht9045::MotorAccessLiveBackend(), "fMotorTest"); }
static void W906_TeachPageClosed() { bInArmXPitch_40mm = false; bOutArmXPitch_40mm = false;   // golden uteach.cpp:2089-2090 (TfTeach::FormClose)
                                     ht9045::MotorAccessPageClosed(ht9045::MotorAccessLiveBackend(), "fTeach"); }
void W906_MotorAccessPageEdges()
{
    const bool mt = W906_WindowEdgeRegister("fMotorTest", 0, &W906_MtPageClosed, false);
    const bool te = W906_WindowEdgeRegister("fTeach", 0, &W906_TeachPageClosed, false);
    std::printf("motor.access: page-close edges registered (fMotorTest %s, fTeach %s) -- WSLINK-B dead-man\n",
                mt ? "ok" : "REFUSED", te ? "ok" : "REFUSED");
}

//AI(W906-ENG1203) 20260929: review HIGH-1 (INBOX 112) -- the wb_serve glue for a stop that reached
//  TPci1203Control::Execute WITHOUT the engine motor route (declared in EtherCAT/Pci1203MotorRoute.h).
//  Called right after LiveBackend::Pci1203Execute's Execute (every WebMotorAccess 1203 command) and after the pci1203
//  page dispatcher's (tools/wb_serve.cpp W906_Dispatch1203Ex). Two halves, both no-ops unless the engine route is installed:
//    1. the route's ledger (Pci1203MotorRouteNoteForeignStop: home job cancelled, axis pending past the next full Poll);
//    2. golden PCIL132_StopMotor's bookkeeping on the engine rows of that (station, axis): fCMD=false
//       (W906_EcForeignStopResetFcmd, Motor/EcatMotorRoute.cpp) -- so a move the alarm path stopped short is
//       re-issued after RETRY instead of reading as arrived (golden MotorMovePosition, mymotor.cpp:5743-5745).
//  Same thread as every other Execute (the tick thread); prints, never raises (Q10).
void W906_EngineRouteForeignStop(const ht9045::Pci1203Cmd& c, const ht9045::Pci1203CmdResult& r)
{
    int station = -1, stationAxis = -1;
    if (!ht9045::Pci1203MotorRouteNoteForeignStop(c, r, &station, &stationAxis)) return;
    const int rows = W906_EcForeignStopResetFcmd(station, stationAxis);
    std::printf("engine motor -> 1203: foreign stop on st %d ax %d -> golden fCMD=false on %d engine row(s) (PCIL132_StopMotor's bookkeeping)\n",
                station, stationAxis, rows);
}
