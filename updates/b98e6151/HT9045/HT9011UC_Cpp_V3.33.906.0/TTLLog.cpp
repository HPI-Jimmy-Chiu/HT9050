// =====================================================================================================================
//  TTLLog.cpp -- golden `void TTLLog(AnsiString Message)` (golden 0618 cpublic.cpp:489-512), its own module.
//  AI(W906-W150) 20261008 (St02-E): W-150 LOG-SPLIT slice 2 (skill hpi-mnetlog-split s7 item 4).  The golden text at
//  cpublic.cpp:669-692 stays gated as the reference copy; its old reason ("no fMain->slTTLLog facade member") went stale with
//  AI(W906-LOGOBJ-W7) 20260927.  The body lives here, not in cpublic.cpp, because it reads SW[] (myswitch.cpp, ht9045_io),
//  which ht9045_globals cannot see; this file is in ht9045_sm (CMakeLists.txt, next to MNetLog.cpp).
//  The log object stays golden's TfMain member fMain->slTTLLog, created by W906_CreateLogObjects (LogObjects.cpp,
//  as9045LogPath+"\\TTL_Signal_LOG" = golden 0618 main.cpp:1526 D:\HT9045_Log\TTL_Signal_LOG), reached through
//  W906_TTLLogObj() so this file does not include forms/fMain.h.  Golden shows it nowhere on screen (file only), so there is
//  no TODO(W906-LOGVIEW).
//  Live callers (golden 0618): main.cpp:24373 InitDIOStstus (cDIOStatus.cpp, #ifndef SOFT_SIMULTE), main.cpp:25493
//  Timer3Timer (MainTimer3.cpp), and (W-150 last slice, AI(W906-W150) 20261010 (St02-E)) FormClose TTLLog("Close") = golden 913
//  main.cpp:12307-12309 through W906_TTLLogClose_St02 at the end of this file (FileRW/MainClose.cpp:1037, SdAdd "12160");
//  atester.cpp's eight stay inside T17 (direct TTL card, not ported).
//  Note: TTL_MODE is 0 (cmydef_core.h:60), the default of TestIF.iTestType.
// =====================================================================================================================
#include "cpublic.h"                // TTLLog declaration (cpublic.h:42)
#include "LogObjects.h"             // W906_TTLLogObj
#include "Public/MyStringList.h"    // TMyStringList::AddTextWithDateTime
#include "cprod.h"                  // TestIF (.iTestType)
#include "cmydef.h"                 // TTL_MODE (cmydef_core.h:60), SwClear0..3 / SwStart0..3 / SwDut0..3 (cmydef_io.h)
#include "myswitch.h"               // SW[], TMySwitch::Status
#include "MachineType.h"            // SOFT_SIMULTE (MachineType.h:48; the ship build defines W906_NO_SOFT_SIMULTE)

void TTLLog(AnsiString Message)                                                 //Steven 20161115 : TTL Log改新版存檔
{
    AnsiString Str;

    if(TestIF.iTestType==TTL_MODE || Message=="Close")
    {
        Str.sprintf("%s, ", Message);

        Str+=(SW[SwClear0].Status())?"1, ":"0, ";
        Str+=(SW[SwClear1].Status())?"1, ":"0, ";
        Str+=(SW[SwClear2].Status())?"1, ":"0, ";
        Str+=(SW[SwClear3].Status())?"1, ":"0, ";
        Str+=(SW[SwStart0].Status())?"1, ":"0, ";
        Str+=(SW[SwStart1].Status())?"1, ":"0, ";
        Str+=(SW[SwStart2].Status())?"1, ":"0, ";
        Str+=(SW[SwStart3].Status())?"1, ":"0, ";
        Str+=(SW[SwDut0  ].Status())?"1, ":"0, ";
        Str+=(SW[SwDut1  ].Status())?"1, ":"0, ";
        Str+=(SW[SwDut2  ].Status())?"1, ":"0, ";
        Str+=(SW[SwDut3  ].Status())?"1 " :"0 ";

        TMyStringList* const sl = W906_TTLLogObj();                             // golden fMain->slTTLLog; null before W906_CreateLogObjects (ctest)
        if(sl != nullptr)
            sl->AddTextWithDateTime(Str);                                       // golden :510
    }
}

// AI(W906-W150) 20261010 (St02-E), W-150 last slice: golden 913 main.cpp:12307-12309 in TfMain::FormClose, after HeaterLog("Close")
//   :12306 --   #ifndef SOFT_SIMULTE / TTLLog("Close"); / #endif   ("Close" is written whatever TestIF.iTestType is, :27 above).
//   Called from FileRW/MainClose.cpp:1037 (St01's shutdown sequence, which lists every golden FormClose line); bExecute=false is
//   that table's listing pass, so nothing is written then.  Returns whether this build logs it (false = SIM build, as golden).
bool W906_TTLLogClose_St02(bool bExecute)
{
#ifndef SOFT_SIMULTE
    if(bExecute)
        TTLLog("Close");                                                        //Steven 20151123 : Log for TTL
    return true;
#else
    (void)bExecute;
    return false;
#endif
}
