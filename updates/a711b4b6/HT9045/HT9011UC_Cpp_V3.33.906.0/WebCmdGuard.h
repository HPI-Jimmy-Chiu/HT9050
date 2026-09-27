// =============================================================================
//  WebCmdGuard.h -- 伺服器端防連點：同一條指令在「還在跑」或「剛跑完 W 毫秒內」
//                   又到一次，就回 busy、不執行（S107-3）
//
//  AI(W906-CMDGUARD) 20260926: NEW-BUILD 基礎建設，不是任何 BCB6 golden 的翻譯。
//    Steven 18:0x 的規則原話：「全部的按鈕事件要小心使用者短時間連點，類似滑鼠
//    double click，可能要進行阻斷」；「Jimmy 可能沒空管，web serv 我們可以改」。
//    設計依盤點報告 scratchpad\dblclick\report.md 第 0／5 節。
//
//  為什麼不是「執行中」旗標
//    WS 指令在 wb_serve 的 tick 執行緒上一次 drain、逐條同步執行
//    （tools/wb_serve.cpp 分派迴圈 `const webbridge::WebCommand& wc = drained[i];`）。
//    所以輪到第二下的時候，第一下一定已經跑完了：
//      * 快指令（塔燈、Counter Clear，ack 5-50 ms）：雙擊的第二下（80-250 ms 後）到時
//        第一下早就完成。
//      * 慢指令（editlist.save 1-2 秒）：第二下在佇列裡等，輪到它時第一下也完成了。
//    「執行中」在兩種情況都是 false，擋不到。只看開始時間的時間窗也不行（慢指令跑完
//    時窗早就過了）。
//
//  判定（一個比較式同時涵蓋兩件事）
//      新指令.pushedUs < 同 key 上一條的完成時間 + W   ==>  busy，不執行
//    * pushedUs 早於上一條完成 = 它是在上一條還在跑（或排隊）時到的 = 佇列去重。
//    * pushedUs 在完成後 W 內   = 時間窗。
//    * 比的是 pushedUs（socket 執行緒 tryPush 收下的時間，CommandQueue.cpp tryPush），
//      不是出佇列的時間 —— 佇列等多久都不會誤判。
//    * 被擋下的那條**不蓋時間**：狂按不會把窗口一直往後延。
//    * pushedUs 為 0（沒經過 tryPush 的指令）就用當下時間。
//
//  key ＝ FNV-1a 64 over  cmd '\x1f' tag '\x1f' value
//    * value 一定要算進去：兩段式確認的 {confirmed:false} -> {confirmed:true}、
//      editlist.save 帶 answers 的重送、counterclear.click 的反向勾選，都是**不同**
//      的指令，不可以互擋。
//    * value 用原字串（頁面同一下雙擊送出的 JSON.stringify 位元組完全相同），不做
//      JSON 正規化。前面加一個型別位元組（沒有 value = '\0'），字串 "1" 與數字 1 不同 key。
//      ⚠ 例外 motor.access（AI(W906-R0927-8) 20260927）：它的 value 是 motor-access.js 整個 request，
//      每按一下都帶新的 seq／id／issuedAt，params 還有頁面從畫面讀的即時值（目前位置、伺服狀態）——
//      原字串當 key 的話同一顆鈕連點兩下永遠是不同 key、擋不到。先拿掉那幾個欄位再算
//      （清單與理由見 WebCmdGuard.cpp kMotorAccessStrip），其餘照原規則：不同軸／不同距離仍是不同 key。
//    * 不含瀏覽器 id、ticket、連線：WebCommand.connId 在 wb_serve 永遠是 0
//      （WebBridgeServer.cpp QueuePush 沒設它），沒辦法依連線分。
//
//  W（時間窗，從完成時間起算）
//    預設 400 ms；env W906_CMDGUARD_MS 可調（十進位毫秒），0 = 整個 guard 關閉；
//    上限 10000 ms（超過就夾到 10000，與 map 清理的 10 秒一致）；值不合法就用預設並印一行。
//    理由：Windows 雙擊上限預設 500 ms（從第一下算），第一條通常 20-100 ms 內完成，
//    從完成起算 400 ms 可以蓋到約 420-500 ms 的慢雙擊。
//
//  預設策略：除了白名單以外全部擋（新加的指令自動受保護）。白名單見 WebCmdGuard.cpp
//    檔頭的兩張表（名稱級、op 級；motor.access 在 op 級，AI(W906-R0927-8) 20260927）。
//
//  時鐘：⚠ MinGW.org GCC 6.3 的 std::chrono::steady_clock **不是單調時鐘**（c++config.h 沒有
//    _GLIBCXX_USE_CLOCK_MONOTONIC，libstdc++ 退回 system_clock／gettimeofday ＝ 牆上時間），
//    校時或手動改時間會讓 pushedUs 與完成時間一起跳。往前跳：只會少擋一次。往回跳：完成時間
//    變成「未來」—— Check 看到就丟掉那一筆、照常執行（否則往回跳一小時會把同一條指令擋一小時）。
//    MSVC 的 steady_clock 是 QPC，沒有這個問題。
//
//  map：只在 tick 執行緒用，不加鎖（MinGW 6.3 沒有 <mutex>，也用不到）。超過 512 筆時
//    （每秒最多一次）清掉 10 秒前完成的；還是超過 4096 筆就整個清空（fail-open：最壞
//    放過一次重複，不會擋錯）。
//
//  蓋不到的地方（都有各自的理由，見報告第 5 節）
//    三個阻塞等待迴圈（本來就對其他指令回 modal-pending）、輸出優先服務
//    W906_ServiceOutputs（只跑 IO／停止，都在白名單）、socket 執行緒上的 control.*、HTTP。
//    AI(W906-R0927-9) 20260927: IO 面板輸出鈕（io.btnPanelClick）不論從哪條路（主分派、輸出優先）都在
//    W906_DispatchIoClick 執行，改由那裡的 W906IoClickGuardScope（本檔最後）擋 —— 同一顆鈕（tag＝Alias）
//    ＋同一個 down 值才擋，down 跟這顆鈕上一次不同一律放行（Jimmy 08:3x 條件，理由見本檔最後一節）。
//    io.btnPanelClick 仍留在名稱級白名單，主分派那條不會被擋兩次。
//
//  依賴：只有 WebBridge/CommandQueue.h（WebCommand）／TagValue 與 Public/cJSON（解析
//    op 級白名單的 value）。沒有任何機台碼。
// =============================================================================
#ifndef W906_WEB_CMD_GUARD_H
#define W906_WEB_CMD_GUARD_H

