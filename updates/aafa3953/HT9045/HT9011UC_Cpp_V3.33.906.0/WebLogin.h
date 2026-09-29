// =============================================================================
//  WebLogin.h -- 主畫面登入（golden TfMain::cbUserSelectChange／stOperatorClick／btLoginClick）給網頁用。
//  Steven 20260924.  說明見 WebLogin.cpp。
//
//  WS 命令（tools/wb_serve.cpp）：
//    auth.mode                      → WebLogin_StateJson()
//    auth.select  tag=0..3 [value=密碼]  → WebLogin_Select（下拉選單模式）；回 needPassword 時頁面開小鍵盤再送
//    auth.login   tag=帳號 value=密碼     → 既有（密碼本模式，WebAuthVerify）；先檢 WebLogin_LoginAllowed
//    auth.logout                    → WebLogin_Logout（btLoginClick 的 Logout 半邊）
// =============================================================================
#ifndef WEBLOGIN_H
#define WEBLOGIN_H

#include <string>

#include "vclcompat/vcl_compat.h"   // AnsiString

enum { WEBLOGIN_OK = 0, WEBLOGIN_NEED_PASSWORD = 1, WEBLOGIN_WRONG_MODE = 2, WEBLOGIN_BAD_ARG = 3,
       WEBLOGIN_BAD_CREDENTIALS = 4, WEBLOGIN_NO_BOOK = 5,
       WEBLOGIN_BAD_PASSWORD = 6,       // 下拉選單：密碼不對，狀態已照 golden 更新（回到 Operator）；頁面發不停機告警
       WEBLOGIN_ALREADY_IN = 7 };       // 密碼本：已登入（golden btLogin Caption=Logout → 按下是登出，不是再登入）

void        WebLogin_Boot();        // 開機一次：InitialSuperVisorPassword、szSupervisor、pwPath、SOFT_SIMULTE／DEBUG 預設 HonPrec
bool        WebLogin_UsesBook();    // golden FormShow :10863 FileExists(pwPath) || bUseLoginDatToSetLevel
int         WebLogin_Select(int itemIndex, bool hasPassword, const AnsiString& password, std::string* msg);
// 密碼本模式：golden cbUserSelectChange 的密碼本分支（main.cpp:14968-15418）。bookOverride 非空時用它當
// 文字密碼本（測試用 W906_PWBOOK_PATH）。回 WEBLOGIN_OK 或 WEBLOGIN_BAD_CREDENTIALS（golden 在這裡會
// ShowErrorMessage("WAR1677") —— 網頁改成不停機告警，由頁面發，C++ 不呼叫 ShowErrorMessage）。
int         WebLogin_BookLogin(const AnsiString& user, const AnsiString& password, const AnsiString& bookOverride,
                               std::string* msg);
bool        WebLogin_LoginAllowed();
void        WebLogin_LoggedIn();
bool        WebLogin_Logout(std::string* msg);
std::string WebLogin_StateJson();
void        W906_WebLoginForceOperator();   // AI(W906-D4) 20260928 (St02-E): golden ChangeTesterConnect Off-Line -> On-Line 強制回 Operator（906_0625_Steven main.cpp:12104-12131）

#endif // WEBLOGIN_H
