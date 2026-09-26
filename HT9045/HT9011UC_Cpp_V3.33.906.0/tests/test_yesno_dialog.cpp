// ===========================================================================
//  tests/test_yesno_dialog.cpp
//
//  AI(W906-YESNO) 20260925：ShowMyMessageBox_YES_NO 由替身換成真的呼叫之後的回歸。
//
//  使用者 20260925 裁決（docs/RULINGS_20260925.md 第 10 條，3A）：「YES/NO 對話框被
//  替身自動回答 → 照 golden 跳網頁對話框，等操作員回答」。在這之前五個 TU 各自放
//  `static int W?_ShowMyMessageBox_YES_NO(...){ return 0; }`，而 golden 只回 1＝Yes／
//  2＝No（mymessbox.dfm pnlYes Tag=1 / pnlNo Tag=2），判 `ret==2` 才拒絕的呼叫點於是
//  **不問就同意**（NB2 R15 普查：三題）。
//
//  驗的東西：
//    A. canary_support.cpp 的 ShowMyMessageBox_YES_NO 本身
//       A1 hook 未裝 -> 回 SimReturn（重設值 0 ＝ 替身時代的值）、capture 與計數正確
//       A2 hook 已裝 -> 回 hook 的答案（1/2/3 原封不動），S1/S2/S3 原樣交給 hook
//       A3 hook 回 0（＝沒有人能回答）-> 退回 SimReturn
//    B. 被自動同意的三題，各自走 golden 的兩個分支（答案經 hook 注入 ＝ wb_serve 的操作員答案）
//       B1 「Check bin setting?」   csystem.cpp CheckContinusStartIsReady（CC_AMKOR_Philippines）
//       B2 「Initial Start???」     csystem.cpp CheckContinusStartIsReady（其他客戶）
//       B3 「Sure To Setting Laser Value / 確定要儲存測距數值？」
//             OmronLaser/LaserSensorShuttle.cpp UseInArmCheckShtFloating／UseOutArmCheckShtFloating case 300
//       每一題都驗：No(2) 走拒絕臂、Yes(1) 走同意臂、hook 未裝時行為＝替身時代（0 ⇒ 同意臂）。
//    C. wb_serve 寫進 show-my-message 信箱的 JSON（tools/wb_dialog_mailbox.h 的
//       YesNoRequestJson／MessageIdleJson）用 cJSON 解析：合法、欄位對、';' 切法對、跳脫對。
//    ⓘ wb_serve 的等待迴圈本身（ForwardShowMyMessageBoxYesNo）是 static、要真的 socket，
//      這裡不驗；瀏覽器那一側另用無頭 Edge 驗過（見回報）。
//
//  ⚠ 真實檔：B3 的「是」會呼叫 fLaserSensor->SaveShuttleLaserValue()，它寫
//    DataPath + GetLastOpenFN() + "\HandlerCondition.Data"（golden 會寫進作用中工單）。
//    這支測試在呼叫前**自己把 DataPath 與 LastDataPath 換成工作目錄底下的暫存夾**
//    （<cwd>\_yesno_scratch），並在寫之前斷言換成功；換不成功就 FAIL 且不呼叫。
//    不碰 D:\HT9045\IniData、system、config。暫存夾刻意不放 %TEMP%（這台端點防護會
//    靜默擋掉 D:\HT9045 以外的寫入，見 tests/test_dialog_mailbox.cpp 檔頭）。
//    B1/B2 的「是」臂用 ART 那一格（Need setting Retest...）當早退出口：只呼叫 ShowMyMessage
//    （canary 記錄替身）就 return false，不往下碰任何檔。
// ===========================================================================
#include "canary_support.h"     // ShowMyMessageBox_YES_NO + seams, ShowMyMessage seam, LastSet
#include "cmydef.h"             // CUSTOMER_CODE, bNeedAskStartMode, USE_AUTO_RETEST, bAutoReTest_ART, bCanRunSCKART, AccessLevel
#include "MachineType.h"        // CC_AMKOR_Philippines, eartInstall, rsmInitial_ART, rsmInitialStart
#include "Config.h"             // IniConfig.bInitialStartNeedAsk
#include "cprod.h"              // BinSelect
#include "CosFunction.h"        // CosFunction.bUseSCKART

