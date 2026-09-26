// AI(W906-CHGLOG) 20260927: see common_ChangeLog.cpp's banner.
#ifndef COMMON_CHANGELOG_H
#define COMMON_CHANGELOG_H

#include "vclcompat/vcl_compat.h"

AnsiString __fastcall TempChangeLog(AnsiString Group, AnsiString Name);         // golden common.h:263 (common.cpp:1802-2037)

// golden WriteIniData change-log blocks; common.cpp calls them through the W906_ChangeLogHook_* pointers (common.h EOF)
void W906_ChangeLog_Bool(AnsiString FileName, AnsiString Group, AnsiString Name, bool ret, bool bValue, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_Int(AnsiString FileName, AnsiString Group, AnsiString Name, int ret, int Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_ULong(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long ret, unsigned long Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_Double(AnsiString FileName, AnsiString Group, AnsiString Name, double ret, double Value, AnsiString Str, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);  // Str = golden's "%0.4f" of Value
void W906_ChangeLog_Str(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString ret, AnsiString Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);

// Points the five common.h hooks at the functions above (wb_serve boot, tools/wb_serve.cpp:3858).
void W906_InstallChangeLogHooks();

#endif // COMMON_CHANGELOG_H