#include "WebBridge/CommandQueue.h"   // webbridge::WebCommand

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ht9045 {

class WebCmdGuard {
public:
    // 時鐘：µs，必須與 WebCommand::pushedUs 同一個時鐘（steady_clock 的 time_since_epoch）。
    // 測試注入假時鐘；0 = SteadyNowUs。
    typedef std::uint64_t (*ClockUs)();

    static const std::uint64_t kDefaultWindowMs = 400;
    static const std::uint64_t kMaxWindowMs     = 10000;
    static const std::size_t   kPurgeAbove      = 512;         // 超過這個筆數才做年齡清理
    static const std::uint64_t kPurgeAgeUs      = 10000000ULL; // 清掉 10 秒前完成的
    static const std::uint64_t kPurgeEveryUs    = 1000000ULL;  // 年齡清理每秒最多一次
    static const std::size_t   kHardCap         = 4096;        // 還是超過就整個清空

    explicit WebCmdGuard(std::uint64_t windowMs = kDefaultWindowMs, ClockUs clock = 0);

    // true = BUSY：呼叫端回 ok:false、*why（字首固定 "busy:"），**不執行**；*key 設 0。
    // false = 照常執行；*key = 執行完要交給 Done() 的 key（0 = 不受保護，Done 是空操作）。
    // 每次擋下都 printf 一行。
    bool Check(const webbridge::WebCommand& wc, std::uint64_t* key, std::string* why);

    // AI(W906-R0927-9) 20260927: 不看白名單、key 由呼叫端決定的同一個判定（Check 的後半段就是它）。
    //   true = BUSY（*why 字首 "busy:"，label 放進括號）；false = 照常執行，之後照樣呼叫 Done(key)。
    //   key 0 = 不受保護（回 false）。W=0 時一律 false。
    //   *why ＝ "busy: same <what> <phrase> (<label>, <N> ms ago)"；phrase 0 ＝ "in progress or just done"
    //   （Check 用的；IO 鈕傳 "within <W> ms"，見本檔最後一節）。
    bool CheckKey(std::uint64_t key, const webbridge::WebCommand& wc, const std::string& what,
                  const std::string& label, std::string* why, const char* phrase = 0);

    // 蓋完成時間（= 時鐘的現在）。key 0 什麼都不做。
    void Done(std::uint64_t key);
    // AI(W906-R0927-9) 20260927: IO 鈕用（W906IoClickGuardScope）。
    //   Forget：刪掉 key 的紀錄（之後同 key 立刻放行）。key 0 什麼都不做。
    //   Touch ：key 還在表裡才蓋完成時間（回 true）；被 Forget／清理掉了就不蓋（回 false）。key 0 回 false。
    void Forget(std::uint64_t key);
    bool Touch(std::uint64_t key);

    void          SetWindowMs(std::uint64_t ms);               // 會夾到 kMaxWindowMs；0 = 關
    std::uint64_t WindowMs() const { return windowUs_ / 1000ULL; }
    std::size_t   Size() const     { return done_.size(); }
    std::uint64_t Blocked() const  { return blocked_; }

    // ---- 純函式（測試直接呼叫） -------------------------------------------------
    // 白名單（名稱級或 op 級）：true = 不受 guard 保護。
    static bool          Exempt(const webbridge::WebCommand& wc);
    // FNV-1a 64；保證非 0（0 保留給「不受保護」）。
    static std::uint64_t KeyOf(const webbridge::WebCommand& wc);
    // AI(W906-R0927-9) 20260927: IO 面板鈕的 key ＝ cmd＋tag（Alias）＋正規化後的 down（Jimmy 08:3x：
    //   Click_ 是「設成指定狀態」不是切換，所以 down 一定要算進去；判定規則見本檔最後一節）。
    //   ButtonKeyOf(wc) ＝ ButtonKeyOf(wc, IoClickDownOf(wc))；第二個多載給「同一顆鈕的其他 down 值」用。
    static std::uint64_t ButtonKeyOf(const webbridge::WebCommand& wc);
    static std::uint64_t ButtonKeyOf(const webbridge::WebCommand& wc, int down);
    // io.btnPanelClick 的 value 正規化成本體看到的 down：0、1，其他一律 -1（本體會拒絕的值）。
    //   跟 tools/wb_serve.cpp W906_DispatchIoClick 取 ioDown 的式子同一套（Int：(int)asInt；Double：(int) 截斷；
    //   Bool：1／0；沒有 value／字串／Null：-1），再照 JsonBridge/IoBtnPanelClick.cpp Click_ 只收 0／1（:221）。
    //   那邊的式子改了，這裡要一起改。
    static int           IoClickDownOf(const webbridge::WebCommand& wc);
    // 與 CommandQueue::tryPush 蓋 pushedUs 完全相同的式子。
    static std::uint64_t SteadyNowUs();
    // W906_CMDGUARD_MS 的解析。NULL／空字串 -> 預設，回 true；十進位數字 -> 該值（夾到
    // kMaxWindowMs），回 true；其他（負號、非數字、尾巴有垃圾） -> 預設，回 false。
    static bool          ParseWindowMs(const char* text, std::uint64_t* ms);

private:
    std::uint64_t Now() const;
    void          Purge(std::uint64_t now);

    std::uint64_t windowUs_;
    ClockUs       clock_;
    std::unordered_map<std::uint64_t, std::uint64_t> done_;   // key -> 最後完成時間（µs）
    std::uint64_t blocked_;
    std::uint64_t lastPurgeUs_;
    bool          purgedOnce_;
};

// wb_serve 用的那一個（第一次呼叫時讀 W906_CMDGUARD_MS，印一行目前的 W）。
WebCmdGuard& WebCmdGuardGlobal();

}  // namespace ht9045

