// =============================================================================
//  WebNoteAuth.h -- 告警框（Alert.Note）的密碼層：golden TfNote::DoPassword／DoUnlockPassword 給網頁用（WS dialog.auth）。
//
//  AI(W906-D026) 20261001 (St01)，todo D-026。本體在 WebLogin.cpp 檔尾（St02 的 :1-1419、B5 的 W906_Reauth 之後，只附加）。
//  golden（V912，cp950，唯讀）：
//    TfNote::FormShow 的密碼段        note.cpp:1496-1502（bAlarmUnlockPassWord）、:1818-1941（Level／bNeedPassWord）
//    TfNote::Start()                  note.cpp:3560（Select[] 迴圈 :3599 DoUnlockPassword、:3604 DoPassword；KeyCode==0 :3724）
//    TfNote::BtnPauseClick()          note.cpp:3866（Select[] 迴圈 :3921 / :3926；KeyCode==0 :4092）
//    TfNote::DoPassword               note.cpp:5277-5429（fSecurity->GetJamLevel，登入＝cbUserSelectChange／stOperatorClick，比等級，
//                                     有密碼本時問完一律登出成 Operator，CC_PTI 除外）
//    TfNote::DoUnlockPassword         note.cpp:5431-5445（asUnlockPassword＝C:\Windows\AlarmUnlock.ini 第一行，main.cpp:11381-11403）
//  設計（同 Q45「甲」與主畫面 auth.login，D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md）：
//    * 比對只在 C++；網頁只收集操作員打的字，經 WS（只聽 127.0.0.1）送 dialog.auth；回應、log、printf 都不含密碼。
//    * 告警信箱（Alarm-dialog-request）的 "auth" 物件照 golden 這一則會不會問（W906_NoteAuthRequestJson），
//      dialog-bridge.js 依它開登入層（Alert.Password.html），送出時叫 HTDialogHost.verifyAuth（web/page/ht9045_dialog_host.js）。
//    * dialog.auth 通過 ＝ golden 那一次按鍵的 DoUnlockPassword／DoPassword 回 true：記一張「這一則、這個按鍵（＋按鈕）」的
//      一次性通行（[W906]，golden 是同一個函式裡接著關框），緊接著的 modal.answer／dialog.response／dialog.notifyAck 用掉它。
//      不綁連線、不綁操作權杖、不綁登入的人（Steven Q64 (3) 20261001 09:4x：知道密碼的人就能解；golden 只比打的字與等級）。
//    * 沒通過、沒送 dialog.auth 就直接回答 ⇒ 等待迴圈拒絕（auth-required），框留著（golden：DoPassword 回 false → return）。
//  本標頭只用標準型別。
// =============================================================================
#ifndef WEBNOTEAUTH_H
#define WEBNOTEAUTH_H

#include <string>

// ---- 貼告警的時候（tools/wb_serve.cpp ForwardShowErrorMessage，紀錄之後、信箱之前）--------------------------------------
// golden TfNote::FormShow 的密碼段（note.cpp:1496-1502、:1818-1941）＋ShowErrorMessage :829（TempCode）／:881-886（sJamArea／sJamCode）。
// requestId＝信箱 requestId＝WS query qid；kcode==0＝通知（只有 KeyCode==0 那一臂）；motorNote＝golden ShowMotorErrorMessage 的 note
// （sJamArea／sJamCode 已由 forms/fNote_ShowError.cpp 設好，TempCode 不變）。
void        W906_NoteAuthArm(const char* requestId, const char* code, int kcode, bool motorNote);
// 信箱 request 的 "auth" 物件（沒有秘密）；golden 這一則不會問 ⇒ 回 ""（呼叫端照舊寫 required:false 那一段，位元組不變）
std::string W906_NoteAuthRequestJson(const char* requestId);

// ---- WS dialog.auth（等待迴圈裡：currentId＝qid、blocking＝true；主迴圈：currentId＝目前的通知 requestId 或 ""）---------
// authId＝指令的 tag；valueJson＝dialog-bridge 的 Dialog-auth-verify 物件（JSON 字串；密碼在 credentials.password）。
// 回 true＝有處理（*reply＝結果 JSON：accepted…，不含密碼）；false＝拒絕（*reply＝原因，英文一行）。
bool        W906_NoteAuthVerify(const std::string& authId, const std::string& valueJson,
                                const std::string& currentId, bool blocking, std::string* reply);

// ---- 關框之前的閘（回 true＝可以關）----------------------------------------------------------------------------------------
// 網頁回答（modal.answer／dialog.response）：k＝選的 K、pressed＝BtnStart／BtnPause／""。false 時 *why＝拒絕原因。
bool        W906_NoteAuthAnswerGate(const char* requestId, int k, const std::string& pressed, std::string* why);
// 實體面板鍵（W906_AlarmIoAnswer 選到的 K）：golden 會跳登入框的時候回 false（[W906] 只能在畫面上輸入，框留著；畫面那張通行不動它）
bool        W906_NoteAuthIoGate(const char* requestId, int k, const std::string& pressed);
// 通知（dialog.notifyAck）：golden BtnPauseClick 的 KeyCode==0 臂（note.cpp:4084-4095）
bool        W906_NoteAuthNoticeGate(const std::string& requestId, std::string* why);

// ---- 測試用 --------------------------------------------------------------------------------------------------------------
void        W906_NoteAuthResetForTest();   // 清掉每一則的狀態、O16／O17 計數、AlarmUnlock.ini 讀過的旗標（不動 asUnlockPassword 以外的全域）
std::string W906_NoteAuthStateJson(const char* requestId);   // 那一則的 golden 旗標（Level、bNeedPassWord…；沒有秘密）

// ---- AI(W906-D034) 20261002 (St01)：todo D-034 —— SpecialPanel 密碼（golden TfNote::PanSpecialNoteClick，V912 note.cpp:5489-5507）--------
// golden FormShow :1574-1616 依 [I] Index 掉料（JAM0303～0306／0314／0315）或 Tester Time Up（WAR07352）與 SpecialErrNote.ini
// （[SUCK] TestSuck＝1、[PASSWORD] Pwd）鎖住告警框：每一個按鍵都直接 return（BtnSkipClick :2867、ScanKey :2908-2913 除了 Alarm Reset、
// BtnStartClick :3858、BtnPauseClick :3870、BtnResetClick :5261），點 PanSpecialNote 輸入那個密碼才解（全域、不是每一則各一份）。
// 網頁：上面同一套（信箱 auth.kind "special-note"、dialog.auth 先比特殊密碼、三個閘先擋）；這一支給實體面板鍵用。
bool        W906_SpecialPanelLocked();     // golden bErrPan_err==true && Pwd!=""（密碼本身不外流）

#endif // WEBNOTEAUTH_H
