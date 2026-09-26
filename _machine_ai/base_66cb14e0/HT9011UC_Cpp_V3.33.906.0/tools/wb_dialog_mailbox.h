// ===========================================================================
//  tools/wb_dialog_mailbox.h
//
//  AI(W906-Q30-8) 20260922：把警報對話框接到同事那一套現成的 HMI。
//
//  使用者 20260922 裁決「依據建議甲，**先優先處理，不安排在晚上**」。
//
//  ## 為什麼是「請求走檔案、回應走 WebSocket」這個混血
//
//  原本賣給使用者的說法是「甲＝檔案信箱，他的頁面一行都不用改」。
//  查證之後那句話**只有請求方向成立**：
//
//    `page/dialog-bridge.js:319-330` 的 `submit()` 有四段 fallback，
//    在量產的 release kiosk（`file:` 協定）下**全部踩空**：
//      1. window.HTDialogHost.submitResponse  -- 沒有人設它（本樹 0 命中）
//      2. window.HTJsonWriter                 -- 只有 debug 模式，且每次開機
//                                                要操作員手動授權資料夾
//      3. chrome.webview.postMessage          -- 只有 WebView2 宿主有
//      4. postMessage(..., '*') 然後 **reject**
//
//    ⇒ 今天的實際行為：框會跳出來、操作員按得下去，然後 submit() reject
//      —— **框不關、沒有檔被寫、C++ 永遠等**。正是使用者要避免的 hang up。
//
//  ⇒ 請求方向走檔案（它天生滿足使用者的要求，見下一段），
//    回應方向走我們既有的 WebSocket（避開那四段死路，也避開檔案競態）。
//    代價是 `background.html` 多一行 `<script>` 載我們的 `ht9045_dialog_host.js`
//    —— 與 Q27 的 Lot Start 完全相同的做法。
//
//  ## ★ 使用者要的性質，檔案這一側天生就有
//
//  使用者原話：「出現異常時候要有畫面可以處理 retry/skip，**如果不小心關閉
//  網頁又開啟，也要跳出**，避免沒辦法解除導致 hang up」。
//
//  `dialog-bridge.js:7-8` 的 `channels[].lastSeq` 是**記憶體狀態**，
//  瀏覽器一重整就歸 0；而 `:402` 的條件是
//  `state === 'pending' && requestId && seq > lastSeq`。
//  ⇒ 只要 request 檔還是 pending，**重開就會再彈一次**。廣播做不到這件事。
//
//  ⚠⚠ 但它的另一半是義務：**C++ 收到回應之後必須把 request 檔改回
//     `state:"idle"`**，否則操作員按 F5 會讓同一筆再彈一次，
//     而 C++ 會收到第二份同 requestId 的回應。見 `DialogMailboxRetire()`。
//
//  ## ⚠ 兩個檔，不是一個
//
//  `dialog-bridge.js:22-43 loadFresh()` 依協定分兩條路：
//    http  -> fetch("JSON/<name>.json?_=<ts>")
//    file: -> <script src="JSON/js/<name>.js?_=<ts>">，讀 window.__HT9045_DATA__[name]
//
//  而量產啟動器 `web/HT9045_Release.cmd` 用的是 **file:** 協定
//  ⇒ **今天瀏覽器只讀 .js 墊片，完全不讀 .json**。
//    只寫 .json 的實作會 100% 沒反應，而且沒有任何錯誤訊息。
//  ⇒ 兩個都寫。順序**先 .json 再 .js** —— .js 是 file: 模式的觸發邊緣，
//    先寫它會讓瀏覽器有機會讀到還沒更新的 .json（雖然今天它不讀，
//    但哪天改走 http 就會變成競態）。
//
//  ⚠ 墊片是 **CRLF**、.json 是 **LF**（實測既有檔）。不要用同一支 writer
//    「統一」它們的 EOL。兩者都**不可以有 BOM**（契約 transport.jsonEncoding
//    寫明 "UTF-8 without BOM"；墊片有 BOM 會被當語法錯，而且是靜默失敗）。
//
//  ## 目錄：是 web\JSON\，不是 D:\HT9045\JSON\
//
//  同事的 `JSON-Simulator/README.md` 與 `JsonBridge.cs:41-43` 寫死
//  `D:\HT9045\JSON\` —— **那個目錄不存在**。實際的執行期根目錄是
//  `D:\HT9045\web`（`tools/websync/sync_web.py` 檔頭的 20260911 裁決，
//  `tools/wb_serve.cpp:1841` 的 root 也是它）。
//  寫錯目錄的症狀：檔案乖乖產生、瀏覽器什麼都沒看到。
// ===========================================================================
#ifndef W906_WB_DIALOG_MAILBOX_H
#define W906_WB_DIALOG_MAILBOX_H