// -----------------------------------------------------------------------------
//  wb_serve 分派迴圈用：每條指令一個，放在迴圈頭。
//    W906CmdGuardScope cmdGuard(wc);
//    if (cmdGuard.busy()) { server.CompleteCommand((unsigned long long)wc.id, false, cmdGuard.why()); continue; }
//  解構時（這一圈結束，分支已經 CompleteCommand 過）蓋完成時間；被擋下的不蓋。
//  不保存 wc 的參考（drained 在這一圈裡不會動，但不依賴它）。
//  Check／Done 丟例外一律吞掉並 fail-open（不擋），tick 迴圈不會因為 guard 掉出去。
// -----------------------------------------------------------------------------
class W906CmdGuardScope {
public:
    explicit W906CmdGuardScope(const webbridge::WebCommand& wc);
    // 測試用：指定 guard 實例（wb_serve 用上面那個，走 WebCmdGuardGlobal）。
    W906CmdGuardScope(ht9045::WebCmdGuard& guard, const webbridge::WebCommand& wc);
    ~W906CmdGuardScope();

    bool               busy() const { return busy_; }
    const std::string& why() const  { return why_; }

    W906CmdGuardScope(const W906CmdGuardScope&) = delete;
    W906CmdGuardScope& operator=(const W906CmdGuardScope&) = delete;

private:
    void Begin(const webbridge::WebCommand& wc);

    ht9045::WebCmdGuard* guard_;
    std::uint64_t        key_;
    bool                 busy_;
    std::string          why_;
};

