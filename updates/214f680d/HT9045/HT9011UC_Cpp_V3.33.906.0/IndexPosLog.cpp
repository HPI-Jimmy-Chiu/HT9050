// =====================================================================================================================
//  IndexPosLog.cpp -- golden LogIndexMaxMinPos (golden 0618 cpublic.cpp:1582-1599) and TfMain::AddIndexPosLog (golden 0618
//  main.cpp:33634-33664), their own module.
//  AI(W906-W150) 20261008 (St02-E): W-150 LOG-SPLIT slice 2 (skill hpi-mnetlog-split s7 item 4, s8 L5).  The golden text at
//  cpublic.cpp:1818-1835 stays gated as the reference copy (its old reason, "fMain->slIndexYMaxMinShift / AddIndexPosLog are
//  missing facade members", is half stale: slIndexYMaxMinShift is on fMain since AI(W906-LOGOBJ-W7) 20260927; AddIndexPosLog
//  is this file's W906_AddIndexPosLog).  ht9045_sm, because LogIndexMaxMinPos calls InitialMaxMinValue (ht9045_motor).
//  * slIndexYMaxMinShift stays golden's TfMain member, created by W906_CreateLogObjects (LogObjects.cpp,
//    as9045LogPath+"\\IndexPos" = golden 0618 main.cpp:1555 D:\HT9045_Log\IndexPos), reached through W906_IndexYMaxMinShiftLogObj().
//  * golden MemoIndexPosLog is a TMemo on the main form's "IndexArmYPos" tab (golden 0618 main.dfm:15626).  It is both the
//    display and the save buffer (SaveToFile on bSave or past 1024 lines), so the buffer and the save stay; only the display
//    goes (TODO(W906-LOGVIEW) below).
//  * The values: iMaxCommandY1.. (cmydef.cpp:5370) are fed by the machine-side code -- RecordIndexPosition is gated at
//    Motor/mymotor.cpp:2646 and InitialMaxMinValue / EncoderTeachingMaxMinCount are empty there (:2845-2846); only the Galil
//    path (Motor/myGALILmotor.cpp:4395) sets them.  Until those land the lines carry 0 -- written as golden writes them.
//  Live callers: cStateRecord.cpp:1456 (golden 0618 main.cpp:26675).  Not yet: FormClose :11929 LogIndexMaxMinPos("Program
//  closed") (FileRW/MainClose.cpp SdAdd "12446" stays "missing", a later slice); asendic_Loader.cpp:273 / :303 keeps its
//  TU-local stub (machine side).
// =====================================================================================================================
#include "IndexPosLog.h"
#include "cpublic.h"                // LogIndexMaxMinPos declaration (cpublic.h:327), GetTimeInfo
#include "LogObjects.h"             // W906_IndexYMaxMinShiftLogObj
#include "Public/MyStringList.h"    // TMyStringList::AddTextWithDateTime / MySaveToFile
#include "cmydef.h"                 // SystemYear.. (cmydef_core.h:224-225), iMaxCommandY1.. (cmydef_rt.h:2489)
#include "common.h"                 // as9045LogPath (common.cpp:240, seam W906_HT9045LOG_ROOT), MyForceDirectories (common.h:341)
#include "Motor/mymotor.h"          // InitialMaxMinValue (Motor/mymotor.h:475)

namespace {
TStringList* IndexPosMemoLines()                                                // stands in for golden fMain->MemoIndexPosLog->Lines
{
    static TStringList* const p = new TStringList();
    return p;
}
}  // namespace

int W906_IndexPosLogLineCount() { return IndexPosMemoLines()->Count; }

//------------------------------------------------------------------------------
void W906_AddIndexPosLog(AnsiString Msg, bool bSave)                            // golden 0618 main.cpp:33634 void TfMain::AddIndexPosLog(AnsiString Msg, bool bSave)
{
    AnsiString StrIndexLog, sFileName;
    TStringList* const Lines = IndexPosMemoLines();                             // golden fMain->MemoIndexPosLog->Lines

    StrIndexLog.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d : %s",
                        SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Msg);

    sFileName.sprintf("%s\\IndexPos\\%04d\\%02d_IndexPosLog\\", as9045LogPath.c_str(), SystemYear, SystemMonth);   // golden literal "D:\\HT9045_Log\\IndexPos\\..." -> as9045LogPath (same value unless the ctest seam W906_HT9045LOG_ROOT is set)
    MyForceDirectories(sFileName);

    if(bSave)
    {
        sFileName.sprintf("%s\\IndexPos\\%04d\\%02d_IndexPosLog\\Z1UpZ2Down_%04d%02d%02d%02d%02d%02d.logs", as9045LogPath.c_str(),
                            SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

        Lines->Add(StrIndexLog);
        Lines->SaveToFile(sFileName);
        Lines->Clear();
    }
    else
    {
        if(Lines->Count>1024)
        {
            sFileName.sprintf("%s\\IndexPos\\%04d\\%02d_IndexPosLog\\Z1UpZ2Down_%04d%02d%02d%02d%02d%02d.logs", as9045LogPath.c_str(),
                                SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            Lines->SaveToFile(sFileName);
            Lines->Clear();
        }
        Lines->Add(StrIndexLog);
    }
    // TODO(W906-LOGVIEW): golden 顯示在 fMain->MemoIndexPosLog（主畫面 IndexArmYPos 頁）；V906 先不顯示，之後換位置顯示（skill hpi-mnetlog-split §8 L5）。
    //   這個框同時是存檔緩衝（L4 同）：上面的緩衝與存檔照 golden 留著，只拿掉顯示。
}
//------------------------------------------------------------------------------
void LogIndexMaxMinPos(AnsiString str)                                          //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，記錄並存檔，一盤tray記錄一次
{
    AnsiString Message="", StrPosRecord="";
    TMyStringList* const sl = W906_IndexYMaxMinShiftLogObj();                   // golden fMain->slIndexYMaxMinShift; null before W906_CreateLogObjects (ctest)

    GetTimeInfo();

    Message.sprintf("Index Y1: MaxCMDY1:%d,MinCMDY1:%d,MaxY1F:%d,MinY1F:%d,MaxY1M:%d,MinY1M:%d", iMaxCommandY1, iMinCommandY1, iMaxTeachY1F, iMinTeachY1F, iMaxTeachY1M, iMinTeachY1M);
    if(sl != nullptr) sl->AddTextWithDateTime(Message);                         // golden :1589
    W906_AddIndexPosLog(Message);                                               // golden :1590 fMain->AddIndexPosLog(Message);

    Message.sprintf("Index Y2: MaxCMDY2:%d,MinCMDY2:%d,MaxY2M:%d,MinY2M:%d,MaxY2R:%d,MinY2R:%d", iMaxCommandY2, iMinCommandY2, iMaxTeachY2M, iMinTeachY2M, iMaxTeachY2R, iMinTeachY2R);
    if(sl != nullptr) sl->AddTextWithDateTime(Message);                         // golden :1593
    W906_AddIndexPosLog(Message);                                               // golden :1594 fMain->AddIndexPosLog(Message);;

    if(sl != nullptr) sl->MySaveToFile();                                       // golden :1596

    InitialMaxMinValue(str);                                                    //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，歸零
}
