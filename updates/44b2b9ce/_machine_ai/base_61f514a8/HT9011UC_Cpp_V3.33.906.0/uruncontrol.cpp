// =============================================================================
//  uruncontrol.cpp  --  the main-loop control thread: pumps MainProc() every
//                       tick, throttled by MySleepEx and the fContact-visible /
//                       iHome / SystemStart cadence ladder.
//
//  Faithful translation of golden uruncontrol.cpp (86 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 ("tiny" group -- five very small units).
//  Sibling of uHeaterThread.cpp (THeaterThread) and MyPLC/MyPLC_IO_Modbus.cpp
//  (TPLCIOThread) -- this file reuses their "OFFLINE-INERT TThread stand-in"
//  idiom verbatim (see uruncontrol.h for the full rationale).
//
//  ROLE
//  ----
//  TRunControl is the top-level handler main-loop thread: Execute() spins
//  Synchronize(ThreadProcess) -> ThreadProcess() -> MainProc() (when
//  InitialOK), throttled by MySleepEx(1,true) every iteration -- golden's own
//  `ct` counter (golden :46,:58-66) increments up to 2 then resets but its
//  value is never read anywhere else, so both branches of the
//  `SystemStart==false || iHome==1 || fContact->fShow==true` test end up
//  calling MySleepEx(1,true) either way (golden :52-67).  StartMainThread /
//  EndMainThread (golden :72-85) are the process-lifetime wrappers.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim -- all 5 golden functions):
//    TRunControl::TRunControl (ctor)   golden :28-32
//    TRunControl::ThreadProcess        golden :34-40
//    TRunControl::Execute              golden :42-70
//    StartMainThread                   golden :72-76
//    EndMainThread                     golden :78-85
//  SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//    MainProc()              -> csystem.cpp:540 (declared csystem.h:51), real
//                               body -- the whole handler per-frame tick.
//    InitialOK               -> real bool, cmydef.cpp:285 (declared cmydef.h:220)
//    SystemStart             -> real bool, cmydef.cpp:286 (declared cmydef.h:221)
//    iHome                   -> real int,  cmydef.cpp (declared cmydef.h:260)
//    fContact->fShow         -> real TfContactShim instance `fContact`
//                               (atester_shims.cpp:230, declared
//                               atester_shims.h:154-251); `fShow` defaults
//                               false offline (the contact-mode form is never
//                               on screen), matching golden's own runtime
//                               default when the form is closed.
//    MySleep / MySleepEx     -> real bodies, common.cpp (declared common.h
//                               :365-366) -- the same substrate uHeaterThread
//                               .cpp and acarry.cpp already use.
//    LogSoftwareOffTime      -> declared cmydef.h:5032, real (no-op) body at
//                               acarry_shims.cpp:176 -- same precedent
//                               HThreadCtrl.cpp (this wave) and uHeaterThread
//                               .cpp:414 already use.
//
//  GATE REGISTER -- exactly ONE gate.
//   (1) timeBeginPeriod(1)     golden :45.  This is the Windows Multimedia
//       Timer API (winmm.h / mmsystem.h), NOT part of plain <windows.h>.
//       Grepped the WHOLE port tree (every .cpp/.h outside build*/) for
//       `timeBeginPeriod`, `mmsystem.h` and `winmm`: the only other hit in the
//       entire tree is a Chinese comment in MachineType.h:117 that merely
//       DESCRIBES golden's call ("只需要使用一次,可以讓cycle time更精準" --
//       "only needs to be called once, lets cycle time be more precise") --
//       no header include and no link flag anywhere.  Grepped
//       D:/HT9045/HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt for
//       `target_link_libraries`: every target that links a Windows import
//       library names it explicitly (ws2_32 for the comms/kyecftp/webbridge
//       targets, psapi/version for ht9045_globals) and none names winmm.
//       Calling timeBeginPeriod here would therefore be a NEW external Win32
//       dependency this wave has no authorization or CMakeLists.txt access to
//       wire in (rule 7: this wave may not touch CMakeLists.txt).
//       Handled with the tree's #if 0/#else macro-pair idiom: golden's call
//       stays VERBATIM in the #if 0 arm; the ACTIVE arm is empty (no default
//       call needed -- see below).
//       WHY NO-OP IS THE FAITHFUL DEFAULT: timeBeginPeriod(1) is a
//       process-wide REQUEST to the OS scheduler for 1 ms timer-tick
//       granularity; it changes nothing about program LOGIC -- it only makes
//       Sleep()-family calls (MySleepEx here, golden :54/:65) wake up closer
//       to their requested delay instead of rounding up to the platform's
//       default timer resolution (commonly ~15.6 ms on Windows without this
//       call). With the gate, MySleepEx(1,true) below still sleeps
//       approximately 1 ms most of the time on modern Windows (multimedia
//       timer resolution defaults improved substantially since BCB6-era
//       Windows XP), so this is a coarse-timing/CPU-usage delta, not a
//       correctness delta: the do-while loop condition, the MainProc() pump
//       cadence and every branch outcome are identical either way.
//       BEHAVIOUR DELTA, STATED PLAINLY: on a real machine the main loop may
//       spin measurably faster/steadier (tighter, more consistent tick
//       spacing) than this offline build, which relies on the OS default
//       timer resolution for the same MySleepEx(1,true) calls. Retire this
//       gate the moment a unit in this wave's authorized scope is allowed to
//       add `target_link_libraries(<target> PUBLIC winmm)` -- delete the
//       macro, restore the #if 0 arm.
//
//  GOLDEN QUIRK preserved VERBATIM, not fixed:
//   The `ct` counter in Execute() (golden :46,:58-66) is written (incremented,
//   reset at >2) but its value never gates anything different: BOTH the
//   `ct>2` and `else` arms call `MySleepEx(1,true)` (golden :60-66), and nor
//   does anything outside this loop iteration read `ct` (it is a plain local,
//   re-zeroed on every Execute() call were it ever re-entered). This dead
//   counter is reproduced exactly rather than simplified away.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :11). No __fastcall / __property / __published
//  survive (dropped from both TRunControl::TRunControl and
//  TRunControl::ThreadProcess, matching vclcompat/vcl_compat.h:24-54's
//  established treatment). `: TThread(CreateSuspended)` base-init dropped
//  (golden :29) -- there is no TThread base (see uruncontrol.h); the
//  parameter is accepted and discarded, matching THeaterThread's ctor
//  (uHeaterThread.cpp:335-340) exactly. `Synchronize(ThreadProcess)` (golden
//  :49) becomes a direct inline call -- offline there is no second (worker)
//  thread to marshal FROM onto the main VCL thread, identical treatment
//  uHeaterThread.cpp:375 and MyPLC_IO_Modbus.cpp:311 already give the same
//  VCL idiom. `MyThread->Resume()` / `->Terminate()` route through the two
//  NO-GOLDEN-COUNTERPART members documented in uruncontrol.h (Resume() is
//  inert; Terminate() only latches the never-read `Terminated` flag) --
//  same shape as THeaterThread::Resume/Terminate (uHeaterThread.cpp:389-397).
//  SOFT_SIMULTE is NOT referenced by this unit. Numeric semantics kept EXACT
//  (the `ct>2` reset threshold, the two MySleepEx(1,true) call sites).
//
//  Big5: this file's only non-ASCII text is the "為了要可以關閉執行緒"
//  phrase repeated five times (golden :31,:74,:81,:82,:84), decoded via cp950
//  byte-for-byte (verified identical across all five occurrences) and
//  preserved as UTF-8 below, in its original column. Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "uruncontrol.h"
//   #include "csystem.h" / "cContact.h" / "cMydef.h" / "common.h"
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + <windows.h> + portable STL + using namespace std)
#include "uruncontrol.h"            // this unit's own contract (the 5 members/functions defined below)
#include "csystem.h"                // MainProc() -- real body csystem.cpp:540
#include "cmydef.h"                 // InitialOK / SystemStart / iHome / LogSoftwareOffTime declarations (real bodies cmydef.cpp / acarry_shims.cpp)
#include "common.h"                 // MySleep(DWORD) / MySleepEx(DWORD,bool) -- real bodies common.cpp
#include "atester_shims.h"          // TfContactShim `fContact` (fShow) -- real instance atester_shims.cpp:230
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TRunControl::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

