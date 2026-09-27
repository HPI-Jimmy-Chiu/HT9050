// =============================================================================
//  forms/fMonitor.cpp  --  TfMonitor member bodies
//
//  AI(W906-FW3-OWM1) 20260828: new file. See forms/fMonitor.h for the full
//  wave banner, GATE register (M-1..M-4) and the WHY-THIS-FILE-IS-THIN
//  explanation (10 of 23 golden bodies excluded entirely, 2 more declared
//  with a fully-gated body -- every one of the 12 reaches MVCtrl, a
//  MonitorTCPIP* this wave leaves permanently NULL because MonitorTCPIP's
//  own .cpp lives in ht9045_sm, unreachable from ht9045_forms).
// =============================================================================
#include "forms/fMonitor.h"
#include "forms/fQwertyKey.h"       // fQwertyKey / ShowQwertyKey -- the 4 MouseDown handlers
#include "cmydef.h"                 // N_INTEGER, N_PORT
#include "aHotPlateSubstrate.h"     // MyDBIProcess -- FormDestroy (verified forms->sm exception)

// AI(W906-FW3-OWM1) 20260828: golden Monitor/MonitorInterface.h/.cpp both
// free tree-wide (class name + global name), so this facade declares the
// global -- same idiom as forms/fCleaning.cpp:56 / forms/fMesSystem.cpp:597 /
// forms/fSmartDiagnostic.cpp:37. Safe at static-init: the ctor below (PORT
// SPLIT, GATE M-1) touches only its own just-`new`'d fields and explicit
// scalar literals, no other global -- in particular, it does NOT eagerly
// read D:\HT9045\system\MVData.ini (that read is LoadTCPIPParament(), called
// live from FormShow instead -- see header banner GATE M-1).
TfMonitor *fMonitor = new TfMonitor();

// -----------------------------------------------------------------------
//  ctor -- PORT SPLIT (task-mandated, static-init-ctor safety rule). golden
//  :23-45. GATE M-1 covers the MonitorTCPIP construction + InitialMemo +
//  the eager LoadTCPIPParament() call (golden :30-32); everything else is
//  field-only / self-owned-widget and stays ACTIVE.
// -----------------------------------------------------------------------
TfMonitor::TfMonitor()
{
    bShow = false;                        // golden :26
    bCommandReady = false;                // golden :27

    MVPageControl->ActivePageIndex = 0;   // golden :29

    // AI(W906-FRW-S110) 20260926: 下面 :32 那一次開機讀檔改由 FileRW/Monitor.cpp 的 FileRW_Monitor_Boot() 在 golden CreateForm(TfMonitor)
    //   （HT9045.cpp:247）的時機做（C 路 Monitor.gen.inc 的 LoadTCPIPParament；元件就是本門面這 7 個物件、sADDRESS／iPORT 也是這裡的）；
    //   :30-31／:33 的 MVCtrl（TCP）仍沒做，交 Jimmy。
#if 0
    // golden :30-32, VERBATIM -- GATE M-1. MonitorTCPIP's ctor/InitialMemo
    // live in Monitor/MonitorTCPIP.cpp (ht9045_sm), not reachable from
    // ht9045_forms; MVCtrl stays permanently NULL this wave (see field
    // default). LoadTCPIPParament() itself IS translated ACTIVE below (and
    // called live from FormShow) -- only this EAGER, static-init-time call
    // is gated (disk-I/O-at-static-init hazard, see header banner).
    MVCtrl=new MonitorTCPIP(new TCPClient(MonitorClientSocket));
    MVCtrl->InitialMemo(MonitorMemo);
    LoadTCPIPParament();
#endif

    sComData = new TStringList();         // golden :35
    iHDSpace = 0;                         // golden :36
    sComData->Clear();                    // golden :37

    for (int i = 0; i < 4; i++)           // golden :39-42
    {
        iStatus[i] = 0;
    }
    iDisconnectCount = 0;                 // golden :43
    MonitorTimer->Enabled = true;         // golden :44
}

// golden :47-55, ACTIVE verbatim.
void TfMonitor::FormShow()
{
    Width  = 610;
    Height = 515;
    Left   = (1024 - Width) / 2;
    Top    = (768 - Height) / 2;
    bShow  = true;
    LoadTCPIPParament();
}

// golden :57-72. GATE M-2: `MVCtrl->Disconnect(); ... delete MVCtrl;` --
// cross-archive method call + delete-of-incomplete-type. GATE M-3:
// `LogSoftwareOffTime(...)` -- ht9045_sm, not a verified exception. Rest
// ACTIVE, including the catch block's MyDBIProcess call (verified exception).
void TfMonitor::FormDestroy()
{
    try
    {
        MonitorTimer->Enabled = false;    // golden :61

#if 0
        // golden :62, VERBATIM -- GATE M-2. MonitorTCPIP::Disconnect is
        // ht9045_sm; MVCtrl permanently NULL this wave (GATE M-1).
        MVCtrl->Disconnect();
#endif

        sComData->Clear();                // golden :63

#if 0
        // golden :64, VERBATIM -- GATE M-2 continued. `delete` on a
        // forward-declared (incomplete) MonitorTCPIP is additionally
        // ill-formed to compile, independent of the link-boundary reason.
        delete MVCtrl;
#endif

        delete sComData;                  // golden :65
    }
    catch (...)
    {
        // golden :69 -- MyDBIProcess is one of the 4 verified
        // ht9045_forms->ht9045_sm exceptions (CMakeLists.txt:725-731).
        MyDBIProcess("Exception", "TfMonitor::FormDestroy");
    }

#if 0
    // golden :71, VERBATIM -- GATE M-3. LogSoftwareOffTime's body
    // (acarry_shims.cpp:255) is ht9045_sm and NOT one of the 4 verified
    // exceptions ("其他 sm／io／db 符號一律退出").
    LogSoftwareOffTime("TfMonitor, FormDestroy"); //Steven 20210526
#endif
}

