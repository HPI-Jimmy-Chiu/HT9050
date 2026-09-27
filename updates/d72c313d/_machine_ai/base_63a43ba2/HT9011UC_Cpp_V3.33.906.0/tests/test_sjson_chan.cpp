// ===========================================================================
//  tests/test_sjson_chan.cpp
//  S8（生產數值）／S9（IO 位元打包 ＋ Motor）／S11（動作通道）的離線不變量。
//
//  AI(W906-SJSON-S8S9S11-TESTS) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 六、八 S8/S9/S11
//
//  ---------------------------------------------------------------------------
//  ⚠⚠ 這支測試**絕對不可以**呼叫 ClarnDataBody() 或 act 的非 dryRun 路徑（AI(W906-CLARN-R12) 20260926：第 12 條起「沒帶 dryRun」就是非 dryRun 路徑）
//  ---------------------------------------------------------------------------
//  那條路會呼叫 WriteLastDataFile()，而它寫的是**硬編路徑**
//  `D:\HT9045\system\lastdata.dat`（cprod.cpp:2022）——
//  那是量產機共用的執行期檔，`--dry` 蓋不到（SKILL §十二 R8），
//  而且這棵樹已經有過一次靜默毀掉 lastdata 的代價。
//  這裡只驗 **dryRun 路徑、守衛、預覽表**。
//  ⇒ 所以下面刻意**不安裝** ClarnDataBody（不呼叫 InstallClarnDataBody()）：
//    就算某一條斷言寫錯、走到了 dryRun=false，守衛 `body-not-installed`
//    仍然會擋下來。兩層保護，因為一層寫錯就是刪檔。預設值（沒帶 dryRun）的斷言一律放在 A61 守衛開著的區塊裡，仍是兩道保護（A61＋body-not-installed）。
//
//  ---------------------------------------------------------------------------
//  各段守什麼
//  ---------------------------------------------------------------------------
//  S8  * 差量恆 >= 0：計數被清零時送 0 而不是負數（負數在畫面上會讀成
//        「產出減少」，但真正發生的事是 Clarn_Data 把計數歸零）。
//      * 第一次上線時差量是 0，不是「從 0 跳到 12,345」。
//      * **靜止時 prod.changed == 0**：這就是 §六「靜止時 0」的可量測形式。
//      * 在位性：來源沒載入時送 null 而且 tag **仍然在快照裡**。
//        不 stage 的話它會被 TagPatch 報成 removed -> 線上編碼成 null，
//        收端看起來一樣，但「停在 0」與「未載入」就此混為一談。
//
//  S9  * base64 編碼正確（RFC 4648 測試向量）。
//      * 影像長度固定。⚠ **四個平面的大小不一樣，而且很容易講錯**
//        （20260923 更正：本段第一版寫「DI/DO 影像 40 bytes -> 56 字元」，
//         那是**在位平面**的大小，不是資料平面的）：
//            io.di        320 bytes -> 428 字元   （每個 port 一個 byte）
//            io.di.valid   40 bytes ->  56 字元   （每個 port 一個 bit）
//            io.do        192 bytes -> 256 字元
//            io.do.valid   24 bytes ->  32 字元
//        固定長度是選 base64 而不是逗號串的理由 —— patch 大小不隨機台忙碌
//        程度漂移。
//      * 沒有監看器（或一個 port 都沒採到）時四個平面**一律 null**，不是
//        320 個 0x00 編出來的 "AAAA..."。後者是一句「我看過了，全部是 0」的謊話。
//      * motor.axes 同理：整個 null，不是 32 列 [0,0,0]。
//
//  S11 * 12 個 Tag 的預覽表與 golden 的 12 個 if 分支一致（逐個列出 ct*）。
//      * Tag==10 的分支是空的 —— 預覽只剩共用尾段。這一條防的是
//        「看到空分支以為漏翻了，順手補一點東西進去」。
//      * 守衛 bA61DisableCleanMUBA 會回報而不是靜靜什麼都不做。
//      * 越界 tag（-1 / 13 / 8.5）被擋下來。
//
//  Build：link god-stack（ChanProduction/ChanAction 讀 LastSet、Prod、
//  IniConfig，那些全域在 ht9045_globals）。
//  ⚠ 本檔不讀任何 ini、不開 socket：所有值都直接寫進全域再讀回來。
//  Non-zero exit on any failure.
// ===========================================================================
#include "JsonBridge/ChanProduction.h"
#include "JsonBridge/ChanIo.h"
#include "JsonBridge/ChanMotor.h"
#include "JsonBridge/ChanAction.h"
#include "JsonBridge/actions/MainClarnData.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cprod.h"
#include "LastSet.h"
#include "Config.h"
#include "cmydef.h"   // CUSTOMER_CODE

