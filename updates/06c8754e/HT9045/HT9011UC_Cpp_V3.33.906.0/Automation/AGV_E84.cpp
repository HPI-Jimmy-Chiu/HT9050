// =============================================================================
//  Automation/AGV_E84.cpp  --  E84 LOADER/UNLOADER HANDSHAKE CORE (W5-Final unit)
//
//  Faithful translation of golden TfAGV member functions + file-scope globals,
//  from  HT9011UC_Code_V3.33.906.0_20260618/Automation/AGV.cpp  (BCB6, Big5).
//  See AGV_E84.h for the verified exact golden line ranges + full scope/
//  demotion/shared-state rationale.  Read that file first.
//
//  Translation wave: W5-Final (AGV_E84 unit)
//  Translator: AI(W5-Final-AGV_E84) 20260711
//
//  WHAT IS ACTIVE
//  ---------------
//  Every switch(Task) case body in DoE84Loader/DoE84Unloader is translated
//  VERBATIM and ACTIVE over the already-translated Sim HAL (Sen[]/SW[]/
//  Cylinder[]/TQPF_Timer -- same idiom as acarry.cpp's Do_Auto_SHT1, proven
//  100%-ACTIVE precedent).  ShowE84Log/E84StatusLog are ACTIVE for every
//  statement EXCEPT the 3 gated spots below.
//
//  GATED DEPENDENCIES (#if 0 // TODO(...) + documented conservative default,
//  established project pattern -- see HANA_ART.cpp:967-973 for the identical
//  "identifier itself gated in common.h" idiom this mirrors):
//
//    1. [RESOLVED -- AI(W5-Final-Integrate) 20260711] fAGV->mmE84Log (golden
//       AGV.h:85, TMemo*) was NOT yet a TfAGV facade member at translate time.
//       INTEGRATE has since added `TfMainMemo *mmE84Log;` to `class TfAGV`
//       (FormsFacade.h) + an explicit `TfAGV::TfAGV(){ mmE84Log = new
//       TfMainMemo(); }` (FormsFacade.cpp) + `#include "FormsFacade.h"` above,
//       and lifted both call sites in ShowE84Log (Lines->Add / Lines->Count+
//       Clear) out of their `#if 0` gates -- they are ACTIVE now (still
//       offline no-op sinks, since TfMainMemo/TfMainMemoLines are themselves
//       offline stand-ins; see FormsFacade.h). This was the ONLY widget touch
//       in the whole core body (verified: grep of DoE84Loader/DoE84Unloader/
//       ShowE84Log/E84StatusLog finds no other VCL control reference).
//
//    2. [RESOLVED -- AI(W906-CommonWaveFile) 20260721] WriteDataToFile(AnsiString,
//       char*,bool) -- golden common.h:296-297. Its common.h declaration is
//       un-gated and the common.cpp body (char* overload used at both call
//       sites here) is now real. Both call sites (1 in ShowE84Log, 1 in
//       E84StatusLog) are un-gated -- the E84DataTxt log file is now actually
//       written (append mode, default bOverWrite=false), a genuine behavior
//       upgrade from the prior no-op. The DirectoryExists/ForceDirectories
//       day-folder housekeeping above each call was already active.
//
//    3. ShowMyMessageBox_YES_NO(AnsiString,AnsiString,AnsiString=NULL) -- golden
//       mymessbox.h:55.  A real VCL modal Yes/No dialog, W7-UI, not translated
//       anywhere in this tree (no canary_support stand-in exists for it,
//       unlike ShowMyMessage which DOES have one -- see canary_support.h:137).
//       2 call sites gated (DoE84Loader/DoE84Unloader case 2000, the rare
//       "detected buffer position disagrees with the AGV's own CS0/CS1 report"
//       path).  ACTIVE conservative default: `ret=2` (declined/"No") --
//       matches the golden dialog's declined-branch behaviour (falls straight
//       to Task=2100 without touching iPlaceWhichBuffer), so an unattended
//       offline run never silently reassigns the buffer index on an
//       unconfirmed sensor mismatch.
//
//  JUDGMENT CALL (flag for review): golden case 5000 in BOTH DoE84Loader and
//  DoE84Unloader calls `btInitalLoad->Click()` / `btInitalUnLoad->Click()` --
//  a TButton widget click, which is a 4th widget touch the originating
//  feasibility pass did not flag (it expected mmE84Log to be the ONLY one --
//  see AGV_E84.h scope note / final translate report "discrepancy" item).
//  Rather than adding a fake TButton dependency to FormsFacade for a
//  synchronous, side-effect-only VCL event, this translation collapses the
//  Click() call to a DIRECT call of its own OnClick handler body -- which is
//  fully visible in golden (AGV.cpp:1055-1065) and is PROVABLY equivalent:
//    void __fastcall TfAGV::btInitalLoadClick(TObject *Sender)
//    { InitialE84LoadTask(); InitialE84LoadSensor(); }
//    void __fastcall TfAGV::btInitalUnLoadClick(TObject *Sender)
//    { InitialE84UnLoaderTask(); InitialE84UnloadSensor(); }
//  VCL's TButton::Click() synchronously fires the assigned OnClick handler
//  with no other side effect (no message-loop reentrancy here, no other
//  handler assigned) -- so calling the two helper functions directly is
//  behaviourally IDENTICAL, not an approximation, and both helpers are
//  already in this unit's own verified scope (:176-218).  Flagged here
//  per-instructions as a discrepancy/judgment call for human review even
//  though the equivalence is provable, since it silently drops a widget
//  reference the upstream feasibility pass didn't anticipate.
//
//  FAITHFUL GOLDEN QUIRKS PRESERVED VERBATIM (not "fixed" -- see CLAUDE.md):
//    - DoE84Loader case 2000: `else if(iCount==1) str="Color";` and
//      `else if(iPlaceWhichBuffer[0]==1) str1="Color";` each repeat a
//      condition already tested one branch earlier (`iCount==1` /
//      `iPlaceWhichBuffer[0]==1`), making the "Color" arm of BOTH ternary-like
//      ladders permanently unreachable (dead branch, golden AGV.cpp:552-554,
//      559-561).  Whichever customer's line actually uses a 3rd (Color)
//      AGV buffer would see "" in that slot of the confirm-dialog message --
//      a real golden bug, preserved as-is.
//    - DoE84Unloader case 1 (golden :614): arms E84UnLoadDelay from
//      `TestIF_File.iE84TimeOut_K12[0][9]` (index **0**, the LOADER's TD0 slot)
//      instead of `[1][9]` (the row this function uses for every OTHER timeout
//      lookup, e.g. :690/:699/:716/:733/:748/:792/:817/:836 all use `[1][...]`).
//      Almost certainly a copy-paste artifact from DoE84Loader, but it is what
//      the shipped golden binary does -- preserved verbatim, NOT corrected to
//      `[1][9]`.
//    - ShowE84Log: computes `asStr` (a spaced "Loader Task: N Message: ..."
//      format, golden :1088) but never uses it again -- the actual persisted/
//      logged text is the differently-formatted `str` reassignment on the
//      very next line (:1090).  Dead store, preserved verbatim.
//    - DoE84Loader case 900 (golden :530-534): a genuinely empty if-body
//      (`if(Sen[SnLoaderTrayHasTray_AGV+iPlaceWhichBuffer[0]].IsOff()) { //comment
//      only }`) -- no side effect, translated as an empty block with the
//      comment preserved.
//    - DoE84Unloader's `static bool bSensorStatus[3]` (golden :591) is
//      computed at case 100 (:670-677) but never read anywhere afterward in
//      the function -- write-only dead state, preserved verbatim (not removed
//      as "unused").
//    - `#ifndef SOFT_SIMULTE` guards (golden :235-238, :599-602) preserved
//      verbatim (not hardcoded to either branch) -- SOFT_SIMULTE is
//      `//#define`d OUT in this tree's MachineType.h, so these blocks are
//      ACTIVE in the current build, exactly as golden.
//
//  Big5: every Chinese comment/string literal decoded via cp950 (python
//  `open(path, encoding='cp950')` / `iconv -f CP950 -t UTF-8`) and reproduced
//  as correct UTF-8 below.  Final gate: ZERO U+FFFD bytes (verified before
//  hand-off).
// =============================================================================
#include "AGV_E84.h"