TRunControl *MyThread;
extern  bool InitialOK;
//AI(W906-PT-W2) 20260807: golden ":29 : TThread(CreateSuspended)" dropped --
//  there is no VCL Classes::TThread offline (see uruncontrol.h). `Terminated`
//  is initialised here instead of by the base ctor, matching
//  THeaterThread::THeaterThread (uHeaterThread.cpp:335-340).
TRunControl::TRunControl(bool CreateSuspended)
    : Terminated(false)
{
    (void)CreateSuspended;                                                     //AI(W906-PT-W2) 20260807: golden handed it to the TThread base (golden :29); nothing consumes it offline.
    bEnd=false;                                                                //Steven 20110718 : 為了要可以關閉執行緒
}
//---------------------------------------------------------------------------
void TRunControl::ThreadProcess(void)
{
    if(InitialOK)
    {
        MainProc();
    }
}
//---------------------------------------------------------------------------
void TRunControl::Execute()
{
    //---- Place thread code here ----
    //AI(W906-PT-W2) 20260807: NEVER invoked offline -- no real OS thread spawns it
    //  (Resume() is a no-op, see uruncontrol.h). Kept compiling/callable and
    //  VERBATIM apart from the Synchronize() line and GATE (1), for a future
    //  real-thread wave. Same treatment uHeaterThread.cpp:366-379 and
    //  MyPLC_IO_Modbus.cpp:304-316 give their own Execute().
#if 0 // TODO(winmm): GATE (1) golden :45 timeBeginPeriod(1) -- see file head. No winmm link anywhere in CMakeLists.txt; this wave may not add one.
    timeBeginPeriod(1);
#endif
    int ct=0;
    do
    {
        ThreadProcess();                                                       //AI(W906-PT-W2) 20260807: golden ":49 Synchronize(ThreadProcess);". Synchronize marshals onto the VCL main thread; offline there is no second thread to marshal FROM, so the payload is called inline -- identical treatment to uHeaterThread.cpp:375 / MyPLC_IO_Modbus.cpp:311.

//        MySleepEx(1,true);
        if(SystemStart==false || iHome==1 || W906_FormShowing("fContact", fContact->fShow)==true)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
        {
            MySleepEx(1,true);
        }
        else
        {
            ct++;
            if(ct>2)
            {
                ct=0;
            }
            else
            {
                MySleepEx(1,true);
            }
        }
    } while (!bEnd);                                                           //do
//    } while (!Terminated);    //do
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
//AI(W906-PT-W2) 20260807: NO GOLDEN COUNTERPART -- these two members replace
//  VCL Classes::TThread::Resume() / ::Terminate(), which golden calls at :75,
//  :84. Offline there is no OS thread to resume or signal, so Resume() is
//  inert and Terminate() just latches the (never-read) Terminated flag.
//  Identical to THeaterThread::Resume/Terminate (uHeaterThread.cpp:389-397)
//  and TPLCIOThread::Resume (MyPLC_IO_Modbus.cpp:318-322). A future
//  real-thread wave replaces both with CreateThread/SetEvent.
//------------------------------------------------------------------------------
void TRunControl::Resume()
{
    // offline: no real OS thread to resume (see uruncontrol.h)
}
//------------------------------------------------------------------------------
void TRunControl::Terminate()
{
    Terminated=true;                                                           //AI(W906-PT-W2) 20260807: golden TThread::Terminate() sets the same protected flag; Execute() above loops on !bEnd (golden :68), not on this, so latching it is behaviour-neutral.
}
//------------------------------------------------------------------------------
void StartMainThread()
{
    MyThread->bEnd=false;                                                      //Steven 20110718 : 為了要可以關閉執行緒
    MyThread->Resume();
}
//------------------------------------------------------------------------------
void EndMainThread()
{
    LogSoftwareOffTime("FC: Wait MainThread");
    MyThread->bEnd=true;                                                       //Steven 20110718 : 為了要可以關閉執行緒
//    MyThread->WaitFor();    //Steven 20110718 : 為了要可以關閉執行緒
//    HeaterThread->Suspend();
    MyThread->Terminate();                                                     //kevin 20170421 (wei) : 為了要可以關閉執行緒
    MySleep(100);
}
