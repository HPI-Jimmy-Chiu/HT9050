// =====================================================================================================================
//  MNetLog.cpp -- golden `bool MNetLog(AnsiString Message)` (golden 0618 Motor/myMN200motor.cpp:2146-2156).
//  AI(W906-W143) 20261007 (St02-E): moved here from Motor/myMN200motor.cpp (its old place keeps the text, commented out, with
//  a pointer here).  The log object stays golden's TfMain member fMain->slMNetLog, created by W906_CreateLogObjects
//  (LogObjects.cpp, as9045LogPath+"\\MNetLog") -- this file reaches it through W906_MNetLogObj() (LogObjects.cpp EOF), so it
//  does not include forms/fMain.h.  The body's old gate (myMN200motor.cpp GATE (g), "fMain.h has no slMNetLog") was stale:
//  forms/fMain.h has had slMNetLog since AI(W906-LOGOBJ-W7) 20260927; until now NOTHING was written to the MNetLog file.
//  ht9045_sm (next to Public/MyStringList.cpp, TMyStringList).
// =====================================================================================================================
#include "MNetLog.h"
#include "LogObjects.h"             // W906_MNetLogObj
#include "Public/MyStringList.h"    // TMyStringList::AddTextWithDateTime

bool MNetLog(AnsiString Message)                                                //Steven 20161115 : MNet Log改新版存檔
{
    if(Message!="")
    {
        TMyStringList* const sl = W906_MNetLogObj();                            // golden fMain->slMNetLog; null before W906_CreateLogObjects (ctest)
        if(sl != nullptr)
            sl->AddTextWithDateTime(Message);                                   // golden :2150
        // TODO(W906-LOGVIEW): golden 顯示在 fMain->mmoMNet（主畫面）；V906 先不顯示，之後換位置顯示（skill hpi-mnetlog-split §8 L1）
        //   golden :2151-2153  if(fMain->mmoMNet->Lines->Count>5000) fMain->mmoMNet->Clear();  fMain->mmoMNet->Lines->Add(Message);
    }
    return true;
}