#include "cmydef.h"          // SystemYear../Word globals; bE84LoaderActionflag/bE84UnloaderActionflag;
                              // Sn*/Sw*/C_* sensor-switch-cylinder index constants; iMMAuto[]
#include "cprod.h"            // TestIF_File (SYSTEM_TEST_IF, .iE84TimeOut_K12[2][11])
#include "cpublic.h"          // GetTimeInfo()
// AI(W906-CommonWaveFile) 20260721: added -- WriteDataToFile (golden common.h:
// 255-256) is called in ShowE84Log/E84StatusLog below; its declaration was
// previously only reachable while gated (never actually needed a header, since
// both call sites were themselves `#if 0`'d). Now that both are un-gated and
// common.cpp's body is real, this file needs the declaration in scope.
#include "common.h"           // WriteDataToFile(char*,char*,bool=false)
#include "mysensor.h"          // Sen[] (TMySensor Sim HAL)
#include "myswitch.h"          // SW[] (TMySwitch Sim HAL)
#include "mycylin.h"           // Cylinder[] (TMyCylinder Sim HAL)
#include "Motor/mymotor.h"     // MOT[] (TTrayMotor), ClearTray
#include "canary_support.h"    // ShowMyMessage (ACTIVE Sim stand-in) + __FUNC__ macro
#include "FormsFacade.h"       // fAGV->mmE84Log (W5-Final-AGV_E84 INTEGRATE: added TfAGV::mmE84Log this pass -- lifts the 2 TODO(FormsFacade-wiring) gates below)

// ---------------------------------------------------------------------------
//  Task cursors + per-call persistent state (golden Automation/AGV.cpp:171-222).
//  Non-static: iE84LoadTask/iE84UnloadTask/iLoaderScanTask/iUnloaderScanTask/
//  iPlaceWhichBuffer/bE84Loaderflag/bE84Unloaderflag are file-scope globals in
//  golden too (external linkage) -- preserved, and iLoaderScanTask/
//  iUnloaderScanTask/bE84Loaderflag/bE84Unloaderflag additionally need to stay
//  extern-visible for the future DoE84LoaderScan/DoE84UnloaderScan wave (see
//  AGV_E84.h "SHARED STATE" note).
// ---------------------------------------------------------------------------
int iE84LoadTask=1;
int iE84UnloadTask=1;
int iLoaderScanTask[3]={1, 1, 1};
int iUnloaderScanTask[3]={1, 1, 1};
int iPlaceWhichBuffer[2];
bool bE84Loaderflag[3]={false, false, false};
bool bE84Unloaderflag[3]={false, false, false};

