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
//  Timer3Timer (MainTimer3.cpp).  Not yet: FormClose :11642-11644 TTLLog("Close") (FileRW/MainClose.cpp SdAdd "12160" stays
//  "missing", a later slice); atester.cpp's eight stay inside T17 (direct TTL card, not ported).
//  Note: TTL_MODE is 0 (cmydef_core.h:60), the default of TestIF.iTestType.
// =====================================================================================================================
#include "cpublic.h"                // TTLLog declaration (cpublic.h:42)
#include "LogObjects.h"             // W906_TTLLogObj
#include "Public/MyStringList.h"    // TMyStringList::AddTextWithDateTime
#include "cprod.h"                  // TestIF (.iTestType)
#include "cmydef.h"                 // TTL_MODE (cmydef_core.h:60), SwClear0..3 / SwStart0..3 / SwDut0..3 (cmydef_io.h)
#include "myswitch.h"               // SW[], TMySwitch::Status

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
