// =============================================================================
//  test_st02_w142_alarmtext.cpp -- W-142: an alarm note shows golden's message line and description (EastSun 1007
//  「之後畫面不要只顯示錯誤碼 要顯示錯誤碼對應的說明」).
//
//  AI(W906-W142) 20261007 (St02-E).  Suite name (add_test): St02_W142AlarmText.  argv[1] = the port tree root (read only).
//
//  Covers forms/fNote_ShowError.cpp (TfNote::ErrShowToForm's record at :272 + the EOF block) and tools/wb_dialog_mailbox.h:
//    [1] W906_AlarmLanguage = golden note.cpp:4402-4424 (CC_KYEC_LEE / iUserLanguage / LastSet.iLanguageCountry);
//    [2] W906_AlarmDescriptionText = golden :4426-4429 / :4479-4491: <Language>\<Code>.dat, else English\, MOT<k>.dat for a
//        motor note, "" when none; Big5 (code page 950) files come back as UTF-8, RTF / ASCII unchanged;
//    [3] W906_AlarmTextForPost: the recorded golden message (one-shot), the fallback when ErrShowToForm did not run for this code
//        or the code is "Unknown Alarm Code", a motor note's own text + MOT<k> key;
//    [4] the real fNote->ErrShowToForm records what golden puts in ShowMessageEdit1;
//    [5] AlarmRequestJson: display.description / descriptionKey written; with both empty the bytes are what they were;
//    [6] source pins: cMyDB.cpp creates <Error>\English\ before the cylinder template; wb_serve.cpp posts W906_AlarmTextForPost's
//        text; the two 1203 "card returned" lines carry AdvMotErrDescribe.
//  Files: only under %TEMP%\ht9045_w142_<tick> (W906_ERROR_ROOT, set here; the test refuses to run when it would resolve under
//  D:\HT9045 outside a build dir -- the \obj\v906\ rule of d74e577e / 2b663dbd).  Every CHECK prints what it read.
// =============================================================================
#include "forms/fNote.h"
#include "tools/wb_dialog_mailbox.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "MachineType.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

std::string W906_AlarmLanguage();
std::string W906_AlarmErrorRoot();
std::string W906_AlarmDescriptionText(const std::string& code, int iMotorErr, std::string* usedPath);
std::string W906_ToUtf8FromGolden(const std::string& bytes);
void W906_NoteShownRecord(const AnsiString& Code, const AnsiString& Mes, int iMotorErr);
void W906_AlarmTextForPost(const char* code, const char* motorNoteMsg, const std::string& fallback,
                           std::string& message, std::string& description, std::string& key);

static int g_pass = 0, g_fail = 0;
static std::string g_got;
#define CHECK(cond, msg)                                                                                         \
    do {                                                                                                         \
        if (cond) { printf("  PASS: %s   [%s]\n", msg, g_got.c_str()); ++g_pass; }                               \
        else      { printf("  FAIL: %s   [got %s]  (line %d)\n", msg, g_got.c_str(), __LINE__); ++g_fail; }      \
        g_got.clear();                                                                                           \
    } while (0)

static std::string g_root;

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
// under the machine's D:\HT9045 tree, except a build dir (<tree>\Obj\V906\...) -- the laptop's rule (d74e577e / 2b663dbd)
static bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

static void Put(const std::string& p, const std::string& bytes)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fwrite(bytes.data(), 1, bytes.size(), f); std::fclose(f); }
}