#include "../tools/wb_dialog_mailbox.h"   // w906dlg::YesNoRequestJson / MessageIdleJson（wb_serve 寫信箱用的那兩支）
#include "Public/cJSON.h"                 // C：用真的 JSON 解析器驗寫出去的字串

#include <windows.h>
#include <cstdio>
#include <string>

// 前置宣告（不 include 各自的標頭：common.h 與 canary_support.h 放在同一個 TU 會重複給預設引數，
//   見 tools/wb_serve.cpp 的同一段說明；LaserSensorShuttle.h 會帶進整個雷射表單層）。
extern AnsiString DataPath;                                        // common.cpp:225（golden common.cpp:33）
extern AnsiString LastDataPath;                                    // common.cpp:229（golden common.cpp:38）
bool CheckContinusStartIsReady();                                  // csystem.cpp（golden csystem.cpp:11663）
bool UseInArmCheckShtFloating(int iSht, bool bReset, bool bSetGold);   // OmronLaser/LaserSensorShuttle.h:41
bool UseOutArmCheckShtFloating(int iSht, bool bReset, bool bSetGold);  // OmronLaser/LaserSensorShuttle.h:42
extern int iUseInArmShtLaserCheckTask;                             // OmronLaser/LaserSensorShuttle.h:34
extern int iUseOutArmShtLaserCheckTask;                            // OmronLaser/LaserSensorShuttle.h:35

static int g_ok = 0, g_fail = 0;
static void Check(bool cond, const char* what)
{
    if (cond) { ++g_ok;   std::printf("ok   %s\n", what); }
    else      { ++g_fail; std::printf("FAIL %s\n", what); }
}

// ---------------------------------------------------------------------------
//  測試用 hook ＝ wb_serve 的 ForwardShowMyMessageBoxYesNo 的替身：記下題目、回預先設定的答案。
// ---------------------------------------------------------------------------
static int         g_hookAnswer = 0;
static int         g_hookCalls  = 0;
static std::string g_hookS1, g_hookS2, g_hookS3;
static int TestHook(const char* s1, const char* s2, const char* s3)
{
    ++g_hookCalls;
    g_hookS1 = s1 ? s1 : "(null)";
    g_hookS2 = s2 ? s2 : "(null)";
    g_hookS3 = s3 ? s3 : "(null)";
    return g_hookAnswer;
}
static void Arm(int answer)
{
    W906_ShowMyMessageBoxYesNo_Reset();
    W906_ShowMyMessage_Reset();
    g_hookAnswer = answer; g_hookCalls = 0; g_hookS1 = g_hookS2 = g_hookS3 = "";
    W906_ShowMyMessageBoxYesNo_Hook = &TestHook;
}
static void Disarm()
{
    W906_ShowMyMessageBoxYesNo_Hook = 0;
    W906_ShowMyMessageBoxYesNo_Reset();
    W906_ShowMyMessage_Reset();
    g_hookCalls = 0;
}

