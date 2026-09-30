// =============================================================================
//  WebOlp.cpp  --  瀏覽器 → OLP 設定指令的白名單橋接（實作）
//
//  AI(W906-P3-OLP) 20260919
//
//  設計理由與「為什麼不走 ProcessBuffer」寫在 WebOlp.h 的檔頭，不在這裡重複。
//  這個檔只放兩樣東西：**白名單表**（含逐條的 golden 行號）與**拒絕名單**
//  （含每一筆缺的是哪一個相依）。
// =============================================================================
#include "WebOlp.h"

#include "Automation/auto9045.h"    // Set*(AnsiString*) 全家 + CheckNeedCleanOut()
#include "LastSet.h"                // LastSet（:587 的 extern；OLPSetBinErr 後置動作要用）
#include "acatchtray_shims.h"       // NewRecordProcess（golden cMyDB.h 的 3 參數版，:439）

#include <cstdio>
#include <cstdlib>

namespace {

// -----------------------------------------------------------------------------
//  golden 的訊框把資料放在 `AnsiString Data[40]`（automation.cpp:1307）。
//  這裡沿用同一個大小 —— 不是因為 40 好看，是因為越界的行為要跟 golden 一致
//  才能拿 golden 當對照。⚠ 40 這個上限本身是 golden 的缺陷（見下面 bindefine
//  那筆拒絕理由），我們不修，但也不把它暴露給瀏覽器。
// -----------------------------------------------------------------------------
const int kGoldenDataSlots = 40;

// -----------------------------------------------------------------------------
//  後置動作 —— golden 在 setter 回來之後還做的事。
//  **漏抄一個就是拿掉一道互鎖**，所以做成具名的 enum 而不是 bool。
// -----------------------------------------------------------------------------
enum PostAction {
    kPostNone = 0,
    // golden automation.cpp:2086-2090（FIXTRAYDEFINE_REQUEST）:
    //     if (atoi(Data[0].c_str()) >= 3) LastSet.OLPSetBinErr[2] = ...;
    //     else                            LastSet.OLPSetBinErr[2] = 0;
    // ★ 這不是裝飾。LastSet.OLPSetBinErr[] 被 ComputeCheckOLPErrorHasErr()
    //   讀走，而那支函式擋在 TfMain::Home()（forms/fMain.cpp:776）與
    //   StartFromWeb()（WebStart.cpp:412）的啟動許可序列上。
    //   Bin/FixTray 設定錯誤 >= 3 就不准啟動 —— Sam 20230921 加的。
    kPostOlpSetBinErr2 = 1
};

struct OlpRow {
    const char* cmd;            // 瀏覽器看到的指令名
    const char* golden;         // golden 的分支名
    int       (*fn)(AnsiString*);
    int         arity;          // 需要幾個資料欄；-1 = 依機台站數動態決定
    bool        guardCleanOut;  // golden 的**分支層**有沒有 CheckNeedCleanOut()
    PostAction  post;
    int         goldenLine;     // golden 分支在移植樹 automation.cpp 的行號
};

// =============================================================================
//  白名單 —— 35 筆
//
//  三個條件同時成立才進表：
//    (a) golden 的分支本體非空
//    (b) setter 在 auto9045.cpp 有真本體（43 個全部有，最小 7 行，#if 0 = 0）
//    (c) **setter 的回傳值反映真實結果**，沒有解析到 W5FA_* 蜃影
//
//  (c) 是真正的邊界。不符的 8 筆在下面的 kOlpDenylist，逐筆寫明缺哪個相依。
//
//  `guardCleanOut` 那一欄是量出來的，不是抄印象：
//    * SetTemperature / SetSoakTime 的本體**只有** CheckSystemStart()
//      （auto9045.cpp:941 / :919），分支層的 CheckNeedCleanOut() 是它們
//      **唯一**的 cleanout 守衛 ⇒ 必須複製。
//    * SetFixTrayDefine / SetDutOnOff / SetMapping 本體內已經有
//      （:610 / :667 / :793）⇒ 分支層那道是第二份，複製它是冪等的。
//  ⇒ 照 golden 的分支原樣填，不做「反正裡面有了」的最佳化。
//
//  ⚠ CheckNeedCleanOut() 本身有一半是**被樁開著的**（auto9045.cpp:1525-1530）:
//        CheckCanChangeRealDummy()==false || HasICUnderMachine()
//    而 CheckCanChangeRealDummy 解析到 W5FA_TfMainExt 的硬編碼 `return true`
//    （auto9045.cpp:208-215，它自己標著「JUDGMENT CALL (flagged for review):
//    golden main.cpp body unavailable」）⇒ 第一個 disjunct 永遠是 false，
//    今天只剩 HasICUnderMachine() 在擋。這不是本波造成的，但用到它的人要知道。
// =============================================================================
const OlpRow kOlpWhitelist[] = {

// --- 有分支層守衛 / 後置動作的四筆（golden automation.cpp:2083-2143）--------
{ "olp.fixTrayDefine", "FIXTRAYDEFINE_REQUEST", &SetFixTrayDefine,  5, false, kPostOlpSetBinErr2, 2083 },
{ "olp.mapping",       "MAPPING_REQUEST",       &SetMapping,       -1, true,  kPostNone,          2094 },
{ "olp.dutOnOff",      "DUT_REQUEST",           &SetDutOnOff,      -1, true,  kPostNone,          2106 },
{ "olp.soakTime",      "SOAK_TIME_REQUEST",     &SetSoakTime,       1, true,  kPostNone,          2118 },
{ "olp.temperature",   "TEMPERATURE_REQUEST",   &SetTemperature,    1, true,  kPostNone,          2131 },

// --- 尾段：golden 一律是 `Data[0] = Set*(Data); CommandProcess(...)`，
//     **沒有**分支層守衛。守衛全在 setter 本體內（CheckSystemStart 等）。
//     照抄 = guardCleanOut 一律 false。--------------------------------------
{ "olp.lowYield",                        "LowYield_REQUEST",                        &SetLowYield,                        3, false, kPostNone, 2226 },
{ "olp.byArmPerSiteDiffYield",           "ByArmPerSiteDiffYield_REQUEST",           &SetByArmPerSiteDiffYield,           3, false, kPostNone, 2231 },
{ "olp.consecFailAlarmByHead",           "ConsecutiveFailureAlarmByHead_REQUEST",   &SetConsecutiveFailureAlarmByHead,   2, false, kPostNone, 2236 },
{ "olp.consecFailAlarmBySocket",         "ConsecutiveFailureAlarmBySocket_REQUEST", &SetConsecutiveFailureAlarmBySocket, 2, false, kPostNone, 2241 },
{ "olp.allSiteFailFor9045",              "AllSiteFailFor9045_REQUEST",              &SetAllSiteFailFor9045,              1, false, kPostNone, 2246 },
{ "olp.contactModeFor9045",              "ContactModeFor9045_REQUEST",              &SetContactModeFor9045,              1, false, kPostNone, 2251 },
{ "olp.contactVacuumMode",               "ContactVacuumMode_REQUEST",               &SetContactVacuumMode,               1, false, kPostNone, 2256 },
{ "olp.contactDropWait",                 "ContactDropWait_REQUEST",                 &SetContactDropWait,                 1, false, kPostNone, 2261 },
{ "olp.slowContactSpeed",                "SlowContactSpeed_REQUEST",                &SetSlowContactSpeed,                1, false, kPostNone, 2266 },
{ "olp.shuttleWaitOutSideCamber",        "ShuttleWaitOutSideCamber_REQUEST",        &SetShuttleWaitOutSideCamber,        1, false, kPostNone, 2271 },
{ "olp.pickShuttleDeviceAfterTested",    "PickShuttleDeviceAfterTested_REQUEST",    &SetPickShuttleDeviceAfterTested,    1, false, kPostNone, 2276 },
{ "olp.pickShuttleDeviceThenWait",       "PickShuttleDeviceThenWaitOnShuttle_REQUEST", &SetPickShuttleDeviceThenWaitOnShuttle, 1, false, kPostNone, 2281 },
{ "olp.pickShuttleDeviceTogether32SiteN","PickShuttleDeviceTogetherFor32SiteN_REQUEST",&SetPickShuttleDeviceTogetherFor32SiteN,1, false, kPostNone, 2286 },
{ "olp.indexArm1Height",                 "IndexArm1Height_REQUEST",                 &SetIndexArm1Height,                 4, false, kPostNone, 2291 },
{ "olp.indexArm2Height",                 "IndexArm2Height_REQUEST",                 &SetIndexArm2Height,                 4, false, kPostNone, 2296 },
{ "olp.testICCheckMode",                 "TestICCheckMode_REQUEST",                 &SetTestICCheckMode,                 1, false, kPostNone, 2301 },
{ "olp.aboveSocket",                     "AboveSocket_REQUEST",                     &SetAboveSocket,                     1, false, kPostNone, 2306 },
{ "olp.hotPlate1",                       "HotPlate1_REQUEST",                       &SetHotPlate1,                       1, false, kPostNone, 2311 },
{ "olp.hotPlate2",                       "HotPlate2_REQUEST",                       &SetHotPlate2,                       1, false, kPostNone, 2316 },
{ "olp.testerInitialMaximumTest",        "TesterInitialMaximumTest_REQUEST",        &SetTesterInitialMaximumTest,        1, false, kPostNone, 2321 },
{ "olp.testerMaximumTest",               "TesterMaximumTest_REQUEST",               &SetTesterMaximumTest,               1, false, kPostNone, 2326 },
{ "olp.testerDummyTest",                 "TesterDummyTest_REQUEST",                 &SetTesterDummyTest,                 1, false, kPostNone, 2331 },
{ "olp.testerStartDelay",                "TesterStartDelay_REQUEST",                &SetTesterStartDelay,                1, false, kPostNone, 2336 },
{ "olp.hotZ1Down",                       "HotZ1Down_REQUEST",                       &SetHotZ1Down,                       1, false, kPostNone, 2341 },
{ "olp.hotShuttleSoakMode",              "HotShuttleSoakMode_REQUEST",              &SetHotShuttleSoakMode,              1, false, kPostNone, 2346 },
{ "olp.ambientCheck",                    "AmbientCheck_REQUEST",                    &SetAmbientCheck,                    1, false, kPostNone, 2351 },
{ "olp.ambientCheckTemp",                "AmbientCheckTemp_REQUEST",                &SetAmbientCheckTemp,                1, false, kPostNone, 2356 },
// golden 的分支名確實拼成 "Tempearture"（automation.cpp:2361），不是筆誤，
// 是 golden 的拼字。對外的指令名用正確拼法，golden 欄保留原樣以便對帳。
{ "olp.temperatureOffset",               "TempeartureOffset_REQUEST",               &SetTemperatureOffset,               1, false, kPostNone, 2361 },
{ "olp.contactCountForOffsetPeriod",     "ContactCountForOffsetPeriod_REQUEST",     &SetContactCountForOffsetPeriod,     1, false, kPostNone, 2366 },
{ "olp.contactCountForCoolDown",         "ContactCountForCoolDown_REQUEST",         &SetContactCountForCoolDown,         1, false, kPostNone, 2371 },
};

const int kOlpWhitelistCount = (int)(sizeof(kOlpWhitelist) / sizeof(kOlpWhitelist[0]));

// =============================================================================
//  拒絕名單 —— 每一筆都寫明**缺的是哪一個相依**，不是「怕」。
//
//  ## A 類：會讓機台動（S3 地盤，要使用者在機台旁）
//    START_REQUEST          golden :1976   打到 TfMain::Start()
//    CLEANOUT_REQUEST       golden :2012
//    HOMEANDSTART_REQUEST   golden :2019   DoHomeAndStart()
//    ONECYCLE_REQUEST       golden :2026   DoOneCycle()
//  ⇒ 這一類不是「相依不存在」，是**範圍**：計畫書已裁決它們屬 S3。
//
//  ## B 類：相依不存在 —— setter 的回傳值來自 W5FA_* 蜃影
//  （auto9045.cpp:198-242。蜃影本體是 `{ return 0; }`，ack 會說「成功」
//   而什麼都沒發生 —— 正是完成條件禁止的「罐頭 ok」。）
//
//   指令                     缺的相依（golden 那一邊的真身）
//   ----------------------  -------------------------------------------------
//   CATEGORY_REQUEST        fBinSel / fShowBinSelect 兩個表單都還沒翻。
//                           SetCategory（:429，83 行）整段只寫蜃影，回傳恆 0。
//                           ⚠ 而且它的後置動作會寫 LastSet.OLPSetBinErr[0]，
//                           恆 0 ⇒ 等於**清掉**一個啟動互鎖。開它比不開危險。
//   BINDEFINE_REQUEST       同上（OLPSetBinErr[1]）。另有一個獨立的
//                           **越界讀**：auto9045.cpp:543 的
//                           `for(i=0; i<=iTestBinCount; i++) Data[i]`
//                           對上 automation.cpp:1307 的 `AnsiString Data[40]`，
//                           而 fTesterIF.cpp:1160 把 iTestBinCount 設到 255、
//                           :1174-1175 的夾限被註解掉。
//                           ★ golden 完全相同（golden automation.cpp:960 /
//                           auto9045.cpp:155）⇒ 依 §0.5 **不修**；但也不能
//                           把 golden 的 UB 路徑變成瀏覽器可達。
//   FIXTRAYDEFINE 例外       它的 setter 有真的 WriteIniData（:606-656），
//                           蜃影只用在 UI 刷新 ⇒ **在**白名單上。
//   SETUP_FILE_NAME_REQUEST fMain 的 cbSetupFileName 是空的蜃影 combo box
//                           （auto9045.cpp:906 IndexOf 必回 -1）⇒ 回傳恆 3
//                           「找不到工作檔」，換配方永遠不會發生。
//                           缺：TfMain::ChangeSetUpFile() 與真的工作檔清單。
//   TEMPMODE_REQUEST        return W5FA_FMain.ChangeTempMode(...) -> 恆 0。
//                           LastSet.iTemperature 有被寫，但真正換溫控模式的
//                           那一步沒發生。缺：golden TfMain::ChangeTempMode()。
//   CONNECTION_REQUEST      return W5FA_FMain.ChangeTesterConnect(...) -> 恆 0。
//                           缺：golden TfMain::ChangeTesterConnect()。
//   TESTMODE_REQUEST        只寫 W5FA_FAutomation.TestMode（蜃影欄位）。
//                           缺：TfAutomation 真身的 TestMode 消費者。
//   LotInfo_REQUEST         回傳恆 2。缺：fLotInfo 表單真身。
//   StartMode_REQUEST       回傳恆 2。缺：TfMain::CheckCanChangeRealDummy()
//                           的真本體（現在是 auto9045.cpp:208-215 的樁）。
//
//  ## C 類：OLP 協定 / 檔案傳輸，對瀏覽器沒有意義或攻擊面太大
//    ON_LINE_REQUEST / INITIATE_REQUEST   OLP 連線握手，沒有 OLP host 就沒有語意
//    PP_UL_REQUEST / PP_DL_REQUEST        工作檔壓縮包上下傳（DoDLRequest 53 行，
//                                         會解壓縮寫檔）—— 另案處理
//    CLEAR_REPORT_REQUEST                 清生產報表，破壞性資料操作 —— 另案
//
//  ## D 類：golden 的本體就是空的（11 筆）
//    AlarmMode / AllSiteFail / ByHeadFail / ByBinAll / SetSiteYield /
//    BinOverLimitSelec / BinOverLimitSet / BinOverCountSet /
//    SetContactTestMode / SetDropHeight / SetReleaseWait
//    ⇒ 驗過 golden automation.cpp:1775-1777 的 AllSiteFail_REQUEST 同樣是 `{}`。
//       **忠實翻譯，不是漏翻。** 沒有東西可以叫，所以不在表上。
//
//  ## E 類：已經有更好的路
//    PAUSE_REQUEST   golden :1959 就是 `bLockByServer=true; SoftStop=true;`，
//                    與 PauseFromWeb() 逐字相同 ⇒ 用既有的 pause.run。
//    RESUME_REQUEST  golden :1967 清 bLockByServer（刻意不清 SoftStop，
//                    見該處 P8 的 preserved-quirk 註記）。
//                    ⚠ 20260920 更正：我原本在這裡寫「沒有它，瀏覽器可以把
//                    機台鎖住而解不開」。**量過之後不成立** ——
//                    `PauseFromWeb()`（WebStart.cpp:3632）只設 SoftStop，
//                    不碰 bLockByServer；而 bLockByServer 的唯二寫入點是
//                    automation.cpp:1961/:1969 那兩個分支，今天沒有任何
//                    呼叫得到（ProcessBuffer 不在 wb_serve 這條路上）。
//                    ⇒ 瀏覽器今天設不了 bLockByServer，不存在解不開的鎖。
//                    排除 RESUME 的代價現在是**零**。
//                    ⚠ 但這是一條會過期的「不存在」宣稱：哪天真的
//                    PAUSE_REQUEST 被接上來（或 ProcessBuffer 被啟用），
//                    ckernel.cpp:865「Lock by Host, need unlocked from HOST
//                    or restart program」那條路就活了，**那時 RESUME 必須
//                    同時接上**，否則才會變成鎖住解不開。
//    PAUSE_REQUEST（第二個，golden :1991）是 golden 的死碼（前面那個永遠先
//                    命中），移植樹已就地註記保留。
// =============================================================================

const OlpRow* FindRow(const std::string& cmd)
{
    for (int i = 0; i < kOlpWhitelistCount; ++i)
        if (cmd == kOlpWhitelist[i].cmd)
            return &kOlpWhitelist[i];
    return 0;
}

const char* StatusText(int st)
{
    switch (st) {
    case kWebOlpOk:           return "設定完成，已寫回配方檔";
    case kWebOlpNeedCleanOut: return "機台裡還有 IC，要先清空才能改設定 (CheckNeedCleanOut)";
    case kWebOlpSystemStart:  return "機台正在運轉，不能改設定 (CheckSystemStart)";
    case kWebOlpRejected:     return "設定值被拒絕（不合法或互相矛盾）";
    default:                  return "setter 回傳了未預期的碼";
    }
}

} // namespace

