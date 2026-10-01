// ===========================================================================
//  TesterComm/UiHome.cpp -- see UiHome.h.  AI(W906-TC-SHARED) 20261001 (St02-E).
// ===========================================================================
#include "TesterComm/UiHome.h"
#include "TesterComm/UiChannel.h"   // JsonEscape

#include <windows.h>
#include <algorithm>
#include <cstdio>

namespace testercomm {

namespace {
std::string Trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(' ');
    if (a == std::string::npos)
        return std::string();
    return s.substr(a, s.find_last_not_of(' ') - a + 1);
}
std::string Strings(const std::vector<std::string>& v)
{
    std::string o = "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(v[i]);
    }
    return o + "]";
}
const char* TagOfKey(const std::string& key, int testType)
{
    if (key == "gpib") return "GPIB";
    if (key == "tcpip") return "TCPIP";
    if (key == "rs232") return testType == 0 ? "TTL" : "RS232";
    return "";
}
std::string Head(int testType)
{
    char b[128];
    std::snprintf(b, sizeof(b), "{\"schema\":\"testercomm.home/1\",\"build\":\"%s\",\"testType\":%d,\"testTypeName\":", HomeBuild(),
                  testType);
    return std::string(b) + JsonEscape(HomeNameOfTestType(testType));
}
}  // namespace

const char* HomeKeyOfTestType(int testType)
{
    switch (testType)
    {
    case 0: return "rs232";   // TTL_MODE: the TTL board through RS232Standard (ruling T17)
    case 1: return "gpib";    // GPIB_MODE
    case 2: return "rs232";   // RS232_MODE
    case 3: return "tcpip";   // TCP_IP_MODE
    default: return "";
    }
}

const char* HomeNameOfTestType(int testType)
{
    switch (testType)
    {
    case 0: return "TTL";
    case 1: return "GPIB";
    case 2: return "RS232";
    case 3: return "TCP/IP";
    default: return "";
    }
}

const char* HomeBuild()
{
#ifdef W906_NO_SOFT_SIMULTE   // the release (SHIP) build: CMake -DW906_NO_SOFT_SIMULTE=ON, SOFT_SIMULTE off (MachineType.h:63-65)
    return "ship";
#else
    return "sim";
#endif
}

const char* HomeLogFromOfKey(const std::string& key)
{
    if (key == "gpib") return "lstRecord";        // golden Main.dfm:818
    if (key == "rs232") return "MemoLog";         // golden MainForm.dfm:3865
    if (key == "tcpip") return "mmTCPIPCommLog";  // golden TesterTCP.dfm:37
    return "";
}

const char* HomeStateOfText(const std::string& text, int enabled)
{
    const std::string t = Trim(text);
    if (t.empty())
        return enabled == 0 ? "off" : "idle";   // RS232Standard MainForm.cpp:717 " " for a site not tested; the initial empty
    if (t == "T") return "testing";             // RS232Standard START / manual test (MainForm.cpp:972, :1699-1702)
    if (t == "999") return "error";             // RS232Standard BA without CE / a closed site with a bin (:733, :752)
    if (t == "----") return "off";              // RS232Standard simulation, site not tested (:1825)
    if (t == "--") return "idle";               // TfTesterTCP ctor / before BINON (TesterTCP.cpp:54, :467)
    for (size_t i = 0; i < t.size(); ++i)
        if (t[i] < '0' || t[i] > '9')
            return "idle";
    // bins are written "%d" / "%4d" / an int (RS232 :706, GPIB "%4d", TCP :492): never a leading zero, so "01" is
    // MyDutPanel's template caption (MyDutPanel.cpp:93)
    return (t.size() > 1 && t[0] == '0') ? "idle" : "bin";
}

std::string HomeSiteJson(int no, const HomeSite& s)
{
    std::string caption = s.caption;
    if (caption.empty())
    {
        char c[16];
        std::snprintf(c, sizeof(c), "Site %02d", no);
        caption = c;
    }
    char b[64];
    std::snprintf(b, sizeof(b), "{\"no\":%d,\"caption\":", no);
    std::string o = b + JsonEscape(caption) + ",\"text\":" + JsonEscape(s.text) + ",\"state\":" +
                    JsonEscape(HomeStateOfText(s.text, s.enabled));
    std::snprintf(b, sizeof(b), ",\"color\":%ld,\"enabled\":", s.color);
    o += b;
    o += s.enabled < 0 ? "null" : (s.enabled ? "true" : "false");
    std::snprintf(b, sizeof(b), ",\"bin\":%ld,\"binText\":", s.bin);
    return o + b + JsonEscape(s.binText) + ",\"ocr\":" + JsonEscape(s.ocr) + "}";
}

unsigned long HomeLogSeq::Advance(const std::vector<std::string>& tail)
{
    // E2 MR !46 n1: untimed lines (golden TCP's bare " ", TesterTCP.cpp:282-283) could only mislead the overlap if the whole
    // window were the same; with the timestamped lines around them that does not happen in practice.
    size_t added = tail.size();
    for (size_t k = 0; k < tail.size(); ++k)   // k new lines: tail[0 .. size-k) must be the previous tail's end
    {
        const size_t m = tail.size() - k;
        if (m <= prev_.size() && std::equal(tail.begin(), tail.begin() + m, prev_.end() - m))
        {
            added = k;
            break;
        }
    }
    seq_ += added;
    prev_ = tail;
    return seq_;
}

std::string HomeFragment(const std::string& source, bool up, const std::vector<std::string>& binItems, bool binEditable,
                         bool enabledEditable, const std::vector<HomeSite>& sites, const std::string& tag,
                         unsigned long logSeq, const std::vector<std::string>& lines)
{
    char b[64];
    std::string o = "{\"source\":" + JsonEscape(source) + ",\"up\":" + (up ? "true" : "false");
    std::snprintf(b, sizeof(b), ",\"siteCount\":%d,\"binItems\":", kHomeSiteCount);
    o += b + Strings(binItems) + ",\"binEditable\":" + (binEditable ? "true" : "false") +
         ",\"enabledEditable\":" + (enabledEditable ? "true" : "false") + ",\"sites\":[";
    for (int i = 0; i < kHomeSiteCount; ++i)
    {
        if (i)
            o += ',';
        o += HomeSiteJson(i + 1, i < (int)sites.size() ? sites[i] : HomeSite());
    }
    std::snprintf(b, sizeof(b), ",\"seq\":%lu,\"lines\":", logSeq);
    return o + "],\"log\":{\"tag\":" + JsonEscape(tag) + ",\"from\":" + JsonEscape(HomeLogFromOfKey(source)) + b +
           Strings(lines) + "}}";
}

std::string HomeCompose(int testType, const std::string& fragment)
{
    const std::string key = HomeKeyOfTestType(testType);
    const bool hasFields = fragment.size() > 2 && fragment[0] == '{' && fragment[fragment.size() - 1] == '}';
    if (!key.empty() && hasFields)
        return Head(testType) + "," + fragment.substr(1);
    const std::string frag = HomeFragment(key.empty() ? "none" : key, false, std::vector<std::string>(), false, false,
                                          std::vector<HomeSite>(), TagOfKey(key, testType), 0, std::vector<std::string>());
    return Head(testType) + "," + frag.substr(1);
}

}  // namespace testercomm

namespace {
volatile LONG g_homeTestType = -1;
}
int W906_TesterCommHomeTestType() { return static_cast<int>(::InterlockedCompareExchange(&g_homeTestType, 0, 0x7fffffff)); }
void W906_TesterCommSetHomeTestType(int testType) { ::InterlockedExchange(&g_homeTestType, testType); }