#include <string>
#include <cstdio>
#include <windows.h>

namespace w906dlg {

// 執行期根目錄底下的信箱。與 wb_serve 的 root 同源，不要各自寫死。
inline std::string MailboxDir(const std::string& webRoot)
{
    return webRoot + "\\JSON\\runtime";   // AI(W906-MAILBOX-RT) 20260924: 執行期信箱搬出版控（使用者裁決，照 Steven 0923 22:39 信）。HTTP 讀 /JSON/<x> 先找 /JSON/runtime/<x>（HttpStatic::Serve），版控的 web/JSON/<x> 退為契約樣本與後備
}

// ---------------------------------------------------------------------------
//  JSON 字串跳脫。只處理契約會用到的：引號、反斜線、控制字元。
//  ⚠ 不做 UTF-8 驗證 —— 這棵樹的原始碼是 UTF-8（CLAUDE.md），
//    進來的字串應該已經是 UTF-8。若哪天接上 Big5 來源，
//    `fetch().json()` 會把不合法位元組變成 U+FFFD 顯示在警報訊息上，
//    **不會丟例外** —— 那是靜默壞掉，要在來源端轉，不是在這裡補。
// ---------------------------------------------------------------------------
inline std::string JsonEscape(const std::string& s)
{
    std::string o;
    o.reserve(s.size() + 8);
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
                    o += buf;
                } else {
                    o += (char)c;
                }
        }
    }
    return o;
}

