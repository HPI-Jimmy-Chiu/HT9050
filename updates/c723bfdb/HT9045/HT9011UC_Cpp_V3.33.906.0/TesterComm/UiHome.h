// ===========================================================================
//  TesterComm/UiHome.h -- the ONE "home" JSON of web/page/testercomm.html's 首頁 tab.
//  AI(W906-TC-SHARED) 20261001 (St02-E).
//
//  Steven 20261001: 「我以為site01~32的面板是共用的!」「然後通訊Log的memo也是共用的」「截圖的元件只有一份, 在首頁」
//  「主要通訊log也是只有一份, 在首頁」「C++端在 32site 與 通訊log部分, 也是整合一個JSON進行發送」
//  「不需要根據不同的格式呼叫不同的JSON」.
//  golden runs ONE interface's program per machine, each with one site panel and one comm memo, and the three site cells
//  are the same (St02-E2's map, review/ST02_TC_HOME_JSON_MAP_20261001.md): group "Site NN" + plSite (state as TEXT only,
//  golden never changes its Color: clSilver) + cbSiteOn + cbBin (the simulated bin) + labOcr.
//    RS232Standard MainForm.dfm:3893 palSite + :3865 MemoLog (MainForm.cpp:2231-2249 ShowCommData); H9046_32GPIB
//    Main.dfm:198 palSite + :818 lstRecord (Main.cpp:701-724 WriteLog); the Handler's TfTesterTCP (TesterTCP.dfm:116
//    plSite01-32, :37 mmTCPIPCommLog, TesterTCP.cpp:276-300 AddTCPIPCommunicationLog).
//  Each engine publishes its part ("<key>.home", HomeFragment); TcpPump (the Handler thread, where TestIF_File lives)
//  picks the active interface's part every 200 ms and publishes "home" (HomeCompose).  For GPIB / RS232 the route composes
//  the same object on GET from the engine's part (E2 MR !46 m1: it keeps updating during Handler modal waits, when TcpPump
//  does not run); TCP/IP is TcpPump's copy.  GET /api/testercomm/home:
//    { "schema":"testercomm.home/1", "build":"sim"|"ship", "testType":0..3, "testTypeName":"TTL"|"GPIB"|"RS232"|"TCP/IP"|"",
//      "source":"gpib"|"rs232"|"tcpip"|"none", "up":bool, "siteCount":32,
//      "binItems":[the mode's cbBin items], "binEditable":bool, "enabledEditable":bool,
//      "sites":[32 x {"no","caption","text","state","color","enabled","bin","binText","ocr"}],
//      "log":{"tag":"GPIB"|"RS232"|"TTL"|"TCPIP","from":"lstRecord"|"MemoLog"|"mmTCPIPCommLog","seq":n,"lines":[...]} }
//  St02-M 20261001 (per golden, no question to Steven): the page does NOT recolour cells (state is informational);
//  log lines go VERBATIM (golden text), the tag and seq beside them; TCP's cbSiteOn is dead in golden -> enabled null.
//  POST /api/testercomm/home?cmd=site <0..31> on 0|1 / site <0..31> bin <index> goes to the active engine.
//  "build" (Steven 20261001 15:0x「debug模式顯示上面的tab沒問題」「release模式下, 只留目前使用中的通訊格式就好」): "sim" = the
//  debug / SIM build (every tab), "ship" = the release build (W906_NO_SOFT_SIMULTE): the page keeps Test Result and the
//  tab of the interface in use ("source") only.
//  The log FILES stay separate (D:\GPIBLOG, D:\RS232Log, D:\HT9045_Log\Test_TCPIP); only the screen is shared.
// ===========================================================================
#ifndef TESTERCOMM_UIHOME_H
#define TESTERCOMM_UIHOME_H

#include <string>
#include <vector>

namespace testercomm {

const int kHomeSiteCount = 32;   // USE_SITE_COUNT (RS232Standard MainForm.cpp:19) / TOTAL_SITE (GPIB Main.h:18) / TfTesterTCP's 32
const long kHomeSilver = 0x00C0C0C0;   // clSilver: golden plSite->Color, never changed by any of the three

// golden TestIF_File.iTestType (cmydef.h:61-64; read default 0, cTesterIF.cpp:572-578): TTL 0 / RS232 2 -> "rs232"
// (TTL goes through RS232Standard, ruling T17), GPIB 1 -> "gpib", TCP_IP 3 -> "tcpip"; anything else -> "".
const char* HomeKeyOfTestType(int testType);
const char* HomeNameOfTestType(int testType);    // "TTL" / "GPIB" / "RS232" / "TCP/IP" / ""
const char* HomeLogFromOfKey(const std::string& key);   // "lstRecord" / "MemoLog" / "mmTCPIPCommLog" / ""
const char* HomeBuild();   // "ship" when built with W906_NO_SOFT_SIMULTE (the release build, MachineType.h:63-65), else "sim"

// the state text of plSite (golden: "T" testing, a number = the bin, "999" error, "----" / " " off, "--" TCP idle,
// "01" the MyDutPanel template caption): "testing" / "bin" / "error" / "off" / "idle".  enabled: 1, 0, -1 = null.
const char* HomeStateOfText(const std::string& text, int enabled);

struct HomeSite   // one golden site cell
{
    std::string caption;   // gpSite->Caption ("Site NN")
    std::string text;      // plSite->Caption
    long color;            // plSite->Color
    int enabled;           // cbSiteOn->Checked: 1 / 0; -1 = null (TCP: golden never reads it)
    long bin;              // cbBin->ItemIndex (-1 = none)
    std::string binText;   // cbBin->Text
    std::string ocr;       // labOcr->Caption
    HomeSite() : color(kHomeSilver), enabled(-1), bin(-1) {}
};
std::string HomeSiteJson(int no, const HomeSite& s);   // no = 1..32; a missing cell -> HomeSite() with "Site NN"

// log.seq: lines added so far.  The engines keep a tail (golden memo / list box, cleared at 10240 / 2000 lines); Advance
// finds the new lines by the overlap with the previous tail (the shortest append that fits; no overlap = all new).
class HomeLogSeq
{
public:
    HomeLogSeq() : seq_(0) {}
    unsigned long Advance(const std::vector<std::string>& tail);
private:
    std::vector<std::string> prev_;
    unsigned long seq_;
};

// an engine's part (no schema / testType: HomeCompose adds them); sites shorter than 32 are padded with defaults
std::string HomeFragment(const std::string& source, bool up, const std::vector<std::string>& binItems, bool binEditable,
                         bool enabledEditable, const std::vector<HomeSite>& sites, const std::string& tag,
                         unsigned long logSeq, const std::vector<std::string>& lines);

// the home JSON (see above); an empty / missing fragment (engine not running) or an unknown testType -> the same keys:
// up false, no bin items, nothing editable, 32 default cells, log seq 0 with no lines
std::string HomeCompose(int testType, const std::string& fragment);

}  // namespace testercomm

// the active interface for POST /api/testercomm/home (TcpPump writes it on the Handler thread; any thread reads it)
int W906_TesterCommHomeTestType();
void W906_TesterCommSetHomeTestType(int testType);

#endif