static std::string Slurp(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string();
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static std::string Short(const std::string& s) { return s.size() > 60 ? s.substr(0, 60) + "..." : s; }

int main(int argc, char** argv)
{
    printf("St02_W142AlarmText\n");
    const std::string tree = argc > 1 ? argv[1] : "";
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w142_" + stamp;
    const std::string err = g_root + "\\Error";
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA(err.c_str(), 0);
    ::CreateDirectoryA((err + "\\English").c_str(), 0);
    ::CreateDirectoryA((err + "\\Chinese").c_str(), 0);
    static std::string s_env;
    s_env = "W906_ERROR_ROOT=" + err;
    HT9045_TEST_PUTENV(s_env.c_str());
    if (UnderMachineTree(W906_AlarmErrorRoot()) || Lower(W906_AlarmErrorRoot()).find(Lower(g_root)) != 0)
    {
        printf("  ABORT: the Error root is not the sandbox (%s) -- nothing was read\n", W906_AlarmErrorRoot().c_str());
        return 2;
    }
    g_got = W906_AlarmErrorRoot();
    CHECK(true, "0. W906_ERROR_ROOT -> the sandbox");

    // golden files: Big5 Chinese, ASCII English, an RTF one, a motor kind
    const std::string big5Zh = std::string("\xA4\xA4\xA4\xE5\xBB\xA1\xA9\xFA");            // 中文說明 in code page 950
    const std::string utf8Zh = std::string("\xE4\xB8\xAD\xE6\x96\x87\xE8\xAA\xAA\xE6\x98\x8E");
    Put(err + "\\Chinese\\WAR0001.dat", big5Zh);
    Put(err + "\\English\\WAR0001.dat", "en WAR0001");
    Put(err + "\\English\\WAR0002.dat", "{\\rtf1\\ansi en WAR0002}");
    Put(err + "\\English\\MOT3.dat", "motor kind 3");

    const int savedCC = CUSTOMER_CODE, savedLang = IniConfig.iUserLanguage, savedCountry = LastSet.iLanguageCountry;

    // [1] language
    CUSTOMER_CODE = 0;
    IniConfig.iUserLanguage = eulEnglish;    g_got = W906_AlarmLanguage(); CHECK(g_got == "English", "1. iUserLanguage English -> English");
    IniConfig.iUserLanguage = eulKorea;      g_got = W906_AlarmLanguage(); CHECK(g_got == "Korea", "1. Korea");
    IniConfig.iUserLanguage = eulSingapore;  g_got = W906_AlarmLanguage(); CHECK(g_got == "Singapore", "1. Singapore");
    IniConfig.iUserLanguage = eulChinese; LastSet.iLanguageCountry = 0; g_got = W906_AlarmLanguage();
    CHECK(g_got == "English", "1. other language, iLanguageCountry 0 -> English (golden :4420-4421)");
    LastSet.iLanguageCountry = 1;            g_got = W906_AlarmLanguage(); CHECK(g_got == "Chinese", "1. iLanguageCountry 1 -> Chinese");
    CUSTOMER_CODE = CC_KYEC_LEE; IniConfig.iUserLanguage = eulEnglish; g_got = W906_AlarmLanguage();
    CHECK(g_got == "Chinese", "1. CC_KYEC_LEE -> Chinese whatever the language (golden :4402-4405)");
    CUSTOMER_CODE = 0;

    // [2] the lookup order (language = Chinese: iUserLanguage eulChinese + iLanguageCountry 1)
    IniConfig.iUserLanguage = eulChinese; LastSet.iLanguageCountry = 1;
    std::string used;
    std::string t = W906_AlarmDescriptionText("WAR0001", -1, &used);
    g_got = Short(used) + " -> " + t;
    CHECK(t == utf8Zh && used.find("\\Chinese\\WAR0001.dat") != std::string::npos, "2. <Language>\\<Code>.dat first (golden :4427), Big5 -> UTF-8");
    t = W906_AlarmDescriptionText("WAR0002", -1, &used);
    g_got = Short(used) + " -> " + t;
    CHECK(t == "{\\rtf1\\ansi en WAR0002}" && used.find("\\English\\WAR0002.dat") != std::string::npos,
          "2. no Chinese file -> English\\ (golden :4481-4483), RTF unchanged");
    t = W906_AlarmDescriptionText("WAR24013", 3, &used);
    g_got = Short(used) + " -> " + t;
    CHECK(t == "motor kind 3" && used.find("\\English\\MOT3.dat") != std::string::npos, "2. a motor note reads MOT<k>.dat (golden :4429 / :4485)");
    t = W906_AlarmDescriptionText("WAR0003", -1, &used);
    g_got = "used=" + used + " text=" + t;
    CHECK(t.empty() && used.empty(), "2. no file anywhere -> \"\" (golden leaves reDescription blank)");
    IniConfig.iUserLanguage = eulEnglish;
    t = W906_AlarmDescriptionText("WAR0001", -1, &used);
    g_got = Short(used) + " -> " + t;
    CHECK(t == "en WAR0001", "2. language English -> English\\WAR0001.dat");

    // [3] what a request shows
    IniConfig.iUserLanguage = eulChinese; LastSet.iLanguageCountry = 1;
    std::string m, d, k;
    W906_NoteShownRecord("WAR0001", "Golden msg : part (Again!!)", -1);
    W906_AlarmTextForPost("WAR0001", 0, "FALLBACK", m, d, k);
    g_got = "msg=" + m + " desc=" + d + " key=" + k;
    CHECK(m == "Golden msg : part (Again!!)" && d == utf8Zh && k.empty(), "3. recorded -> golden ShowMessageEdit1 text + the description");
    W906_AlarmTextForPost("WAR0001", 0, "FALLBACK", m, d, k);
    g_got = "msg=" + m + " desc=" + d;
    CHECK(m == "FALLBACK" && d.empty(), "3. one-shot: a second post without a new record -> the fallback, no description");
    W906_NoteShownRecord("WAR0001", "Golden msg", -1);
    W906_AlarmTextForPost("WAR0002", 0, "FALLBACK2", m, d, k);
    g_got = "msg=" + m + " desc=" + d;
    CHECK(m == "FALLBACK2" && d.empty(), "3. the record belongs to another code -> the fallback");
    W906_NoteShownRecord("MES16441", "Unknown Alarm Code : x", -1);
    W906_AlarmTextForPost("MES16441", 0, "MES16441 : x", m, d, k);
    g_got = "msg=" + m;
    CHECK(m == "MES16441 : x", "3. \"Unknown Alarm Code\" (not in the live txt) -> the code + errPart fallback (the page adds its index text)");
    W906_NoteShownRecord("WAR24013", "Motor message", 3);
    W906_AlarmTextForPost("WAR24013", "Golden motor text", "FALLBACK", m, d, k);
    g_got = "msg=" + m + " desc=" + d + " key=" + k;
    CHECK(m == "Golden motor text" && d == "motor kind 3" && k == "MOT3", "3. motor note: its own text, MOT3.dat, key MOT3");
    W906_NoteShownRecord("WAR0001", AnsiString(big5Zh.c_str()), -1);
    W906_AlarmTextForPost("WAR0001", 0, "FALLBACK", m, d, k);
    g_got = "msg bytes=" + std::to_string(m.size());
    CHECK(m == utf8Zh, "3. a Big5 message (AlarmCodeList.txt) is sent as UTF-8");

    // [4] the real ErrShowToForm records it
    if (fNote != 0)
    {
        fNote->ErrShowToForm("WAR0002", "Unit", "Real golden message", -1);
        W906_AlarmTextForPost("WAR0002", 0, "FALLBACK", m, d, k);
        g_got = "msg=" + m + " desc=" + d + " sAlarmMes=" + std::string(sAlarmMes.c_str());
        CHECK(m == "Real golden message" && d == "{\\rtf1\\ansi en WAR0002}" && std::string(sAlarmMes.c_str()) == "Real golden message",
              "4. fNote->ErrShowToForm (forms/fNote_ShowError.cpp:272) records Mes + iMotorErr for the post");
    }
    else { g_got = "fNote is null"; CHECK(false, "4. fNote exists"); }

    // [5] the request JSON
    const std::string base = w906dlg::AlarmRequestJson(7, "q7", "WAR0001", 4, 1, "", "msg", true);
    const std::string withD = w906dlg::AlarmRequestJson(7, "q7", "WAR0001", 4, 1, "", "msg", true, std::string(), "a\"b\r\nc", "MOT3");
    const std::string emptyD = w906dlg::AlarmRequestJson(7, "q7", "WAR0001", 4, 1, "", "msg", true, std::string(), std::string(), std::string());
    g_got = Short(withD.substr(withD.find("\"display\"")));
    CHECK(withD.find("\"description\":\"a\\\"b\\r\\nc\",\"descriptionKey\":\"MOT3\",\"flushPanel\":null") != std::string::npos,
          "5. display.description escaped + descriptionKey");
    g_got = "same=" + std::string(emptyD == base ? "yes" : "no");
    CHECK(emptyD == base && base.find("\"description\":\"\",\"flushPanel\":null") != std::string::npos && base.find("descriptionKey") == std::string::npos,
          "5. both empty -> the bytes before W-142 (no descriptionKey key)");

    // [6] source pins (argv[1] = the port root)
    if (tree.empty()) { g_got = "no argv[1]"; CHECK(false, "6. argv[1] = the port tree root"); }
    else
    {
        const std::string db = Slurp(tree + "/cMyDB.cpp");
        const size_t f = db.find("MyForceDirectories(W906ErrorDir()+\"English\\\\\", __func__);");
        const size_t loop = db.find("for(int i=0; i<MaxCylinderItem; i++)", f == std::string::npos ? 0 : f);
        const size_t save = db.find("slAlarmDescrpt->SaveToFile(TextPath);", loop == std::string::npos ? 0 : loop);
        g_got = "mkdir@" + std::to_string((long)f) + " loop@" + std::to_string((long)loop) + " save@" + std::to_string((long)save);
        CHECK(f != std::string::npos && loop != std::string::npos && save != std::string::npos && db.find('\n', f) < loop && db.find('\n', db.find('\n', f) + 1) > loop,
              "6. cMyDB.cpp: <Error>\\English\\ is created right before the cylinder template loop that saves JAM31nnn.dat");
        const std::string ws = Slurp(tree + "/tools/wb_serve.cpp");
        g_got = "wb_serve.cpp " + std::to_string(ws.size()) + " bytes";
        CHECK(ws.find("W906_AlarmTextForPost(code, g_w906MotorNoteMsg, W906_AlarmMsgWithErrPart(code), w906Msg, w906Desc, w906Key);") != std::string::npos &&
              ws.find("\n        w906Msg,") != std::string::npos && ws.find("g_w906MotorNoteMsg ? std::string(g_w906MotorNoteMsg) : W906_AlarmMsgWithErrPart(code),") == std::string::npos &&
              ws.find("W906_NoteAuthRequestJson(requestId.c_str()), w906Desc, w906Key);") != std::string::npos,
              "6. wb_serve.cpp DialogMailboxPostAlarm posts the message / description W906_AlarmTextForPost chose");
        const std::string gr = Slurp(tree + "/EtherCAT/Pci1203GaliRouteCore.cpp"), wm = Slurp(tree + "/WebMotorAccess.cpp");
        const std::string pin = "\"card returned \" + ht9045::AdvMotErrDescribe(r.ret)";
        g_got = "GaliRouteCore " + std::string(gr.find(pin) != std::string::npos ? "yes" : "no") + ", WebMotorAccess " + (wm.find(pin) != std::string::npos ? "yes" : "no");
        CHECK(gr.find(pin) != std::string::npos && wm.find(pin) != std::string::npos, "6. both \"card returned\" lines carry AdvMotErrDescribe");
    }

    CUSTOMER_CODE = savedCC; IniConfig.iUserLanguage = savedLang; LastSet.iLanguageCountry = savedCountry;
    printf("St02_W142AlarmText: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