// ---------------------------------------------------------------------------
//  原子寫一個檔。沿用本樹既有的作法（tools/wb_serve.cpp:905-917）：
//  .tmp_wb -> MoveFileExA(MOVEFILE_REPLACE_EXISTING)。
//
//  ⚠ **回傳值一定要檢查。** 我們寫的是 wb_serve 自己正在 serve 的目錄，
//    Windows 上撞到伺服器開著檔的那一瞬間會回 sharing violation。
//    瀏覽器那側是良性的（fetch 失敗被 .catch 吞掉，100 ms 後再試），
//    **C++ 這側不是** —— 替換失敗而不檢查，就會得到
//    「我寫了、對方沒看到」而且沒有任何錯誤訊息。
// ---------------------------------------------------------------------------
inline bool AtomicWrite(const std::string& path, const std::string& body)
{
    const std::string tmp = path + ".tmp_wb";
    {
        FILE* o = ::fopen(tmp.c_str(), "wb");      // "wb" -> 不做 CRLF 轉換
        if (!o) return false;
        if (!body.empty())
            ::fwrite(body.data(), 1, body.size(), o);
        ::fflush(o);
        ::fclose(o);
    }
    // ⚠⚠ 20260922 端到端實測改大的。原本是 5 次 × 20 ms = 100 ms，
    //   而探針自己在跑 `copytree` 備份那個目錄時就足以撞出 sharing violation
    //   —— 結果 `.json` 寫成功、墊片沒寫成，而那正是
    //   「在量產的 file: 模式下完全沒反應且不報錯」的形狀。
    //   ⇒ 20 次 × 50 ms = 1 秒。對 100 ms 輪詢的對方來說這仍然是即時的，
    //     但足以撐過防毒掃描或備份程式短暫開檔。
    for (int attempt = 0; attempt < 20; ++attempt) {
        if (::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
            return true;
        ::Sleep(50);
    }
    std::printf("  ⚠ 原子替換失敗（重試 20 次）：%s\n", path.c_str());
    ::remove(tmp.c_str());
    return false;
}

// ---------------------------------------------------------------------------
//  把一份 JSON 放進信箱：**.json（LF）與 js/<name>.js（CRLF 墊片）兩個都寫**。
//
//  墊片格式（實測 web/JSON/js/Alarm-dialog-request.js，636 bytes、CRLF）：
//      window.__HT9045_DATA__=window.__HT9045_DATA__||{};
//      window.__HT9045_DATA__["<name>"]={...};
//  key 必須逐字等於 loadFresh 傳進來的 name，也就是**檔名去掉副檔名**。
//  真正承重的只有「解析得出 window.__HT9045_DATA__[name] 且該值 truthy」
//  （dialog-bridge.js:38 `value ? resolve(value) : reject('empty shim')`）。
// ---------------------------------------------------------------------------
inline bool MailboxPut(const std::string& dir, const std::string& name,
                       const std::string& json)
{
    const bool okJson = AtomicWrite(dir + "\\" + name + ".json", json + "\n");

    std::string shim;
    shim  = "window.__HT9045_DATA__=window.__HT9045_DATA__||{};\r\n";
    shim += "window.__HT9045_DATA__[\"" + name + "\"]=" + json + ";\r\n";
    const bool okShim = AtomicWrite(dir + "\\js\\" + name + ".js", shim);

    // ⚠ 兩個都要成功才算數。只成功一個 = 在某一種啟動模式下完全沒作用，
    //   而且不報錯 —— 那比兩個都失敗更難查。
    //
    // ⚠⚠ 而且要**講出是哪一個失敗**。20260922 端到端實測時就踩到：
    //   `.json` 成功、墊片失敗，而量產（file: 協定）只讀墊片
    //   ⇒ 現場的症狀是「警報完全沒跳出來」，但 log 只會說「寫入失敗」，
    //     看不出是哪一半 —— 那會讓人往完全錯的方向查。
    if (okJson != okShim) {
        std::printf("  ⚠⚠ 信箱只寫成一半：.json=%s 墊片=%s（%s）\n"
                    "     量產是 file: 協定，**只讀墊片** ——"
                    " 墊片沒寫成等於警報完全不會出現。\n",
                    okJson ? "OK" : "FAIL", okShim ? "OK" : "FAIL", name.c_str());
    }
    return okJson && okShim;
}

// ===========================================================================
//  AI(W906-Q30-IDLE) 20260923：開機時把「上一個行程留下的待答請求」重設成 idle 種子。
//
//  使用者 20260923 裁決（Ifor 同日現場回報 MES0920 框關不掉之後）：
//  「wb_serve 開機時把請求檔重設成 idle 種子」。
//
//  ## 為什麼一定要在開機做
//
//  請求檔是**跨行程存活**的，而能回答它的 qid 只活在寫它的那個行程裡：
//    1. 某次執行寫了 state:"pending"（MailboxPut），然後行程結束
//       （正常關站、被 kill、當掉都一樣 —— 沒有任何一條路會替它收尾；
//        DialogMailboxRetire() 只在「收到回應之後」才跑）。
//    2. 之後每次開頁，dialog-bridge.js 的 loadFresh() 都讀到 pending，
//       而 lastSeq 是記憶體狀態、重整就歸 0 ⇒ **每次都再畫一次**。
//    3. 但答不掉：回應要走 WebSocket 帶 qid（本檔檔頭「回應走 WebSocket」那段），
//       新行程根本不認得那個 qid。Ifor 實測 HTDialogHost.pending() 是 null，
//       選 RETRY 再按 START，Alarm-dialog-response.json 仍是 seq=0 / idle。
//  ⇒ 唯一知道「上一個行程已經不在了」的時間點就是本行程開機。在那裡清掉。
//
//  ## 只清 C++ 擁有的三個「請求」通道
//
//  契約 Dialog-bridge-contract.json 的 producer：Alarm／Message 請求是 C++
//  ShowErrorMessage／ShowMyMessage，Dialog-close-request 是 C++ IO 流程。
//  ⇒ 開機時由 C++ 重設是它自己的檔。**回應檔（producer = HTML）不碰。**
//
//  ## 為什麼逐字寫回版控的種子，而且「已經是 idle 就不寫」
//
//  這三組檔在 git 裡（web/JSON/*.json 與 web/JSON/js/*.js）。
//    * 已經是 idle ⇒ 完全不寫：不製造 mtime 變動，也不跟同時開著的頁面搶檔。
//    * 需要重設 ⇒ 寫回與 git HEAD 相同內容（.json 排版版 LF、墊片 CRLF，
//      與 MailboxPut 的 EOL 慣例相同；core.autocrlf=true 下 git status 看起來是乾淨的）。
//  種子若改了，tests/test_dialog_mailbox.cpp 會對著 web/JSON 的檔抓出漂移。
//
//  ⚠ 前提：**一個 web 根目錄只能有一個 wb_serve。** 若另一個 wb_serve 正在
//    serve 同一個根目錄而且手上有待答的框，這裡會把它清掉（它的 C++ 會繼續等
//    WebSocket 那條路的回應，框則要等它下一次 PostAlarm 才會再出現）。
//    兩個 wb_serve 共用一個信箱本來就不是支援的組態（seq 會打架）。
// ===========================================================================
namespace seed {
// 以下六個常數由 git HEAD 的 web/JSON 種子逐字產生（20260923，84d9619）。

// web/JSON/Alarm-dialog-request.json -- 排版版，逐字（含結尾換行）
static const char kSeedPretty_Alarm_dialog_request[] =
    "{\n"
    "  \"schemaVersion\": \"1.0.0\",\n"
    "  \"channel\": \"show-error-message\",\n"
    "  \"seq\": 0,\n"
    "  \"requestId\": \"\",\n"
    "  \"state\": \"idle\",\n"
    "  \"requestedAt\": null,\n"
    "  \"function\": \"ShowErrorMessage\",\n"
    "  \"blocking\": true,\n"
    "  \"arguments\": {\n"
    "    \"code\": \"\",\n"
    "    \"kCode\": 0,\n"
    "    \"position\": 0,\n"
    "    \"duplicateError\": false,\n"
    "    \"errorPart\": \"\"\n"
    "  },\n"
    "  \"display\": {\n"
    "    \"alarmType\": null,\n"
    "    \"unitName\": \"\",\n"
    "    \"message\": \"\",\n"
    "    \"jamArea\": \"\",\n"
    "    \"description\": \"\",\n"
    "    \"flushPanel\": null\n"
    "  },\n"
    "  \"auth\": {\n"
    "    \"required\": false,\n"
    "    \"kind\": \"none\",\n"
    "    \"level\": null,\n"
    "    \"title\": \"Password\",\n"
    "    \"prompt\": \"\",\n"
    "    \"userIdRequired\": true,\n"
    "    \"defaultUserId\": null\n"
    "  },\n"
    "  \"buttons\": [],\n"
    "  \"closePolicy\": \"acknowledge-only\",\n"
    "  \"error\": null\n"
    "}\n";
// web/JSON/js/Alarm-dialog-request.js 第 2 行 `=` 後面到 `;` 之前的值 -- 壓縮版，逐字
static const char kSeedCompact_Alarm_dialog_request[] =
    "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-error-message\",\"seq\":0,\"requestId\":\"\",\"state\":\"idle\",\"r"
    "equestedAt\":null,\"function\":\"ShowErrorMessage\",\"blocking\":true,\"arguments\":{\"code\":\"\",\"kCode\":0,"
    "\"position\":0,\"duplicateError\":false,\"errorPart\":\"\"},\"display\":{\"alarmType\":null,\"unitName\":\"\",\"m"
    "essage\":\"\",\"jamArea\":\"\",\"description\":\"\",\"flushPanel\":null},\"auth\":{\"required\":false,\"kind\":\"non"
    "e\",\"level\":null,\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,\"defaultUserId\":null},\"butt"
    "ons\":[],\"closePolicy\":\"acknowledge-only\",\"error\":null}";

// web/JSON/Message-dialog-request.json -- 排版版，逐字（含結尾換行）
static const char kSeedPretty_Message_dialog_request[] =
    "{\n"
    "  \"schemaVersion\": \"1.0.0\",\n"
    "  \"channel\": \"show-my-message\",\n"
    "  \"seq\": 0,\n"
    "  \"requestId\": \"\",\n"
    "  \"state\": \"idle\",\n"
    "  \"requestedAt\": null,\n"
    "  \"function\": \"ShowMyMessage\",\n"
    "  \"blocking\": true,\n"
    "  \"arguments\": {\n"
    "    \"s1\": \"\",\n"
    "    \"s2\": \"\",\n"
    "    \"s3\": \"\",\n"
    "    \"ok\": false,\n"
    "    \"servoOff\": false\n"
    "  },\n"
    "  \"display\": {\n"
    "    \"primaryText\": \"\",\n"
    "    \"secondaryText\": \"\",\n"
    "    \"buttonLabel\": \"Pause\",\n"
    "    \"showAlarmReset\": false,\n"
    "    \"buttonEnabled\": true\n"
    "  },\n"
    "  \"runtime\": {\n"
    "    \"secsGemAlarm\": false,\n"
    "    \"haltHandler\": false,\n"
    "    \"systemInitialOK\": false,\n"
    "    \"employeeIdCheck\": false\n"
    "  },\n"
    "  \"requestedSideEffects\": {\n"
    "    \"pauseHandler\": true,\n"
    "    \"stopAllMotor\": false,\n"
    "    \"servoOffInArmXY\": false\n"
    "  },\n"
    "  \"auth\": {\n"
    "    \"required\": false,\n"
    "    \"kind\": \"none\",\n"
    "    \"level\": null,\n"
    "    \"title\": \"Password\",\n"
    "    \"prompt\": \"\",\n"
    "    \"userIdRequired\": true,\n"
    "    \"defaultUserId\": null\n"
    "  },\n"
    "  \"error\": null\n"
    "}\n";
// web/JSON/js/Message-dialog-request.js 第 2 行 `=` 後面到 `;` 之前的值 -- 壓縮版，逐字
static const char kSeedCompact_Message_dialog_request[] =
    "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-my-message\",\"seq\":0,\"requestId\":\"\",\"state\":\"idle\",\"requ"
    "estedAt\":null,\"function\":\"ShowMyMessage\",\"blocking\":true,\"arguments\":{\"s1\":\"\",\"s2\":\"\",\"s3\":\"\",\"o"
    "k\":false,\"servoOff\":false},\"display\":{\"primaryText\":\"\",\"secondaryText\":\"\",\"buttonLabel\":\"Pause\","
    "\"showAlarmReset\":false,\"buttonEnabled\":true},\"runtime\":{\"secsGemAlarm\":false,\"haltHandler\":false"
    ",\"systemInitialOK\":false,\"employeeIdCheck\":false},\"requestedSideEffects\":{\"pauseHandler\":true,\"s"
    "topAllMotor\":false,\"servoOffInArmXY\":false},\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,"
    "\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,\"defaultUserId\":null},\"error\":null}";

// web/JSON/Dialog-close-request.json -- 排版版，逐字（含結尾換行）
static const char kSeedPretty_Dialog_close_request[] =
    "{\n"
    "  \"schemaVersion\": \"1.0.0\",\n"
    "  \"channel\": \"dialog-close\",\n"
    "  \"seq\": 0,\n"
    "  \"closeRequestId\": \"\",\n"
    "  \"state\": \"idle\",\n"
    "  \"requestedAt\": null,\n"
    "  \"target\": {\n"
    "    \"channel\": \"\",\n"
    "    \"requestId\": \"\",\n"
    "    \"requestSeq\": 0\n"
    "  },\n"
    "  \"trigger\": {\n"
    "    \"source\": \"io\",\n"
    "    \"inputName\": \"\",\n"
    "    \"detectedAt\": null\n"
    "  },\n"
    "  \"resolvedAction\": {\n"
    "    \"name\": \"NONE\",\n"
    "    \"code\": null\n"
    "  },\n"
    "  \"closeReason\": \"external-io\",\n"
    "  \"error\": null\n"
    "}\n";
// web/JSON/js/Dialog-close-request.js 第 2 行 `=` 後面到 `;` 之前的值 -- 壓縮版，逐字
static const char kSeedCompact_Dialog_close_request[] =
    "{\"schemaVersion\":\"1.0.0\",\"channel\":\"dialog-close\",\"seq\":0,\"closeRequestId\":\"\",\"state\":\"idle\",\"re"
    "questedAt\":null,\"target\":{\"channel\":\"\",\"requestId\":\"\",\"requestSeq\":0},\"trigger\":{\"source\":\"io\",\""
    "inputName\":\"\",\"detectedAt\":null},\"resolvedAction\":{\"name\":\"NONE\",\"code\":null},\"closeReason\":\"ext"
    "ernal-io\",\"error\":null}";
} // namespace seed

struct IdleSeed { const char* name; const char* pretty; const char* compact; };

inline const IdleSeed* IdleSeeds(int& count)
{
    static const IdleSeed seeds[] = {
        { "Alarm-dialog-request",   seed::kSeedPretty_Alarm_dialog_request,   seed::kSeedCompact_Alarm_dialog_request },
        { "Message-dialog-request", seed::kSeedPretty_Message_dialog_request, seed::kSeedCompact_Message_dialog_request },
        { "Dialog-close-request",   seed::kSeedPretty_Dialog_close_request,   seed::kSeedCompact_Dialog_close_request },
    };
    count = (int)(sizeof(seeds) / sizeof(seeds[0]));
    return seeds;
}

// 墊片的格式必須與 MailboxPut() 寫的逐字相同（CRLF，兩行）。
inline std::string ShimFor(const std::string& name, const std::string& compactJson)
{
    return "window.__HT9045_DATA__=window.__HT9045_DATA__||{};\r\n"
           "window.__HT9045_DATA__[\"" + name + "\"]=" + compactJson + ";\r\n";
}

inline bool ReadWholeFile(const std::string& path, std::string& out)
{
    out.clear();
    FILE* f = ::fopen(path.c_str(), "rb");
    if (!f) return false;
    char buf[4096];
    std::size_t n;
    while ((n = ::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    ::fclose(f);
    return true;
}

// 排版版是 `"state": "idle"`，壓縮版（MailboxPut／墊片）是 `"state":"idle"`。
// 三個請求通道的 JSON 都只有頂層一個 "state" 鍵（種子實測），所以子字串比對夠用。
inline bool TextSaysIdle(const std::string& s)
{
    return s.find("\"state\":\"idle\"") != std::string::npos ||
           s.find("\"state\": \"idle\"") != std::string::npos;
}

// 回傳：這次實際重設了幾個通道。.json 與墊片**兩個都要是 idle** 才算 idle ——
// 量產的 file: 模式只讀墊片，http 模式只讀 .json，任一個 pending 都會彈框。
inline int MailboxResetStale(const std::string& dir)
{
    int count = 0;
    const IdleSeed* seeds = IdleSeeds(count);
    int reset = 0;
    for (int i = 0; i < count; ++i) {
        const std::string name     = seeds[i].name;
        const std::string jsonPath = dir + "\\" + name + ".json";
        const std::string shimPath = dir + "\\js\\" + name + ".js";

        // 上一個行程死在 AtomicWrite 的 fopen(.tmp_wb) 與 MoveFileExA 之間，
        // 會留下這種暫存檔（20260923 主工作樹就有一顆 Alarm-dialog-request.js.tmp_wb）。
        const std::string tmps[2] = { jsonPath + ".tmp_wb", shimPath + ".tmp_wb" };
        for (int t = 0; t < 2; ++t) {
            if (::GetFileAttributesA(tmps[t].c_str()) == INVALID_FILE_ATTRIBUTES) continue;
            if (::DeleteFileA(tmps[t].c_str()))
                std::printf("  信箱：刪除上次沒收尾的暫存檔 %s\n", tmps[t].c_str());
            else
                std::printf("  ⚠ 信箱：上次沒收尾的暫存檔刪不掉 %s\n", tmps[t].c_str());
        }

        std::string cur;
        const bool jsonIdle = ReadWholeFile(jsonPath, cur) && TextSaysIdle(cur);
        const bool shimIdle = ReadWholeFile(shimPath, cur) && TextSaysIdle(cur);
        if (jsonIdle && shimIdle) continue;

        const bool okJ = AtomicWrite(jsonPath, seeds[i].pretty);
        const bool okS = AtomicWrite(shimPath, ShimFor(name, seeds[i].compact));
        std::printf("  信箱：%s 開機時不是 idle（.json=%s 墊片=%s）-> 重設成 idle 種子：%s\n",
                    name.c_str(), jsonIdle ? "idle" : "非idle或缺檔",
                    shimIdle ? "idle" : "非idle或缺檔",
                    (okJ && okS) ? "OK" : "⚠ 失敗（下次開頁可能仍會彈出舊框）");
        if (okJ && okS) ++reset;
    }
    return reset;
}

// 自 1970-01-01 起的毫秒數。拿來當 seq 的起點（見 wb_serve.cpp g_dialogSeq）。
// 2026 年約 1.79e12，遠小於 JS 的安全整數上限 2^53-1（約 9.0e15）。
inline unsigned long long UnixMillisNow()
{
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER u;
    u.LowPart  = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return (u.QuadPart - 116444736000000000ULL) / 10000ULL;
}

} // namespace w906dlg

#endif // W906_WB_DIALOG_MAILBOX_H
