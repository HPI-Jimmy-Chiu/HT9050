// =============================================================================
//  tests/test_testercomm_home.cpp -- the ONE home JSON of testercomm.html's 首頁 (TesterComm/UiHome.h).
//  AI(W906-TC-SHARED) 20261001 (St02-E).  Suite: TesterComm_HomeJson (both configs).
//
//  Steven 20261001: 「C++端在 32site 與 通訊log部分, 也是整合一個JSON進行發送」「不需要根據不同的格式呼叫不同的JSON」;
//  the shape is St02-E2's map (ST02_TC_HOME_JSON_MAP_20261001.md §3) with St02-M's rulings (lines verbatim, no colour).
//    1. the testType mapping (golden TestIF_File.iTestType, cmydef.h:61-64): 0 TTL / 2 RS232 -> rs232, 1 -> gpib,
//       3 -> tcpip, anything else -> "" (source "none").
//    2. the plSite state text (golden: "T", a bin number, "999", "----", " ", "--", the template "01").
//    3. log.seq: the overlap of the previous tail (append, a capped tail, a clear, no change).
//    4. the merged object for every testType: valid JSON, the SAME keys in the SAME order whether the engine runs or
//       not; 32 cells with the same 9 keys; log {tag, from, seq, lines}; a line goes verbatim (quotes, backslash).
//    5. the route: GET /api/testercomm/home returns what TcpPump published (TCP/IP, before the first beat) or, for GPIB /
//       RS232, composes it from the engine's own part on the spot (E2 MR !46 m1); POST home "site ..." goes to the active
//       engine's queue (GPIB / RS232; TCP/IP: bin only), anything else is refused; the key list names home.
//  Memory only: UiChannel and the route function, no engine, no socket, no file.
// =============================================================================
#include "TesterComm/UiHome.h"
#include "TesterComm/UiChannel.h"
#include "TesterComm/Handler/TesterCommWiring.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- a small JSON reader: validity + the key order of one object ------------------------------------------------
struct J
{
    const std::string& s;
    size_t i;
    J(const std::string& t, size_t at) : s(t), i(at) {}
    void ws() { while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) ++i; }
    bool lit(const char* w) { size_t n = std::strlen(w); if (s.compare(i, n, w) != 0) return false; i += n; return true; }
    bool str(std::string* out)
    {
        if (i >= s.size() || s[i] != '"') return false;
        ++i;
        std::string v;
        while (i < s.size() && s[i] != '"')
        {
            if (s[i] == '\\') { if (i + 1 >= s.size()) return false; v += s[i + 1]; i += 2; continue; }
            if ((unsigned char)s[i] < 0x20) return false;
            v += s[i++];
        }
        if (i >= s.size()) return false;
        ++i;
        if (out) *out = v;
        return true;
    }
    bool num() { size_t a = i; if (i < s.size() && s[i] == '-') ++i; while (i < s.size() && s[i] >= '0' && s[i] <= '9') ++i; return i > a; }
    bool val(std::vector<std::string>* keys, std::vector<std::string>* strings)
    {
        ws();
        if (i >= s.size()) return false;
        if (s[i] == '{') return obj(keys);
        if (s[i] == '[') { ++i; ws(); if (i < s.size() && s[i] == ']') { ++i; return true; }
                           for (;;) { if (!val(0, strings)) return false; ws(); if (i < s.size() && s[i] == ',') { ++i; continue; }
                                      if (i < s.size() && s[i] == ']') { ++i; return true; } return false; } }
        if (s[i] == '"') { std::string v; if (!str(&v)) return false; if (strings) strings->push_back(v); return true; }
        if (lit("true") || lit("false") || lit("null")) return true;
        return num();
    }
    bool obj(std::vector<std::string>* keys)
    {
        ++i; ws();
        if (i < s.size() && s[i] == '}') { ++i; return true; }
        for (;;)
        {
            ws();
            std::string k;
            if (!str(&k)) return false;
            if (keys) keys->push_back(k);
            ws();
            if (i >= s.size() || s[i] != ':') return false;
            ++i;
            if (!val(0, 0)) return false;
            ws();
            if (i < s.size() && s[i] == ',') { ++i; continue; }
            if (i < s.size() && s[i] == '}') { ++i; return true; }
            return false;
        }
    }
};
static bool TopKeys(const std::string& json, std::vector<std::string>* keys)
{
    J j(json, 0);
    j.ws();
    if (j.i >= json.size() || json[j.i] != '{') return false;
    if (!j.obj(keys)) return false;
    j.ws();
    return j.i == json.size();
}
static bool KeysAt(const std::string& json, const char* marker, std::vector<std::string>* keys)   // the object right after marker
{
    const size_t a = json.find(marker);
    if (a == std::string::npos) return false;
    J j(json, a + std::strlen(marker));
    return j.i < json.size() && json[j.i] == '{' && j.obj(keys);
}
static bool StringsAt(const std::string& json, const char* marker, std::vector<std::string>* out)   // the array right after marker
{
    const size_t a = json.find(marker);
    if (a == std::string::npos) return false;
    J j(json, a + std::strlen(marker));
    return j.i < json.size() && json[j.i] == '[' && j.val(0, out);
}
static size_t Count(const std::string& s, const char* what)
{
    size_t n = 0;
    for (size_t a = s.find(what); a != std::string::npos; a = s.find(what, a + 1)) ++n;
    return n;
}
static std::string Join(const std::vector<std::string>& v)
{
    std::string o;
    for (size_t i = 0; i < v.size(); ++i) { if (i) o += ','; o += v[i]; }
    return o;
}

