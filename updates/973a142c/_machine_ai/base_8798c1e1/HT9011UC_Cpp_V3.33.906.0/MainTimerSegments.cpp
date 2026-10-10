//------------------------------------------------------------------------------
// MainTimerSegments.cpp -- golden TfMain timer segments that wb_serve runs from PumpTick (WebBridgeTags.cpp), the port's
//   only periodic entry (the same seam as W906_FlushFlagTick / W906_CounterRefreshTick). One function per segment.
//------------------------------------------------------------------------------
// AI(W906-DOORLOCK) 20261001: RULINGS_20261001 #0 / #5, census 129 (e) E-T1-012 (INBOX 65) -- golden TfMain::Timer1Timer
//   main.cpp:3051-3076, the safe-door lock that follows SystemStart and the magazine door locks right after it (:3065-3076),
//   line for line (below). Until now only the boot error
//   path wrote SW[SwSafeDoorLock] (cinitial.cpp:8130-8215), so on a machine with SAFE_DOOR_LOCK the door was never locked
//   while the machine ran.
//   "Is that page open" -- golden fiosetview->fShow / fContact->fShow -- is answered the port's way, W906_FormShowing
//   (csystem.h; the web page table WebPageTable.cpp: fiosetview -> "io", fContact -> "contact"), never by a member that
//   reads false (pt-wave-loop trap #6).
//   Called by PumpTick (WebBridgeTags.cpp) every beat, like W906_FlushFlagTick. Of golden Timer1Timer's early returns that
//   come before :3051, the two that are plain flags are kept as guards: the employee-ID wait (:2861-2929 returns every tick
//   while [N07] EnableEmployeeIdCheck and bEnableEmployeeIDCheck) and the SECS/GEM alarm (:2938-2959). Not kept: the
//   SYN-TEK / MN200 card-state returns (:3008-3030, CheckPCI_L112State() / CheckPCI_MN200State() == 2) -- nothing in the
//   port runs those checks yet (git grep 20261001); that IO-card segment of Timer1Timer is its own back-fill.
//------------------------------------------------------------------------------
#include "cmydef.h"
#include "Config.h"           // IniConfig.bN07_EnableEmployeeIdCheak
#include "mysensor.h"         // Sen[]
#include "myswitch.h"         // SW[]
#include "csystem.h"          // W906_FormShowing
#include "atester_shims.h"    // fContact (TfContactShim, its own fShow member)
#include "cContact.h"         // CONTACT_MANUAL_GET_HEIGHT
#include "LastSet.h"          // LastSet.bBigFan
#include "MachineType.h"       // CC_ASE_CL / CC_HONPREC_QC

namespace ht9045 {

void W906_SafeDoorLockTick()
{
    if(IniConfig.bN07_EnableEmployeeIdCheak==true && bEnableEmployeeIDCheck==true)   // golden :2861-2929: Timer1Timer returned before :3051
        return;
    if(bSECSGEMAlarm && bSECSGEM_NoteAlarm==false)                              // golden :2938-2959: ditto
        return;

    if(SAFE_DOOR_LOCK)                                                          //20111130  Dell  //20111215 Dell
    {
        if(Enable_PLCSafety_IO==true && Sen[SnSafeMode].IsOff())                //jou 20231016 : 增加CE PLC 非安全模式不打開SafeDoorLock
        {
            SW[SwSafeDoorLock].OnOff(true);
        }
        else if(W906_FormShowing("fiosetview", false)==false)                   //jou 2011-12-16 進IO不控制SwSafeDoorLock   [golden: fiosetview->fShow==false]
        {
            if(W906_FormShowing("fContact", fContact->fShow) && iContactMode==CONTACT_MANUAL_GET_HEIGHT)      //Steven 20120131 : 手K時要放開汽缸   [golden: fContact->fShow]
                SW[SwSafeDoorLock].OnOff(false);
            else
                SW[SwSafeDoorLock].OnOff(SystemStart);
        }
    }

    if(AUTO3_IS_MAGAZINE==1)                                                    //JerryYang 20220909 : add magazine
    {
        if(W906_FormShowing("fiosetview", false)==false)                        //[golden: fiosetview->fShow==false]
        {
            if(iMagazineStatus==0)                                              //Ifor 20240103 add:Magazine 退Tray 時開門
                SW[SwMagazineSafeDoorLock].OnOff(SystemStart);
            else
                SW[SwMagazineSafeDoorLock].OnOff(false);
            SW[SwMagazineSafeDoor2LockOn].OnOff(SystemStart);
            SW[SwMagazineSafeDoor2LockOff].OnOff(!SystemStart);
        }
    }
}

}  // namespace ht9045

//------------------------------------------------------------------------------
// AI(W906-BIGFAN) 20261001: RULINGS_20261001 #0 / #5, census 129 (e) E-T2-001 / (c)(d) C1-003 -- golden TfMain::Timer2Timer
//   main.cpp:20964-20968, line for line: the big fan follows LastSet.bBigFan (the main page's FAN button, FileRW/MainClick.cpp
//   act.main.fan, only flips the flag -- golden's button does the same) and, for ASE_CL / HONPREC QC, the fan direction
//   follows [C01] FanDirection. Before this nothing drove SW[SwBigFan] (only cinitial.cpp names it).
//   Timer2's own guards before :20964 are fShow (TfMain is always shown while wb_serve runs), InitialOK (PumpTick runs only
//   after it) and its re-entry flag (one thread here) -- nothing to keep. Timer2 is golden's 1 s timer (main.dfm Timer2, VCL
//   default Interval); here once per 500 ms beat -- the same two idempotent OnOff writes.
//   ⚠ golden: CC_HONPREC_QC is 0, the value of a CUSTOMER_CODE that was never set -- kept verbatim.
//------------------------------------------------------------------------------
namespace ht9045 {

void W906_Timer2FanTick()
{
    if(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_HONPREC_QC)
    {
        SW[SwFanDirection].OnOff(IniConfig.bC01_FanDirection);
    }
    SW[SwBigFan].OnOff(LastSet.bBigFan);
}

}  // namespace ht9045