// -----------------------------------------------------------------------------
WebOlpResult WebOlpInvoke(const std::string& cmd, const std::vector<std::string>& args)
{
    WebOlpResult r;
    r.accepted = false;
    r.status   = kWebOlpNotWhitelisted;

    const OlpRow* row = FindRow(cmd);
    if (row == 0) {
        r.detail = "指令不在白名單上：" + cmd;
        return r;
    }
    r.golden = row->golden;

    // --- 欄數檢查 ------------------------------------------------------------
    // arity < 0 = 依機台站數動態決定（SetMapping / SetDutOnOff 走
    // `index = i*MaxJ+j`，上限量過是 2x8 = 16，塞得進 golden 的 Data[40]）。
    // 這種的只能檢查「不超過 golden 的欄位數」，多寡由 setter 自己判 ——
    // 跟 golden 收到 host 訊框時的處境一模一樣（V_Total 說幾個就填幾個）。
    const int given = (int)args.size();
    if (given > kGoldenDataSlots) {
        r.status = kWebOlpBadArity;
        char buf[128];
        std::snprintf(buf, sizeof(buf),
                      "資料欄 %d 個，超過 golden 訊框的上限 %d", given, kGoldenDataSlots);
        r.detail = buf;
        return r;
    }
    if (row->arity >= 0 && given != row->arity) {
        r.status = kWebOlpBadArity;
        char buf[128];
        std::snprintf(buf, sizeof(buf),
                      "%s 需要 %d 個資料欄，收到 %d 個", row->golden, row->arity, given);
        r.detail = buf;
        return r;
    }

    // --- 照 golden 的訊框形狀擺資料 -----------------------------------------
    // 固定 40 格，沒填到的維持空字串 —— golden 的 ProcessBuffer 也是這樣
    // （automation.cpp:1337 只填 V_Total 個，其餘留著 AnsiString 的預設值）。
    AnsiString Data[kGoldenDataSlots];
    for (int i = 0; i < given; ++i)
        Data[i] = AnsiString(args[i].c_str());

    // --- golden 的分支層守衛 -------------------------------------------------
    // 例：golden automation.cpp:2131-2143（TEMPERATURE_REQUEST）
    //     if (CheckNeedCleanOut() == false) { SetTemperature(Data); Data[0]="0"; }
    //     else                              { Data[0] = "1"; }
    if (row->guardCleanOut && CheckNeedCleanOut()) {
        r.accepted = true;
        r.status   = kWebOlpNeedCleanOut;
        r.detail   = StatusText(kWebOlpNeedCleanOut);
        return r;
    }

    // --- 叫 golden 的真本體 --------------------------------------------------
    const int st = row->fn(Data);

    // --- golden 的後置動作 ---------------------------------------------------
    if (row->post == kPostOlpSetBinErr2) {
        // golden automation.cpp:2086-2090 逐字
        LastSet.OLPSetBinErr[2] = (st >= 3) ? st : 0;
    }

    r.accepted = true;
    r.status   = st;

    // ★ ack 帶真值，不是罐頭 ok。完成條件明文要求這一點，而 P2b 的教訓是
    //   「值在合法範圍內」跟「值有意義」是兩件事。
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s -> %d (%s)",
                  row->golden, st, StatusText(st));
    r.detail = buf;

    // golden 對 Bin 類設定有專門的紀錄（automation.cpp:2062 / :2073 / :2085
    // 的 NewRecordProcess）。這裡走移植樹既有的 RecordProcess，**不**走
    // automation.cpp 的 ShowMSG —— 那條路會在 500 行時寫出 log 檔。
    if (row->post == kPostOlpSetBinErr2)
        NewRecordProcess("", AnsiString(row->golden), AnsiString(r.detail.c_str()));

    return r;
}

// -----------------------------------------------------------------------------
std::vector<WebOlpCommandInfo> WebOlpListCommands()
{
    std::vector<WebOlpCommandInfo> out;
    out.reserve(kOlpWhitelistCount);
    for (int i = 0; i < kOlpWhitelistCount; ++i) {
        WebOlpCommandInfo info;
        info.cmd           = kOlpWhitelist[i].cmd;
        info.golden        = kOlpWhitelist[i].golden;
        info.arity         = kOlpWhitelist[i].arity;
        info.guardCleanOut = kOlpWhitelist[i].guardCleanOut;
        out.push_back(info);
    }
    return out;
}