// ---------------------------------------------------------------------------
//  A. 本體
// ---------------------------------------------------------------------------
static void TestCore()
{
    std::printf("\n== A. ShowMyMessageBox_YES_NO 本體 ==\n");
    Disarm();
    int v = ShowMyMessageBox_YES_NO("Q1", "S2");
    Check(v == 0, "A1 hook 未裝 -> 回 SimReturn 重設值 0（替身時代的值）");
    Check(W906_ShowMyMessageBoxYesNo_Count == 1, "A1 計數 1");
    Check(W906_ShowMyMessageBoxYesNo_LastS1 == "Q1" && W906_ShowMyMessageBoxYesNo_LastS2 == "S2", "A1 capture S1/S2");
    W906_ShowMyMessageBoxYesNo_SimReturn = 2;
    v = ShowMyMessageBox_YES_NO("Q1b", "");
    Check(v == 2, "A1 hook 未裝、SimReturn=2 -> 回 2（seam 可設）");

    Arm(1);
    v = ShowMyMessageBox_YES_NO("Q2", "中文;sub", "S3x");
    Check(v == 1, "A2 hook 答 Yes -> 回 1");
    Check(g_hookCalls == 1, "A2 hook 被呼叫 1 次");
    Check(g_hookS1 == "Q2" && g_hookS2 == "中文;sub" && g_hookS3 == "S3x", "A2 S1/S2/S3 原樣交給 hook（UTF-8 不動、';' 不切 —— 切是畫面那側的事）");
    Arm(2);
    Check(ShowMyMessageBox_YES_NO("Q3", "") == 2, "A2 hook 答 No -> 回 2");
    Check(g_hookS3 == "", "A2 S3 省略 -> hook 收到空字串（不是 NULL）");
    Arm(3);
    Check(ShowMyMessageBox_YES_NO("Q4", "") == 3, "A2 hook 回 3（golden：框已開著）-> 原封不動回 3");

    Arm(0);
    W906_ShowMyMessageBoxYesNo_SimReturn = 2;
    v = ShowMyMessageBox_YES_NO("Q5", "");
    Check(v == 2 && g_hookCalls == 1, "A3 hook 回 0（沒有人能回答）-> 退回 SimReturn");
    Disarm();
}

// ---------------------------------------------------------------------------
//  B1 / B2. CheckContinusStartIsReady 的兩題
// ---------------------------------------------------------------------------
static void PrepStartCheck(int customer)
{
    IniConfig.bInitialStartNeedAsk = true;
    bNeedAskStartMode              = true;
    CUSTOMER_CODE                  = customer;
    AccessLevel                    = 3;
    // 「是」臂之後的第一個出口（csystem.cpp 同函式，golden :11736-11751）：ART 裝了、
    //   bin 沒設 autoretest ⇒ ShowMyMessage("Need setting Retest ...") 並 return false。
    bCanRunSCKART                  = false;
    USE_AUTO_RETEST                = eartInstall;
    bAutoReTest_ART                = true;
    CosFunction.bUseSCKART         = false;
    LastSet.iRunStartMode          = rsmInitial_ART;
    for (int i = 0; i < 3; i++) BinSelect[4].bAutoRetest[i] = false;
}
static void UnprepStartCheck()
{
    IniConfig.bInitialStartNeedAsk = false;
    bNeedAskStartMode              = false;
    CUSTOMER_CODE                  = 0;
    USE_AUTO_RETEST                = eartUninstall;
    bAutoReTest_ART                = false;
    LastSet.iRunStartMode          = 0;
}
static const char* kArtExit = "Need setting Retest on Combine bin in ART_Normal bin page";

