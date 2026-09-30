// =============================================================================
//  JsonBridge/IoBtnPanelClick.cpp -- HW.IoSetView's output buttons.
//
//  AI(W906-IOWEB-P17) 20260925: new file, wb_serve only. User 20260925:
//  "輸出請幫我接上" / "請幫我接上1203".
//
//  WIRE:  {cmd:"io.btnPanelClick", tag:<Alias>, value:<Down after the click, 0|1>}
//  ACK :  ok:true  + {alias, down, raw, address{...}, accepted, issued, dryRun,
//                     ret, why, exCall, sideEffects[]}     -- the write went through
//                     MyLaneIO and reached TPci1203Control::Execute (DRY or LIVE)
//         ok:false + a plain-text reason                    -- nothing was written
//
//  THIS IS golden Tfiosetview::BtnPanelClick (iosetview.cpp:1030-1206), TBtnPanelLane
//  arm, ISABase==ePCI1203 branch, with the VCL removed:
//
//    golden                                          here
//    ------------------------------------------      ---------------------------------
//    Ptr->Down=!Ptr->Down;                 :1087     the page sends the new Down (value)
//    (Down&&OutType==1)||(!Down&&OutType==0)         same test, same operands
//      ? MyLaneIO.IOBitOn(OutRing,OutIP,   :1091-1099  MyLaneIO.IOBitOn/IOBitOff with the
//        OutPort,OutBit,ISABase,Alias)                 row's Lane/IP/Port/Bit/ISABase/Alias
//      : MyLaneIO.IOBitOff(...)                        -- so IdleCheckSafeDoorByCylinder and
//                                                      the OutPortData cache run as in golden
//                                                      (for what the door check does NOT
//                                                      cover here, see DEVIATION c)
//    fPadInterface->SendSwitchStatus(Ptr)  :1095     NOT PORTED (DEVIATION a)
//    repaint from Down                     :1120     the page paints from the card read-back
//    SwFMotorBreaker/SwBMotorBreaker/      :1155-1205 same four address matches, same
//      SwMotorRelay/SwServerON side effects           fAllMotorHome / HomeFlag resets
//    TestSuck + IndexHasIC() -> Suck()     :1040     refused (DEVIATION b)
//
//  and the checks golden made OUTSIDE the click, because a modal VCL form made
//  them sufficient there and a browser page does not:
//    * `if(SystemStart) return;` before the form can open    (main.cpp:27946)
//    * a button is Enabled only for an enabled row             (iosetview.cpp:2130/:2210)
//    * IndexHasIC() disables 10 buttons: tempBtn[8] + the two EP switches
//      (iosetview.cpp:253-255, :404-411); EP_Install==0 disables the EP switches
//      even without an IC (:432-436)
//    * the Index Z1/Z2 brakes (SwFMotorBreaker / SwBMotorBreaker = "Free"):
//      golden never lets them stay released on an axis that has lost power --
//      FormShow forces both Off (:306-307), Timer1Timer runs
//      LockIndexMotorAndDoHomeProcess() when IsIndexMotorOutOfPower() (:108-113),
//      and DoSystem runs IndexMotorBreakerOFF() every tick in the same state
//      (golden csystem.cpp:4279-4294, gated `#if 0 // GATE G04` in this port).
//      Here: releasing one is REFUSED while IsIndexMotorOutOfPower(), with
//      golden's own exception (SnFMotorDown / SnBMotorDown pressed and
//      bD48PowerOffEmgCanNotUseZ1Z2 off; golden csystem.cpp:1084/:1094).
//      ⚠ On this machine the motor layer is not armed yet (uiDevhand==0, so
//      MOT[].Led[iServoOn] never reads true) and IsIndexMotorOutOfPower() is
//      therefore always true: the web cannot release those two brakes at all.
//      That is the safe direction for a vertical axis and it is deliberate.
//
//  DEVIATIONS (each deliberate, each recorded here so the next reader does not
//  "restore" golden by accident):
//    (a) SendSwitchStatus: fPadInterface is not ported in this tree.
//    (b) TestSuck with an IC on the Index is golden's Suck()/Destroy() sequence
//        run by Timer1Timer; not ported, so it is refused rather than turned into
//        a raw write that golden never makes in that state.
//    (c) The row is found through HSys.mapIOTable, not through SW[] -> sucker ->
//        Cylinder[] names (iosetview.cpp:2116-2226). Same address either way for
//        a row that HAS an engine object -- those objects are bound from
//        mapIOTable by the same name.
//        ⚠ BUT 54 of this machine's 92 enabled 1203 outputs have NO engine object
//        in either tree (review F1, 20260925): the HT9050's second coils
//        (C_Load_UpOff, C_TrayZ_SelectorOff, ...), C_SafeDoorIndexLock,
//        C_InPnPDrop1-4, the *EdgeClip / *DrawerLock family, and a few Sw* rows.
//        golden 906 has no BUTTONS for them at all (its iosetview.dfm carries no
//        such Alias) -- they come from EastSun's HT9050 layout. They are the "<-"
//        / "v" halves of the Loader panel the user asked to drive, so they are
//        written here rather than refused.
//        ⚠⚠ THE IDLE SAFE-DOOR INTERLOCK DOES NOT COVER THEM. IdleCheckSafeDoor-
//        ByCylinder only gates addresses that the Cylinder[] loop marked in
//        bIdleNeedCheckSafeDoor (cinitial.cpp:5434), and nothing marks these.
//        (The bound HT9050 cylinders that ARE marked-eligible sit on golden's own
//        bypass list, cinitial.cpp:5299-5304, so in practice no HT9050 1203
//        output is door-gated in golden either -- and the door sensors
//        themselves are not on the 1203 table yet, WEEKEND_PLAN_20260925.md:278.)
//        Recorded in MachineType.h's pre-LIVE notes; it is an operating fact the
//        person at the machine must know, not something this file can fix.
//    (d) Down comes from the page (the card's DO read-back, drawn as the fill),
//        not from IOOutBitStatus. golden seeds Down from the OutPortData cache
//        (iosetview.cpp:2233), and that cache aliases 14 DO pairs for 1203 rows
//        (see EtherCAT/Pci1203IoRoute.h) -- it would show the wrong state.
//    (e) NO ACCESS-LEVEL CHECK, anywhere on this path (review F3). golden checks
//        fSecurity->Insufficient(4) at form entry (main.cpp:27949) and disables
//        whole tabs by LevelSet.AccessLevel[58..66] (iosetview.cpp:~764). Neither
//        exists for the web: with GATE (SEC1) closed Insufficient() returns false
//        unconditionally (forms/fSecurity.h "EMERGENT BEHAVIOUR") and would refuse
//        every click, and the password gates P-R1/P-S1 are closed (CLAUDE.md), so
//        there is no working level to check against. What does limit it today:
//        the single-operator token, and wb_serve binding 127.0.0.1 only
//        (bindAddress default), so only a browser ON THIS PC can reach it.
//    (g) Every Alias-bearing button is treated as a BtnPanelClick button. golden
//        wires a few components to other handlers (btnC_*_UpClick when the Z is a
//        motor -- refused below; BtnNumPanelTestClick, btnAllVacuumClick, ...).
//        Latent on this machine's table; a table that enables such a row would
//        get a DO write where golden does something else.
//    (i) golden's leave-the-form bookkeeping is not ported (review RG-4): every click
//        sets bOutDataChange (iosetview.cpp:1036), sbIOClick backs the outputs up
//        before ShowModal (MyLaneIO.BackUpOutputData, main.cpp:27956) and, on close,
//        asks 「IO已經改變,是否回復測貨狀態?」 and restores them (main.cpp:27987-27993).
//        The web IO window never "closes" (the iframe is only hidden), so there is no
//        moment to ask. Outputs changed from the web STAY changed. (golden's restore
//        could not have restored a 1203 point anyway: RestoreOutputData walks rings
//        0-1, IP<64, port<4 with the MotionNet default ISABase, MyLaneIo.cpp:520-535.)
//    (h) Sucker_On / Sucker_Off rows use their OWN row's Enable. golden uses the   //AI(W906-INBOX132) 20261001: (h) corrected
//        sucker SENSOR row's Enable (pTempSuck->Enable, iosetview.cpp:2159). The binder
//        does fill pSuck (cinitial.cpp:493-654, the IO-table half, live since 0924; only the
//        BDE else-half :656-877 is #if 0) -- this page just matches rows by alias. HT9050:
//        every sucker row is Enable=0 (machines/HT9050, 0925), so none can be clicked.
//    (f) A click whose address cannot be written (no card byte, byte unreadable,
//        station is a drive, control not armed) is refused BEFORE IOBitOn, and so
//        before golden's side effects. golden would run IOBitOn into a failure it
//        only logs, then run the side effects anyway.
// =============================================================================
#include "MachineType.h"          // ePCI1203, CC_ASE_KaohSiung, SOFT_SIMULTE -- first (tools/macro_order_gate.ps1)
#include "vclcompat/vcl_compat.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include "cmydef.h"              // SystemStart, fAllMotorHome, SwFMotorBreaker.., MTestZ1/2, TOTAL_MOTOR, CUSTOMER_CODE
#include "csystem.h"              // IndexHasIC(), IsIndexMotorOutOfPower()
#include "Config.h"               // IniConfig.bD48PowerOffEmgCanNotUseZ1Z2
#include "mysensor.h"             // Sen[] (SnFMotorDown / SnBMotorDown)
#include "database.h"             // HSys.mapIOTable / HSys.IOTable / TIODATA
#include "MyLaneIo.h"             // MyLaneIO
#include "myswitch.h"             // SW[] / MAX_SWITCH_ITEM
#include "mycylin.h"              // Cylinder[] / MaxCylinderItem
#include "Motor/mymotor.h"        // MOT[]
#include "canary_support.h"       // ShowMyMessage
#include "JsonBridge/ChanIo.h"    // IoDirectionOfType
#include "EtherCAT/Pci1203IoRoute.h"
#include "WebBridge/JsonWriter.h"

