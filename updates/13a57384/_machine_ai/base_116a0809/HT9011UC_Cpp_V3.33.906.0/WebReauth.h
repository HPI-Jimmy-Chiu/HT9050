// =============================================================================
//  WebReauth.h -- Q45「甲」重新登入（golden TfSetup::DoPassword／TfConfiguration::DoPassword）給網頁存檔用。
//
//  AI(W906-Q45-B5) 20260930 (St01).  本體在 WebLogin.cpp 檔尾（St02 的 :1-1419 之後，只附加）。
//  設計：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §2.0、§3.2（Steven Q45「按照你的建議執行」
//  子題 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A；20260929「請按照 bcb 的邏輯處理」）。
//  golden（V912，cp950，唯讀）：
//    TfSetup::DoPassword          cSetUp.cpp:4325-4362        （存檔鈕 sbUpdateClick :3573-3606 呼叫；關 RTC／OCR）
//    TfConfiguration::DoPassword  cConfiguration.cpp:6482-6529（cbM01Click :6531-6544 呼叫；M01 監控功能 16 格）
//    有密碼本（FileExists(pwPath)）→ TfMain::cbUserSelectChange 的密碼本分支（WebLogin.cpp WebLogin_BookCompare，St02）
//    沒有密碼本                    → TfMain::stOperatorClick（main.cpp:13193-13324；SetUp 問的時候走 :13283-13301 那一臂）
//
//  存檔訊息（WS editlist.save 的 value，跟 widgets 並列，**不可以**放進 widgets）：
//      "reauth": { "point": "rtcOff"|"ocrOff"|"m01", "userId": "..."（密碼本模式）, "password": "..." }
//      "reauth": { "point": "...", "cancelled": true }          ← 登入框按取消＝golden 空白帳密＝錯（Q45-4＝A）
//  tools/wb_serve.cpp 的 editlist.save 一解析就呼叫 W906_ReauthTake（拿出來、把 JSON 裡的字清成 0、存進本檔的暫存），
//  存檔流程裡的 golden DoPassword 才用它；存完 W906_ReauthAck 把結果（不含密碼）併進回應，W906_ReauthClear 清掉暫存。
//  密碼不印、不記、不回給網頁（RULINGS S41／S55；Q45-1＝A：原字只走本機 127.0.0.1，同主畫面 auth.login）。
//  本標頭只用標準型別（可以接在 FileRW 產生檔的 #define 之後 include）。
// =============================================================================
#ifndef WEBREAUTH_H
#define WEBREAUTH_H

#include <functional>
#include <string>
#include <vector>

struct cJSON;   // Public/cJSON.h（typedef struct cJSON cJSON）

// 頁面按存檔時帶來的答案（W906_Reauth 的輸入）
struct W906ReauthAnswer {
    bool        present = false;     // 這次存檔有帶 reauth
    bool        cancelled = false;   // 登入框按了取消
    std::string point;               // "rtcOff"／"ocrOff"／"m01"
    std::string userId;              // 密碼本模式的帳號（UTF-8）；下拉選單模式沒有
    std::string password;            // UTF-8；W906_Reauth 不會複製到結果、不印
};

// golden DoPassword 的結果（W906_ReauthAck 轉成 editlist.save 回應的 "reauth"；沒有密碼）
struct W906ReauthResult {
    bool        handled = true;      // false＝golden 這裡會問，但這次存檔沒帶答案 → 呼叫端照舊（SetUp 視同密碼錯、Configuration 整次拒存）
    bool        answered = false;    // 這次存檔有帶 reauth
    bool        asked = false;       // golden 這一次會跳登入框
    bool        passed = true;       // golden DoPassword 的回傳值（bFlag）
    bool        cancelled = false;
    bool        loggedOut = false;   // Configuration 版問完登出成 Operator（有密碼本時，golden :6518-6525）
    std::string point;
    std::string mode;                // "book"（有密碼本）／"select"（下拉選單）＝golden bTechComExist
    int         levelItem = 0;       // 權限表項目：SetUp 37（Tools - CCD）、Configuration 92（Contact - Contact parameter）
    int         required = 0;        // LevelSet.AccessLevel[levelItem]
    int         levelBefore = 0;     // 問之前的 AccessLevel
    int         level = 0;           // 問完（含登出）的 AccessLevel
    std::string alarm;               // 密碼本比對錯：golden ShowErrorMessage("WAR1677")／讀不到密碼本 "WAR1681" —— 網頁發不停機告警
    std::string reason;              // 英文一行（不含帳號、密碼）
    std::string golden;              // golden 出處
    std::vector<std::string> reverted;   // golden 失敗時改回的勾選框（SetUp :3578-3581、Configuration :6538-6541）
    std::string login;               // 問完的登入狀態（WebLogin_StateJson）
};