static void TestStartCheck(int customer, const char* question, const char* tag)
{
    char buf[256];
    std::printf("\n== %s 「%s」(CUSTOMER_CODE=%d) ==\n", tag, question, customer);

    // No -> golden `if(ret==2) return false;`，不動 bNeedAskStartMode
    PrepStartCheck(customer);
    Arm(2);
    bool r = CheckContinusStartIsReady();
    std::snprintf(buf, sizeof(buf), "%s No：hook 問的題目是「%s」", tag, question);
    Check(g_hookCalls == 1 && g_hookS1 == question, buf);
    std::snprintf(buf, sizeof(buf), "%s No：回 false（拒絕啟動）且 bNeedAskStartMode 仍為 true（下次按 START 還會問）", tag);
    Check(r == false && bNeedAskStartMode == true, buf);
    std::snprintf(buf, sizeof(buf), "%s No：沒有走到「是」臂之後的 ART 出口", tag);
    Check(W906_ShowMyMessage_Count == 0, buf);

    // Yes -> golden else 臂：bNeedAskStartMode=false，往下走到 ART 出口
    PrepStartCheck(customer);
    Arm(1);
    r = CheckContinusStartIsReady();
    std::snprintf(buf, sizeof(buf), "%s Yes：bNeedAskStartMode=false（golden else 臂）", tag);
    Check(g_hookCalls == 1 && bNeedAskStartMode == false, buf);
    std::snprintf(buf, sizeof(buf), "%s Yes：往下走到 ART 出口（ShowMyMessage 最後一則＝%s）", tag, kArtExit);
    Check(r == false && W906_ShowMyMessage_LastS1 == kArtExit, buf);
    if (customer == CC_AMKOR_Philippines) {
        Check(AccessLevel == 0, "B1 Yes：AMKOR 臂把 AccessLevel 降成 0（golden :11686）");
    }

    // hook 未裝（沒有操作員的宿主）：SimReturn 0 ＝ 替身時代 ⇒ 不問就走 else 臂 —— 釘住「未裝 hook 行為不變」
    PrepStartCheck(customer);
    Disarm();
    r = CheckContinusStartIsReady();
    std::snprintf(buf, sizeof(buf), "%s hook 未裝：與替身時代相同（回 0 ⇒ 走同意臂，bNeedAskStartMode=false）", tag);
    Check(W906_ShowMyMessageBoxYesNo_Count == 1 && bNeedAskStartMode == false && W906_ShowMyMessage_LastS1 == kArtExit, buf);

    UnprepStartCheck();
    Disarm();
}

// ---------------------------------------------------------------------------
//  B3. 雷射測距值存檔確認（兩個函式各一處）
// ---------------------------------------------------------------------------
static std::string g_scratch;
static std::string Slurp(const std::string& path)
{
    std::string s;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return s;
    char b[4096]; size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}