// -----------------------------------------------------------------------------
//  AI(W906-R0927-9) 20260927: IO 面板輸出鈕的防連點（RULINGS_20260927 第 2 條第 9 題 A：
//  「同一顆鈕 400 ms 內只算一次」，加上 Jimmy 08:3x 的條件：判斷鍵用「同一顆鈕＋同一個 down 值」，
//  down 跟上一次不同就一律放行）。NEW-BUILD，不是 golden 的翻譯 —— golden 的 BtnPanelClick
//  （iosetview.cpp:1030-1206）每一下都切（Down=!Down），連點兩下＝切兩次＝回到原狀。
//
//  為什麼不在分派迴圈頭（W906CmdGuardScope）：面板鈕走「輸出優先」W906_ServiceOutputs
//    （wb_serve.cpp 檔尾），根本不經過迴圈頭。io.btnPanelClick 真正執行的地方只有 W906_DispatchIoClick
//    （EastSun 的 IO 範圍）：wb_serve.cpp 裡只有兩處呼叫它 —— 主分派鏈，與 W906_ServiceOutputs（後者又從
//    主迴圈、1203 Poll、api cache、tag 發布的讓出點進來；告警框等待中的也是走它）。所以擋在那支函式的第一行。
//
//  為什麼一定要看 down（Jimmy 08:3x）：移植的 Click_（JsonBridge/IoBtnPanelClick.cpp:337）不是切換，是
//    「設成指定狀態」—— value＝按下之後的 Down（0／1，頁面依卡片回讀算，同檔檔頭 WIRE 與 DEVIATION d），
//    依 IO_Table 的 InType（outType）對應成開或關。只看鈕名的話，400 ms 內送「開」再送「關」，「關」會被擋，
//    輸出就留在開（吹氣、氣缸）。
//  判定（key ＝ cmd＋tag＋IoClickDownOf 正規化後的 down：0、1，或 -1＝本體會拒絕的值）：
//    * 同一顆鈕、同一個 down：W 內第二下、或第一下還在跑時到的 —— busy。同一個狀態再設一次不會改變輸出，
//      擋掉它，輸出仍然等於操作員最後要的狀態。
//    * down 跟這顆鈕上一次放行的不同：一律放行。做法：一下被放行時，先把同一顆鈕「另外兩個 down 值」的紀錄
//      刪掉（Forget），所以 400 ms 內 開→關→開 三下都會執行，輸出等於最後一下。
//    * 不同 Alias（例：Loader 的 "<-" 與 "v" 兩顆）不互擋。
//  完成時間：放行時先蓋一次（執行中又到同一個狀態的也擋），解構時（函式 return 之後，CompleteCommand 已經發過）
//    再蓋一次，但只在自己的紀錄還在時（Touch）—— 若執行中有「另一個 down」的一下插進來跑完（巢狀），它已經把
//    這筆刪掉，這一下就不再蓋，表裡留的是較晚到的那個狀態。失敗的點擊也蓋（同主 guard）。
//  時鐘、W（W906_CMDGUARD_MS，0＝關）、"busy:" 字首、被擋不蓋時間：全部跟 W906CmdGuardScope 同一個 guard 實例
//    （WebCmdGuardGlobal）。沒有 tag 的不算一顆鈕（本體會拒絕），不擋也不記。
//  busy 訊息（ok=false）＝ Jimmy 給的字＋鈕名、down、距上一下完成的毫秒數（400 是目前的 W）：
//    busy: same button and state within 400 ms (io.btnPanelClick C_Load_Up down=1, 120 ms ago)
//
//  用法 —— W906_DispatchIoClick 函式本體第一行（同一行插入，不移動行號）：
//    W906IoClickGuardScope ioClickGuard(wc); if (ioClickGuard.busy()) { server.CompleteCommand((unsigned long long)wc.id, false, ioClickGuard.why()); return; }
//  Check／Done 丟例外一律吞掉並 fail-open（不擋）。
// -----------------------------------------------------------------------------
class W906IoClickGuardScope {
public:
    explicit W906IoClickGuardScope(const webbridge::WebCommand& wc);
    // 測試用：指定 guard 實例
    W906IoClickGuardScope(ht9045::WebCmdGuard& guard, const webbridge::WebCommand& wc);
    ~W906IoClickGuardScope();

    bool               busy() const { return busy_; }
    const std::string& why() const  { return why_; }

    W906IoClickGuardScope(const W906IoClickGuardScope&) = delete;
    W906IoClickGuardScope& operator=(const W906IoClickGuardScope&) = delete;

private:
    void Begin(const webbridge::WebCommand& wc);

    ht9045::WebCmdGuard* guard_;
    std::uint64_t        key_;      // 0 = 解構時不蓋（被擋、W=0、沒有 tag、例外）
    bool                 busy_;
    std::string          why_;
};

#endif  // W906_WEB_CMD_GUARD_H
