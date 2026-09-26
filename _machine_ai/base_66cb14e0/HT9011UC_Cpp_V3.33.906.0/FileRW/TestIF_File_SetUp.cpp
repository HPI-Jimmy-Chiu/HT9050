// ===========================================================================
//  FileRW/TestIF_File_SetUp.cpp -- TestIF_File 的 TfSetup 半邊（C 形狀：具名替身）
//  （<recipe>\HandlerCondition.Data；存檔順帶 Contact.Data 與 Temperature.Data [ATC]）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_SetUp.py（每條 replace／blocks 附原因）。
//
//  golden TfSetup（cSetUp.cpp，912）由 tools/gen_editlist.py 轉成 TestIF_File_SetUp.gen.inc：
//    建構子（:123）＝ 32 個 site 下拉／列欄標籤／rgSensor1..N 的替身陣列、ScrollBar1->Max；
//    FormShow（:1615）＝開頁（讀檔器＋DoIniDataToForm＋ScrollBar1Change＋權限）；
//    sbUpdateClick（:3492）＝存檔鈕（CHSetError／CheckShuttlePitch／RTC-OCR 密碼 → SaveSetupFile → Contact 重讀 →
//    SECS → SetWorkParameter）。沒有 HTEditList：存檔流程讀的替身全部是 mustSend。
//  結構名不是 TestIF_File：那個 tag 已被 A 形狀 FileRW/TestIF_File.cpp（kBridge_TfSetup）佔用。
//
//  讀檔器接移植樹 fSetup->ReadFile()（原因見設定檔檔頭），所以開機仍由 wb_serve 的 fSetup->Init()＋ReadFile() 讀檔；
//  本檔的 FileRW_Setup_Boot() 只做 golden 建構子（替身），必須在 fSetup->Init() 之後（tSiteMap 由它建）。
// ===========================================================================
#include "FileRW/TestIF_File_SetUp.gen.inc"

#include <cstdio>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// 存檔流程讀的替身：產生器掃出的（kSU_SaveReads）＋ 經指標陣列讀的（產生器的掃描看不到）：
//   TestSiteCH[r][c]（cbAa..cbDh，SaveSetupFile／CHSetError／sbUpdateClick）、MyTempRGBox[i]（rgSensor1..N，SaveSetupFile）。
//   BuildReads() 在 SU_CreateSaveProxies() 之後呼叫（要用 ELFind 判型別）。
std::vector<std::string> g_readNames;
std::vector<const char*> g_reads;

void BuildReads()
{
    // 容器（gbRTC／gbOcr：golden 存檔只讀 ->Enabled，是開頁 FormShow 設的伺服器端狀態，不是頁面值）不列必送
    for (const char* n : kSU_SaveReads) {
        TControl* c = filerw::ELFind("TfSetup", n);
        if (dynamic_cast<TGroupBox*>(c) || dynamic_cast<TPanel*>(c) || dynamic_cast<TTabSheet*>(c)) continue;
        g_readNames.push_back(n);
    }
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
        for (int c = 0; c < MAX_SOCKET_COL; ++c) {
            std::string s("cb");
            s += char('A' + r);
            s += char('a' + c);
            g_readNames.push_back(s);
        }
    for (int i = 0; i < iSnSocketCnt; ++i) g_readNames.push_back("rgSensor" + std::to_string(i + 1));
    for (const std::string& s : g_readNames) g_reads.push_back(s.c_str());
}

// 沒寫檔時把替身還原成檔案值：golden FormClose（:3363）的 ReadFile() ＋ DoIniDataToForm()（FormClose 其餘的 Auto Site Mapping
// 清除／SetRunStartMode 是關表單的機台副作用，不在「還原替身」的範圍，不跑）
void Reload()
{
    SU_PortReadFile();
    SU_DoIniDataToForm();
}

filerw::PageDesc g_page = {
    "TestIF_File_SetUp", "TfSetup", "Setup.SetUp.html",
    nullptr, nullptr, 0,
    kSU_SaveReads, (int)(sizeof(kSU_SaveReads) / sizeof(kSU_SaveReads[0])),
    &SU_FormShow, &SU_sbUpdateClick, "SaveSetupFile", &Reload, &Booted,
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace

// golden TfSetup 建構（HT9045.cpp:184 CreateForm）。前提：fSetup->Init() 已跑（tSiteMap）。
void FileRW_Setup_Boot()
{
    if (g_booted) return;
    // ScrollBar1：golden TScrollBar（DFM Max=8、OnChange=ScrollBar1Change）。產生器把 TScrollBar 當 TControl，
    // 這裡先建成 filerw::ELTrackBar（VCL 語意：Position 夾在 Min..Max、值變了觸發 OnChange），之後產生碼的
    // EL<TControl>("TfSetup","ScrollBar1") 拿到的是同一個物件。
    {
        filerw::ELTrackBar* sb = EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1");
        sb->DfmInit(0, 8, 0);
        sb->OnChange = &SU_ScrollBar1Change;
    }
    SU_DfmItems();
    SU_DfmState();
    SU_TfSetup();
    // rgSensor1..N（golden 建構子動態產生，DFM 沒有）的容器：golden Parent=scrlbxSocketSensor（DFM 父＝grpSocketSensor）
    {
        static std::string kids[iSnSocketCnt];
        static const char* pairs[iSnSocketCnt][2];
        for (int i = 0; i < iSnSocketCnt; ++i) {
            kids[i] = "rgSensor" + std::to_string(i + 1);
            pairs[i][0] = kids[i].c_str();
            pairs[i][1] = "scrlbxSocketSensor";
        }
        EL<TControl>("TfSetup", "scrlbxSocketSensor");
        filerw::ELSetParents("TfSetup", pairs, iSnSocketCnt);
    }
    SU_CreateSaveProxies();
    SU_CreateContainerProxies();
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    std::printf("FileRW TestIF_File_SetUp: TfSetup proxies ready (%d save reads) -- golden cSetUp.cpp ctor :123\n",
                g_page.nSaveReads);
    g_booted = true;
}

// ---- golden fContact->DutCount()／ReadFile()：移植樹 fContact 是 TfContactShim（atester_shims.h），
//      真表單是 fContactForm（forms/fContact.h:1634）。
#undef ReadFile   // 設定檔 members 的 #define ReadFile SU_PortReadFile 只給 golden 轉出來的程式用
#include "forms/fContact.h"

void FileRW_Setup_ContactDutCount() { fContactForm->DutCount(); }   // golden cSetUp.cpp:3633
void FileRW_Setup_ContactReadFile() { fContactForm->ReadFile(); }   // golden cSetUp.cpp:3642