namespace {

bool SameAddress(int sw, const TIODATA* r)
{
    return SW[sw].Enable &&
           SW[sw].Ring == r->iLane && SW[sw].IP == r->iIP &&
           SW[sw].Port == r->iPort && SW[sw].Bit == r->iBit;
}

std::string Str(const AnsiString& s) { return std::string(s.c_str()); }

// ---------------------------------------------------------------------------
//  AI(W906-IOWEB-P21) 20260925: CLICK-LATENCY LOG. User: "我點擊按鈕 有時候需要等上
//  2秒線圈才會有反應 這是json 的問題 還是1203的問題?" / "你寫個LOG 給我測測看不就知道了?"
//
//  A click travels: browser -> socket thread (CommandQueue::tryPush stamps
//  WebCommand::pushedUs) -> waits in the queue until the ONE tick thread drains it
//  -> MyLaneIO.IOBitOn -> route -> Execute -> the card. The tick thread also runs
//  PumpTick (500 ms beat), the 1203 Poll (200 ms clock), the /api cache rebuild and
//  the tag publish (AI(W906-LAT-1) 20260925), and while it does any of those the
//  click waits -- except where the phase lends the thread to outputs (Poll, cache,
//  publish: W906_ServiceOutputs in tools/wb_serve.cpp). wb_serve marks each of those
//  phases (W906_IoTiming below, same-line calls in tools/wb_serve.cpp), so the log
//  can say WHICH one the click was waiting behind -- not just that it waited.
//  All of it runs on the tick thread: no lock.
// ---------------------------------------------------------------------------
std::uint64_t NowUs_()
{
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct Phase { int kind; std::uint64_t t0, t1; };
const int kPhaseRing = 256;
Phase         s_phase[kPhaseRing];
int           s_phaseHead = 0;
//AI(W906-LAT-1) 20260925: + phase 4 = PublishHandlerTags (wb_serve marks it 7/8). The 14:19 log's
//  "其他" (up to 43.6 ms) was every tick-thread moment no phase claimed, and the publish -- 7,084
//  tags staged after every Poll, beat, drained batch and served output -- was the unmarked suspect
//  (outputlatency report, step 1). Marking it turns that guess into a column.
const int kPhaseKinds = 5;   // [0] unused, 1..4
std::uint64_t s_phaseOpen[kPhaseKinds] = { 0, 0, 0, 0, 0 };
const char* const kPhaseName[kPhaseKinds] = { "", "跑流程(PumpTick)", "讀卡(1203 Poll)", "畫面資料(api cache)", "發布(publish)" };

// Microseconds the tick thread spent in phase `kind` inside [from, to].
std::uint64_t PhaseOverlapUs(int kind, std::uint64_t from, std::uint64_t to)
{
    std::uint64_t sum = 0;
    for (int i = 0; i < kPhaseRing; ++i) {
        const Phase& p = s_phase[i];
        if (p.kind != kind || p.t1 <= from || p.t0 >= to) continue;
        const std::uint64_t a = p.t0 > from ? p.t0 : from;
        const std::uint64_t b = p.t1 < to ? p.t1 : to;
        if (b > a) sum += b - a;
    }
    return sum;
}

}  // namespace

//  what: 1/2 = PumpTick begin/end, 3/4 = 1203 Poll begin/end, 5/6 = api cache begin/end,
//        7/8 = PublishHandlerTags begin/end (AI(W906-LAT-1) 20260925).
void W906_IoTiming(int what)
{
    const std::uint64_t now = NowUs_();
    const int kind = (what + 1) / 2;
    if (kind < 1 || kind >= kPhaseKinds) return;
    if (what % 2) { s_phaseOpen[kind] = now; return; }
    if (s_phaseOpen[kind] == 0) return;
    Phase& p = s_phase[s_phaseHead];
    p.kind = kind; p.t0 = s_phaseOpen[kind]; p.t1 = now;
    s_phaseHead = (s_phaseHead + 1) % kPhaseRing;
    s_phaseOpen[kind] = 0;
}
#define W906_IO_PAGE_NO_GUARDS 1   //AI(W906-IO-NOGUARD) 20260929: EastSun「IO畫面一律不要卡控，讓我測試」-- 1 = every golden rule below that greys a button (running, SoftStart, IO_Table Enable=0, engine object off, Index-has-IC, EP_Install, Z-motor lanes, Index brake without power) is SKIPPED and listed in the reply's guardsBypassed; only the physical limits stay (not a 1203 point, an input, no route). 0 = golden's guards again. Occupies a blank line, no line moves
static bool Click_(const std::string& alias, int desiredDown, std::string& out, std::uint64_t* execUs)
{   std::vector<std::string> bypassed;   //AI(W906-IO-NOGUARD) 20260929
    using namespace ht9045;

    if (alias.empty()) { out = "io.btnPanelClick：tag（Alias）是空的"; return false; }
    if (desiredDown != 0 && desiredDown != 1) {
        out = "io.btnPanelClick：value 必須是 0 或 1（按下之後按鈕的 Down 狀態）";
        return false;
    }
    if (SystemStart && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("運轉中 SystemStart");  else if (SystemStart) {   // golden main.cpp:27946 -- the IO form cannot even be opened while running   //AI(W906-IO-NOGUARD) 20260929
        out = "機台運轉中（SystemStart）不能手動切輸出 —— golden 運轉中打不開 IO 畫面（main.cpp:27946）";
        return false;
    }
    //  review S2: a START or HOME that has been ARMED but not yet taken (SoftStart is
    //  consumed by ScanSystemSensor on the next pump tick, ckernel.cpp; Home() sets it
    //  too, fMain.cpp:976) is not SystemStart yet. golden never meets this state from
    //  the IO form: MainProc returns while the form is shown (golden csystem.cpp:16887-
    //  16888 `if(fiosetview->fShow) return;`), so nothing can arm a start behind it. The
    //  web IO page is always "open", so the window has to be refused here instead.
    //  ⚠ NOT iHome (fixed 20260925 after the first LIVE click on the machine did nothing):
    //    iHome is golden's "a home is REQUIRED" flag, `int iHome =1;` at cmydef.cpp:322,
    //    and it stays 1 on an idle machine that has not homed -- testing it refused every
    //    single click. SoftStart covers a home that has been pressed and not yet taken.
    if (SoftStart && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("正要啟動／回原點 SoftStart");  else if (SoftStart) {   //AI(W906-IO-NOGUARD) 20260929
        out = "機台正要啟動或回原點（SoftStart）——這時不能手動切輸出；golden 在 IO 畫面開著時根本不會進入這個狀態";
        return false;
    }

    HSys.mapIOTableIter = HSys.mapIOTable.find(AnsiString(alias.c_str()));
    if (HSys.mapIOTableIter == HSys.mapIOTable.end()) {
        out = "IO_Table 沒有 Alias「" + alias + "」";
        return false;
    }
    const int idx = std::atoi(HSys.mapIOTableIter->second.c_str());
    if (idx < 0 || idx >= (int)HSys.IOTable.size() || HSys.IOTable[idx] == 0) {
        out = "IO_Table 的索引壞了（Alias「" + alias + "」）";
        return false;
    }
    const TIODATA* r = HSys.IOTable[idx];
    if (r->iISABase != ePCI1203) {
        char b[160];
        std::snprintf(b, sizeof(b), "「%s」不是 1203 的點（ISABase=%d），這個按鈕只接 PCIE-1203", alias.c_str(), r->iISABase);
        out = b;
        return false;
    }
    if (sjson::IoDirectionOfType(Str(r->Type)) != sjson::kIoDirOut) {
        out = "「" + alias + "」是輸入（" + Str(r->Type) + "），不能寫";
        return false;
    }
    if (r->iEnable != 1 && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("IO_Table Enable=0");  else if (r->iEnable != 1) {   // golden: the button's Enabled comes from the object's Enable (iosetview.cpp:2130/:2210)   //AI(W906-IO-NOGUARD) 20260929
        out = "「" + alias + "」在 IO_Table 是 Enable=0";
        return false;
    }
    //  golden SetCompomentIO resolves SW[].Name, then the suckers, then Cylinder[].CylinderName
    //  (iosetview.cpp:2116-2226), and the button's Enabled is THAT OBJECT's Enable -- which the
    //  engine may have switched off AFTER binding the row (e.g. cinitial.cpp:5242/:5248 for
    //  AUTO_EMPTY_COLOR=0, :5286-5287 for USE_AUTO_RETEST=0: C_Auto1_Up is Enable=1 in the table
    //  but its Cylinder[] is off on this machine). Same order here; a row with no engine object
    //  (the HT9050-only outputs, DEVIATION c) falls back to the row's own Enable above.
    {
        const AnsiString a(alias.c_str());
        bool found = false, enabled = false;
        for (int i = 0; i < MAX_SWITCH_ITEM && !found; ++i)
            if (SW[i].Name == a) { found = true; enabled = SW[i].Enable; }
        for (int i = 0; i < MaxCylinderItem && !found; ++i)
            if (Cylinder[i].CylinderName == a) { found = true; enabled = Cylinder[i].Enable; }
        if (found && !enabled && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("引擎物件被設定停用（USE_AUTO_RETEST／AUTO_EMPTY_COLOR 等）");  else if (found && !enabled) {   //AI(W906-IO-NOGUARD) 20260929: e.g. C_Auto2_Up with USE_AUTO_RETEST=0 (cinitial.cpp:5296-5304)
            out = "「" + alias + "」的引擎物件被設定停用（例如 USE_AUTO_RETEST／AUTO_EMPTY_COLOR 沒開），golden 這顆按鈕是灰的";
            return false;
        }
    }
    const bool indexHasIC = IndexHasIC();
    if (indexHasIC && alias.find("TestSuck") != std::string::npos && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("Index 上有 IC（TestSuck 走 Suck()/Destroy()）");  else if (indexHasIC && alias.find("TestSuck") != std::string::npos) {   // golden :1040, DEVIATION (b)   //AI(W906-IO-NOGUARD) 20260929
        out = "Index 上有 IC：golden 對 TestSuck 走 Suck()／Destroy() 流程（iosetview.cpp:1040-1083），這條還沒移植，所以不送";
        return false;
    }
    if (indexHasIC) {   // golden iosetview.cpp:253-255 (tempBtn[8]) + :404-411 (+ bplEpSwitch1/2)
        static const char* const kLocked[] = {
            "SwBMotorBreaker", "SwFMotorBreaker", "SwMotorRelay", "SwServerON",
            "SwZ1SuckMode0", "SwZ1SuckMode1", "SwZ2SuckMode0", "SwZ2SuckMode1",
            "SwEpArm1", "SwEpArm2" };   // bplEpSwitch1/2 (iosetview.dfm Alias)
        for (std::size_t i = 0; i < sizeof(kLocked) / sizeof(kLocked[0]); ++i)
            if (alias == kLocked[i] && W906_IO_PAGE_NO_GUARDS) { bypassed.push_back("Index 上有 IC 時這顆鈕停用"); break; }  else if (alias == kLocked[i]) {   //AI(W906-IO-NOGUARD) 20260929
                out = "Index 上有 IC：golden 會把「" + alias + "」這顆按鈕停用（iosetview.cpp:404-411）";
                return false;
            }
    }
    if (EP_Install == 0 && (alias == "SwEpArm1" || alias == "SwEpArm2") && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("EP_Install=0");  else if (EP_Install == 0 && (alias == "SwEpArm1" || alias == "SwEpArm2")) {   // golden :432-436   //AI(W906-IO-NOGUARD) 20260929
        out = "EP_Install=0：golden 會把「" + alias + "」這顆按鈕停用（iosetview.cpp:432-436）";
        return false;
    }

    //  golden iosetview.cpp:830-975 wires these cylinder buttons to BtnPanelClick ONLY when the
    //  lane's Z is not a motor; otherwise the dfm's own OnClick stays -- btnC_Load_UpClick and
    //  its siblings (:3335-3379), a MOT[...] move, not a DO write. Not ported, so refused.
    //  This machine: Gerneral.ini LOAD_Z_USE_MOTOR=0, AUTO_EMPTY_COLOR=0 -> none of this fires.
    {
        struct ZLane { const char* a; const char* b; int k; bool needsAutoEmptyColor; };
        static const ZLane kZ[] = {
            { "C_Load_Middle",    "C_Load_Up",    0, false },   // :830
            { "C_Empty_Middle",   "C_Empty_Up",   1, true  },   // :847  AUTO_EMPTY_COLOR==0 || ...
            { "C_Color_Middle",   "C_Color_Up",   2, true  },   // :864
            { "C_Auto1_Selector", "C_Auto1_Up",   3, false },   // :881
            { "C_Auto2_Selector", "C_Auto2_Up",   4, false },   // :898
            { "C_Auto3_Selector", "C_Auto3_Up",   5, false },   // :915
            { "C_Auto4_Selector", "C_Auto4_Up",   6, false },   // :936
            { "C_Auto5_Selector", "C_Auto5_Up",   7, false },   // :953
            { "C_Auto6_Selector", "C_Auto6_Up",   8, false } }; // :970
        for (std::size_t i = 0; i < sizeof(kZ) / sizeof(kZ[0]); ++i) {
            if (alias != kZ[i].a && alias != kZ[i].b) continue;
            const bool motorClick = LOAD_Z_USE_MOTOR[kZ[i].k] && (!kZ[i].needsAutoEmptyColor || AUTO_EMPTY_COLOR != 0);
            if (motorClick && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("LOAD_Z_USE_MOTOR：golden 按下去是 Z 馬達移動，這裡照寫 DO");  else if (motorClick) {   //AI(W906-IO-NOGUARD) 20260929
                out = "「" + alias + "」在 LOAD_Z_USE_MOTOR 開著時，golden 按下去是 Z 馬達移動（iosetview.cpp:3335-3379），不是寫 DO；這條還沒移植，所以不送";
                return false;
            }
        }
    }

    // golden OutType <- the row's InType: golden cinitial.cpp:1419 `SW[i].Type =HSys.IOTable[iSw]->iInType;`
    // and :4528 `Cylinder[i].OutType =HSys.IOTable[iCy]->iInType;` (port: cinitial.cpp:1550 and the InitCylinder loop).
    const int outType = r->iInType;
    const bool on = (desiredDown == 1 && outType == 1) || (desiredDown == 0 && outType == 0);

    //  The Index Z1/Z2 brakes (header: "the checks golden made OUTSIDE the click").
    //  golden IndexMotorBreakerOFF (csystem.cpp:1082-1102) is what DoSystem / Timer1Timer
    //  run while IsIndexMotorOutOfPower(): the brake is released only while the manual
    //  "down" key is held and bD48PowerOffEmgCanNotUseZ1Z2 is off. A web click must not
    //  release it in any other case -- a vertical axis with no servo falls.
    //  ⚠ "release" is the ON coil; OFF (engage the brake) is never refused here.
    if (on && (alias == "SwFMotorBreaker" || alias == "SwBMotorBreaker") && IsIndexMotorOutOfPower()) {
        const bool front = (alias == "SwFMotorBreaker");
        const bool manualDown = Sen[front ? SnFMotorDown : SnBMotorDown].IsOn() &&
                                IniConfig.bD48PowerOffEmgCanNotUseZ1Z2 == false;
        if (!manualDown && W906_IO_PAGE_NO_GUARDS) bypassed.push_back("⚠ Index 剎車釋放：Index 馬達沒有動力（golden 會鎖住剎車，垂直軸可能下滑）");  else if (!manualDown) {   //AI(W906-IO-NOGUARD) 20260929
            out = std::string("「") + alias + "」是 Index " + (front ? "Z1" : "Z2") +
                  " 的剎車釋放：Index 馬達目前沒有動力（EMG／系統斷電／伺服沒開，IsIndexMotorOutOfPower），"
                  "golden 在這個狀態會強制鎖住剎車（IndexMotorBreakerOFF，csystem.cpp:1082-1102），所以不送。"
                  "要手動降軸請按住機台上的下降鍵（" + (front ? "SnFMotorDown" : "SnBMotorDown") + "）";
            return false;
        }
    }

    int slot = -1, stationChan = -1;
    std::string why;
    if (!Pci1203RouteInstalled()) {
        out = "引擎 IO 沒有接到 1203（WB_ENGINE_IO_1203 沒開，或這是 SIM 組態）——沒有送出";
        return false;
    }
    if (!Pci1203RouteCanWriteBit(r->iLane, r->iIP, r->iPort, &slot, &stationChan, why)) {   // DEVIATION (f)
        out = "「" + alias + "」" + why;
        return false;
    }

    const unsigned long before = Pci1203RouteLastWrite().seq;
    Pci1203RouteSetSource("web");
    const std::uint64_t tExec0 = NowUs_();
    if (on) MyLaneIO.IOBitOn (r->iLane, r->iIP, r->iPort, r->iBit, r->iISABase, r->Alias);   // golden :1094
    else    MyLaneIO.IOBitOff(r->iLane, r->iIP, r->iPort, r->iBit, r->iISABase, r->Alias);   // golden :1099
    if (execUs) *execUs = NowUs_() - tExec0;   // IOBitOn -> route -> Execute -> Acm_DaqDoSetBitEx (+ the one-byte read-back)
    Pci1203RouteSetSource("engine");
    // golden :1095/:1100 fPadInterface->SendSwitchStatus(Ptr) -- DEVIATION (a)

    // golden :1155-1205 -- by ADDRESS, on ON and OFF alike, whatever the write returned.
    std::vector<std::string> side;
    const bool fBreaker = SameAddress(SwFMotorBreaker, r);
    const bool bBreaker = SameAddress(SwBMotorBreaker, r);
    const bool relay    = SameAddress(SwMotorRelay, r);
    const bool servoOn  = SameAddress(SwServerON, r);
    if (fBreaker) {
        fAllMotorHome = false;
        MOT[MTestZ1].HomeFlag = 0;
        side.push_back("SwFMotorBreaker: fAllMotorHome=false, MOT[MTestZ1].HomeFlag=0");
    }
    if (bBreaker) {
        fAllMotorHome = false;
        MOT[MTestZ2].HomeFlag = 0;
        side.push_back("SwBMotorBreaker: fAllMotorHome=false, MOT[MTestZ2].HomeFlag=0");
    }
    if (relay || servoOn) {
        fAllMotorHome = false;
        for (int i = 0; i < TOTAL_MOTOR; i++)
            MOT[i].HomeFlag = 0;
        side.push_back(relay ? "SwMotorRelay: fAllMotorHome=false, 所有 MOT[].HomeFlag=0（要重新回原點）"
                             : "SwServerON: fAllMotorHome=false, 所有 MOT[].HomeFlag=0（要重新回原點）");
    }
    if ((fBreaker || bBreaker || relay || servoOn) && CUSTOMER_CODE == CC_ASE_KaohSiung)
        ShowMyMessage("SwFMotorBreaker");   // golden :1164/:1176/:1190/:1204 -- the same text for all four

    const Pci1203RouteWrite& w = Pci1203RouteLastWrite();
    if (w.seq == before) {
        //  MyLaneIO returned before its backend call. For a 1203 point that is
        //  IdleCheckSafeDoorByCylinder (MyLaneIo.cpp:261/:326) -- golden returns
        //  silently there; the web gets the reason instead of nothing.
        out = "「" + alias + "」沒有送出：安全門互鎖擋下（IdleCheckSafeDoorByCylinder，門沒關好時閒置中不能動氣缸）";
        std::printf("io.btnPanelClick %s down=%d -> BLOCKED by IdleCheckSafeDoorByCylinder\n", alias.c_str(), desiredDown);
        return false;
    }

    std::printf("io.btnPanelClick %s down=%d raw=%d -> %s  %s%s%s\n",
                alias.c_str(), desiredDown, on ? 1 : 0,
                !w.accepted ? "REFUSED" : w.issued ? (w.ret == 0 ? "ISSUED OK" : "ISSUED, VENDOR ERROR") : "DRY RUN",
                w.exCall.c_str(), w.why.empty() ? "" : "  -- ", w.why.c_str());

    if (!w.accepted) {
        out = "「" + alias + "」被 1203 命令面拒絕：" + w.why;
        return false;
    }

    webbridge::JsonWriter j;
    j.BeginObject();
    j.Key("alias").String(alias);
    j.Key("down").Number((wb_int64)desiredDown);
    j.Key("raw").Number((wb_int64)(on ? 1 : 0));  j.Key("guardsBypassed").BeginArray(); for (std::size_t bi = 0; bi < bypassed.size(); ++bi) j.String(bypassed[bi]); j.EndArray();   //AI(W906-IO-NOGUARD) 20260929: which golden rules this click skipped (empty = none applied)
    j.Key("outType").Number((wb_int64)outType);
    j.Key("address").BeginObject();
    j.Key("lane").Number((wb_int64)r->iLane);
    j.Key("ip").Number((wb_int64)r->iIP);
    j.Key("port").Number((wb_int64)r->iPort);
    j.Key("bit").Number((wb_int64)r->iBit);
    j.Key("ring").Number((wb_int64)w.ring);
    j.Key("station").Number((wb_int64)w.station);
    j.Key("stationChan").Number((wb_int64)w.stationChan);
    j.Key("slot").Number((wb_int64)w.slot);
    j.EndObject();
    j.Key("accepted").Bool(w.accepted);
    j.Key("issued").Bool(w.issued);
    j.Key("dryRun").Bool(w.dryRun);
    j.Key("ret").Number((wb_int64)w.ret);
    j.Key("vendorOk").Bool(w.issued && w.ret == 0);
    if (!w.why.empty()) j.Key("why").String(w.why);
    j.Key("exCall").String(w.exCall);
    j.Key("sideEffects").BeginArray();
    for (std::size_t i = 0; i < side.size(); ++i) j.String(side[i]);
    j.EndArray();
    j.Key("golden").String("iosetview.cpp:1030-1206 BtnPanelClick");
    j.EndObject();
    out = j.Ok() ? j.Str() : std::string("{\"error\":\"json writer misuse\"}");
    return true;
}

// ---------------------------------------------------------------------------
//  The entry point wb_serve calls (tools/wb_serve.cpp, io.btnPanelClick branch).
//  AI(W906-IOWEB-P21) 20260925: wraps Click_ with the latency log described above.
//  `pushedUs` = WebCommand::pushedUs (0 when unknown).
//  Where it goes: the console line, the ack ("timing" object on success, a
//  trailing sentence on a refusal), and -- when the env var W906_IOTIMING_LOG
//  names a file (IOWEB F5 sets it to ..\runcfg\logs\io_click_timing.csv) -- one
//  CSV row per click. Unset = no file.
// ---------------------------------------------------------------------------
bool W906_IoBtnPanelClick(const std::string& alias, int desiredDown, std::string& out,
                          unsigned long long pushedUs)
{
    const std::uint64_t tDispatch = NowUs_();
    std::uint64_t execUs = 0;
    const bool ok = Click_(alias, desiredDown, out, &execUs);
    const std::uint64_t tDone = NowUs_();

    const double queueMs = (pushedUs && tDispatch >= pushedUs) ? (tDispatch - pushedUs) / 1000.0 : -1.0;
    double busyMs[kPhaseKinds] = { 0, 0, 0, 0, 0 };
    double busySum = 0;
    if (queueMs >= 0) {
        for (int k = 1; k < kPhaseKinds; ++k) {
            busyMs[k] = PhaseOverlapUs(k, pushedUs, tDispatch) / 1000.0;
            busySum += busyMs[k];
        }
    }
    const double otherMs = queueMs >= 0 ? (queueMs - busySum > 0 ? queueMs - busySum : 0) : -1.0;
    const double execMs = execUs / 1000.0;
    const double serverMs = (pushedUs && tDone >= pushedUs) ? (tDone - pushedUs) / 1000.0 : -1.0;
    const char* result = !ok ? "拒絕" : (out.find("\"issued\":true") != std::string::npos ? "已送到1203" : "DRY RUN");

    char line[512];
    std::snprintf(line, sizeof(line),
                  "排隊 %.0f ms（%s %.0f、%s %.0f、%s %.0f、%s %.0f、其他 %.0f）；送卡 %.1f ms；伺服器端合計 %.0f ms",
                  queueMs, kPhaseName[1], busyMs[1], kPhaseName[2], busyMs[2], kPhaseName[3], busyMs[3],
                  kPhaseName[4], busyMs[4], otherMs, execMs, serverMs);
    std::printf("io.btnPanelClick %s timing: %s\n", alias.c_str(), line);

    if (ok && !out.empty() && out[out.size() - 1] == '}') {
        char t[384];
        std::snprintf(t, sizeof(t),
                      ",\"timing\":{\"queueMs\":%.1f,\"pumpMs\":%.1f,\"pollMs\":%.1f,\"cacheMs\":%.1f,"
                      "\"pubMs\":%.1f,\"otherMs\":%.1f,\"execMs\":%.2f,\"serverMs\":%.1f}",
                      queueMs, busyMs[1], busyMs[2], busyMs[3], busyMs[4], otherMs, execMs, serverMs);
        out.insert(out.size() - 1, t);
    } else if (!ok) {
        out += std::string("　｜計時：") + line;
    }

    const char* path = std::getenv("W906_IOTIMING_LOG");
    if (path && *path) {
        //AI(W906-LAT-1) 20260925: 發布ms is the LAST column, not next to 畫面資料ms, so the rows
        //  already in runcfg\logs\io_click_timing.csv (11 columns, measured 13:27-14:19) keep every
        //  column where it was. ⚠ But 其他ms changes meaning from here on (the publish is taken out
        //  of it), so the first row this process writes into an OLD file is preceded by the new
        //  header line -- a visible marker of where the column set changed, instead of a silent one.
        static const char kHeader[] = "時間,Alias,Down,結果,排隊ms,跑流程ms,讀卡ms,畫面資料ms,其他ms,送卡ms,伺服器端合計ms,發布ms";
        static bool headerChecked = false;
        FILE* f = std::fopen(path, "ab");
        if (f) {
            std::fseek(f, 0, SEEK_END);
            if (std::ftell(f) == 0) {   // new file: UTF-8 BOM so Excel shows the Chinese header
                std::fputs("\xEF\xBB\xBF", f);
                std::fputs(kHeader, f);
                std::fputs("\r\n", f);
            } else if (!headerChecked) {
                //AI(W906-MT-FIX1) 20260926 (review 20260926): the WHOLE file is scanned once, not
                //  just line 1. Line 1 of an old file is the old 11-column header and never changes,
                //  so the first-line test was false on every process start and each F5 run appended
                //  one more 12-column header into the middle of the data. Now the header is inserted
                //  only while NO line of the file carries 發布ms yet -- i.e. exactly once, at the real
                //  change point the comment above promises. Read in blocks with a small overlap so a
                //  header that straddles two blocks is still seen; the log is small and this runs once
                //  per process.
                static const char kMark[] = "發布ms";
                const std::size_t markLen = sizeof(kMark) - 1;
                bool hasNewHeader = false;
                FILE* r = std::fopen(path, "rb");
                if (r) {
                    std::string window;
                    char buf[4096];
                    std::size_t got;
                    while (!hasNewHeader && (got = std::fread(buf, 1, sizeof(buf), r)) > 0) {
                        window.append(buf, got);
                        if (window.find(kMark) != std::string::npos) hasNewHeader = true;
                        else if (window.size() >= markLen) window.erase(0, window.size() - (markLen - 1));
                    }
                    std::fclose(r);
                }
                if (!hasNewHeader) {
                    std::fputs(kHeader, f);
                    std::fputs("\r\n", f);
                }
            }
            headerChecked = true;
            const std::time_t now = std::time(0);
            const std::tm lt = *std::localtime(&now);
            const long ms = (long)((std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count()) % 1000);
            std::fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d.%03ld,%s,%d,%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.2f,%.1f,%.1f\r\n",
                         lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, ms,
                         alias.c_str(), desiredDown, result, queueMs, busyMs[1], busyMs[2], busyMs[3],
                         otherMs, execMs, serverMs, busyMs[4]);
            std::fclose(f);
        }
    }
    return ok;
}

//AI(W906-IO-NOGUARD) 20260929: the same switch for wb_serve's three dialog wait loops (tools/wb_serve.cpp ShowErrorMessage /
//  YesNo / MyMessageBox waits): with the IO page unguarded, an io.btnPanelClick is served while a dialog is open too, instead of
//  "modal-pending" (golden's ShowModal made the IO form unreachable). 0 in W906_IO_PAGE_NO_GUARDS restores that.
bool W906_IoPageNoGuards() { return W906_IO_PAGE_NO_GUARDS != 0; }