static const char* const kLine = "2026-10-01, 13:05:01.123, Tester ==> \"BINON\" C:\\x";   // quotes + a backslash, verbatim

static std::string SampleFragment(const std::string& key, bool up)
{
    std::vector<testercomm::HomeSite> sites(32);
    for (int i = 0; i < 32; ++i)
    {
        char cap[16];
        std::snprintf(cap, sizeof(cap), "Site %02d", i + 1);
        sites[i].caption = cap;
        sites[i].text = i == 0 ? "T" : (i == 1 ? "   3" : "01");
        sites[i].enabled = key == "tcpip" ? -1 : (i < 16 ? 1 : 0);
        sites[i].bin = 1;
        sites[i].binText = "2";
        sites[i].ocr = i == 3 ? "2D\"X" : "";
    }
    std::vector<std::string> items;
    items.push_back("1");
    items.push_back("1..5");
    std::vector<std::string> lines(1, kLine);
    return testercomm::HomeFragment(key, up, items, true, key != "tcpip", sites, key == "gpib" ? "GPIB" : "RS232", 7, lines);
}

int main()
{
    std::printf("TesterComm_HomeJson\n");
    const char* const kHomeKeys = "schema,build,testType,testTypeName,source,up,siteCount,binItems,binEditable,enabledEditable,sites,log";
#ifdef W906_NO_SOFT_SIMULTE
    const char* const kBuild = "ship";   // release: the page keeps Test Result + the interface in use (Steven 1001 15:0x)
#else
    const char* const kBuild = "sim";    // debug / SIM: every tab
#endif

    std::printf(" 1. the testType mapping\n");
    CHECK(std::string(testercomm::HomeKeyOfTestType(0)) == "rs232" && std::string(testercomm::HomeKeyOfTestType(2)) == "rs232");
    CHECK(std::string(testercomm::HomeKeyOfTestType(1)) == "gpib" && std::string(testercomm::HomeKeyOfTestType(3)) == "tcpip");
    CHECK(std::string(testercomm::HomeKeyOfTestType(7)) == "" && std::string(testercomm::HomeKeyOfTestType(-1)) == "");
    CHECK(std::string(testercomm::HomeNameOfTestType(0)) == "TTL" && std::string(testercomm::HomeNameOfTestType(3)) == "TCP/IP");
    CHECK(std::string(testercomm::HomeBuild()) == kBuild);
    CHECK(std::string(testercomm::HomeLogFromOfKey("gpib")) == "lstRecord" && std::string(testercomm::HomeLogFromOfKey("rs232")) == "MemoLog" &&
          std::string(testercomm::HomeLogFromOfKey("tcpip")) == "mmTCPIPCommLog" && std::string(testercomm::HomeLogFromOfKey("none")) == "");

    std::printf(" 2. the plSite state text\n");
    CHECK(std::string(testercomm::HomeStateOfText("T", 1)) == "testing");
    CHECK(std::string(testercomm::HomeStateOfText("   3", 1)) == "bin" && std::string(testercomm::HomeStateOfText("12", 1)) == "bin" &&
          std::string(testercomm::HomeStateOfText("0", 1)) == "bin");
    CHECK(std::string(testercomm::HomeStateOfText("999", 1)) == "error");
    CHECK(std::string(testercomm::HomeStateOfText("----", 0)) == "off" && std::string(testercomm::HomeStateOfText(" ", 0)) == "off");
    CHECK(std::string(testercomm::HomeStateOfText(" ", 1)) == "idle" && std::string(testercomm::HomeStateOfText("", -1)) == "idle");
    CHECK(std::string(testercomm::HomeStateOfText("--", -1)) == "idle" && std::string(testercomm::HomeStateOfText("01", 0)) == "idle");

    std::printf(" 3. log.seq\n");
    {
        testercomm::HomeLogSeq q;
        std::vector<std::string> t;
        CHECK(q.Advance(t) == 0);
        t.push_back("a"); t.push_back("b");
        CHECK(q.Advance(t) == 2);
        CHECK(q.Advance(t) == 2);                       // nothing new
        t.push_back("c");
        CHECK(q.Advance(t) == 3);                       // one appended
        t.erase(t.begin()); t.push_back("d");           // a capped tail: b c d
        CHECK(q.Advance(t) == 4);
        t.clear(); t.push_back("x");                    // golden cleared the memo, then one line
        CHECK(q.Advance(t) == 5);
        t.clear();
        CHECK(q.Advance(t) == 5);
    }

    std::printf(" 4. the merged object: valid JSON, the same keys for every testType\n");
    for (int tt = -1; tt <= 4; ++tt)
    {
        const std::string key = testercomm::HomeKeyOfTestType(tt);
        for (int run = 0; run < 2; ++run)   // run 0: the engine published its part; run 1: it did not
        {
            const std::string frag = run == 0 && !key.empty() ? SampleFragment(key, true) : std::string();
            const std::string home = testercomm::HomeCompose(tt, frag);
            std::vector<std::string> keys, cell, log;
            const bool ok = TopKeys(home, &keys);
            char what[160];
            std::snprintf(what, sizeof(what), "testType %d %s: valid JSON, keys %s", tt, run == 0 ? "running" : "not running", Join(keys).c_str());
            check(ok && Join(keys) == kHomeKeys, what, __LINE__);
            CHECK(KeysAt(home, "\"sites\":[", &cell) && Join(cell) == "no,caption,text,state,color,enabled,bin,binText,ocr");
            CHECK(KeysAt(home, "\"log\":", &log) && Join(log) == "tag,from,seq,lines");
            CHECK(Count(home, "{\"no\":") == 32);
            if (run == 1 || key.empty())
                CHECK(home.find("\"up\":false") != std::string::npos && home.find("\"binItems\":[]") != std::string::npos &&
                      home.find("\"seq\":0,\"lines\":[]") != std::string::npos && home.find("\"enabled\":null") != std::string::npos &&
                      home.find(key.empty() ? "\"source\":\"none\"" : ("\"source\":\"" + key + "\"").c_str()) != std::string::npos);
        }
    }
    {
        const std::string home = testercomm::HomeCompose(1, SampleFragment("gpib", true));
        const std::string head = std::string("{\"schema\":\"testercomm.home/1\",\"build\":\"") + kBuild +
                                 "\",\"testType\":1,\"testTypeName\":\"GPIB\",\"source\":\"gpib\",\"up\":true";
        CHECK(home.compare(0, head.size(), head) == 0);
        std::vector<std::string> lines;
        CHECK(StringsAt(home, "\"lines\":", &lines) && lines.size() == 1 && lines[0] == kLine);   // verbatim
        CHECK(home.find("\"no\":1,\"caption\":\"Site 01\",\"text\":\"T\",\"state\":\"testing\",\"color\":12632256,\"enabled\":true") != std::string::npos);
        CHECK(home.find("\"text\":\"   3\",\"state\":\"bin\"") != std::string::npos && home.find("\"enabled\":false") != std::string::npos);
        CHECK(home.find("\"log\":{\"tag\":\"GPIB\",\"from\":\"lstRecord\",\"seq\":7") != std::string::npos);
        CHECK(testercomm::HomeCompose(3, "{}") == testercomm::HomeCompose(3, ""));   // an empty part counts as none
        const std::string tcp = testercomm::HomeCompose(3, SampleFragment("tcpip", true));
        CHECK(tcp.find("\"enabledEditable\":false") != std::string::npos && Count(tcp, "\"enabled\":null") == 32);
    }

    std::printf(" 5. the route\n");
    {
        int status = 0;
        std::string ct, body;
        CHECK(W906_TesterCommHttp("GET", "/api/testercomm", "", true, &status, &ct, &body) && status == 200 &&
              body.find("\"home\"") != std::string::npos);
        const std::string published = testercomm::HomeCompose(2, SampleFragment("rs232", true));
        testercomm::UiChannel::Instance().Publish("home", published);
        CHECK(W906_TesterCommHttp("GET", "/api/testercomm/home", "", true, &status, &ct, &body) && status == 200 && body == published);
        testercomm::UiChannel::Instance().Take("gpib");
        testercomm::UiChannel::Instance().Take("rs232");
        testercomm::UiChannel::Instance().Take("tcpip");
        W906_TesterCommSetHomeTestType(1);
        CHECK(W906_TesterCommHomeTestType() == 1);
        CHECK(W906_TesterCommHttp("POST", "/api/testercomm/home", "cmd=site%203%20on%201", true, &status, &ct, &body) && status == 202);
        std::vector<std::string> q = testercomm::UiChannel::Instance().Take("gpib");
        CHECK(q.size() == 1 && q[0] == "site 3 on 1");
        W906_TesterCommSetHomeTestType(0);   // TTL -> the RS232Standard engine
        CHECK(W906_TesterCommHttp("POST", "/api/testercomm/home", "cmd=site%204%20bin%202", true, &status, &ct, &body) && status == 202);
        q = testercomm::UiChannel::Instance().Take("rs232");
        CHECK(q.size() == 1 && q[0] == "site 4 bin 2");
        W906_TesterCommSetHomeTestType(3);   // TCP/IP: cbSimulateBin yes, the dead cbSiteOn no
        CHECK(W906_TesterCommHttp("POST", "/api/testercomm/home", "cmd=site%201%20on%201", true, &status, &ct, &body) && status == 200 &&
              body.find("\"queued\":false") != std::string::npos);
        CHECK(W906_TesterCommHttp("POST", "/api/testercomm/home", "cmd=site%201%20bin%208", true, &status, &ct, &body) && status == 202);
        q = testercomm::UiChannel::Instance().Take("tcpip");
        CHECK(q.size() == 1 && q[0] == "site 1 bin 8");
        W906_TesterCommSetHomeTestType(1);
        CHECK(W906_TesterCommHttp("POST", "/api/testercomm/home", "cmd=click%20btnManualStart", true, &status, &ct, &body) && status == 200 &&
              body.find("\"queued\":false") != std::string::npos);   // home takes site commands only
        CHECK(testercomm::UiChannel::Instance().Take("gpib").empty());

        // m1: GPIB / RS232 home is composed on GET from "<key>.home" (TcpPump's "home" may be stale during a modal wait)
        testercomm::UiChannel::Instance().Publish("home", published);   // stale: an RS232 object
        const std::string gpibPart = SampleFragment("gpib", true);
        testercomm::UiChannel::Instance().Publish("gpib.home", gpibPart);
        W906_TesterCommSetHomeTestType(1);
        CHECK(W906_TesterCommHttp("GET", "/api/testercomm/home", "", true, &status, &ct, &body) && status == 200 &&
              body == testercomm::HomeCompose(1, gpibPart));
        testercomm::UiChannel::Instance().Clear("rs232.home");
        W906_TesterCommSetHomeTestType(2);   // the RS232 engine has not published: up false, the same keys, not 404
        std::vector<std::string> k2;
        CHECK(W906_TesterCommHttp("GET", "/api/testercomm/home", "", true, &status, &ct, &body) && status == 200 &&
              body == testercomm::HomeCompose(2, "") && TopKeys(body, &k2) && Join(k2) == kHomeKeys);
        W906_TesterCommSetHomeTestType(3);   // TCP/IP: TcpPump's copy as published
        CHECK(W906_TesterCommHttp("GET", "/api/testercomm/home", "", true, &status, &ct, &body) && status == 200 && body == published);
        testercomm::UiChannel::Instance().Clear("gpib.home");
        testercomm::UiChannel::Instance().Clear("home");
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