// ---------------------------------------------------------------------------
//  E84LoadDelay/E84UnLoadDelay (golden :220-221) -- verified: referenced ONLY
//  inside DoE84Loader/DoE84Unloader respectively (grep-checked against the
//  whole golden tree), never by DoE84LoaderScan/DoE84UnloaderScan or any other
//  TU.  Genuinely TU-local persistent SM state -- kept `static` (internal
//  linkage) here per the "own file-scope static locals unless shared
//  component" convention, tighter than golden's plain external-linkage globals.
// ---------------------------------------------------------------------------
static TQPF_Timer E84LoadDelay;
static TQPF_Timer E84UnLoadDelay;

//------------------------------------------------------------------------------
void InitialE84LoadTask()
{
    iE84LoadTask=1;
}
//------------------------------------------------------------------------------
void InitialE84UnLoaderTask()
{
    iE84UnloadTask=1;
}
//------------------------------------------------------------------------------
void InitialLoaderScanTask()
{
    iLoaderScanTask[0]=1;
    iLoaderScanTask[1]=1;
    iLoaderScanTask[2]=1;
}
//------------------------------------------------------------------------------
void InitialUnLoaderScanTask()
{
    iUnloaderScanTask[0]=1;
    iUnloaderScanTask[1]=1;
    iUnloaderScanTask[2]=1;
}
//------------------------------------------------------------------------------
void InitialE84LoadSensor()
{
    SW[SwE84_1_LREQ].Off();
    SW[SwE84_1_UREQ].Off();
    SW[SwE84_1_VA].Off();
    SW[SwE84_1_READY].Off();
    SW[SwE84_1_VS0].Off();
    SW[SwE84_1_VS1].Off();
}
//------------------------------------------------------------------------------
void InitialE84UnloadSensor()
{
    SW[SwE84_2_LREQ].Off();
    SW[SwE84_2_UREQ].Off();
    SW[SwE84_2_VA].Off();
    SW[SwE84_2_READY].Off();
    SW[SwE84_2_VS0].Off();
    SW[SwE84_2_VS1].Off();
}
//------------------------------------------------------------------------------
void DoE84Loader()
{
    static bool bCS0=false, bCS1=false;
    static int iCount=-1;

    AnsiString str="", str1="", asStr="";
    int &Task=iE84LoadTask;
    int ret;
    int SenIndex[3]={SnLoaderTrayHasTray_AGV, SnEmptyTrayHasTray_AGV, SnColorTrayHasTray_AGV};
    int C_MiddleIndex[3]={C_Load_Middle, C_Empty_Middle, C_Color_Middle};

    #ifndef SOFT_SIMULTE
    SW[SwE84_1_ES].On();
    SW[SwE84_1_HOAVBL].On();
    #endif

    switch(Task)
    {
        case 1:     // AGV車到位 通訊啟動 (AGV in position, comm handshake starts)
            if(SW[SwE84_1_HOAVBL].Status()==true && Sen[SnE84_1_GO].IsOn())
            {
                // golden :245-246 -- dead code, commented out in golden itself:
                //if(TestIF_File.bAGVRunningMusic)
                //    SW[SwMusic1].On();

                iPlaceWhichBuffer[0]=10;
                if(Sen[SnE84_1_CS0].IsOn() || Sen[SnE84_1_CS1].IsOn())
                {
                    str.sprintf("Sensor_Go On");
                    ShowE84Log(str,1,1);
                    E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][9]);       //TD
                    bCS0=false;
                    bCS1=false;
                    iCount=-1;
                    Task=50;
                }
                else
                {
                    //未知位置 (unknown position) -- golden: empty else, no action
                }
            }
            break;
        case 50:
            if(Sen[SnE84_1_VALID].IsOn())
            {
                str.sprintf("Sensor_VALID On");
                ShowE84Log(str,50,1);
                Task=100;
            }
            else if(E84LoadDelay.Off())
            {
                str.sprintf("Sensor_VALID Is Not On And TD0 Time Out");
                ShowE84Log(str,50,3);
                Task=5000;  //Auto Recover
            }
            break;
        case 100:
            if(Sen[SnE84_1_CS0].IsOn())
            {
                str.sprintf("Sensor_CS0 On");
                ShowE84Log(str,100,1);
                bCS0=true;
            }

            if(Sen[SnE84_1_CS1].IsOn())
            {
                str.sprintf("Sensor_CS1 On");
                ShowE84Log(str,100,1);
                bCS1=true;
            }

            if(bCS0==true && bCS1==false)
                iPlaceWhichBuffer[0]=0;
            else if(bCS0==false && bCS1==true)
                iPlaceWhichBuffer[0]=1;
            else if(bCS0==true && bCS1==true)
                iPlaceWhichBuffer[0]=2;

            if(iPlaceWhichBuffer[0]==10)
            {
//                E84LoadDelay.SetMSAndOn(500);
                str.sprintf("Sensor_CS/CS1 All Off");
                ShowE84Log(str,100,1);
                Task=100;
                break;
            }

            if(Cylinder[C_MiddleIndex[iPlaceWhichBuffer[0]]].OffStatus()==true)
            {
                bE84LoaderActionflag[iPlaceWhichBuffer[0]]=true;
                if(iPlaceWhichBuffer[0]==0)
                    asStr="Loader";
                else if(iPlaceWhichBuffer[0]==1)
                    asStr="Empty";
                else if(iPlaceWhichBuffer[0]==2)
                    asStr="Color";

                str.sprintf("Place To Buffer : %s", asStr);
                ShowE84Log(str,100,1);
                E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][6]);           //TA1
                Task=200;
            }
            break;
        case 200:
            if(SW[SwE84_1_UREQ].Status()==false)
            {
                SW[SwE84_1_UREQ].On();
                E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][0]);           //TP1
                str.sprintf("Switch_UREQ On");
                ShowE84Log(str,200,1);
                Task=300;
            }
            else if(E84LoadDelay.Off())
            {
                str.sprintf("Switch_UREQ Is Not On And TA1 Time Out");
                ShowE84Log(str,200,3);
                Task=5000;
            }
            break;
        case 300:
            if(SW[SwE84_1_UREQ].Status()==true)
            {
                if(Sen[SnE84_1_TRREQ].IsOn())
                {
                    E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][7]);       //TA2
                    str.sprintf("Sensor_TRREQ On");
                    ShowE84Log(str,300,1);
                    Task=400;
                }
                else if(E84LoadDelay.Off())
                {
                    str.sprintf("Sensor_TRREQ Is Not On And TP1 Time Out");
                    ShowE84Log(str,300,3);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 400:
            if(SW[SwE84_1_READY].Status()==false)
            {
                SW[SwE84_1_READY].On();
                E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][1]);           //TP2
                str.sprintf("Switch_READY On");
                ShowE84Log(str,400,1);
                Task=500;
            }
            else if(E84LoadDelay.Off())
            {
                str.sprintf("Switch_READY Is Not On And TA2 Time Out");
                ShowE84Log(str,400,3);
                Task=5000;
            }
            break;
        case 500:
            if(Sen[SnE84_1_BUSY].IsOn())
            {
                E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][2]);           //TP3
                str.sprintf("Sensor_BUSY On");
                ShowE84Log(str,500,1);

                bCS0=false;
                bCS1=false;
                if(Sen[SnE84_1_CS0].IsOn())
                {
                    str.sprintf("Sensor_CS0 On");
                    ShowE84Log(str,500,1);
                    bCS0=true;
                }

                if(Sen[SnE84_1_CS1].IsOn())
                {
                    str.sprintf("Sensor_CS1 On");
                    ShowE84Log(str,500,1);
                    bCS1=true;
                }

                if(bCS0==true && bCS1==false)
                    iCount=0;
                else if(bCS0==false && bCS1==true)
                    iCount=1;
                else if(bCS0==true && bCS1==true)
                    iCount=2;
                else
                    iCount=100;

                Task=600;
            }
            else if(E84LoadDelay.Off())
            {
                str.sprintf("Sensor_BUSY Is Not On And TP2 Time Out");
                ShowE84Log(str,500,3);
                Task=5000;  //Auto Recover
            }
            break;
        case 600:
            if(SW[SwE84_1_UREQ].Status()==true)
            {
                if(Sen[SenIndex[iPlaceWhichBuffer[0]]].IsOn()==true)
                {
                    SW[SwE84_1_UREQ].Off();
                    E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][3]);           //TP4
                    str.sprintf("Switch_UREQ OFF");
                    ShowE84Log(str,600,1);
                    Task=700;
                }
                else if(E84LoadDelay.Off())
                {
                    if(iPlaceWhichBuffer[0]==0)
                        asStr="Loader";
                    else if(iPlaceWhichBuffer[0]==1)
                        asStr="Empty";
                    else if(iPlaceWhichBuffer[0]==2)
                        asStr="Color";

                    str.sprintf("Sensor %s Has Tray Is Not On And TP3 Time Out",asStr);
                    ShowE84Log(str,600,3);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 700:
            if(SW[SwE84_1_UREQ].Status()==false)
            {
                if(Sen[SnE84_1_TRREQ].IsOff() && Sen[SnE84_1_COMPT].IsOn())
                {
                    E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][8]);       //TA3
                    str.sprintf("Sensor_TRREQ OFF And Sensor_COMPT On");
                    ShowE84Log(str,700,1);
                    Task=800;
                }
                else if(E84LoadDelay.Off())
                {
                    str.sprintf("Sensor_TRREQ Is Not OFF Or Sensor_COMPT Is Not On / TP4 Time Out");
                    ShowE84Log(str,700,3);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 800:
            if(SW[SwE84_1_READY].Status()==true)
            {
                if(iCount!=100)
                {
                    SW[SwE84_1_READY].Off();
                    E84LoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][4]);           //TP5
                    str.sprintf("Switch_READY OFF");
                    ShowE84Log(str,800,1);
                    Task=900;
                }
                else
                {
                    bCS0=false;
                    bCS1=false;
                    if(Sen[SnE84_1_CS0].IsOn())
                    {
                        str.sprintf("Sensor_CS0 On");
                        ShowE84Log(str,800,1);
                        bCS0=true;
                    }

                    if(Sen[SnE84_1_CS1].IsOn())
                    {
                        str.sprintf("Sensor_CS1 On");
                        ShowE84Log(str,800,1);
                        bCS1=true;
                    }

                    if(bCS0==true && bCS1==false)
                        iCount=0;
                    else if(bCS0==false && bCS1==true)
                        iCount=1;
                    else if(bCS0==true && bCS1==true)
                        iCount=2;
                    else
                        iCount=100;
                }
            }
            else if(E84LoadDelay.Off())
            {
                str.sprintf("Switch_READY Is Not OFF / TA3 Time Out");
                ShowE84Log(str,800,3);
                Task=5000;  //Auto Recover
            }
            break;
        case 900:
            if(SW[SwE84_1_READY].Status()==false)
            {
                if(Sen[SnE84_1_VALID].IsOff() && Sen[SnE84_1_COMPT].IsOff() && Sen[SnE84_1_CS0].IsOff() && Sen[SnE84_1_CS1].IsOff())
                {
                    if(iCount==iPlaceWhichBuffer[0])
                    {
                        str.sprintf("Finish Loader Action");      //Finish
                        ShowE84Log(str,900,1);

//                        if(iPlaceWhichBuffer[0]==0)
//                            iAGVFinishTrayCount++;

                        bE84LoaderActionflag[iPlaceWhichBuffer[0]]=false;
                        bE84Loaderflag[iPlaceWhichBuffer[0]]=false;

                        Task=1000;
                    }
                    else
                    {
                        Task=2000;
                    }

                    if(Sen[SnLoaderTrayHasTray_AGV+iPlaceWhichBuffer[0]].IsOff())
                    {
                        //Loader 區只有一盤tray (golden: empty body, no action -- comment only)
                    }
                                     //Finish
                }
                else if(E84LoadDelay.Off())
                {
                    str.sprintf("Sensor_VALID Or Sensor_COMPT Or Sensor_CS0 Or Sensor_CS1 Is Not OFF / TP5 Time Out");
                    ShowE84Log(str,900,3);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 1000:
            bE84LoaderActionflag[iPlaceWhichBuffer[0]]=false;
            bE84Loaderflag[iPlaceWhichBuffer[0]]=false;
            Task=1;
            break;
        case 2000:
            // AI(W5-Final-AGV_E84) 20260711: golden bug preserved verbatim -- the
            // `iCount==1` / `iPlaceWhichBuffer[0]==1` conditions each repeat the
            // branch immediately above, so the "Color" arm of BOTH ladders below
            // is permanently unreachable (golden AGV.cpp:552-554, 559-561). NOT
            // corrected to `iCount==2` / `iPlaceWhichBuffer[0]==2`.
            if(iCount==0)
                str="Loader";
            else if(iCount==1)
                str="Empty";
            else if(iCount==1)
                str="Color";

            if(iPlaceWhichBuffer[0]==0)
                str1="Loader";
            else if(iPlaceWhichBuffer[0]==1)
                str1="Empty";
            else if(iPlaceWhichBuffer[0]==1)
                str1="Color";

            asStr.sprintf("AGV Place To %s,Start Buffer is %s",str, str1);
            // AI(W5-Final-AGV_E84) 20260711: ShowMyMessageBox_YES_NO (golden
            // mymessbox.h:55) is a real VCL modal Yes/No dialog, W7-UI,
            // untranslated anywhere in this tree (see file-head GATED
            // DEPENDENCIES #3).  Conservative default: ret=2 ("No"/declined),
            // matching the golden dialog's declined-branch behaviour (falls
            // straight to Task=2100 without touching iPlaceWhichBuffer) -- an
            // unattended offline run never silently reassigns the buffer index
            // on an unconfirmed CS0/CS1 mismatch.
            #if 0 // TODO(W7-UI): golden AGV.cpp:565 -- ShowMyMessageBox_YES_NO real modal dialog
            ret=ShowMyMessageBox_YES_NO(asStr, "是否要修改Port狀態?");
            #else
            ret=2;
            #endif // TODO(W7-UI)
            if(ret==1)
            {
                iPlaceWhichBuffer[0]=iCount;
                bE84LoaderActionflag[iPlaceWhichBuffer[0]]=false;
                bE84Loaderflag[iPlaceWhichBuffer[0]]=false;
            }
            Task=2100;
            break;
        case 2100:
            str.sprintf("Finish Loader Action");
            ShowE84Log(str,900,1);     // golden logs this under task-tag 900 (same tag as the normal-path finish message at case 900), verbatim
            bE84LoaderActionflag[iPlaceWhichBuffer[0]]=false;
            bE84Loaderflag[iPlaceWhichBuffer[0]]=false;
            Task=1000;                                                  //Finish
            break;
        case 5000:              //Auto Recover
            // AI(W5-Final-AGV_E84) 20260711: golden calls `btInitalLoad->Click()`
            // here (a TButton widget). Collapsed to a direct call of that
            // button's own OnClick handler body (golden AGV.cpp:1055-1058, fully
            // visible, side-effect-only, no other handler assigned) -- provably
            // equivalent, not an approximation. See file-head JUDGMENT CALL note
            // (flagged for review: this is a widget touch the upstream scope
            // assessment did not anticipate).
            InitialE84LoadTask();
            InitialE84LoadSensor();
            break;
    }
}
//---------------------------------------------------------------------------
void DoE84Unloader()
{
    static int iCount=-1;
    static bool bCS0=false, bCS1=false;
    static bool bSensorStatus[3]={false, false, false};    // golden: computed at case 100, never read afterward -- write-only dead state, preserved verbatim

    AnsiString str="", asStr="";
    int &Task=iE84UnloadTask;
    int SenIndex[3]={SnAuto1TrayHasTray, SnAuto2TrayHasTray, SnAuto3TrayHasTray};
    int C_MiddleIndex[3]={C_Auto1_Selector, C_Auto2_Selector, C_Auto3_Selector};
    int ret;

    #ifndef SOFT_SIMULTE
    SW[SwE84_2_ES].On();
    SW[SwE84_2_HOAVBL].On();
    #endif

    switch(Task)
    {
        case 1:     // AGV車到位 通訊啟動 (AGV in position, comm handshake starts)
            if(SW[SwE84_2_HOAVBL].Status()==true && Sen[SnE84_2_GO].IsOn())
            {
                iPlaceWhichBuffer[1]=10;
                if(Sen[SnE84_2_CS0].IsOn() || Sen[SnE84_2_CS1].IsOn())
                {
                    str.sprintf("Sensor_Go On");
                    ShowE84Log(str,1,2);
                    // AI(W5-Final-AGV_E84) 20260711: golden bug preserved
                    // verbatim -- indexes `iE84TimeOut_K12[0][9]` (row 0, the
                    // LOADER's TD0 slot) here, whereas every OTHER timeout in
                    // THIS function indexes row `[1]` (see case 200/300/400/
                    // 500/600/700/800 below). Almost certainly a copy-paste
                    // artifact from DoE84Loader; kept as-is (golden AGV.cpp:614),
                    // NOT corrected to `[1][9]`.
                    E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[0][9]);       //TD0
                    bCS0=false;
                    bCS1=false;
                    iCount=-1;
                    Task=50;
                }
                else
                {
                    //未知位置 (unknown position) -- golden: empty else, no action
                }
            }
            break;
        case 50:
            if(Sen[SnE84_2_VALID].IsOn())
            {
                str.sprintf("Sensor_VALID On");
                ShowE84Log(str,50,2);
                Task=100;
            }
            else if(E84UnLoadDelay.Off())
            {
                str.sprintf("Sensor_VALID Is Not On And TD0 Time Out");
                ShowE84Log(str,50,4);
                Task=5000;  //Auto Recover
            }
            break;
        case 100:
            if(Sen[SnE84_2_CS0].IsOn())
            {
                str.sprintf("Sensor_CS0 On");
                ShowE84Log(str,10,2);
                bCS0=true;
            }

            if(Sen[SnE84_2_CS1].IsOn())
            {
                str.sprintf("Sensor_CS1 On");
                ShowE84Log(str,10,2);
                bCS1=true;
            }

            if(bCS0==true && bCS1==false)
                iPlaceWhichBuffer[1]=0;
            else if(bCS0==false && bCS1==true)
                iPlaceWhichBuffer[1]=1;
            else if(bCS0==true && bCS1==true)
                iPlaceWhichBuffer[1]=2;

            if(iPlaceWhichBuffer[1]==10)
            {
                str.sprintf("Sensor_CS/CS1 All Off");
                ShowE84Log(str,100,2);
                Task=100;
                break;
            }

            for(int i=0; i<3; i++)
            {
                bSensorStatus[i]=false;
                if(Sen[SenIndex[i]].IsOn())
                {
                    bSensorStatus[i]=true;
                }
            }

            if(Cylinder[C_MiddleIndex[iPlaceWhichBuffer[1]]].OffStatus()==true)
            {
                bE84UnloaderActionflag[iPlaceWhichBuffer[1]]=true;
                if(iPlaceWhichBuffer[1]==0)
                    asStr="Auto 1";
                else if(iPlaceWhichBuffer[1]==1)
                    asStr="Auto 2";
                else if(iPlaceWhichBuffer[1]==2)
                    asStr="Auto 3";

                str.sprintf("Pick from Buffer : %s", asStr);
                E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][6]);           //TA1
                ShowE84Log(str,100,2);
                Task=200;
            }
            break;
        case 200:
            if(SW[SwE84_2_LREQ].Status()==false)
            {
                SW[SwE84_2_LREQ].On();
                E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][0]);           //TP1
                str.sprintf("Switch_LREQ On");
                ShowE84Log(str,200,2);
                Task=300;
            }
            else if(E84UnLoadDelay.Off())
            {
                str.sprintf("Switch_LREQ Is Not On And TA1 Time Out");
                ShowE84Log(str,200,4);
                Task=5000;
            }
            break;
        case 300:
            if(SW[SwE84_2_LREQ].Status()==true)
            {
                if(Sen[SnE84_2_TRREQ].IsOn())
                {
                    E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][7]);       //TA2
                    str.sprintf("Sensor_TRREQ On");
                    ShowE84Log(str,300,2);
                    Task=400;
                }
                else if(E84UnLoadDelay.Off())
                {
                    str.sprintf("Sensor_TRREQ Is Not On And TP1 Time Out");
                    ShowE84Log(str,300,4);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 400:
            if(SW[SwE84_2_READY].Status()==false)
            {
                SW[SwE84_2_READY].On();
                E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][1]);           //TP2
                str.sprintf("Switch_READY On");
                ShowE84Log(str,400,2);
                Task=500;
            }
            else if(E84UnLoadDelay.Off())
            {
                str.sprintf("Switch_READY Is Not On And TA2 Time Out");
                ShowE84Log(str,400,4);
                Task=5000;
            }
            break;
        case 500:
            if(Sen[SnE84_2_BUSY].IsOn())
            {
                E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][2]);           //TP3
                str.sprintf("Sensor_BUSY On");
                ShowE84Log(str,500,2);

                bCS0=false;
                bCS1=false;
                if(Sen[SnE84_2_CS0].IsOn())
                {
                    str.sprintf("Sensor_CS0 On");
                    ShowE84Log(str,500,2);
                    bCS0=true;
                }

                if(Sen[SnE84_2_CS1].IsOn())
                {
                    str.sprintf("Sensor_CS1 On");
                    ShowE84Log(str,500,2);
                    bCS1=true;
                }

                if(bCS0==true && bCS1==false)
                    iCount=0;
                else if(bCS0==false && bCS1==true)
                    iCount=1;
                else if(bCS0==true && bCS1==true)
                    iCount=2;
                else
                    iCount=100;

                Task=600;
            }
            else if(E84UnLoadDelay.Off())
            {
                str.sprintf("Sensor_BUSY Is Not On And TP2 Time Out");
                ShowE84Log(str,500,4);
                Task=5000;  //Auto Recover
            }
            break;
        case 600:
            if(SW[SwE84_2_LREQ].Status()==true)
            {
                if(Sen[SenIndex[iPlaceWhichBuffer[1]]].IsOff()==true)
                {
                    SW[SwE84_2_LREQ].Off();
                    E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][3]);           //TP4
                    str.sprintf("Switch_UREQ OFF");
                    ShowE84Log(str,600,2);
                    Task=700;
                }
                else if(E84UnLoadDelay.Off())
                {
                    if(iPlaceWhichBuffer[1]==0)
                        asStr="Auto 1";
                    else if(iPlaceWhichBuffer[1]==1)
                        asStr="Auto 2";
                    else if(iPlaceWhichBuffer[1]==2)
                        asStr="Auto 3";

                    str.sprintf("Sensor %s Has Tray Is Not On And TP3 Time Out",asStr);
                    ShowE84Log(str,600,4);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 700:
            if(SW[SwE84_2_LREQ].Status()==false)
            {
                if(Sen[SnE84_2_TRREQ].IsOff() && Sen[SnE84_2_COMPT].IsOn())
                {
                    E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][8]);       //TA3
                    str.sprintf("Sensor_TRREQ OFF And Sensor_COMPT On");
                    ShowE84Log(str,700,2);
                    Task=800;
                }
                else if(E84UnLoadDelay.Off())
                {
                    str.sprintf("Sensor_TRREQ Is Not OFF Or Sensor_COMPT Is Not On / TP4 Time Out");
                    ShowE84Log(str,700,4);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 800:
            if(SW[SwE84_2_READY].Status()==true)
            {
                if(iCount!=100)
                {
                    SW[SwE84_2_READY].Off();
                    E84UnLoadDelay.SetSecAndOn(TestIF_File.iE84TimeOut_K12[1][4]);           //TP5
                    str.sprintf("Switch_READY OFF");
                    ShowE84Log(str,800,2);
                    Task=900;
                }
                else
                {
                    bCS0=false;
                    bCS1=false;
                    if(Sen[SnE84_2_CS0].IsOn())
                    {
                        str.sprintf("Sensor_CS0 On");
                        ShowE84Log(str,800,2);
                        bCS0=true;
                    }

                    if(Sen[SnE84_2_CS1].IsOn())
                    {
                        str.sprintf("Sensor_CS1 On");
                        ShowE84Log(str,800,2);
                        bCS1=true;
                    }

                    if(bCS0==true && bCS1==false)
                        iCount=0;
                    else if(bCS0==false && bCS1==true)
                        iCount=1;
                    else if(bCS0==true && bCS1==true)
                        iCount=2;
                    else
                        iCount=100;
                }
            }
            else if(E84UnLoadDelay.Off())
            {
                str.sprintf("Switch_READY Is Not OFF / TA3 Time Out");
                ShowE84Log(str,800,4);
                Task=5000;  //Auto Recover
            }
            break;
        case 900:
            if(SW[SwE84_2_READY].Status()==false)
            {
                if(Sen[SnE84_2_VALID].IsOff() && Sen[SnE84_2_COMPT].IsOff() && Sen[SnE84_2_CS0].IsOff() && Sen[SnE84_2_CS1].IsOff())
                {
                    if(iCount==iPlaceWhichBuffer[1])
                    {
                        str.sprintf("Finish Unloader Action");
                        ShowE84Log(str,900,2);
                        bE84UnloaderActionflag[iPlaceWhichBuffer[1]]=false;
                        bE84Unloaderflag[iPlaceWhichBuffer[1]]=false;
                        Task=1000;                                                  //Finish
                    }
                    else
                    {
                        Task=2000;
                    }
                }
                else if(E84UnLoadDelay.Off())
                {
                    str.sprintf("Sensor_VALID Or Sensor_COMPT Or Sensor_CS0 Or Sensor_CS1 Is Not OFF / TP5 Time Out");
                    ShowE84Log(str,900,4);
                    Task=5000;  //Auto Recover
                }
            }
            break;
        case 1000:
            bE84UnloaderActionflag[iPlaceWhichBuffer[1]]=false;
            bE84Unloaderflag[iPlaceWhichBuffer[1]]=false;
            MOT[iMMAuto[iPlaceWhichBuffer[1]]].ClearTray(__FUNC__);             //AI(general) 20260401 (RogerYang) : clear software tray after AMR pickup
            Task=1;
            break;
        case 2000:
            asStr.sprintf("AGV Pick Form Auto %d,Start Buffer is Auto %d",iCount+1, iPlaceWhichBuffer[1]+1);
            // AI(W5-Final-AGV_E84) 20260711: same gate as DoE84Loader case 2000
            // -- see file-head GATED DEPENDENCIES #3. Conservative default:
            // ret=2 ("No"/declined).
            #if 0 // TODO(W7-UI): golden AGV.cpp:910 -- ShowMyMessageBox_YES_NO real modal dialog
            ret=ShowMyMessageBox_YES_NO(asStr, "是否要修改Port狀態?");
            #else
            ret=2;
            #endif // TODO(W7-UI)
            if(ret==1)
            {
                iPlaceWhichBuffer[1]=iCount;
            }
            Task=2100;
            break;
        case 2100:
            str.sprintf("Finish Unloader Action");
            ShowE84Log(str,900,2);     // golden logs this under task-tag 900 (same tag as the normal-path finish message at case 900), verbatim
            Task=1000;                                                  //Finish
            break;
        case 5000:              //Auto Recover
            // AI(W5-Final-AGV_E84) 20260711: same Click()->direct-call collapse
            // as DoE84Loader case 5000 -- see file-head JUDGMENT CALL note.
            // golden handler (AGV.cpp:1061-1064):
            //   void __fastcall TfAGV::btInitalUnLoadClick(TObject *Sender)
            //   { InitialE84UnLoaderTask(); InitialE84UnloadSensor(); }
            InitialE84UnLoaderTask();
            InitialE84UnloadSensor();
            break;
    }
}
//---------------------------------------------------------------------------
void ShowE84Log(AnsiString str, int iFunction, int iLoader)
{
    AnsiString sFileName="", sMegTime="", asE84Data, asLoader;
    AnsiString asStr="";

    // AI(W906-BA-BU0g) 20260911: getenv-overridable root. Unset -- production
    // and the real handler -- is golden's literal, byte for byte.
    // WHY: tests/test_agv_e84.cpp drives ShowE84Log/E84StatusLog for real, so
    // every ctest run appended to the LIVE machine tree
    // D:\HT9045_Log\E84DataTxt\<YYYYMM>\<MMDD>\. Measured in THIS tree on
    // 20260911: 20260911.txt and 20260911_Status.txt were both written by a
    // ctest run. The test passed throughout -- a passing test is the most
    // dangerous writer there is, because nothing suspects it.
    asE84Data = getenv("W906_E84DATA_ROOT")
              ? AnsiString(getenv("W906_E84DATA_ROOT")) + AnsiString("\\")
              : AnsiString("D:\\HT9045_Log\\E84DataTxt\\");
    GetTimeInfo();
    sFileName.sprintf("%s%04d%02d\\%02d%02d", asE84Data, SystemYear, SystemMonth, SystemMonth, SystemDate);
    if(!(DirectoryExists(sFileName)))
        ForceDirectories(sFileName);

    sFileName.sprintf("%s%04d%02d\\%02d%02d\\%04d%02d%02d.txt", asE84Data, SystemYear, SystemMonth, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate);

    sMegTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d %03d",SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);

    if(str!="")
    {
        if(iLoader==1 || iLoader==3)
            asLoader="Loader";
        else if(iLoader==2 || iLoader==4)
            asLoader="Unloader";

        // AI(W5-Final-AGV_E84) 20260711: golden computes `asStr` here (a nicely
        // spaced "Loader Task: 1 Message: ..." format) but never uses it again
        // -- the actual logged/persisted text is the differently-formatted
        // `str` reassignment on the very next line.  Faithful golden dead
        // store, preserved verbatim (golden AGV.cpp:1088-1090).
        asStr.sprintf("%s Task: %d Message: %s",asLoader ,iFunction ,str);

        str=sMegTime+","+asLoader+str+"Task"+iFunction;

        // AI(W5-Final-Integrate) 20260711: fAGV->mmE84Log (golden AGV.h:85,
        // TMemo*) -- TfAGV::mmE84Log added to FormsFacade.h this integrate
        // pass; gate lifted (was TODO(FormsFacade-wiring), golden AGV.cpp:1091).
        // AI(W906-MEMO) 20260927: TfMainMemoLines stores lines now (FormWidgets.h); golden bounds this memo at >=500 below,
        // so its 4096 cap never fires here.
        fAGV->mmE84Log->Lines->Add(str);

        // AI(W906-CommonWaveFile) 20260721: WriteDataToFile's common.h
        // declaration is un-gated and its common.cpp body is now real this
        // wave -- un-gating this call site (was blocked solely on this gate).
        WriteDataToFile(sFileName.c_str() , str.c_str());

        if(iLoader==3 || iLoader==4)
            ShowMyMessage(str);
    }

    // AI(W5-Final-Integrate) 20260711: same FormsFacade gap as above (mmE84Log)
    // -- gate lifted (golden AGV.cpp:1098-1099). `->Clear()` (not `->Lines->
    // Clear()`) matches this shim's actual shape (TfMainMemo::Clear(), not
    // TfMainMemoLines) -- same established call form as acarry.cpp's
    // fMain->meShuttle1/2->Clear() (TfMainMemo precedent).
    if(fAGV->mmE84Log->Lines->Count>=500)
        fAGV->mmE84Log->Clear();
}
//---------------------------------------------------------------------------
void E84StatusLog(AnsiString str, bool bflag)
{
    AnsiString sFileName="", sMegTime="", asE84Data;
    AnsiString asStr="";

    // AI(W906-BA-BU0g) 20260911: getenv-overridable root. Unset -- production
    // and the real handler -- is golden's literal, byte for byte.
    // WHY: tests/test_agv_e84.cpp drives ShowE84Log/E84StatusLog for real, so
    // every ctest run appended to the LIVE machine tree
    // D:\HT9045_Log\E84DataTxt\<YYYYMM>\<MMDD>\. Measured in THIS tree on
    // 20260911: 20260911.txt and 20260911_Status.txt were both written by a
    // ctest run. The test passed throughout -- a passing test is the most
    // dangerous writer there is, because nothing suspects it.
    asE84Data = getenv("W906_E84DATA_ROOT")
              ? AnsiString(getenv("W906_E84DATA_ROOT")) + AnsiString("\\")
              : AnsiString("D:\\HT9045_Log\\E84DataTxt\\");
    GetTimeInfo();
    sFileName.sprintf("%s%04d%02d\\%02d%02d", asE84Data, SystemYear, SystemMonth, SystemMonth, SystemDate);
    if(!(DirectoryExists(sFileName)))
        ForceDirectories(sFileName);

    sFileName.sprintf("%s%04d%02d\\%02d%02d\\%04d%02d%02d_Status.txt", asE84Data, SystemYear, SystemMonth, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate);
    sMegTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d %03d",SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);

    if(str!="")
    {
        if(bflag)
            asStr="Off->On";
        else
            asStr="On->Off";

        str=sMegTime+" "+str+" "+asStr;

        // AI(W906-CommonWaveFile) 20260721: same un-gate as ShowE84Log above --
        // WriteDataToFile is real as of this wave.
        WriteDataToFile(sFileName.c_str() , str.c_str());
    }
}
