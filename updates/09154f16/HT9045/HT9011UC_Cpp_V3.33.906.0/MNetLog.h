// =====================================================================================================================
//  MNetLog.h -- golden `bool MNetLog(AnsiString Message)` (golden 0618 Motor/myMN200motor.cpp:2146-2156), its own module.
//  AI(W906-W143) 20261007 (St02-E): laptop card W-143 = the !296 proposal option A (Steven 1007 10:1x「我想把 main.cpp 裡面的
//  mnetlog 獨立出來，這樣就不用大家都去 include main.h」, C++ tree only; skill hpi-mnetlog-split).  Golden declared it by hand in
//  six files (`extern bool MNetLog(AnsiString Message);`); here they include this.  Body: MNetLog.cpp.
// =====================================================================================================================
#ifndef HT9045_MNETLOG_H
#define HT9045_MNETLOG_H

#include "vclcompat/vcl_compat.h"   // AnsiString

// Appends Message (with date / time) to the MNetLog file: as9045LogPath\MNetLog\YYYY\MM\... (TMyStringList, TByDay) --
// golden D:\HT9045_Log\MNetLog.  "" writes nothing.  Always returns true (golden).
bool MNetLog(AnsiString Message);

#endif