static bool FileThere(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

static bool PrepScratch()
{
    char cwd[MAX_PATH] = {0};
    ::GetCurrentDirectoryA(MAX_PATH, cwd);
    g_scratch = std::string(cwd) + "\\_yesno_scratch";
    ::CreateDirectoryA(g_scratch.c_str(), NULL);
    ::CreateDirectoryA((g_scratch + "\\Data").c_str(), NULL);
    const std::string setup = g_scratch + "\\SetUp.inf";
    FILE* f = std::fopen(setup.c_str(), "wb");
    if (!f) return false;
    std::fputs("YESNO_TEST\r\n", f);
    std::fclose(f);
    DataPath     = AnsiString((g_scratch + "\\Data\\").c_str());
    LastDataPath = AnsiString(setup.c_str());
    // 吊帶：寫之前斷言兩個路徑真的都換掉了（不是 D:\HT9045\...）
    const std::string dp = DataPath.c_str(), lp = LastDataPath.c_str();
    const bool ok = dp.find(g_scratch) == 0 && lp.find(g_scratch) == 0 &&
                    dp.find("D:\\HT9045\\IniData") == std::string::npos;
    Check(ok, "B3 前置：DataPath／LastDataPath 已換成 <cwd>\\_yesno_scratch（不寫真實工單）");
    return ok;
}

static void TestLaser(bool outArm)
{
    const char* tag  = outArm ? "B3b UseOutArmCheckShtFloating" : "B3a UseInArmCheckShtFloating";
    const char* done = outArm ? "請取出Out Shuttle上IC" : "請取出Shuttle上IC";
    const char* key  = outArm ? "Laser Value Out Shuttle1 0000" : "Laser Value Shuttle1 0000";
    int& task = outArm ? iUseOutArmShtLaserCheckTask : iUseInArmShtLaserCheckTask;
    const std::string hc = g_scratch + "\\Data\\YESNO_TEST\\HandlerCondition.Data";
    char buf[256];
    std::printf("\n== %s case 300「Sure To Setting Laser Value」==\n", tag);

    // No -> golden `if(ret==2) Task=400;`，不存檔
    ::DeleteFileA(hc.c_str());
    Arm(2);
    task = 300;
    bool r = outArm ? UseOutArmCheckShtFloating(0, false, false) : UseInArmCheckShtFloating(0, false, false);
    std::snprintf(buf, sizeof(buf), "%s No：hook 問的是「Sure To Setting Laser Value」", tag);
    Check(g_hookCalls == 1 && g_hookS1 == "Sure To Setting Laser Value", buf);
    std::snprintf(buf, sizeof(buf), "%s No：Task=400、回 false、沒有存測距值", tag);
    Check(task == 400 && r == false && !FileThere(hc) && W906_ShowMyMessage_Count == 0, buf);

    // Yes -> golden else 臂：SaveShuttleLaserValue + ShowMyMessage(done) + flag=true
    ::DeleteFileA(hc.c_str());
    Arm(1);
    task = 300;
    r = outArm ? UseOutArmCheckShtFloating(0, false, false) : UseInArmCheckShtFloating(0, false, false);
    std::snprintf(buf, sizeof(buf), "%s Yes：回 true、Task 留在 300、ShowMyMessage「%s」", tag, done);
    Check(r == true && task == 300 && W906_ShowMyMessage_LastS1 == done, buf);
    std::snprintf(buf, sizeof(buf), "%s Yes：測距值真的存了（暫存夾的 HandlerCondition.Data 有 [Laser] %s）", tag, key);
    Check(Slurp(hc).find(key) != std::string::npos, buf);

    // hook 未裝：替身時代的行為（0 ⇒ else 臂 ⇒ 存檔）—— 就是 NB2 R15 說的「不問就存」
    ::DeleteFileA(hc.c_str());
    Disarm();
    task = 300;
    r = outArm ? UseOutArmCheckShtFloating(0, false, false) : UseInArmCheckShtFloating(0, false, false);
    std::snprintf(buf, sizeof(buf), "%s hook 未裝：與替身時代相同（回 0 ⇒ 不問就存）", tag);
    Check(r == true && FileThere(hc), buf);

    task = 1;
    Disarm();
}

// ---------------------------------------------------------------------------
//  C. 信箱請求的 JSON（wb_serve 的 DialogMailboxPostYesNo／DialogMailboxRetireMessage 寫的就是這兩支的輸出）
//     半截或壞掉的 JSON ＝ 框不出來、C++ 在等，而且只有對方解析時才看得到 —— 所以用真的 cJSON 驗。
// ---------------------------------------------------------------------------
static const char* Str(cJSON* o, const char* k) { cJSON* i = o ? cJSON_GetObjectItem(o, k) : 0; return (i && i->valuestring) ? i->valuestring : "(missing)"; }
static cJSON* Obj(cJSON* o, const char* k) { return o ? cJSON_GetObjectItem(o, k) : 0; }
static bool IsTrue(cJSON* o, const char* k) { return cJSON_IsTrue(Obj(o, k)) != 0; }

static void TestMailboxJson()
{
    std::printf("\n== C. 信箱 JSON ==\n");
    const unsigned long long seq = 1790000000123ULL;   // 開機毫秒數量級（g_dialogSeq 起點）
    std::string j = w906dlg::YesNoRequestJson(seq, "42", "Sure To Setting Laser Value", "確定要儲存測距數值？", "", false, true);
    cJSON* r = cJSON_Parse(j.c_str());
    Check(r != 0, "C1 YES/NO 請求是合法 JSON");
    Check(std::string(Str(r, "channel")) == "show-my-message" && std::string(Str(r, "state")) == "pending" &&
          std::string(Str(r, "requestId")) == "42" && std::string(Str(r, "function")) == "ShowMyMessageBox_YES_NO",
          "C1 channel/state/requestId/function");
    cJSON* sq = Obj(r, "seq");
    Check(sq && (unsigned long long)sq->valuedouble == seq, "C1 seq 原值（>= 1，dialog-bridge.js 的 seq > lastSeq 才會畫）");
    cJSON* d = Obj(r, "display");
    Check(IsTrue(d, "yesNo") && !IsTrue(d, "buttonEnabled"), "C1 display.yesNo=true、buttonEnabled=false（畫 Yes/No、藏 Pause）");
    Check(std::string(Str(d, "primaryText")) == "Sure To Setting Laser Value" &&
          std::string(Str(d, "secondaryText")) == "確定要儲存測距數值？" && std::string(Str(d, "subText")) == "",
          "C1 沒有 ';'：secondaryText=S2、subText 空（golden :1043-1047）");
    Check(IsTrue(Obj(r, "requestedSideEffects"), "stopAllMotor") && IsTrue(Obj(r, "requestedSideEffects"), "pauseHandler"),
          "C1 requestedSideEffects.stopAllMotor=true（route() 靠它分到會停機的頁）");
    Check(!IsTrue(Obj(r, "auth"), "required"), "C1 auth.required=false（golden pnlYesClick 不要密碼）");
    cJSON_Delete(r);

    j = w906dlg::YesNoRequestJson(7, "7", "q\"uo\\te\r\nnl", "探針：請按是或否;sys.echoYesNo", "s3", true, false);
    r = cJSON_Parse(j.c_str());
    Check(r != 0, "C2 題目含引號／反斜線／CRLF 仍是合法 JSON");
    Check(std::string(Str(Obj(r, "arguments"), "s1")) == "q\"uo\\te\r\nnl", "C2 s1 跳脫後原樣還原");
    d = Obj(r, "display");
    Check(std::string(Str(d, "secondaryText")) == "探針：請按是或否" && std::string(Str(d, "subText")) == "sys.echoYesNo",
          "C2 有 ';'：切成 secondaryText／subText（golden :1034-1041）");
    Check(std::string(Str(Obj(r, "arguments"), "s2")) == "探針：請按是或否;sys.echoYesNo", "C2 arguments.s2 保留原字串（dialog-page.js 用它判斷有沒有 ';'）");
    Check(!IsTrue(Obj(r, "requestedSideEffects"), "pauseHandler") && IsTrue(Obj(r, "runtime"), "systemInitialOK"),
          "C2 pauseHandler／systemInitialOK 照參數");
    cJSON_Delete(r);

    j = w906dlg::MessageIdleJson(1790000000999ULL);
    r = cJSON_Parse(j.c_str());
    Check(r != 0 && std::string(Str(r, "state")) == "idle" && std::string(Str(r, "channel")) == "show-my-message",
          "C3 退役 JSON 合法且 state=idle");
    sq = Obj(r, "seq");
    Check(sq && (unsigned long long)sq->valuedouble == 1790000000999ULL, "C3 退役 JSON 的 seq 換成新值");
    cJSON_Delete(r);
}

int main()
{
    std::printf("test_yesno_dialog -- AI(W906-YESNO) 20260925\n");
    TestCore();
    TestMailboxJson();
    TestStartCheck(CC_AMKOR_Philippines, "Check bin setting?", "B1");
    TestStartCheck(0,                    "Initial Start???",   "B2");

    const AnsiString savedData = DataPath, savedLast = LastDataPath;
    if (PrepScratch()) {
        TestLaser(false);
        TestLaser(true);
    } else {
        std::printf("FAIL B3 跳過：暫存夾準備失敗，**不**呼叫會寫檔的雷射存檔臂\n");
        ++g_fail;
    }
    DataPath = savedData;
    LastDataPath = savedLast;

    std::printf("\n-----------------------------------------------------------------\n");
    std::printf("  test_yesno_dialog: %d passed, %d failed\n", g_ok, g_fail);
    std::printf("  RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    std::printf("-----------------------------------------------------------------\n");
    return g_fail == 0 ? 0 : 1;
}