#include <cstdio>
#include <cstring>
#include <string>

static int g_fail = 0;
static int g_pass = 0;
static void check(bool cond, const char* what, const char* file, int line) {
    if (cond) { ++g_pass; }
    else { std::printf("FAIL %s:%d  %s\n", file, line, what); ++g_fail; }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

using namespace ht9045::sjson;

static bool has(const std::string& h, const char* n) {
    return h.find(n) != std::string::npos;
}

// 跑一個 publish 週期並把快照取回來。
static webbridge::TagSnapshotView stageOnce(std::size_t* staged = 0) {
    static webbridge::TagSnapshot snap;
    snap.beginPublish();
    const std::size_t n = StageProduction(snap);
    if (staged) *staged = n;
    snap.commitPublish();
    return snap.read();
}

int main() {
    // =====================================================================
    //  S8 -- 生產數值
    // =====================================================================

    // ---- 來源沒載入：全 null，而且 tag 仍然在快照裡 ---------------------
    std::memset(&LastSet, 0, sizeof(LAST_GENERAL_SET));
    ProductionResetForTest();
    {
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(v.tags.count("prod.live") == 1);
        CHECK(v.tags.find("prod.live")->second.asBool(true) == false);
        // 絕對值的 tag 在，值是 null。
        CHECK(v.tags.count("prod.index") == 1);
        CHECK(v.tags.find("prod.index")->second.isNull());
        CHECK(v.tags.count("prod.loading.d") == 1);
        CHECK(v.tags.find("prod.loading.d")->second.isNull());
        CHECK(ProductionChangedLastTick() == 0);
    }

    // ---- 載入之後：第一 tick 的差量必須是 0 -----------------------------
    LastSet.iIndexCount = 100;
    LastSet.SendCT[0]   = 50;
    ProductionResetForTest();
    {
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(v.tags.find("prod.live")->second.asBool(false) == true);
        CHECK(v.tags.find("prod.index")->second.asInt(-1) == 100);
        // 見檔頭 S8 第 2 條：第一次不可以噴出一個假的巨大差量。
        CHECK(v.tags.find("prod.index.d")->second.asInt(-1) == 0);
        CHECK(v.tags.find("prod.loading.d")->second.asInt(-1) == 0);
    }

    // ---- 靜止：changed == 0（§六「靜止時 0」的可量測形式） --------------
    {
        stageOnce();
        CHECK(ProductionChangedLastTick() == 0);
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(ProductionChangedLastTick() == 0);
        CHECK(v.tags.find("prod.changed")->second.asInt(-1) == 0);
    }

    // ---- 動了：差量出得來，ver 前進 -------------------------------------
    unsigned long long verBefore = 0, verAfter = 0;
    {
        const webbridge::TagSnapshotView v0 = stageOnce();
        verBefore = (unsigned long long)v0.tags.find("prod.ver")->second.asInt(0);
        LastSet.iIndexCount = 107;
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(v.tags.find("prod.index")->second.asInt(-1) == 107);
        CHECK(v.tags.find("prod.index.d")->second.asInt(-1) == 7);
        CHECK(ProductionChangedLastTick() == 1);
        verAfter = (unsigned long long)v.tags.find("prod.ver")->second.asInt(0);
        CHECK(verAfter == verBefore + 1);
    }

    // ---- 清零：差量送 0，不送負數（檔頭 S8 第 1 條） ---------------------
    {
        LastSet.iIndexCount = 0;
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(v.tags.find("prod.index")->second.asInt(-1) == 0);
        CHECK(v.tags.find("prod.index.d")->second.asInt(-1) == 0);
    }

    // ---- 絕對值不重複發：sort.loading 已經有了，prod 只補差量 ------------
    {
        const webbridge::TagSnapshotView v = stageOnce();
        CHECK(v.tags.count("prod.loading.d") == 1);
        // ⚠ 這一條擋的是「順手也送一份絕對值」——兩套命名同一個數字，
        //   正是 S7 檔頭記下的、等著漂掉的形狀。
        CHECK(v.tags.count("prod.loading") == 0);
        CHECK(v.tags.count("prod.sorted") == 0);
        CHECK(v.tags.count("prod.sorted.d") == 1);
    }

    {
        const std::string s = ProductionSchemaJson();
        CHECK(has(s, "\"binding\":\"prod\""));
        CHECK(has(s, "sort.loading"));      // 交叉指向既有的絕對值 tag
        CHECK(has(s, "idleContract"));
        CHECK(has(s, "deltaRule"));
    }

    // =====================================================================
    //  S9 -- IO 位元打包
    // =====================================================================

    // RFC 4648 的測試向量。自己寫的編碼器就得自己證明它是對的。
    {
        const unsigned char f[]  = { 'f' };
        const unsigned char fo[] = { 'f','o' };
        const unsigned char foo[]= { 'f','o','o' };
        CHECK(Base64Encode(f,   1) == "Zg==");
        CHECK(Base64Encode(fo,  2) == "Zm8=");
        CHECK(Base64Encode(foo, 3) == "Zm9v");
        const unsigned char zero[3] = { 0, 0, 0 };
        CHECK(Base64Encode(zero, 3) == "AAAA");
        const unsigned char ff[3] = { 0xFF, 0xFF, 0xFF };
        CHECK(Base64Encode(ff, 3) == "////");
    }

    // AI(W906-SJSON-S9) 20260923: **資料平面**的長度 —— 這是 SKILL.md §六 與
    //   派工單算錯的那一條，所以一定要有測試釘住，不能只靠註解。
    //   1203 的一個 port 是一個 byte（Pci1203Monitor.h:1099 byteData 是
    //   unsigned char），所以 DI 是 320 bytes 不是 40 bytes。
    //   ⚠ 沒有這兩條斷言的話，有人照規格把緩衝改成 40 bytes，測試還是全綠。
    {
        unsigned char diData[320], doData[192];
        std::memset(diData, 0x5A, sizeof(diData));
        std::memset(doData, 0xA5, sizeof(doData));
        CHECK(Base64Encode(diData, sizeof(diData)).size() == 428);   // io.di
        CHECK(Base64Encode(doData, sizeof(doData)).size() == 256);   // io.do
        // 這兩個數字必須與 schema 宣告的 b64len 一致，否則收端會切錯。
        const std::string sch = IoSchemaJson();
        CHECK(has(sch, "\"b64len\":428"));
        CHECK(has(sch, "\"b64len\":256"));
    }

    // **在位平面**的長度：40 bytes -> 56 字元、24 bytes -> 32 字元。
    {
        unsigned char di[40], dobuf[24];
        std::memset(di, 0x00, sizeof(di));
        std::memset(dobuf, 0xA5, sizeof(dobuf));
        CHECK(Base64Encode(di, sizeof(di)).size() == 56);
        CHECK(Base64Encode(dobuf, sizeof(dobuf)).size() == 32);
        // 值變了長度不變 —— 這是選 base64 的理由。
        std::memset(di, 0xFF, sizeof(di));
        CHECK(Base64Encode(di, sizeof(di)).size() == 56);
    }

    // 沒有監看器：四個平面一律 null（檔頭 S9 第 3 條）。
    // ⚠ 這支測試離線跑，Pci1203Monitor() 必然是 0；這正是要驗的那個狀態。
    {
        webbridge::TagSnapshot snap;
        snap.beginPublish();
        const std::size_t n = StageIo(snap);
        CHECK(n == 7);
        snap.commitPublish();
        const webbridge::TagSnapshotView v = snap.read();
        CHECK(v.tags.find("io.di")->second.isNull());
        CHECK(v.tags.find("io.di.valid")->second.isNull());
        CHECK(v.tags.find("io.do")->second.isNull());
        CHECK(v.tags.find("io.do.valid")->second.isNull());
        // ver 一定要送得出來（它是 patch 的觸發點，不是量測值）。
        CHECK(v.tags.find("io.ver")->second.isInt());
    }
    {
        const std::string s = IoSchemaJson();
        CHECK(has(s, "\"anyLive\":false"));
        // AI(W906-SJSON-S9) 20260923: anyLive 與 monitorPresent 必須是**兩個**
        //   欄位。第一版只有 anyLive 而且判準是 `mon != 0`，實跑時（沒有
        //   HAVE_PCI1203 的 binary）監看器物件存在但 0 個 port，schema 於是
        //   自稱 anyLive:true —— 一句謊話。見 ChanIo.cpp StageIo() 的更正註解。
        CHECK(has(s, "\"monitorPresent\""));
        CHECK(has(s, "\"bytes\":320"));          // DI 影像 320 byte
        CHECK(has(s, "\"bytes\":40"));           // DI valid 平面 40 byte
        CHECK(has(s, "\"bytes\":192"));          // DO 影像
        CHECK(has(s, "\"bytes\":24"));           // DO valid 平面
        CHECK(has(s, "lsb-first-within-byte"));
        // 誠實欄位：512 個舊 tag 還在。
        CHECK(has(s, "\"legacyPerPortTagsStillPublished\":true"));
        CHECK(has(s, "\"legacyPerPortTagCount\":512"));
    }

    // =====================================================================
    //  S9 -- Motor
    // =====================================================================
    {
        webbridge::TagSnapshot snap;
        snap.beginPublish();
        const std::size_t n = StageMotor(snap);
        CHECK(n == 3);
        snap.commitPublish();
        const webbridge::TagSnapshotView v = snap.read();
        // 整個 null，不是 32 列 [0,0,0]（0 是合法座標、0 是 STA_AX_DISABLE）。
        CHECK(v.tags.find("motor.axes")->second.isNull());
        CHECK(v.tags.find("motor.count")->second.isNull());
    }
    {
        const std::string s = MotorSchemaJson();
        CHECK(has(s, "\"rows\":32"));
        CHECK(has(s, "\"name\":\"actPos\""));
        CHECK(has(s, "\"name\":\"cmdVel\""));
        CHECK(has(s, "\"name\":\"state\""));
        CHECK(has(s, "\"anyLive\":false"));
        CHECK(has(s, "\"monitorPresent\""));   // 同 io：兩件事要分開送
        CHECK(has(s, "rowNullMeans"));
        // 32 列的身分表要在（沒有它，第 5 列是哪根馬達無解）。
        CHECK(has(s, "\"idx\":31"));
    }

    // =====================================================================
    //  S11 -- 動作通道（**只走 dryRun 與守衛**，見檔頭的 ⚠⚠）
    // =====================================================================
    CHECK(IsActionCommand("act.main.clarnData"));
    CHECK(IsActionCommand("act.counterClear.exe"));
    CHECK(!IsActionCommand("counter.clear"));
    CHECK(!IsActionCommand("struct.put"));

    // 本體沒安裝（這支刻意不裝）-> 守衛擋下，而且**明講**原因。
    IniConfig.bA61DisableCleanMUBA = false;
    {
        const std::string r =
            HandleAction("act.main.clarnData", "{\"tag\":8,\"dryRun\":true}");
        CHECK(has(r, "\"executed\":false"));
        CHECK(has(r, "\"guard\":\"body-not-installed\""));
        CHECK(!ClarnDataBodyInstalled());
    }

    // golden 自己的守衛：回報，不是靜靜沒反應（SKILL §4.5 規則 2）。
    IniConfig.bA61DisableCleanMUBA = true;
    {
        const std::string r =
            HandleAction("act.main.clarnData", "{\"tag\":1,\"dryRun\":true}");
        CHECK(has(r, "\"guard\":\"A61DisableCleanMUBA\""));
        CHECK(has(r, "V912 main.cpp:15468"));  CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":1}"), "\"dryRun\":false"));  CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":1,\"dryRun\":false}"), "\"dryRun\":false"));  CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":1,\"dryRun\":\"true\"}"), "\"dryRun\":true"));  CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":1}"), "\"guard\":\"A61DisableCleanMUBA\""));   //AI(W906-CLARN-R12) 20260926: 第 12 條 —— 沒帶／明確 false＝執行路徑（這裡被 A61 守衛擋下，不會碰 lastdata）；字串 "true"＝型別不對＝預覽
        // 守衛擋下時預覽也是空的 —— 連共用尾段都不會跑。
        CHECK(ClarnDataPreview(1).empty());
    }
    IniConfig.bA61DisableCleanMUBA = false;

    // 壞參數。
    {
        CHECK(has(HandleAction("act.main.clarnData", "not json"), "bad-payload"));
        CHECK(has(HandleAction("act.main.clarnData", "{\"dryRun\":true}"), "bad-payload"));
        CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":13}"), "bad-tag"));
        CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":-1}"), "bad-tag"));
        CHECK(has(HandleAction("act.main.clarnData", "{\"tag\":8.5}"), "bad-tag"));
        CHECK(has(HandleAction("act.nope.nope", "{}"), "unknown-action"));
    }

    // AI(W906-GB-P2e) 20260926: act.main.testerConnect -- golden imgTesterClick (main.cpp:29736) returns while the
    //   machine runs; the action reports it and changes nothing.  (The full toggle needs ChangeTesterConnect = P2d.)
    {
        const bool oldStart = SystemStart;
        const int oldTester = LastSet.iTester;
        SystemStart = true;
        const std::string tc = HandleAction("act.main.testerConnect", "{}");
        CHECK(has(tc, "system-running") || has(tc, "forms-not-created"));
        CHECK(has(tc, "\"executed\":false"));
        CHECK(LastSet.iTester == oldTester);
        SystemStart = oldStart;
    }
    { const std::string cr = HandleAction("act.main.clearRecord", "{\"dryRun\":true}"); CHECK(!has(cr, "unknown-action") && has(cr, "\"executed\":false")); }   // AI(W906-S119) 20260927
    { const std::string md = HandleAction("act.main.meShuttle2Dbl", "{\"dryRun\":true}"); CHECK(!has(md, "unknown-action") && has(md, "\"executed\":false")); }   // AI(W906-S119) 20260927

    // ---- 預覽表 vs golden 的 12 個分支 ----------------------------------
    //  ⚠ 這是把 golden 的分支抄第二遍之後唯一擋得住「兩邊漂掉」的東西。
    //     改 ClarnDataBody 的分支就要改 ClarnDataPreview，這裡會紅。
    CUSTOMER_CODE = 957;                     // 不是 SCS/ASE_Korea/AMKOR/KYEC_LEE
    IniConfig.bVTESTFunction = false;
    {
        // Tag 1：Loading + Sort + Tester + Index
        const std::vector<std::string> p1 = ClarnDataPreview(1);
        CHECK(p1.size() == 4 + 5);           // 4 個 ct* + 5 行共用尾段
        CHECK(p1[0] == "ctLoadingCounts");
        CHECK(p1[1] == "ctTraySortCount");
        CHECK(p1[2] == "ctTesterCategory");
        CHECK(p1[3] == "ctIndexCount");

        // Tag 2：同 Tag1 但**沒有** TraySortCount（golden 自己註解掉那行）
        const std::vector<std::string> p2 = ClarnDataPreview(2);
        CHECK(p2.size() == 3 + 5);
        bool sawSort = false;
        for (std::size_t i = 0; i + 5 < p2.size() + 0u; ++i)
            if (p2[i] == "ctTraySortCount") sawSort = true;
        CHECK(!sawSort);

        // Tag 8：VTEST 關著 -> Tester 會清；CUSTOMER_CODE 不在三個例外裡
        //        -> Loading 會清。
        const std::vector<std::string> p8 = ClarnDataPreview(8);
        CHECK(p8.size() == 4 + 5);
        CHECK(p8[0] == "ctLoadingCounts");
        CHECK(p8[2] == "ctTesterCategory");

        // VTEST 打開 -> Tester 不清。golden :15584 的條件。
        IniConfig.bVTESTFunction = true;
        CHECK(ClarnDataPreview(8).size() == 3 + 5);
        IniConfig.bVTESTFunction = false;

        // 客戶碼在三個例外裡 -> Loading 不清。golden :15576-15578。
        CUSTOMER_CODE = CC_SCS;
        CHECK(ClarnDataPreview(8).size() == 3 + 5);
        CUSTOMER_CODE = 957;

        // Tag 10：golden 的分支是空的 -> 只剩共用尾段（檔頭 S11 第 2 條）
        CHECK(ClarnDataPreview(10).size() == 5);

        // Tag 12：只清 Loading 與 FailBin
        const std::vector<std::string> p12 = ClarnDataPreview(12);
        CHECK(p12.size() == 2 + 5);
        CHECK(p12[0] == "ctLoadingCounts");
        CHECK(p12[1] == "ctFailBinCount");

        // 共用尾段一定有 WriteLastDataFile —— 這是那條寫真實檔的路，
        // 預覽不講出來的話，操作員按下去不知道會動到什麼。
        const std::vector<std::string> pe = ClarnDataPreview(10);
        bool sawWrite = false;
        for (std::size_t i = 0; i < pe.size(); ++i)
            if (pe[i].find("WriteLastDataFile") != std::string::npos) sawWrite = true;
        CHECK(sawWrite);
    }

    // Tag 0：依 bCTClear[iMode][] 選擇。iMode 由 iRunStartMode 決定。
    {
        LastSet.iRunStartMode = 0;                       // <= rsmContinuRetest -> iMode 0
        for (int m = 0; m < 2; ++m)
            for (int c = 0; c < 7; ++c) LastSet.bCTClear[m][c] = false;
        // 一個都沒勾 -> 只有兩行 Yield 動作 + 5 行尾段
        CHECK(ClarnDataPreview(0).size() == 2 + 5);
        LastSet.bCTClear[0][ctLoadingCounts] = true;
        LastSet.bCTClear[0][ctTimeData]      = true;
        const std::vector<std::string> p0 = ClarnDataPreview(0);
        CHECK(p0.size() == 2 + 2 + 5);
        CHECK(p0[0] == "ctLoadingCounts");
        CHECK(p0[1] == "ctTimeData");
        // iMode 1 的那一排沒勾 -> 換模式就沒有 ct*
        LastSet.iRunStartMode = rsmContinuRetest + 1;
        CHECK(ClarnDataPreview(0).size() == 2 + 5);
    }

    // schema：13 個 tag 的語意與呼叫點數都要在
    {
        const std::string s = ActionSchemaJson();
        CHECK(has(s, "act.main.clarnData"));
        CHECK(has(s, "act.counterClear.exe"));
        CHECK(has(s, "\"aliasOf\":\"counter.clear"));
        CHECK(has(s, "\"goldenCallerTotal\":46"));
        CHECK(has(s, "lastdata.dat"));
        // 13 個 tag（0..12）都要列到
        CHECK(has(s, "\"tag\":0,"));
        CHECK(has(s, "\"tag\":12,"));
        // 兩個動作的 dryRun 預設都寫明了（AI(W906-CLARN-R12) 20260926：clarnData 從第 12 條起也是 false）
        CHECK(has(s, "\"dryRunDefault\""));  CHECK(has(s, "\"act.main.clarnData\":false"));  CHECK(has(s, "\"act.counterClear.exe\":false"));
    }

    std::printf("test_sjson_chan: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
