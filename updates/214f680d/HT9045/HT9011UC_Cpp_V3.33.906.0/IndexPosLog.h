// =====================================================================================================================
//  IndexPosLog.h -- the Index Y position log: golden TfMain::AddIndexPosLog (golden 0618 main.cpp:33634-33664) as a free
//  function.  LogIndexMaxMinPos (golden 0618 cpublic.cpp:1582-1599) keeps its cpublic.h:327 declaration; both bodies are in
//  IndexPosLog.cpp.  AI(W906-W150) 20261008 (St02-E): W-150 LOG-SPLIT slice 2 (skill hpi-mnetlog-split s7 / s8 L5).
//  Golden callers outside main.cpp write `fMain->AddIndexPosLog(...)`; here they include this and call W906_AddIndexPosLog.
// =====================================================================================================================
#ifndef HT9045_INDEXPOSLOG_H
#define HT9045_INDEXPOSLOG_H

#include "vclcompat/vcl_compat.h"   // AnsiString

// Adds "YYYY-MM-DD, hh:mm:ss.mmm : Msg" (the caller's last GetTimeInfo, like golden) to the buffer that stands in for
// golden fMain->MemoIndexPosLog.  bSave, or more than 1024 lines already buffered, writes the buffer to
// as9045LogPath\IndexPos\YYYY\MM_IndexPosLog\Z1UpZ2Down_YYYYMMDDhhmmss.logs and clears it (golden D:\HT9045_Log\IndexPos\...).
void W906_AddIndexPosLog(AnsiString Msg, bool bSave = false);
int  W906_IndexPosLogLineCount();   // lines in that buffer now (ctest)

#endif