// golden :74-77, ACTIVE verbatim. Unread `TObject *Sender, TCloseAction
// &Action` dropped (D-2) -- neither is read in golden's own body, and
// TCloseAction has zero port anywhere in this tree.
void TfMonitor::FormClose()
{
    bShow = false;
}

// golden :79-96. GATE M-1: the trailing `MVCtrl->InitialSocket(...)` --
// cross-archive method call, MVCtrl permanently NULL this wave. Every other
// line is a pure TIniFile READ (vclcompat::TIniFile::Read* never writes
// through -- see vclcompat/IniFiles.h's own "BCB6 SEMANTICS REPLICATED"
// banner) plus own-widget ->Text/->Checked assignment, so the rest stays
// ACTIVE. Called live from FormShow (golden :54), not from the ctor -- see
// header banner GATE M-1 for why the ctor does not call this eagerly.
void TfMonitor::LoadTCPIPParament()
{
    TIniFile *pFile = new TIniFile("D:\\HT9045\\system\\MVData.ini");

    sADDRESS = pFile->ReadString("Setup", "IP", "127.0.0.1");
    iPORT    = pFile->ReadInteger("Setup", "Port", 7000);

    edMVAddress->Text = sADDRESS;
    edMVPort->Text    = AnsiString(iPORT);
    cbWhenHDFullAlarm->Checked = pFile->ReadBool("Specific", "HD_Space", false);
    edLowHDSpace->Text         = AnsiString(pFile->ReadInteger("Specific", "HD_Space_Low", 0));
    edWhenHDFullPrompt->Text   = AnsiString(pFile->ReadInteger("Specific", "HD_Space_Low_Prompt", 0));
    cbAfterHandlerTrayFeedMonitor1ClosedVideo->Checked = pFile->ReadBool("Specific", "HD_Closed_Wait", false);
    edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime->Text =
        AnsiString(pFile->ReadInteger("Specific", "HD_Closed_Wait_Time", 0));

#if 0
    // golden :94, VERBATIM -- GATE M-1.
    MVCtrl->InitialSocket(sADDRESS, iPORT);
#endif
    delete pFile;
}

// golden :138-141, ACTIVE verbatim. golden's inherited TForm::Close() ->
// this facade's own permanent offline no-op (D-5).
void TfMonitor::sbMVExitClick()
{
    Close();
}

// golden :334-337, ACTIVE verbatim.
void TfMonitor::sbMVLogClearClick()
{
    MonitorMemo->Clear();
}

// golden :350-363. GATE M-4 -- predetermined name (csystem.cpp #if 0 sites).
// Whole body gated: sends the literal "SR0%d\r\n" relay command over MVCtrl
// (ht9045_sm, cross-archive AND the explicit "開／關影像串流" safety
// exclusion). The guard checks alone have no observable effect without the
// send they guard -- see header banner GATE M-4.
void TfMonitor::OpenMonitorVedio(int index)
{
#if 0
    // golden :352-362, VERBATIM -- GATE M-4.
    AnsiString aSendData;

    if(iStatus[index]==1 || bCommandReady==false)
        return;

    if(sComData->Count==0)
    {
        aSendData.sprintf("SR0%d\r\n", index);
        MVCtrl->SendCommand(aSendData);
        sComData->Add(aSendData);
    }
#endif
    (void)index;   // suppress -Wunused-parameter while the body above is gated
}

// golden :365-378. GATE M-4 -- predetermined name, same reasoning as
// OpenMonitorVedio above ("SS0%d\r\n" stop-relay command).
void TfMonitor::StopMonitorVedio(int index)
{
#if 0
    // golden :367-377, VERBATIM -- GATE M-4.
    AnsiString aSendData;

    if(iStatus[index]==0 || bCommandReady==false)
        return;

    if(sComData->Count==0)
    {
        aSendData.sprintf("SS0%d\r\n", index);
        MVCtrl->SendCommand(aSendData);
        sComData->Add(aSendData);
    }
#endif
    (void)index;   // suppress -Wunused-parameter while the body above is gated
}

// golden :412-417, ACTIVE verbatim. Unread `TMouseButton Button,
// TShiftState Shift, int X, int Y` dropped (never read in golden's own
// body); Sender IS read (cast), kept.
void TfMonitor::edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTimeMouseDown(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 3000, 0);   //JerryYang 20160622
}

// golden :419-423, ACTIVE verbatim.
void TfMonitor::edWhenHDFullPromptMouseDown(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 99, 0);   //JerryYang 20160622
}

// golden :425-429, ACTIVE verbatim.
void TfMonitor::edLowHDSpaceMouseDown(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 99, 0);   //JerryYang 20160622
}

// golden :431-435, ACTIVE verbatim.
void TfMonitor::edMVPortMouseDown(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_PORT);
}