// ---- 核心：golden 兩支 DoPassword --------------------------------------------------------------------------
// tag：WS editlist.* 的 tag —— "TestIF_File_SetUp"（SetUp 版，第 37 項、沒有「第 92 項＝0」那種捷徑、不登出）或 "IniConfig"
// （Configuration 版，第 92 項、＝0 直接過、有密碼本時問完一律登出成 Operator）。setupNeedPassword：SetUp 版才用，＝golden
// fSetup->bNeedPassword（stOperatorClick 那一臂的條件，main.cpp:13283）。回 golden 的 bFlag；r->handled=false 時回 false、狀態沒動。
bool W906_Reauth(const char* tag, bool setupNeedPassword, const W906ReauthAnswer& a, W906ReauthResult* r);

// ---- tools/wb_serve.cpp editlist.save 用 --------------------------------------------------------------------
// 從 value 的 JSON（root）拿出 "reauth"（在 widgets 旁邊）存進暫存、把 JSON 裡的字清成 0；widgets 裡面也找得到 "reauth" 就拿掉並拒絕。
// 回 ""＝可以繼續；否則是拒絕原因（英文一行，不含帳號、密碼）。每次呼叫先清掉上一次的暫存。
std::string W906_ReauthTake(cJSON* root, const std::string& tag);
void        W906_ReauthAck(std::string* ack);   // 存檔成功（200）時：有帶 reauth 或 golden 問過 → 在回應物件開頭加 "reauth":{...}
void        W906_ReauthClear();                 // 清掉暫存（密碼先清成 0）；editlist.save 一律在結束時呼叫
bool        W906_ReauthHasAnswer(const char* tag);   // 這次存檔帶了這一頁的 reauth（還沒用掉）

// ---- 存檔流程裡的呼叫點 ------------------------------------------------------------------------------------
// FileRW/TestIF_File_SetUp.gen.inc SU_DoPassword（golden cSetUp.cpp:3576 DoPassword()）：golden TfSetup::DoPassword；
//   *bHandled=false ＝ 沒帶答案 → 呼叫端照舊 filerw::ELPasswordRefused。後四個 bool 只用來記「golden 會改回哪幾格」（:3578-3581）。
bool W906_ReauthSetupDoPassword(bool bNeedPassword, bool bOCRNeedPassword, bool bRtcChecked, bool bOcrChecked, bool* bHandled);
// FileRW/IniConfig.cpp（golden cbM01Click :6531-6544）：changed＝這次改過的 M01 格子；失敗時逐格呼叫 revert（golden :6540
//   ptr->Checked=!ptr->Checked）。沒有改過任何格子就不要呼叫（golden 沒點就沒有 DoPassword）。
bool W906_ReauthConfigM01(const std::vector<std::string>& changed, const std::function<void(const std::string&)>& revert,
                          bool* bHandled);

// ---- editlist.get 的 extra.auth（開頁；C++→網頁，沒有秘密）----------------------------------------------------
// SetUp：rtcArmed／ocrArmed＝golden sbUpdateClick 那兩條件扣掉勾選值（沒被 [RTC Lock by file] 鎖、gbRTC／gbOcr 可用；
//   REAL_TIME_CCD 由本函式自己看），rtcAtOpen／ocrAtOpen＝golden bNeedPassword／bOCRNeedPassword（開頁時勾著）。
// IniConfig：四個 bool 不用；回 m01（重新登入）＋廠商密碼三點＋SG_PW 一點（armed:false，網頁版不提供）。
std::string W906_ReauthOpenJson(const char* tag, bool rtcArmed, bool rtcAtOpen, bool ocrArmed, bool ocrAtOpen);
// 一個點的控制項名稱（golden 元件名＝頁面 id）："m01" 16 格、"rtcOff"、"ocrOff"、"c12"、"a27"、"n07_5"、"sgpw"
const char* const* W906_ReauthControls(const char* point, int* n);

#endif // WEBREAUTH_H
