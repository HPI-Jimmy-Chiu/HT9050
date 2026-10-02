// ===========================================================================
//  FileRW/IniConfig_N04.cpp -- golden TfConfiguration 建構子的 [N04] Machine Info 那一段（本機名稱／IP）。
//
//  //AI(W906-SETUPA-N04) 20261002 新檔（手寫）。EastSun 1001「請檢查每個頁面元件…用枚舉 每個東西都檢查」：
//    Config.Configuration 的 [N04] Machine Info（N 分頁，這台看得見）裡 edtN04_Host／mmoN04_IP 在網頁上是空的 ——
//    golden 建構子 cConfiguration.cpp:121-147 開程式時就把本機名稱、本機每一個 IP 填進去，移植樹的 FileRW_IniConfig_Boot
//    （FileRW/IniConfig.cpp，golden 建構子的 C 路版）只做到 :117 ReadLastSetIni，這一段沒有人做。
//  golden（906 cConfiguration.cpp，cp950）逐行：
//    :121-122 WSAStartup(MAKEWORD(2,0))
//    :126-127 gethostname(HostName, 80); edtN04_Host->Text=HostName;          （dfm :19829-19836 Enabled=False，只顯示）
//    :128-134 CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false ⇒ edN04_ID->Text=HostName、Enabled=false、
//             IniConfig.SocketHandlerID=edN04_ID->Text、IniConfig.sGPIBMachineID=IniConfig.SocketHandlerID
//    :135     lpHostEnt=gethostbyname(HostName)
//    :141-146 每一個 h_addr_list → inet_ntoa → mmoN04_IP->Lines->Add(IP)
//    :147     WSACleanup()
//  只讀本機網路設定、不寫檔、不碰機台。差異（寫在這裡，不是 golden 的行為）：
//    * golden :141 gethostbyname 回 NULL 時會當掉（lpHostEnt->h_addr_list）；這裡改成 IP 清單留空。
//    * vclcompat 的 TMemo 把 Lines 和 Text 分開存（vclcompat/Controls.h TMemo），C 路 proxies 送的是 Text
//      （FileRW/_EditList.cpp PutValue）⇒ Lines 照 golden 加，Text 也照 VCL TMemo.Text 的樣子（每行接 CRLF）填一份給頁面。
// ===========================================================================
#include <winsock2.h>

#include <string>
#include <vector>

#include "FileRW/_EditList.h"
#include "cmydef.h"      // CUSTOMER_CODE、bEnable_KLT_Function
#include "Config.h"      // IniConfig

using filerw::EL;

// 本機名稱與 IP（golden :121-147 的網路那一半；測試直接呼叫）。回傳 false＝gethostname 失敗（golden 不檢查，Text 會是空字串）
bool W906_IniConfig_N04HostIP(std::string* host, std::vector<std::string>* ips)
{
    host->clear();
    ips->clear();
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);                                       // golden :122 初始化WINSOCK
    char HostName[80];                                                          // golden :123 存放本機名稱
    ZeroMemory(HostName, sizeof(HostName));                                     // golden :125
    const bool ok = gethostname(HostName, sizeof(HostName)) == 0;               // golden :126 取得本機名稱
    *host = HostName;
    if (ok) {
        LPHOSTENT lpHostEnt = gethostbyname(HostName);                          // golden :135
        for (int i = 0; lpHostEnt && lpHostEnt->h_addr_list[i] != NULL; i++) { // golden :141（golden 沒有 NULL 檢查）
            struct in_addr* p = (struct in_addr*)(lpHostEnt->h_addr_list[i]);  // golden :143
            ips->push_back(inet_ntoa(*p));                                      // golden :144-145
        }
    }
    WSACleanup();                                                               // golden :147
    return ok;
}

// golden 建構子 :121-147 的 C 路版：填具名替身（開機一次，同 golden 建構子；FileRW_IniConfig_Boot 呼叫）
void FileRW_IniConfig_N04Ctor()
{
    std::string host;
    std::vector<std::string> ips;
    W906_IniConfig_N04HostIP(&host, &ips);

    TEdit* edHost = EL<TEdit>("TfConfiguration", "edtN04_Host");
    TMemo* mmoIP  = EL<TMemo>("TfConfiguration", "mmoN04_IP");
    {
        const char* const pr[2][2] = { { "edtN04_Host", "gbN04" }, { "mmoN04_IP", "gbN04" } };   // dfm :19743 gbN04 > :19829／:19837
        filerw::ELSetParents("TfConfiguration", pr, 2);
    }
    edHost->Enabled = false;                                                    // dfm :19834 Enabled = False（ELKeep 預設 true）
    edHost->Text = host.c_str();                                                // golden :127 edtN04_Host->Text=HostName;
    if (CUSTOMER_CODE == CC_KYEC_LEE && bEnable_KLT_Function == false)          // golden :128-134
    {
        TEdit* edId = EL<TEdit>("TfConfiguration", "edN04_ID");
        edId->Text = AnsiString(host.c_str());
        edId->Enabled = false;
        IniConfig.SocketHandlerID = edId->Text.c_str();
        IniConfig.sGPIBMachineID = IniConfig.SocketHandlerID;
    }
    AnsiString all;
    for (std::size_t i = 0; i < ips.size(); ++i) {
        mmoIP->Lines->Add(AnsiString(ips[i].c_str()));                          // golden :145 mmoN04_IP->Lines->Add(IP);
        all += AnsiString(ips[i].c_str()) + "\r\n";                             // VCL TMemo.Text＝每行接 CRLF（見檔頭）
    }
    mmoIP->Text = all;
}
