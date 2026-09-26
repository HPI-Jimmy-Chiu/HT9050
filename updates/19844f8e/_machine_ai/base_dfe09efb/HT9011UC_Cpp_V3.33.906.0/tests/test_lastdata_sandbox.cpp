// ===========================================================================
//  tests/test_lastdata_sandbox.cpp
//
//  AI(W906-LASTDATA-TESTDIR) 20260926: ctest 不可以寫到機台真實的 lastdata*.dat。
//
//  cprod.cpp 的 ReadLastDataFile／WriteLastDataFile 照 golden 把三個檔寫死在 D:\HT9045\system\。
//  UpdateMainOperateMode 翻活之後，開機、讀配方、切運轉模式都會走到 WriteLastDataFile，所以每個測試行程
//  都由 tests/test_bootstrap.cpp 在 main() 之前轉進自己的沙盒（cprod.cpp 檔尾 W906_LastDataPath）。
//
//    [1] 這個行程已被轉向：鉤子回的是沙盒路徑，不是 golden 的字串
//    [2] 真的呼叫 WriteLastDataFile(true)：沙盒裡三個檔都等於 LastSet 的位元組；
//        D:\HT9045\system 的三個檔跟開始時逐位元組相同（存在與否也相同）
//    [3] ReadLastDataFile() 讀回的是沙盒（改過的欄位讀得回來）
//    [4] 對照組：拿掉這個行程的環境變數 ⇒ 鉤子原封回傳 golden 字串（含 d:／D: 大小寫）
//        ＝正式程式（沒有 test_bootstrap）的行為
//
//  ⚠ WriteLastDataFile() 寫的不只 lastdata：它還一定會寫 AuthPath+"config.ini"（Vibrate_Time、P65_QAMode、SocketContact，
//    bNotContact==false 時再加 O_Count 接觸壽命計數，cprod.cpp:2082-2175）。20260926 第一版沒轉 AuthPath，gate 把真實
//    D:\HT9045\config\config.ini 的 O_14～16ContactSet 從 6000 寫成 0、iVibratorUnloader 寫成 0（sysguard 抓到、已從快照還原）。
//    所以這裡照 test_ga1_cprod.cpp:354 的做法，呼叫前把 AuthPath 指到沙盒、呼叫後還原；並把真實 config.ini 也列進前後比對。
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "vclcompat/vcl_compat.h"
#include "cprod.h"
#include "LastSet.h"
#include "common.h"      // AuthPath

AnsiString W906_LastDataPath(const char* goldenPath);   // cprod.cpp 檔尾

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_lastdata_sandbox.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string Slurp(const char* p)
{
    FILE* f = std::fopen(p, "rb");
    if (!f) return std::string("<missing>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

int main()
{
    static const char* const kReal[3] = {
        "D:\\HT9045\\system\\lastdata.dat", "D:\\HT9045\\system\\lastdata_backup.dat", "D:\\HT9045\\system\\lastdata_backup2.dat" };
    std::string before[3];
    for (int i = 0; i < 3; ++i) before[i] = Slurp(kReal[i]);
    const std::string cfgBefore = Slurp("D:\\HT9045\\config\\config.ini");

    char name[64];
    std::snprintf(name, sizeof(name), "W906_CTEST_LASTDATA_DIR_%lu", static_cast<unsigned long>(GetCurrentProcessId()));
    char dir[MAX_PATH] = {0};
    const DWORD n = GetEnvironmentVariableA(name, dir, sizeof(dir));

    std::printf("[1] this test process is redirected by test_bootstrap.cpp\n");
    CHECK(n > 0 && n < sizeof(dir));
    CHECK(std::strstr(dir, "w906_ctest_lastdata_") != 0);
    const AnsiString p0 = W906_LastDataPath(kReal[0]);
    CHECK(p0 != AnsiString(kReal[0]));
    CHECK(p0 == AnsiString(dir) + AnsiString("lastdata.dat"));

    std::printf("[2] WriteLastDataFile(true) writes the sandbox, never D:\\HT9045\\system\n");
    const unsigned char saved = static_cast<unsigned char>(LastSet.LastOpenFilename[0]);
    LastSet.LastOpenFilename[0] = 'Z';                     // 一個看得出來的值
    const AnsiString savedAuth = AuthPath;
    AuthPath = AnsiString(dir);                            // config.ini 也寫進沙盒（見檔頭 ⚠）
    const bool wrote = WriteLastDataFile(true, false);     // 整支都跑（含 O_Count）
    AuthPath = savedAuth;
    CHECK(wrote == true);
    const std::string blob(reinterpret_cast<const char*>(&LastSet.LastOpenFilename[0]), sizeof(LAST_GENERAL_SET));
    static const char* const kLeaf[3] = { "lastdata.dat", "lastdata_backup.dat", "lastdata_backup2.dat" };
    for (int i = 0; i < 3; ++i) {
        const std::string sb = Slurp((std::string(dir) + kLeaf[i]).c_str());
        CHECK(sb == blob);                                 // 沙盒裡是這個行程的 LastSet
        CHECK(Slurp(kReal[i]) == before[i]);               // 真實檔一個位元組都沒變
    }
    CHECK(Slurp((std::string(dir) + "config.ini").c_str()) != "<missing>");   // config.ini 的那些鍵寫進了沙盒
    CHECK(Slurp("D:\\HT9045\\config\\config.ini") == cfgBefore);          // 真實 config.ini 沒被碰

    std::printf("[3] ReadLastDataFile() reads the sandbox back\n");
    LastSet.LastOpenFilename[0] = 'A';
    ReadLastDataFile();
    CHECK(LastSet.LastOpenFilename[0] == 'Z');
    LastSet.LastOpenFilename[0] = static_cast<char>(saved);

    std::printf("[4] control: without this process's variable the hook returns golden's literal unchanged\n");
    SetEnvironmentVariableA(name, NULL);
    CHECK(W906_LastDataPath(kReal[0]) == AnsiString(kReal[0]));
    CHECK(W906_LastDataPath("d:\\HT9045\\system\\lastdata_backup.dat") == AnsiString("d:\\HT9045\\system\\lastdata_backup.dat"));
    SetEnvironmentVariableA(name, dir);                    // 放回去（bootstrap 結束時用它自己記的目錄清掉）

    for (int i = 0; i < 3; ++i) CHECK(Slurp(kReal[i]) == before[i]);
    CHECK(Slurp("D:\\HT9045\\config\\config.ini") == cfgBefore);
    DeleteFileA((std::string(dir) + "config.ini").c_str());   // bootstrap 只清它自己放的三個檔，這個由本測試清，資料夾才刪得掉
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
