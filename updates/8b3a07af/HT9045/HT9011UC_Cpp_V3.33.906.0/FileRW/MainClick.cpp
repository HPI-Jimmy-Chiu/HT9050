// ===========================================================================
//  FileRW/MainClick.cpp -- golden TfMain 主畫面按鈕處理器裡「讀寫檔的那一半」（V912 main.cpp）。
//
//  //AI(W906-FRW-P8) 20260926: 新檔（Steven 團隊）。Steven 20260926 S52「先以大量把讀寫檔進行移植為首要工作」；
//    清單來源：20260925 靜態盤點 cmydef_io_audit.md 第四節 P8（行號已照 V912 重查）。
//    golden 一律照 V912：D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 → UTF-8）。
//
//  兩項：
//    (P8-a) bHasEnteredPEModel —— golden TfMain::sbPEModelClick（main.cpp:33724-33778）主畫面「PE Model」鈕：
//           進 PE 模式寫 system\Gerneral.ini [System] bHasEnteredPEModel=1。讀檔端：移植樹 database.cpp:1520
//           （SYSTEM_MODULAR::ReadGeneralIni，開機 LoadMachineConfig 跑，live；golden database.cpp:1395）。
//           另一個寫入點 golden KYECFTP/FTPClient.cpp:1017-1021（FTP 下載 Setup File 後清旗標）與讀取點 :1142（上傳前擋）
//           在 TfFTPClient，移植樹 KYECFTP/ 沒翻這幾行 —— KYEC 客戶專屬的 FTP 表單，不在本項。
//           本檔只翻按鈕本體的非 VCL 部分，入口 W906_Main_PEModelOp（WS 動作的本體；wb_serve.cpp 分派與網頁按鈕見交件報告，
//           wb_serve.cpp 是共用檔，本檔不改它）。
//    (P8-b) bTestSiteUse —— golden TfMain::mtDutOnOffMouseUp（main.cpp:29930-30615）主畫面 Site 開關的
//           :30329-30374（寫配方 Temperature.Data [ATC] ATC7CH1..4Enabled 再讀回 Temperature.bATC7ChannelEnabled[]）。
//           :30375 ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, …) 是對 ATC 溫控器的通訊（外部設備）→ 不在這裡，
//           回傳兩個字串給呼叫端送。整個 mtDutOnOffMouseUp（守衛、LastSet.bUseTestSocket、ChangeATCSiteUse、Auto Site Mapping、
//           site log、送 ATC7）是底層流程＋外部設備 → Jimmy；Jimmy 翻那支時在 golden :30329 的位置呼叫
//           W906_Main_DutOnOff_SaveATC7Channels()，回 true 就照 golden :30375 送 ATC7。
//
//  AI(W906-FRW-S100) 20260926: RULINGS_20260926 S100（主畫面零星讀寫，golden V912 main.cpp；D:\HT9045 那份，行號同原檔）再加三項：
//    (S100-a) golden TfMain::sbSetupClick（main.cpp:28457-28498）Setup 視窗關掉後的尾段 :28472-28497 ＝ W906_Main_sbSetupClickTail：
//             DoStructUnitConvert／SetWorkParameter 重讀參數，_8Site1X4 模式另外寫配方 Contact.Data 4 鍵再讀回（golden 的怪行為，
//             照翻，見該函式註解）。呼叫端 FileRW/TestIF_File_SetUp.cpp SaveFlow（網頁 Setup 頁存檔之後；時機見該處）。
//    (S100-b) golden TfMain::cb_MainAutoSkipSwitchClick（main.cpp:31841-31847）＝ W906_Main_AutoSkipSwitchOp：config.ini [Tray]
//             bRecordSkipPosition。那顆勾選框只在 CC_ASE_CL 看得見（main.cpp:11155，客戶專屬 S25）。
//    (S100-c) golden TfMain::SetToDefineValue1Click（main.cpp:26672-26750，StringGrid2 的右鍵選單 PopupMenu4）＝ W906_Main_SetToDefineValueOp：
//             GetDummyVacuum()（cAuthority.cpp:486，golden cAuthority.cpp:392）讀 config\Security_new.def [Dummy Vacuum] 4 鍵，
//             再把各吸嘴的 VacuumOn/OffTime 與 LastSet 的 dummy 時間設成它（吸嘴那段在 FileRW/_KitSuck.cpp，理由見該處）。
//    (b)(c) 的 WS 分派片段見交件報告（tools/wb_serve.cpp 是共用檔，本檔不改它）。
//
//  建置：本檔不屬於 gen_editlist.py 的產生清單。要照 FileRW/MainBoot.cpp 的前例列進 CMakeLists.txt 的 add_executable(wb_serve …)
//    （共用檔，片段見交件報告）。沒列進去之前，本檔的函式沒有任何呼叫端，不影響建置；接上分派之後沒列 ⇒ 連結失敗（看得見）。
//    本檔只被 wb_serve 用；不要搬進任何 archive（陷阱 #2）。
//
//  ⚠ 會碰到的真實檔：
//    D:\HT9045\system\Gerneral.ini [System] bHasEnteredPEModel —— (P8-a) 進 PE 模式時寫 1（asGeneralPath；--dry 會導到 scratch）。
//    D:\HT9045\IniData\Data\<配方>\Temperature.Data [ATC] ATC7CH1..4Enabled —— (P8-b) 只在 Temperature.bATC70Active 時寫。
//    事件記錄：RecordProcess("Enable PE Model"／"Disable PE Model")（golden 同；D:\HT9045_Log 下的 EventLog／sqlite，依 bUseMDB）。
//    AI(W906-FRW-S100) 20260926:
//    D:\HT9045\IniData\Data\<配方>\Contact.Data [Mode] Contact／Vacuum、[Test Arm1]／[Test Arm2] Drop —— (S100-a) 只在 TestIF_File.iTestMode==_8Site1X4。
//    D:\HT9045\config\config.ini [Tray] bRecordSkipPosition —— (S100-b) 只在 CC_ASE_CL。
//    D:\HT9045\config\Security_new.def —— (S100-c) GetDummyVacuum 以讀為主；檔案不在時 CheckFile 建一份預設檔、缺鍵時 CheckAndReadIniData 補寫。
// ===========================================================================
#include "cmydef.h"             // bHasEnteredPEModel／bEnablePEModel／bFTPDownloadSetupFile／bTestSiteUse／CUSTOMER_CODE／iTestRunMode
#include "cprod.h"              // Temperature／TestIF_File／Prod
#include "Config.h"             // IniConfig.bC12UsePEMode
#include "CosFunction.h"        // CosFunction.bUsePEModelFunction／bHiSiliconFunction
#include "common.h"             // WriteIniDataGeneral／WriteIniData／ReadIniData／GetRecipeFileName
#include "cMyDB.h"              // RecordProcess（本體 canary_support.cpp）
#include "forms/fSecurity.h"    // fSecurity->Insufficient
#include "JsonBridge/FormJson.h"   // ht9045::formjson::FormLock／FormUnlock
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"

#include <cstdio>
#include <string>
#include <vector>

#include "MachineType.h"        // AI(W906-FRW-S100) 20260926: _8Site1X4／DropPlaceShiftContact
#include "forms/fMain.h"        // AI(W906-FRW-S100) 20260926: fMain->UpdateMainOperateMode／ShowTestHeadComp（S100-a）
#include "forms/fTestCategory.h"   // AI(W906-FRW-S100) 20260926: fTestCategory->AdjFormData／ShowTestCategory（S100-a）
#include "forms/fContactCT.h"   // AI(W906-FRW-S100) 20260926: fContactCT->ShowFormComp（S100-a）

// AI(W906-FRW-S100) 20260926: 不 include 它們的標頭（cinitial.h 帶進 aHotPlateSubstrate.h，與 cMyDB.h 的 MyDBIProcess 兩參數版模稜兩可；
//   cAuthority.h 帶 language.h），只宣告要用的（全域範圍；同 FileRW/Temperature.cpp 的做法）。
bool SetWorkParameter();                                                        // cinitial.h:76（golden cinitial.h:17）
void DoStructUnitConvert();                                                     // cUnitConvert.h:118（golden cUnitConvert.h:5）
void GetDummyVacuum();                                                          // cAuthority.h:78（本體 cAuthority.cpp:486；golden cAuthority.cpp:392）
void FileRW_Setup_ContactReadFile();                                            // FileRW/TestIF_File_SetUp.cpp 檔尾（golden fContact->ReadFile() → 移植樹 fContactForm->ReadFile()）
void FileRW_SetDummyVacuumToSucks();                                            // FileRW/_KitSuck.cpp（golden main.cpp:26680-26749）
namespace filerw { void ELTodo(const char* what); }                             // FileRW/_EditList.h（本檔不 include 它：HTEditList.h）

// csystem.h:55 的宣告（本體 csystem.cpp:13298，golden csystem.cpp:13226）。不 include csystem.h：只要這一支，少拉一串標頭
// （陷阱 #3 的兩個 TMyKitSuck 就是從標頭鏈進來的；同 FileRW/MainBoot.cpp 對 ReadWriteBinCountMode 的做法）。
bool HasICUnderMachine();

namespace {
struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};
}  // namespace

// ===========================================================================
//  (P8-a) golden TfMain::sbPEModelClick（main.cpp:33724-33778）
// ===========================================================================
// golden sbPEModel 的畫面狀態（TSpeedButton）：Tag 0/1 ＝ OFF/ON（DFM main.dfm sbPEModel 沒設 Tag ⇒ 0，Caption 'PE Model OFF'）。
//   全樹只有 sbPEModelClick 會改它（grep V912 `sbPEModel->Tag` 20260926：只有 :33729/:33731/:33744）。
static int        s_iPEModelTag     = 0;
static AnsiString s_asPEModelCaption = "PE Model OFF";

// golden main.cpp:11261-11262（TfMain::FormShow）：sbPEModel->Visible 的條件。按鈕看不見，使用者就按不到。
static bool W906_Main_PEModelVisible()
{
    return (CosFunction.bUsePEModelFunction &&                                  //Ifor 20160825 add PE 工程模式 By CosFunction
            IniConfig.bC12UsePEMode);                                           //Ifor 20160830 (Steven) 海思不可用PE模式    //Ifor 20161229 KYEC喬智說海思版本也要開啟PE模式
}

// 回 0＝golden 第一道守衛 return（什麼都沒動）；1＝進 PE 模式；2＝離開 PE 模式。
// *ui：golden 對主畫面三個輸入框設的 Enabled（VCL 那幾行，交給頁面套）；*todo：沒做到的 golden 段落。
static int W906_Main_sbPEModelClick(std::vector<std::pair<std::string, bool> >* ui, std::vector<std::string>* todo)
{
    // //AI(W906-FRW-P8) 20260926: 照翻 golden :33726，但條件看起來寫反了 —— HasICUnderMachine()（golden csystem.cpp:13226）
    //   是「機台內有 IC 回 true」，所以這一行的意思是「沒有權限 而且 機台內沒有 IC 才 return」：
    //   有權限 → 不管有沒有 IC 都切得了；沒權限但機台內有 IC → 也切得了。與同行註解「機台內有IC不可切換PE模式 && PE模式權限卡控」
    //   的意圖相反（應該是 Insufficient(128)==false || HasICUnderMachine()==true）。要不要改是 Steven／Jimmy 的決定，這裡不順手修。
    //   Insufficient(128) 預設 bAlarm=true：權限不足時 golden 會跳「權限不足」訊息（移植樹走 ShowMyMessage）。
    //   //AI(W906-FRW-P8) 20260927: 照翻 golden（Steven 20260927 Q13＝A）；疑似寫反，待 Jimmy 在 BCB6 確認原意。
    //     （D:\HT9045 主 repo V912 的行號：函式頭 main.cpp:33726、這一行守衛 :33728。）
    if(fSecurity->Insufficient(128)==false && HasICUnderMachine()==false)       //Ifor 20160826 add 機台內有IC不可切換PE模式 && PE模式權限卡控
        return 0;

    if(s_iPEModelTag==0)
    {
        s_iPEModelTag=1;
        s_asPEModelCaption = "PE Model ON";                                     // golden: sbPEModel->Caption = "PE Model ON";
        ;                                                                       // golden: sbPEModel->Font->Color = clGreen;（UI-only，頁面依 tag 著色）
        bHasEnteredPEModel=true;                                                //Ifor 20160822 進入PE工程模式
        bEnablePEModel=true;                                                    //Ifor 20160822 啟動PE工程模式
        ui->push_back(std::make_pair(std::string("edATCAmbientTemper"), true)); // golden: edATCAmbientTemper->Enabled=true;   //Ifor 20160411 開啟常溫溫度修改
        ui->push_back(std::make_pair(std::string("edWorkTemperBase"), true));   // golden: edWorkTemperBase->Enabled=true;     //Ifor 20160411 開啟工作溫度修改
        ui->push_back(std::make_pair(std::string("edSoakTime"), true));         // golden: edSoakTime->Enabled=true;           //Ifor 20160822 開啟Soak Time 修改
        WriteIniDataGeneral("System", "bHasEnteredPEModel",bHasEnteredPEModel);                                         //Ifor 20160824 避免 PE模式修改後程式被關閉上傳
        RecordProcess("Enable PE Model");
        return 1;
    }
    else
    {
        s_iPEModelTag=0;
        s_asPEModelCaption = "PE Model OFF";                                    // golden: sbPEModel->Caption = "PE Model OFF";
        ;                                                                       // golden: sbPEModel->Font->Color = clRed;（UI-only）
        bEnablePEModel=false;                                                   //Ifor 20160822 關閉PE工程模式
        if(CosFunction.bHiSiliconFunction==true)                                //Ifor 20160822 關閉PE功能模式且為海思版本需鎖定溫度相關修改
        {
            ui->push_back(std::make_pair(std::string("edATCAmbientTemper"), false));   // golden: edATCAmbientTemper->Enabled=false;
            ui->push_back(std::make_pair(std::string("edWorkTemperBase"), false));     // golden: edWorkTemperBase->Enabled=false;
            ui->push_back(std::make_pair(std::string("edSoakTime"), false));           // golden: edSoakTime->Enabled=false;
        }
        bFTPDownloadSetupFile=false;                                            //Ifor 20160822 關閉PE工程模式需強制下載Setup File
        RecordProcess("Disable PE Model");
        // 注意：golden 離開 PE 模式「不」把 bHasEnteredPEModel 清回 false、也不寫檔 —— 只有 FTP 下載 Setup File 成功才清
        //   （golden KYECFTP/FTPClient.cpp:1017-1021）。所以一旦進過 PE 模式，Gerneral.ini 就一直是 1，直到下載工作檔。照 golden。
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20160830 add 離開PE模式 要將修改的Bin Cons Fail 強制寫回 True
        {
            // //AI(W906-FRW-P8) 20260926: GATE —— 客戶專屬（CC_KYEC_LEE）且要額外移植（S25）：golden 最後兩行把字串塞進
            //   fBinSel->sBinConsFail[iTestRunMode] 再程式按 fBinSel->spbSave（TfBinSel 存檔 Binasgn.Data）；移植樹 BinSel 的存檔
            //   是網頁 C 路（FileRW/BinSelect.cpp），沒有「程式按存檔鈕」的入口。只做前半（改 Prod.bConsFail）而不存檔，
            //   記憶體與檔案會不一致，所以整段一起擋，回報待辦。
#if 0 // GATE(W906-FRW-P8-KYEC): golden main.cpp:33757-33776 —— VERBATIM
            AnsiString StrData="";
            for(int i=0; i<TEST_MAX_BIN; i++)
            {
                if(Prod.iT6CatData[i]==Prod.iIfErrorT6)
                {
                    StrData=StrData+"1";
                    Prod.bConsFail[i]=true;
                }
                else
                {
                    StrData=StrData+"0";
                }

                if(i!=TEST_MAX_BIN)
                    StrData=StrData+",";
            }
            fBinSel->sBinConsFail[iTestRunMode]->CommaText=StrData;
            fBinSel->spbSave->Click();
#endif
            todo->push_back("golden main.cpp:33757-33776 (CC_KYEC_LEE) not done: force Bin Cons Fail back to true "
                            "(Prod.bConsFail + fBinSel->spbSave->Click(), Binasgn.Data) -- no programmatic BinSel save in the port");
        }
        return 2;
    }
}

// WS 動作的本體（wb_serve.cpp 分派片段見交件報告；呼叫端不持鎖，本函式自己持 FormLock）。
//   payload {"op":"get"}   → 目前狀態（不寫檔）
//   payload {"op":"click"} → golden sbPEModelClick（按鈕看不見時拒絕：golden 使用者按不到看不見的鈕）
// *ok：執行了 golden 本體（含第一道守衛 return）或 get ＝ true；參數錯、按鈕看不見 ＝ false。
std::string W906_Main_PEModelOp(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;
    std::string op = "get";
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0) {
            w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-payload")
             .Key("detail").String("value 不是合法 JSON（要 {\"op\":\"get\"|\"click\"}）").EndObject();
            return w.Str();
        }
        const cJSON* jo = cJSON_GetObjectItemCaseSensitive(root, "op");
        if (jo && cJSON_IsString(jo) && jo->valuestring) op = jo->valuestring;
        cJSON_Delete(root);
    }
    if (op != "get" && op != "click") {
        w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-op").Key("detail").String(op).EndObject();
        return w.Str();
    }

    FormLockGuard lock;                                                         // 改全域旗標、寫 Gerneral.ini
    const bool visible = W906_Main_PEModelVisible();
    int rc = -1;
    std::vector<std::pair<std::string, bool> > ui;
    std::vector<std::string> todo;
    if (op == "click") {
        if (!visible) {
            w.BeginObject().Key("executed").Bool(false).Key("guard").String("button-hidden")
             .Key("detail").String("golden main.cpp:11261: sbPEModel->Visible = CosFunction.bUsePEModelFunction && IniConfig.bC12UsePEMode "
                                   "is false on this machine -- the operator cannot press it").EndObject();
            return w.Str();
        }
        rc = W906_Main_sbPEModelClick(&ui, &todo);
        std::printf("act.main.peModel click -> %s (bHasEnteredPEModel=%d bEnablePEModel=%d)\n",
                    rc == 0 ? "guard" : (rc == 1 ? "ON" : "OFF"), (int)bHasEnteredPEModel, (int)bEnablePEModel);
    }
    if (ok) *ok = true;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:33724-33778 TfMain::sbPEModelClick");
    if (op == "click") {
        w.Key("executed").Bool(rc != 0);
        if (rc == 0) w.Key("guard").String("insufficient-128-and-no-ic");   // golden :33726 的 return（條件原樣，見 AI 註解）
        else w.Key("result").String(rc == 1 ? "on" : "off");
        w.Key("wrote").BeginArray();
        if (rc == 1) w.String("system\\Gerneral.ini [System] bHasEnteredPEModel=1");
        w.EndArray();
        w.Key("ui").BeginObject();
        for (std::size_t i = 0; i < ui.size(); ++i) w.Key(ui[i].first).BeginObject().Key("enabled").Bool(ui[i].second).EndObject();
        w.EndObject();
        w.Key("todo").BeginArray();
        for (std::size_t i = 0; i < todo.size(); ++i) w.String(todo[i]);
        w.EndArray();
    }
    w.Key("visible").Bool(visible);
    w.Key("tag").Number((wb_int64)s_iPEModelTag);
    w.Key("caption").String(s_asPEModelCaption.c_str());
    w.Key("bHasEnteredPEModel").Bool(bHasEnteredPEModel);
    w.Key("bEnablePEModel").Bool(bEnablePEModel);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  (P8-b) golden TfMain::mtDutOnOffMouseUp（main.cpp:29930）的 :30329-30374
// ===========================================================================
// golden 位置：bSiteUseEE==false 分支，開關 Site 之後（bTestSiteUse[Z][Y][X] 已翻轉、ChangeATCSiteUse、SaveCloseOpenSiteEven、
//   ShowTestHeadComp、Auto Site Mapping、AmkorSendMessage、myLog.Save_SiteStatusLog 之後）。bSiteUseEE==true（工程師開關）那一支
//   golden 沒有這一段（:30603 之後直接結束）。
// 回 true ＝ Temperature.bATC70Active：檔已寫、Temperature.bATC7ChannelEnabled[] 已讀回，*asData1／*asData2 是 golden :30372-30373
//   組好的字串 —— 呼叫端照 golden :30375 送 ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, *asData1, *asData2)（外部設備，Jimmy）。
// 回 false ＝ 不是 ATC 7.0，golden 什麼都不寫。
// ⚠ 寫的是配方檔（目前工單的 Temperature.Data），不是 system 檔；只有 TestIF_File.iTestMode==DualSite（1x2）時才真的寫鍵
//   （golden switch 只有這一個 case），其他模式只讀回舊值。
bool W906_Main_DutOnOff_SaveATC7Channels(AnsiString* asData1Out, AnsiString* asData2Out)
{
    AnsiString szDir=GetRecipeFileName("Temperature.Data");
    AnsiString asData1="", asData2="";
    if (asData1Out) *asData1Out = "";
    if (asData2Out) *asData2Out = "";

    if(Temperature.bATC70Active==true)
    {
        if(TestIF_File.iShuttleMode==0)                                     //Dual Shuttle
        {
            switch(TestIF_File.iTestMode)
            {
                case DualSite:                                              //1x2    //Ifor 20160314 修正ATC 7.0 Dual Shuttle 關Site 異常
                    WriteIniData(szDir, "ATC", "ATC7CH1Enabled", bTestSiteUse[0][0][0]);
                    WriteIniData(szDir, "ATC", "ATC7CH2Enabled", bTestSiteUse[0][0][1]);
                    WriteIniData(szDir, "ATC", "ATC7CH3Enabled", bTestSiteUse[1][0][0]);
                    WriteIniData(szDir, "ATC", "ATC7CH4Enabled", bTestSiteUse[1][0][1]);
                break;
            }
        }
        else if(TestIF_File.iShuttleMode==1)                                //Single Suttle
        {
            switch(TestIF_File.iTestMode)
            {
                case DualSite:                                              //1x2
                    if(TestIF_File.iShuttle_Sel==0)
                    {
                        WriteIniData(szDir, "ATC", "ATC7CH1Enabled", bTestSiteUse[0][0][0]);
                        WriteIniData(szDir, "ATC", "ATC7CH2Enabled", bTestSiteUse[0][0][1]);
                        WriteIniData(szDir, "ATC", "ATC7CH3Enabled", false);
                        WriteIniData(szDir, "ATC", "ATC7CH4Enabled", false);
                    }
                    else if(TestIF_File.iShuttle_Sel==1)
                    {
                        WriteIniData(szDir, "ATC", "ATC7CH1Enabled", false);
                        WriteIniData(szDir, "ATC", "ATC7CH2Enabled", false);
                        WriteIniData(szDir, "ATC", "ATC7CH3Enabled", bTestSiteUse[1][0][0]);
                        WriteIniData(szDir, "ATC", "ATC7CH4Enabled", bTestSiteUse[1][0][1]);
                    }
                    break;
            }
        }
        Temperature.bATC7ChannelEnabled[0] = ReadIniData(szDir, "ATC", "ATC7CH1Enabled", false);
        Temperature.bATC7ChannelEnabled[1] = ReadIniData(szDir, "ATC", "ATC7CH2Enabled", false);
        Temperature.bATC7ChannelEnabled[2] = ReadIniData(szDir, "ATC", "ATC7CH3Enabled", false);
        Temperature.bATC7ChannelEnabled[3] = ReadIniData(szDir, "ATC", "ATC7CH4Enabled", false);

        asData1.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[0], Temperature.bATC7ChannelEnabled[1]);
        asData2.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[2], Temperature.bATC7ChannelEnabled[3]);
        // golden :30375 ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, asData1, asData2); —— 外部設備通訊，由呼叫端送（見上）
        if (asData1Out) *asData1Out = asData1;
        if (asData2Out) *asData2Out = asData2;
        return true;
    }
    return false;
}

// ===========================================================================
//  (S100-a) golden TfMain::sbSetupClick（V912 main.cpp:28457-28498）—— 主畫面「Set Up」鈕
//  AI(W906-FRW-S100) 20260926: RULINGS_20260926 S100。golden 的順序：:28459-28468 ASE K15 密碼（bASEK15UsePW，客戶專屬，不在這裡）
//    → :28469 NewRecordProcess("MES2177","Enter Set Up") → :28471 fSetup->ShowModal() → **視窗關掉之後**的尾段 :28472-28497（本函式）。
//    網頁的 Setup 頁（Setup.SetUp.html，FileRW/TestIF_File_SetUp.cpp）開頁＝editlist.get，沒有「關頁」事件 → 呼叫端在「A02 關表單
//    （closed）」與「真的寫了檔（SaveSetupFile）」之後跑本函式（同 FileRW/Temperature.cpp S88 MainTempOffsetTail 的做法；時機已由 RULINGS_20260926 S107-1 定案「存檔後就跑，不改成等 Exit」—— AI(W906-FRW-S158) 20260927 更正原本的「時機待 Steven」）。
//  ⚠ golden 的怪行為，照翻不修（:28488-28497）：_8Site1X4（海思 8Site 1x4）模式下，**每一次關 Setup 視窗**都把目前配方
//    Contact.Data 的 [Mode] Contact 改成 DropPlaceShiftContact（7）、[Mode] Vacuum 改成 1、兩個 Test Arm 的 Drop 改寫 —— 不管使用者
//    有沒有改、有沒有按存檔，Contact 頁上改過的這幾個值下次關 Setup 就被蓋回去。
//    ⚠⚠ 而且 Drop 寫進去的不是 2.00：golden 傳的是字串常值 "2.00"（const char*），WriteIniData 的多載裡 const char* → bool
//    是標準轉換、→ AnsiString 是使用者定義轉換，所以選到 bool 版 ⇒ WriteBool(true) ⇒ 檔案裡是 Drop=1（DeviceForm 讀成 1.0）。
//    移植樹的多載集合與 golden 相同（common.h:247-252），結果一樣（20260926 以 static_assert(decltype) 在 g++ 上確認選到 bool 版；
//    BCB6 依同一條 C++ 規則）。看起來 golden 本意是 2.00 —— 要不要改是 Steven／Jimmy 的決定，這裡不順手修。
//  其餘（純版面，照呼叫）：fTestCategory->AdjFormData、fContactCT->ShowFormComp、ShowTestCategory（移植樹門面；畫面由 tag 更新）。
//  fAutomation->AmkorSendMessage(0)（:28477）移植樹沒有 → GATE（同 S88）。
//  ⚠ 會寫的真實檔：<DataPath><配方>\Contact.Data 四鍵（只在 _8Site1X4）；SetWorkParameter 以讀為主（golden 同）。
//    Contact.Data 的 C 路擁有者是 FileRW/DeviceForm_File.cpp（Contact 頁）：網頁同時開著 Contact 頁、之後在那一頁存檔，會把這四鍵
//    寫成那一頁開頁時的值（golden 是 ShowModal，兩個視窗不會同時開）——跟 S90 同一類，不在本項處理，交件報告列給 Steven。
// ===========================================================================
void W906_Main_sbSetupClickTail()
{
    DoStructUnitConvert();                                                      // golden main.cpp:28472

    fTestCategory->AdjFormData();                                               // golden :28474（移植樹門面 cTestCategory.cpp，純版面）
    fContactCT->ShowFormComp();                                                 // golden :28475（移植樹門面，純版面）
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28476
    if(CosFunction.bAmkorFunction || CUSTOMER_CODE==CC_QUALCOMM)                // golden TfAutomation::AmkorSendMessage（automation.cpp:2302）第一道條件；其他客戶 golden 什麼都不送
        filerw::ELTodo("golden main.cpp:28477 fAutomation->AmkorSendMessage(0) (ATK site-map UDP message, bAmkorFunction/CC_QUALCOMM) -- TfAutomation::AmkorSendMessage is not ported");
#if 0 // GATE(W906-FRW-S100-AMKOR) golden main.cpp:28477 -- TfAutomation::AmkorSendMessage 移植樹沒有（Automation/automation.h:331；UDP 對外通訊、客戶專屬；同 S88）。VERBATIM
    fAutomation->AmkorSendMessage(0);                                           //Steven 20120330 : ATK Site Map Monitorning
#endif // GATE(W906-FRW-S100-AMKOR)
    fMain->UpdateMainOperateMode();                                             // golden :28478（AI(W906-FRW-S158) 20260927 更正：不是計數 stub —— wb_serve 開機裝了真本體 forms/fMain_OperateMode.cpp:139（tools/wb_serve.cpp:4065 W906_InstallUpdateMainOperateMode），會切加熱器繼電器、送 ATC7 指令）

    //ChungHung 20130910 alter for SCK can close site by Index
    fMain->ShowTestHeadComp(false);                                             //ChungHung 20130910 alter for SCK can close site by Index   // golden :28481（移植樹 forms/fMain.cpp 空殼）
    if(IniConfig.bA09_ByArmCloseSite)
    {
        fTestCategory->ShowTestCategory(0);
        fTestCategory->ShowTestCategory(1);
    }

    AnsiString szDir="";
    if(TestIF_File.iTestMode==_8Site1X4)                                        // golden :28489（怪行為與 Drop=1 見上）
    {
        szDir=GetRecipeFileName("Contact.Data");
        WriteIniData(szDir, "Mode", "Contact",          DropPlaceShiftContact);
        WriteIniData(szDir, "Mode", "Vacuum",           1);
        WriteIniData(szDir, "Test Arm1", "Drop",        "2.00");                // ⚠ const char* → bool 多載：實際寫 1（golden 同，見上）
        WriteIniData(szDir, "Test Arm2", "Drop",        "2.00");                // ⚠ 同上
        FileRW_Setup_ContactReadFile();                                         // golden :28496 fContact->ReadFile()（移植樹 fContactForm->ReadFile()，同 FileRW/TestIF_File_SetUp.gen.inc :3121 的轉接）
        std::printf("sbSetupClick tail: _8Site1X4 -> wrote %s [Mode] Contact=%d Vacuum=1, [Test Arm1/2] Drop (bool overload -> 1), then reread Contact\n",
                    szDir.c_str(), (int)DropPlaceShiftContact);
    }
}

// ===========================================================================
//  (S100-b) golden TfMain::cb_MainAutoSkipSwitchClick（V912 main.cpp:31841-31847）—— 主畫面「Auto Skip」勾選框
//  AI(W906-FRW-S100) 20260926: RULINGS_20260926 S100。
//    看得見：golden TfMain::FormShow :11155 cb_MainAutoSkipSwitch->Visible=(CUSTOMER_CODE==CC_ASE_CL) —— 客戶專屬（S25）；
//            :11156 Checked=IniConfig.bRecordSkipPosition。
//    按得到：golden ChangeLevelAttr :12946／:12956 —— SystemStart 時 Enabled=false；否則 Enabled=(AccessLevel>=LevelSet.AccessLevel[147])。
//    按下（TCheckBox OnClick，勾選狀態已經變了）：IniConfig.bRecordSkipPosition=Checked，寫 config.ini [Tray] bRecordSkipPosition。
//  config.ini 的擁有者是 FileRW/IniConfig.cpp：這一鍵的另一個寫者是 SaveLastSetIni → ProcessLastSetIni_Tray（cprod.cpp:2678-2679，
//    只在 CC_ASE_CL）寫的是記憶體 IniConfig.bRecordSkipPosition；本函式先改記憶體再寫檔（golden :31845-31846），不會被蓋回舊值。
//    IniConfig 頁的 HTEditList 沒有這一鍵。
//  payload {"op":"get"} → 目前狀態；{"op":"click","checked":bool}（按下之後的勾選狀態）→ golden 本體。
//  *ok：get 或執行了本體 ＝ true；參數錯、看不見、按不到 ＝ false。呼叫端不持鎖，本函式自己持 FormLock。
// ===========================================================================
static bool W906_Main_AutoSkipVisible() { return (CUSTOMER_CODE==CC_ASE_CL); }                                       // golden main.cpp:11155
static bool W906_Main_AutoSkipEnabled() { return SystemStart ? false : (AccessLevel>=LevelSet.AccessLevel[147]); }   // golden main.cpp:12946／:12956

static void W906_Main_cb_MainAutoSkipSwitchClick(bool bChecked)
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bRecordSkipPosition = bChecked;                                   // golden: cb_MainAutoSkipSwitch->Checked
    WriteIniData(sPath, "Tray", "bRecordSkipPosition", IniConfig.bRecordSkipPosition);                                  //jou 2013-05-30 Record Skip position
}

std::string W906_Main_AutoSkipSwitchOp(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;
    std::string op = "get";
    bool hasChecked = false, checked = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0) {
            w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-payload")
             .Key("detail").String("value 不是合法 JSON（要 {\"op\":\"get\"} 或 {\"op\":\"click\",\"checked\":bool}）").EndObject();
            return w.Str();
        }
        const cJSON* jo = cJSON_GetObjectItemCaseSensitive(root, "op");
        if (jo && cJSON_IsString(jo) && jo->valuestring) op = jo->valuestring;
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "checked");
        if (jc && cJSON_IsBool(jc)) { hasChecked = true; checked = cJSON_IsTrue(jc) != 0; }
        cJSON_Delete(root);
    }
    if (op != "get" && op != "click") {
        w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-op").Key("detail").String(op).EndObject();
        return w.Str();
    }
    if (op == "click" && !hasChecked) {
        w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-payload")
         .Key("detail").String("click 要帶 checked（按下之後的勾選狀態）").EndObject();
        return w.Str();
    }

    FormLockGuard lock;                                                         // 改 IniConfig、寫 config.ini
    const bool visible = W906_Main_AutoSkipVisible();
    const bool enabled = W906_Main_AutoSkipEnabled();
    if (op == "click") {
        if (!visible || !enabled) {
            w.BeginObject().Key("executed").Bool(false).Key("guard").String(!visible ? "customer-only" : "disabled")
             .Key("detail").String(!visible
                 ? "golden main.cpp:11155: cb_MainAutoSkipSwitch->Visible=(CUSTOMER_CODE==CC_ASE_CL) -- hidden on this machine (customer-specific, S25)"
                 : "golden main.cpp:12946/12956: disabled while SystemStart, else needs AccessLevel>=LevelSet.AccessLevel[147]")
             .EndObject();
            return w.Str();
        }
        W906_Main_cb_MainAutoSkipSwitchClick(checked);
        std::printf("act.main.autoSkip click -> config.ini [Tray] bRecordSkipPosition=%d\n", (int)IniConfig.bRecordSkipPosition);
    }
    if (ok) *ok = true;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:31841-31847 TfMain::cb_MainAutoSkipSwitchClick（Visible :11155、Enabled :12946/:12956）");
    if (op == "click") {
        w.Key("executed").Bool(true);
        w.Key("wrote").BeginArray().String("config\\config.ini [Tray] bRecordSkipPosition").EndArray();
    }
    w.Key("visible").Bool(visible);
    w.Key("enabled").Bool(enabled);
    w.Key("checked").Bool(IniConfig.bRecordSkipPosition);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  (S100-c) golden TfMain::SetToDefineValue1Click（V912 main.cpp:26672-26750）—— StringGrid2（Comm View 頁 tsInArm 的吸嘴真空
//    時間表）的右鍵選單 PopupMenu4「Set To Define Value」（main.dfm:17317-17331）
//  AI(W906-FRW-S100) 20260926: RULINGS_20260926 S100。GetDummyVacuum（cAuthority.cpp:486）原本沒有呼叫者，這是 golden 唯一的呼叫點。
//    選單怎麼出來：golden StringGrid2DblClick（main.cpp:25128-25145）—— SystemStart 時不出；Row 是 0／5／10／15／20 或 >21 不出；
//      Col<3 或 Col==5 不出；AccessLevel<iDefSupervisorLevel 不出；都過了才 PopupMenu4->Popup。所以這裡照這四條重查
//      （不信任前端），row／col 由頁面送（雙擊的那一格，StringGrid2SelectCell :25121 記的 iStringGrid2Col／Row）。
//    按下選單：:26675 MessageDlg「Sure Set To Define Value? (確定設定為預設值？)」mbYes/mbNo → 網頁兩段式確認
//      （confirmed=false 回 needConfirm＋golden 字樣；confirmed=true 才執行）。
//    執行：:26679 GetDummyVacuum()（讀 Security_new.def [Dummy Vacuum] 4 鍵進 DummyVacuum）→ :26680-26749 吸嘴時間與 LastSet 的 dummy 時間
//      （FileRW/_KitSuck.cpp FileRW_SetDummyVacuumToSucks：InArmSuck 等 TMyKitSuck 在 aHotPlateSubstrate.h，本檔帶 cMyDB.h，兩個一起
//      include 會讓 MyDBIProcess 兩參數呼叫模稜兩可 —— 同 cSortCT.cpp 的說明，所以放到那一支 TU）。
//    ⚠ 只改記憶體：吸嘴的 VacuumOn/OffTime（真空建立／破真空的時間基準）與 LastSet.i*VacuumDummy*Time；golden 本段不寫 lastdata.dat，
//      下一次 WriteLastDataFile（Clear Count、換配方…）才落地。會影響機台的吸取判斷時間 —— 交 Jimmy 確認（交件報告）。
//    ⚠ 會碰的真實檔：D:\HT9045\config\Security_new.def（CheckFile：檔案不在就建一份預設檔；CheckAndReadIniData：缺鍵補寫，golden 同）。
//  payload {"row":int,"col":int,"confirmed":bool}。*ok：走到確認框或執行完 ＝ true；守衛擋下／參數錯 ＝ false。自己持 FormLock。
// ===========================================================================
std::string W906_Main_SetToDefineValueOp(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;
    bool confirmed = false;
    int row = -1, col = -1;
    bool hasCell = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0) {
            w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-payload")
             .Key("detail").String("value 不是合法 JSON（要 {\"row\":int,\"col\":int,\"confirmed\":bool}）").EndObject();
            return w.Str();
        }
        const cJSON* jr = cJSON_GetObjectItemCaseSensitive(root, "row");
        const cJSON* jcol = cJSON_GetObjectItemCaseSensitive(root, "col");
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
        if (jr && cJSON_IsNumber(jr) && jcol && cJSON_IsNumber(jcol)) { row = jr->valueint; col = jcol->valueint; hasCell = true; }
        confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));             // 明確 true 才算確認過
        cJSON_Delete(root);
    }
    if (!hasCell) {
        w.BeginObject().Key("executed").Bool(false).Key("guard").String("bad-payload")
         .Key("detail").String("要帶 row／col（StringGrid2 雙擊的那一格；golden 選單只在特定格子雙擊才出現）").EndObject();
        return w.Str();
    }

    FormLockGuard lock;                                                         // 改吸嘴時間、LastSet，讀 Security_new.def
    // golden StringGrid2DblClick :25131-25141（選單出不出來）—— 每次重查
    const char* guard = 0;
    const char* detail = 0;
    if(SystemStart==true)
        { guard = "SystemStart"; detail = "golden main.cpp:25131: running -- the popup menu does not open"; }
    else if(row==0  || row==5 || row==10 || row==15 || row==20 || row>21)
        { guard = "bad-cell"; detail = "golden main.cpp:25134-25136: header/separator row or row>21 -- the popup menu does not open"; }
    else if(col<3 || col==5)
        { guard = "bad-cell"; detail = "golden main.cpp:25138-25139: col<3 or col==5 -- the popup menu does not open"; }
    else if(AccessLevel<iDefSupervisorLevel)                                    //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
        { guard = "not-authorized"; detail = "golden main.cpp:25140: AccessLevel<iDefSupervisorLevel -- the popup menu does not open"; }
    if (guard) {
        w.BeginObject().Key("executed").Bool(false).Key("guard").String(guard).Key("goldenLine").String("V912 main.cpp:25128-25145 StringGrid2DblClick")
         .Key("detail").String(detail).EndObject();
        return w.Str();
    }
    // golden :26675-26677 MessageDlg(mbYes/mbNo) → 網頁兩段式
    if (!confirmed) {
        if (ok) *ok = true;
        w.BeginObject().Key("executed").Bool(false).Key("needConfirm").Bool(true)
         .Key("prompt").BeginArray().String("Sure Set To Define Value? (確定設定為預設值？)").EndArray()
         .Key("goldenLine").String("V912 main.cpp:26675").EndObject();
        return w.Str();
    }

    GetDummyVacuum();                                                           // golden :26679
    FileRW_SetDummyVacuumToSucks();                                             // golden :26680-26749
    std::printf("act.main.setToDefineValue -> Dummy Vacuum arm on/off %d/%d index on/off %d/%d (memory only)\n",
                DummyVacuum.iArmVacuumOn, DummyVacuum.iArmVacuumOff, DummyVacuum.iIndexVacuumOn, DummyVacuum.iIndexVacuumOff);
    if (ok) *ok = true;
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("golden").String("V912 main.cpp:26672-26750 TfMain::SetToDefineValue1Click（GetDummyVacuum cAuthority.cpp:392）");
    w.Key("dummyVacuum").BeginObject();
    w.Key("armOn").Number((wb_int64)DummyVacuum.iArmVacuumOn);
    w.Key("armOff").Number((wb_int64)DummyVacuum.iArmVacuumOff);
    w.Key("indexOn").Number((wb_int64)DummyVacuum.iIndexVacuumOn);
    w.Key("indexOff").Number((wb_int64)DummyVacuum.iIndexVacuumOff);
    w.EndObject();
    w.Key("catchTray").BeginObject().Key("on").Number((wb_int64)650).Key("off").Number((wb_int64)31).EndObject();   // golden :26724-26725 寫死
    w.Key("read").BeginArray().String("config\\Security_new.def [Dummy Vacuum]（缺檔 CheckFile 建預設檔、缺鍵補寫）").EndArray();
    w.Key("memoryOnly").Bool(true);                                             // golden 本段不寫 lastdata.dat
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 盤點第一節第 3 項「關窗尾段」第一批 —— golden TfMain 主畫面設定鈕 sbXxxClick 在
//    設定視窗 ShowModal 回來（視窗關掉）之後的尾段。方案 docs/Q41_CLOSETAIL_PLAN_20260927.md（§3 逐頁表、§4 掛法、§5 步驟）；
//    宣告 FileRW/MainClickTail.h。golden 一律 V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950；
//    本段每一個 golden 行號 20260927 重新核對過）。本批四頁：LU-2 Ld_ULd、SP-6 Speed、BS-2 BinSel、CT-4 Contact；
//    其餘五頁（TrayForm、HotPlate、TrayAssignment、Yield、Configuration）等別的同事交件後再接（方案 §5.3、§5.6-§5.9）。
//  裁決：
//    * RULINGS_20260926 S107-1（Steven 20260926 17:5x，docs/RULINGS_20260926.md:284）「存檔後就跑，不改成等 Exit」——
//      溫度頁（FileRW/Temperature.cpp MainTempOffsetTail）、SetUp 頁（本檔 W906_Main_sbSetupClickTail）已照做；
//      decisions-pending R85（St01 建議 A，Steven 沒反對就照 A）把同一做法延伸到其餘設定頁：本批 Ld_ULd、Speed、BinSel。
//    * R84（A）：Contact 不是關窗尾段 —— golden :28314 fContact->Show()（非 modal），換算＋重載參數在開窗當下就跑 → 掛在開頁。
//    * R86（A）：每支開頭再查一次 SystemStart||SoftStart，運轉中不跑、印一行、回原因（呼叫端放進 filerw::ELTodo）。
//      [W906] golden 運轉中打不開這些視窗（V912 main.cpp:3970-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來、
//      :29033-29034 sbSettingClick if(SystemStart) return;、Speed 自己 :28680-28681 if(SystemStart && iHome) return;）——
//      這是把 golden 的前提在伺服器端重查一次，不是改行為。C 路存檔 editlist.save 已先擋（RULINGS_20260927 #7，
//      tools/wb_serve.cpp:5303），這裡是第二道；Contact 的開頁 editlist.get 只有開窗閘查 SystemStart（FileRW/_EditPage.cpp
//      GTools → GNotRunning），SoftStart（START／回原點已按、主迴圈還沒接手）只有這裡擋。
//  共同寫法：開頭 filerw::ELMark(<函式名>)（ack.session.trace）；跑了印 "close-tail <函式名> -> ran: …"，沒跑印
//    "close-tail <函式名> -> not run: …" 並在 trace 多記 "<函式名>:running"（wb_serve 主控台；探針
//    tools/webprobe/q41_closetail_probe.py 兩個都看）。回傳 nullptr＝跑了；非 nullptr＝沒跑的原因（本段的靜態緩衝，
//    下一次呼叫前有效；呼叫端立刻交給 filerw::ELTodo，它會複製）。
//  ShowMyMessage：尾段裡的 SetWorkParameter 在 rotate shuttle 不支援的模式會跳 ShowMyMessage（移植樹 cinitial.cpp:7270，
//    golden 同）—— 照現成兩頁（MainTempOffsetTail、W906_Main_sbSetupClickTail）的做法原樣呼叫、不另外攔：在 editlist.save／get
//    持 FormLock 期間會變成網頁的阻塞框（方案 §九，還沒實測過）。DoStructUnitConvert（cUnitConvert.cpp:675）、ShowBinSel
//    （cShowBinSelect.cpp:1187）本體沒有訊息框（20260927 讀過）。
//  ⚠ 會寫的檔（本批四支）：只有 SetWorkParameter（BS-2、CT-4 會叫），以讀為主 —— ReadTechData 缺鍵時補寫
//    D:\HT9045\system\Gerneral.ini [Shuttle] CHECK_RANGE／iInShtZRange（cinitial.cpp:16137-16138 CheckAndReadIniDataGeneral）、
//    fTeach->ReadFile 第一次把舊 tech 轉 teach.ini 時會寫（forms/fTeachPara.cpp:370 起）；:9352 fTrayAssignment->ReadFile 只讀。
//    DoStructUnitConvert、ShowBinSel 只改記憶體；EventReport 目前是模擬計數（SECSGEM/SecsEventReport.cpp:15），不送 MES。
//  頭段（ShowModal 之前的 NewRecordProcess("MES21xx","Enter …")、Contact 的 RTC vision rtInspEnd）不在本項（方案 §八 4）。
// ===========================================================================
#include "forms/fShowBinSelect.h"      // AI(W906-FRW-S158) 20260927 [W906]: fShowBinSelect->ShowBinSel（BS-2；只帶 vclcompat／MachineType，不碰兩個 TMyKitSuck）
#include "SECSGEM/SecsEventType.h"     // AI(W906-FRW-S158) 20260927 [W906]: SECS_EVENT.EnterSpeed（SP-6）
#include "SECSGEM/SecsEventReport.h"   // AI(W906-FRW-S158) 20260927 [W906]: EventReport（SP-6）
#include "FileRW/MainClickTail.h"      // AI(W906-FRW-S158) 20260927 [W906]: 本段四支的宣告

namespace filerw { void ELMark(const char* what); }                             // FileRW/_EditList.h:150（本檔不 include 它，理由同 :74）

namespace {
// R86（見本段檔頭）。機台停著回 nullptr；運轉中印一行、trace 記 "<fn>:running"、回原因（給呼叫端 ELTodo）。
const char* CloseTailRunning(const char* fn, const char* golden, const char* what)
{
    if (!(SystemStart || SoftStart)) return nullptr;
    static std::string s_why;
    s_why = std::string(fn) + " not run (R86): " +
            (SystemStart ? "機台運轉中（SystemStart）" : "機台正要啟動或回原點（SoftStart）") +
            "——golden " + golden + " 的尾段（" + what + "）這次沒跑；golden 運轉中打不開這個視窗"
            "（V912 main.cpp:3970-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來）。停機後這一頁再存一次（Contact 頁：再開一次）就會跑";
    filerw::ELMark((std::string(fn) + ":running").c_str());
    std::printf("close-tail %s -> not run: SystemStart=%d SoftStart=%d (R86; golden %s)\n",
                fn, (int)SystemStart, (int)SoftStart, golden);
    return s_why.c_str();
}
void CloseTailRan(const char* fn, const char* what)
{
    std::printf("close-tail %s -> ran: %s\n", fn, what);
}
}  // namespace

// ---------------------------------------------------------------------------
//  LU-2　golden TfMain::sbLdUldClick（V912 main.cpp:28448-28455）—— 工具選單「Ld / ULd」鈕：
//    :28450 sbLdUld->Down=false（畫面）→ :28452 NewRecordProcess("MES2176","Enter Load / Unload")（頭段）
//    → :28453 fLd_ULd->ShowModal() → 視窗關掉之後的尾段 :28454（本函式，golden 只有這一行）。
//  呼叫端：FileRW/Ld_UldDelayTime.cpp SaveFlow（A02 關窗先跑 golden FormClose cLd_ULd.cpp:170；有寫檔就直接跑，S107-1）。
//  效果：Ld_UldDelayTime 本身不在換算範圍（DoStructUnitConvert 換 TestIF／Device／HotPlate／ArmOffset／ArmSpeed／DefForm），
//    只是把那幾個結構的 *_File 值再換一次成執行值（golden 同）。
// ---------------------------------------------------------------------------
const char* W906_Main_sbLdUldClickTail()
{
    filerw::ELMark("W906_Main_sbLdUldClickTail");                               // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbLdUldClickTail", "V912 main.cpp:28454 sbLdUldClick", "DoStructUnitConvert"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28454

    CloseTailRan("W906_Main_sbLdUldClickTail", "DoStructUnitConvert (golden V912 main.cpp:28454)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  SP-6　golden TfMain::sbSpeedClick（V912 main.cpp:28677-28700）—— 主畫面工具列「Speed」鈕（main.dfm:832，不經選單）：
//    :28679 sbSpeed->Down=false → :28680-28692 守衛（SystemStart&&iHome、CC_SPIL_CHINA_SUZHOU、Insufficient(3)；開頁閘
//    FileRW/_EditPage.cpp GSpeed 已照翻）→ :28693 ProceeToolBar()、:28694 NewRecordProcess("MES2187","Enter Speed")（頭段）
//    → :28695 fSpeed->ShowModal() → 視窗關掉之後的尾段 :28696-28699（本函式）。
//  呼叫端：FileRW/ArmSpeed_File.cpp SaveFlow。golden Speed 存檔鈕沒有 A02 守衛、沒有 Close()（cSpeed.cpp:1433 起一定寫檔），
//    所以只有「有寫檔 → 存完就跑」一種（S107-1）；golden 關窗的 FormClose（cSpeed.cpp:1272：fShow=false、ReadFile、DoIniDataToForm）
//    不跑（頁面還開著；存檔鈕自己已經重讀）。
//  ⚠ SECS「Enter Speed Page」（:28698-28699）golden 是 ShowModal 回來之後、也就是**關窗時**才送（Q41 盤點表 SP-6 寫「進頁時送」
//    不對，方案 §八 2）。golden 一次開關送一次；網頁每存一次送一次。移植樹 EventReport 目前是模擬計數（SECSGEM/SecsEventReport.cpp:15），
//    不送 MES —— SECS 真的接上之後再看要不要改成一次開頁只送一次（方案 §七末段，照既有裁決處理、不另開題）。
// ---------------------------------------------------------------------------
const char* W906_Main_sbSpeedClickTail()
{
    filerw::ELMark("W906_Main_sbSpeedClickTail");                               // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbSpeedClickTail", "V912 main.cpp:28696-28699 sbSpeedClick",
                                           "DoStructUnitConvert、SECS EnterSpeed"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28696

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem   // golden :28698
        EventReport(SECS_EVENT.EnterSpeed);                                     // golden :28699（關窗時送，見上；移植樹是模擬計數）

    CloseTailRan("W906_Main_sbSpeedClickTail", IniConfig.bEnable_SECS_GEM
                     ? "DoStructUnitConvert + EventReport(SECS_EVENT.EnterSpeed) (golden V912 main.cpp:28696-28699)"
                     : "DoStructUnitConvert (golden V912 main.cpp:28696; bEnable_SECS_GEM off, no EnterSpeed)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  BS-2　golden TfMain::sbBinClick（V912 main.cpp:28319-28328）—— 工具選單「Bin」鈕：
//    :28321 sbBin->Down=false → :28323 NewRecordProcess("MES2171","Enter Bin")（頭段）→ :28324 fBinSel->ShowModal()
//    → 視窗關掉（golden FormClose cBinSel.cpp:2137-2145：bShow=false、ReadFile(false,false,"")、InitShowBinDigital —— 在呼叫端做）
//    → 尾段 :28325-28327（本函式）。
//  呼叫端：FileRW/BinSelect.cpp BinCloseTail（A02 關窗先照 FormClose 還原再跑；有寫檔就直接跑，S107-1）。
//  ⚠ ShowBinSel 不是純畫面：它重算 iBinTray／bUnloadHasBin／iTrayLastBin／BinAssign（出料分 bin 在用：移植樹 acatchtray.cpp:8698、
//    csystem.cpp:14074 讀 iBinTray），START 時 golden 也不會重跑它 —— 這一頁沒跑尾段的話，停機期間與下一批的分 bin 狀態是舊的。
// ---------------------------------------------------------------------------
const char* W906_Main_sbBinClickTail()
{
    filerw::ELMark("W906_Main_sbBinClickTail");                                 // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbBinClickTail", "V912 main.cpp:28325-28327 sbBinClick",
                                           "DoStructUnitConvert、ShowBinSel、SetWorkParameter"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28325
    fShowBinSelect->ShowBinSel();                                               // golden :28326（移植樹 cShowBinSelect.cpp:1187）
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28327

    CloseTailRan("W906_Main_sbBinClickTail", "DoStructUnitConvert + ShowBinSel + SetWorkParameter (golden V912 main.cpp:28325-28327)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  CT-4　golden TfMain::sbContactClick（V912 main.cpp:28302-28317）—— 工具選單「Contact」鈕：
//    :28304 sbContact->Down=false → :28306 NewRecordProcess("MES2170","Enter Contact")（頭段）
//    → :28308-28312 IndexHasIC()==false && REAL_TIME_CCD==true && !COM2->bCCDDummyRum 時 COM2->SendCommToVision(rtInspEnd)＋
//      InitRealTimeCCDPara（RTC vision 機台通訊，頭段，不在本項 → Jimmy）
//    → :28314 fContact->Show()（//Jimmychiu 20240731 : ShowModal --> Show）→ :28315-28316（本函式）。
//  ⚠ golden 的怪處（照翻不修，R84＝A）：:28316 行尾註解「存檔後要重新load參數」是 ShowModal 時代的寫法；改成非 modal 的 Show()
//    之後，這兩行在「Contact 視窗剛打開」就跑完（用的是開窗前的值），存檔後反而不跑（golden Contact 存檔鈕 cContact.cpp:14179 起
//    沒有 SetWorkParameter —— 20260927 grep golden V912 cContact.cpp「SetWorkParameter」0 筆）。看起來原本的用意在 20240731 改版
//    之後就沒了；要不要改是 Jimmy／Steven 的決定，這裡照 golden 在開頁跑。
//    網頁存完一定自動重新開頁（引擎「寫完一定重讀」）→ editlist.get 又跑一次本函式，所以網頁上存完也會重載一次
//    （BCB 版同樣的操作要關掉再開一次 Contact 視窗、或按 START 才換成新值）。
//  呼叫端：FileRW/DeviceForm_File.cpp FormShowAndSnap（＝editlist.get 的 golden FormShow；golden Show() 觸發 FormShow 之後才回來
//    跑這兩行）。editlist.get 是「只看」也會送的指令，開窗閘只查 SystemStart（FileRW/_EditPage.cpp GTools）→ SoftStart 時只有這裡擋。
// ---------------------------------------------------------------------------
const char* W906_Main_sbContactClickOpen()
{
    filerw::ELMark("W906_Main_sbContactClickOpen");                             // FileRW：trace（editlist.get 的 ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbContactClickOpen", "V912 main.cpp:28315-28316 sbContactClick",
                                           "DoStructUnitConvert、SetWorkParameter"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28315
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28316（怪處見上）

    CloseTailRan("W906_Main_sbContactClickOpen", "DoStructUnitConvert + SetWorkParameter (golden V912 main.cpp:28315-28316, after Show())");
    return nullptr;
}

// ===========================================================================
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 盤點第一節第 3 項「關窗尾段」第二批 —— TF-5 TrayForm、HP-3 HotPlate、TA-6 TrayAssignment、
//    YM-5 Yield、CC-E15 Configuration（方案 docs/Q41_CLOSETAIL_PLAN_20260927.md §3、§5.3、§5.6-§5.9）。宣告 FileRW/MainClickTail.h 檔尾。
//    golden 一律 V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950；本段每一個 golden 行號 20260927 重新核對過）。
//  裁決與共同寫法同第一批（上面那段檔頭）：RULINGS_20260926 S107-1（存檔後就跑）、decisions-pending R85（其餘頁照 S107-1，A）、
//    R86（每支開頭 CloseTailRunning 再查 SystemStart||SoftStart，A）；CloseTailRunning／CloseTailRan 共用第一批那兩支。
//    Configuration 例外：網頁存檔＝golden FormClose（FileRW/IniConfig.cpp FileRW_IniConfig_Save），golden 關窗不論存不存都跑尾段
//    → 呼叫端不看有沒有存成（與 golden 完全一樣，R85 背景）。
//  HP-3 是 A 形狀（form.save，JsonBridge/FormJson.cpp FormSave）：沒有 filerw session —— 尾段裡的 filerw::ELMark 記到上一次 C 路
//    session 的殘留裡（下一次 SessionBegin 就清掉，不會出現在任何 ack），沒跑的原因由呼叫端交給 J.Todo（ack.todo），跑了只看主控台
//    "close-tail … -> ran"。LoadAutoCleanData 裡 golden 的 ShowMyMessage（C 路轉成 filerw::ELMessage，例 TestIF_File_Cleaning.gen.inc
//    「The site Y-pitch can not use arm 2 for auto clean!!」）在 HP-3 也一樣記到殘留 session、網頁看不到（交件列出）。
//  ⚠ 會寫的檔（本批新增；SetWorkParameter 那幾個見第一批檔頭）：
//    * FileRW_Cleaning_LoadAutoCleanData（HP-3 每次；CC-E15 只在 E43 有變）：<DataPath><配方>\HandlerCondition.Data [Configuration]
//      iIndexArmAutoCleanCnt 每次都寫（ReadWriteAutoCleanCount(true) 尾），另有條件式寫入（iAutoClean_Mode==0、ShiftHeight<=0、InitialOK 且
//      片數改變、Fix3 有 Bin、bEnableAutoCleanFunction==false）；bE43_1_AutoCleanCountSaveFolder 時寫
//      D:\HT9045\IniData\DefineAutoClean\AutoCleanCount.Data（清單見 FileRW/TestIF_File_Cleaning.cpp 檔頭 :11-21）。
//    * fMain->SetStartModeData（YM-5 每次；CC-E15 只在四個條件任一成立）→ SetRunStartMode（RunStartMode.cpp）：:883 fMain->SaveRunMode
//      → D:\HT9045\system\RunMode.txt、:922-923 NewRecordProcess MES2107／ChangeLog（事件紀錄，每存一次多兩筆）、:768
//      fMain->UpdateMainOperateMode（lastdata.dat）、:884 fSCKART->AccessFile(true)、A37 且 Initial 類模式時 :761／:764 兩份 Bundle ID 清單。
//    * fMain->UpdateMainOperateMode（CC-E15 每次）：D:\HT9045\system\lastdata.dat，並切加熱器繼電器、送 ATC7 指令（真本體
//      forms/fMain_OperateMode.cpp:139；HT9050 機台驗證要機台端在場，RULINGS_20260927 第 16 條）。
//    * fYieldMonitoring->ReadFile（CC-E15 每次）：<配方>\Tester.Data（缺鍵補寫＋明寫的 WriteIniData，uYieldMonitoring.cpp:2662-2668）。
//    * fSCKART->AccessFile(true)（CC-E15 每次）：CosFunction.bUseSCKART 且缺鍵時寫 <配方>\Tester.Data [AutoRetest] iTesterType=1
//      （forms/fSCKART.cpp:151；bUseSCKART 關時第一行 return）。
//    GetHotPlateYHalfPos、ShowBinSel、fSortCT->UpForm、ShowTestHeadComp、ShowTestCategory 只改記憶體／畫面。
//  InitialOK：GetHotPlateYHalfPos（ainarm_SearchPlacePlate.cpp:134）第一行 `if(InitialOK==false) return;`、LoadAutoCleanData 的片數寫檔
//    也看 InitialOK —— 照 golden 呼叫、不另外假設。wb_serve 裡 InitialOK 只在 PumpInit 成功時設 true（WebBridgeTags.cpp:563；
//    主控台 "spine pump: ARMED"）；FileRW/IniConfig.cpp:159 說「wb_serve 沒有把 InitialOK 設成 true」與此不一致（方案 §八 6，那段不動）。
//    HP-3 跑完印的那一行帶 InitialOK，驗收時以主控台為準。
//  頭段（ShowModal 之前的 NewRecordProcess("MES2173"／"MES2174"／"MES2175"／"MES2178"／"MES2185","Enter …")）不在本項：R101＝A 已裁決
//    （RULINGS_20260926 S165「要記」，在 C 路開頁 editlist.get 時記），另外派工（等 C 路開頁共用層交件後）；
//    Yield :28504-28506 Insufficient(39)、Configuration :28603-28607 CC_SCS Insufficient(30) 是開窗閘，也不在這裡。
// ===========================================================================
#include "forms/fSortCT.h"             // AI(W906-FRW-S158) 20260927 [W906]: fSortCT->UpForm（TA-6；只帶 forms/FormWidgets.h → vclcompat）
#include "forms/fYieldMonitoring.h"    // AI(W906-FRW-S158) 20260927 [W906]: fYieldMonitoring->ReadFile（CC-E15；只帶 vclcompat／MachineType）
#include "forms/fSCKART.h"             // AI(W906-FRW-S158) 20260927 [W906]: fSCKART->AccessFile（CC-E15；只帶 forms/FormWidgets.h）

// 不 include 它們的標頭（ainarm_SearchPlacePlate.h 帶進入料臂／HotPlate 的一串，陷阱 #3 的兩個 TMyKitSuck 就從那裡來；
//   FileRW/TestIF_File_Cleaning.cpp 沒有標頭），只宣告要用的（全域範圍，同本檔 :69-74 的做法）。
void GetHotPlateYHalfPos();                                                     // ainarm_SearchPlacePlate.h:49（本體 ainarm_SearchPlacePlate.cpp:132）
void FileRW_Cleaning_LoadAutoCleanData();                                       // FileRW/TestIF_File_Cleaning.cpp（golden fCleaning->LoadAutoCleanData() 的外部呼叫點）

// ---------------------------------------------------------------------------
//  TF-5　golden TfMain::sbTrayFormClick（V912 main.cpp:28404-28414）—— 工具選單「Tray Form」鈕：
//    :28406 sbTrayForm->Down=false → :28408 NewRecordProcess("MES2173","Enter Tray Form")（頭段）→ :28409 fTrayForm->ShowModal()
//    → 視窗關掉之後的尾段 :28411-28413（本函式）。
//  呼叫端：FileRW/UserDefForm_File.cpp SaveFlow（A02 關窗先跑 golden FormClose cTrayForm.cpp:559；有寫檔就直接跑，S107-1／R85）。
//  :28413 fShowMessage->ShowTrayDeviceDir() 不翻（寫成註解）：golden uShowMessage.cpp:183 只改主畫面 Tray／IC 方向圖的
//    Visible／Top／Left（IniConfig.bShowTrayAndDeviceDir && UserDefForm_File[0].bEnableIndicator），純畫面；移植樹 forms/fShowMessage.h
//    沒有這支（20260927 grep ShowTrayDeviceDir 全樹 .h/.cpp 0 筆），網頁主畫面也沒有這張圖、沒有 tag 送 UserDefForm[0].iTrayDirection
//    —— 要做是主畫面網頁那邊的事（交件列出）。
// ---------------------------------------------------------------------------
const char* W906_Main_sbTrayFormClickTail()
{
    filerw::ELMark("W906_Main_sbTrayFormClickTail");                            // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbTrayFormClickTail", "V912 main.cpp:28411-28413 sbTrayFormClick",
                                           "DoStructUnitConvert、SetWorkParameter"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28411
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28412
    // fShowMessage->ShowTrayDeviceDir();                                       // golden :28413 —— 純畫面、移植樹沒有（見上），不翻

    CloseTailRan("W906_Main_sbTrayFormClickTail",
                 "DoStructUnitConvert + SetWorkParameter (golden V912 main.cpp:28411-28412; :28413 ShowTrayDeviceDir is main-screen display only, not ported)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  HP-3　golden TfMain::sbPlateFormClick（V912 main.cpp:28416-28426）—— 工具選單「Plate Form」鈕：
//    :28418 sbPlateForm->Down=false → :28420 NewRecordProcess("MES2174","Enter Plate Form")（頭段）→ :28421 fHotPlate->ShowModal()
//    → 視窗關掉之後的尾段 :28422-28425（本函式）。
//  呼叫端：產生檔 FileRW/HotPlateForm_File.cpp 的 SaveFlow（A 形狀，不可手改；設定在 tools/formbridge/TfHotPlate.py 的 saveFlowAfter，
//    tools/gen_formbridge.py 的可選鍵）：J.M("closed")（golden spbSaveClick :446 Close()，先跑 golden FormClose cHotPlate.cpp:401 的
//    fHotPlate->ReadFile）或 J.M("saved")（B_SaveSetupFile 一進來就設）→ 本函式；沒跑的原因進 J.Todo（ack.todo）。
//  ⚠ 這兩行改的是入料臂把 IC 放到加熱盤上的路徑參數：GetHotPlateYHalfPos 重算 b6x20HP／b4x11HP／b4x10HP_2x2／b8x16HP_2x2
//    （iYHalf 依 HotPlateForm.YPitch、InArmSuck.iPickRow），LoadAutoCleanData 重讀 Auto Clean 結構（E43 用 Hotplate1 時讀 HotPlate.Data）。
//    運轉中不跑（R86；form.save 本身運轉中也已擋，R87 JsonBridge/FormJson.cpp:119）——golden 運轉中打不開 HotPlate 視窗。
// ---------------------------------------------------------------------------
const char* W906_Main_sbPlateFormClickTail()
{
    filerw::ELMark("W906_Main_sbPlateFormClickTail");                           // A 形狀沒有 filerw session（見本段檔頭），只為與其他頁同形
    if (const char* why = CloseTailRunning("W906_Main_sbPlateFormClickTail", "V912 main.cpp:28422-28425 sbPlateFormClick",
                                           "DoStructUnitConvert、SetWorkParameter、LoadAutoCleanData、GetHotPlateYHalfPos"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28422
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28423
    FileRW_Cleaning_LoadAutoCleanData();                                        //ChungHung 20131120 AutoClean use Hotplate1   // golden :28424 fCleaning->LoadAutoCleanData()（還沒開機會印一行並跳過）
    GetHotPlateYHalfPos();                                                      // golden :28425（InitialOK==false 時 golden 自己第一行 return，見本段檔頭）

    CloseTailRan("W906_Main_sbPlateFormClickTail", InitialOK
                     ? "DoStructUnitConvert + SetWorkParameter + LoadAutoCleanData + GetHotPlateYHalfPos (golden V912 main.cpp:28422-28425; InitialOK=1)"
                     : "DoStructUnitConvert + SetWorkParameter + LoadAutoCleanData + GetHotPlateYHalfPos (golden V912 main.cpp:28422-28425; InitialOK=0 -> GetHotPlateYHalfPos returned at its golden guard, ainarm_SearchPlacePlate.cpp:134)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  TA-6　golden TfMain::sbTrayAssignClick（V912 main.cpp:28428-28438）—— 工具選單「Tray Assignment」鈕：
//    :28430 sbTrayAssign->Down=false → :28432 NewRecordProcess("MES2175","Enter Tray Assignment")（頭段）
//    → :28433 fTrayAssignment->ShowModal() → 視窗關掉之後的尾段 :28434-28437（本函式）。
//  呼叫端：FileRW/TrayForm.cpp SaveFlow（⚠ 檔名 TrayForm.cpp 是 Tray Assignment 頁；A02 關窗先跑 golden FormClose cTrayAssignment.cpp:1325；
//    有寫檔就直接跑，S107-1／R85）。
//  ⚠ ShowBinSel 不是純畫面（同 BS-2）：重算 iBinTray／bUnloadHasBin／iTrayLastBin／BinAssign（出料分 bin 在用：移植樹 acatchtray.cpp:8698、
//    csystem.cpp:14074 讀 iBinTray），START 時 golden 也不會重跑它 —— 改了 Tray 用途沒跑這一行，停機期間與下一批的分 bin 狀態就是舊的。
//    fSortCT->UpForm 只改 SortCT 面板的 Visible／Top／Left／Height（forms/fSortCT.cpp:488 註解）。
// ---------------------------------------------------------------------------
const char* W906_Main_sbTrayAssignClickTail()
{
    filerw::ELMark("W906_Main_sbTrayAssignClickTail");                          // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbTrayAssignClickTail", "V912 main.cpp:28434-28437 sbTrayAssignClick",
                                           "DoStructUnitConvert、SetWorkParameter、ShowBinSel、SortCT UpForm"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28434
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28435
    fShowBinSelect->ShowBinSel();                                               // golden :28436（移植樹 cShowBinSelect.cpp:1187）
    fSortCT->UpForm();                                                          //kevin 20110901 分割fix tray   // golden :28437（移植樹 forms/fSortCT.cpp:488）

    CloseTailRan("W906_Main_sbTrayAssignClickTail",
                 "DoStructUnitConvert + SetWorkParameter + ShowBinSel + fSortCT->UpForm (golden V912 main.cpp:28434-28437)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  YM-5　golden TfMain::sbYieldClick（V912 main.cpp:28500-28513）—— 工具選單「Yield」鈕：
//    :28502 sbYield->Down=false → :28504-28506 Insufficient(39) 開窗閘（不在這裡）→ :28508 NewRecordProcess("MES2178","Enter Yield Monitoring")
//    （頭段）→ :28509 fYieldMonitoring->ShowModal() → 視窗關掉之後的尾段 :28510-28512（本函式）。
//  呼叫端：FileRW/TestIF_File_YieldMonitoring.cpp SaveFlow（A02 關窗先跑 golden FormClose uYieldMonitoring.cpp:3288；有寫檔就直接跑，
//    S107-1／R85）。golden 另一個關窗的鈕 btnOkClick（uYieldMonitoring.cpp:3190，ReadFile＋主畫面 Yield 狀態＋Close）是盤點 YM-3，不在本項。
//  ⚠ SetStartModeData 會寫檔（本段檔頭）：golden 一次開關記一筆 MES2107／ChangeLog，網頁每存一次記一筆。
// ---------------------------------------------------------------------------
const char* W906_Main_sbYieldClickTail()
{
    filerw::ELMark("W906_Main_sbYieldClickTail");                               // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbYieldClickTail", "V912 main.cpp:28510-28512 sbYieldClick",
                                           "DoStructUnitConvert、SetWorkParameter、SetStartModeData"))
        return why;

    DoStructUnitConvert();                                                      // golden main.cpp:28510
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden :28511
    fMain->SetStartModeData();                                                  // golden :28512 SetStartModeData()（TfMain 成員；移植樹本體 RunStartMode.cpp:1298 W906-SSMD）

    CloseTailRan("W906_Main_sbYieldClickTail",
                 "DoStructUnitConvert + SetWorkParameter + SetStartModeData (golden V912 main.cpp:28510-28512)");
    return nullptr;
}

// ---------------------------------------------------------------------------
//  CC-E15　golden TfMain::sbConfigurationClick（V912 main.cpp:28599-28658）—— 工具選單「Configuration」鈕：
//    :28601 sbConfiguration->Down=false → :28603-28607 CC_SCS 時 Insufficient(30)（客戶專屬開窗閘，不在這裡）
//    → :28609-28613 五個函式內 static → **:28615 bBackupState、:28618-28622 五個舊值（開窗前記，W906_Main_sbConfigurationClickHead）**
//    → :28616 NewRecordProcess("MES2185","Enter Configuration")（頭段，R101＝A／S165，另外派工）→ :28624 fConfiguration->ShowModal()
//    → 視窗關掉之後的尾段 :28626-28657（W906_Main_sbConfigurationClickTail）。
//  呼叫端：FileRW/IniConfig.cpp —— 開頁 IniConfigPageJson 在 IC_FormShow（golden FormShow，ShowModal 裡面）之前記舊值；存檔
//    FileRW_IniConfig_Save 在 IC_FormClose（golden FormClose＝網頁的存檔）與「沒存成就重讀」之後跑尾段，**不看有沒有存成**
//    （golden 關窗一律跑；答 NO 時 IniConfig 仍是檔案值，比對結果＝沒變，條件式的兩段自然不跑，同 golden）。
//    密碼守衛整次拒存（IniConfig.cpp IC_PasswordGuard）與其他 4xx 在 FormClose 之前 return ＝ golden 視窗還開著 → 不跑。
//  golden 的 static bool bRefreshStartMode0..4 每次開窗前都重新指派，實際上就是「開窗前的值」；這裡放在本檔的匿名 namespace。
//  :28626-28632 CC_AMKOR_Korea（IniConfig.bQAMode＝bA78EnableQAMode、主畫面 sbQAMode->Visible、A78 變了 SetStartModeData）：
//    客戶專屬，RULINGS_20260925 S25「先暫時跳過，註記就好」→ GATE，CUSTOMER_CODE 相符時記 ELTodo（同 S88／S100 對 AmkorSendMessage 的寫法）。
// ---------------------------------------------------------------------------
namespace {
bool bBackupState=false;                                                        // golden :28615 區域變數（開窗前的 IniConfig.bE43AutoCleanUseHotplate）
bool bRefreshStartMode0=false;                                                  // golden :28609 static
bool bRefreshStartMode1=false;                                                  // golden :28610 static
bool bRefreshStartMode2=false;                                                  // golden :28611 static
bool bRefreshStartMode3=false;                                                  //Steven 20220919 : 針對Auto Site Map開關整理Start Mode   // golden :28612 static
bool bRefreshStartMode4=false;                                                  //AI(ht9045-config) 20260806 (RogerYang) : #R260723-ATK-H9-01   // golden :28613 static
bool s_bConfigurationHeadRan=false;                                             // [W906] 保險：尾段之前一定開過頁（IniConfig.cpp g_formShown 閘），沒開過就當「都沒變」
}  // namespace

void W906_Main_sbConfigurationClickHead()
{
    bBackupState=IniConfig.bE43AutoCleanUseHotplate;                            // golden main.cpp:28615

    bRefreshStartMode0=IniConfig.bA10_AutoReTest;                               // golden :28618
    bRefreshStartMode1=IniConfig.bI37_EnableFIFOMode;                           // golden :28619
    bRefreshStartMode2=IniConfig.bA51EnableEQCMode;                             //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode   // golden :28620
    bRefreshStartMode3=IniConfig.bI21EnableASM;                                 //Steven 20220919 : 針對Auto Site Map開關整理Start Mode   // golden :28621
    bRefreshStartMode4=IniConfig.bA78EnableQAMode;                              //AI(ht9045-config) 20260806 (RogerYang) : #R260723-ATK-H9-01   // golden :28622
    s_bConfigurationHeadRan=true;
    std::printf("close-tail W906_Main_sbConfigurationClickHead -> recorded E43=%d A10=%d I37=%d A51=%d I21=%d A78=%d (golden V912 main.cpp:28615-28622, before ShowModal)\n",
                (int)bBackupState, (int)bRefreshStartMode0, (int)bRefreshStartMode1, (int)bRefreshStartMode2,
                (int)bRefreshStartMode3, (int)bRefreshStartMode4);
}

const char* W906_Main_sbConfigurationClickTail()
{
    filerw::ELMark("W906_Main_sbConfigurationClickTail");                       // FileRW：存檔流程 trace（ack.session.trace）
    if (const char* why = CloseTailRunning("W906_Main_sbConfigurationClickTail", "V912 main.cpp:28626-28657 sbConfigurationClick",
                                           "SetWorkParameter、UpdateMainOperateMode、Yield ReadFile、SCKART AccessFile、LoadAutoCleanData／SetStartModeData（有變才跑）"))
        return why;
    if (!s_bConfigurationHeadRan) {                                             // [W906] 不會發生（見上）；發生了就用現值＝都沒變
        filerw::ELTodo("W906_Main_sbConfigurationClickTail: the page-open head (golden main.cpp:28615-28622) never ran -- old values taken as current, the E43 / start-mode reruns are skipped");
        W906_Main_sbConfigurationClickHead();
    }

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           // golden :28626 第一道條件；其他客戶 golden 什麼都不做
        filerw::ELTodo(bRefreshStartMode4 != IniConfig.bA78EnableQAMode
                       ? "golden main.cpp:28626-28632 CC_AMKOR_Korea (IniConfig.bQAMode=bA78EnableQAMode, sbQAMode->Visible, SetStartModeData because A78 changed) -- customer-specific, skipped (RULINGS_20260925 S25)"
                       : "golden main.cpp:28626-28632 CC_AMKOR_Korea (IniConfig.bQAMode=bA78EnableQAMode, sbQAMode->Visible; A78 unchanged) -- customer-specific, skipped (RULINGS_20260925 S25)");
#if 0 // GATE(W906-FRW-S158-AMKORKR) golden main.cpp:28626-28632 -- CC_AMKOR_Korea 客戶專屬（RULINGS_20260925 S25 先跳過、註記）；sbQAMode 是主畫面按鈕（網頁主畫面沒有）。VERBATIM
    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //AI(ht9045-config) 20260806 (RogerYang) : #R260723-ATK-H9-01 A77 有變才重建下拉,避免無謂重跑 SetRunStartMode
    {
        IniConfig.bQAMode = IniConfig.bA78EnableQAMode;      //總閘與面板一致(未變則 no-op)
        sbQAMode->Visible = IniConfig.bQAMode;               //按鈕即時同步(idempotent)
        if(bRefreshStartMode4 != IniConfig.bA78EnableQAMode) //只有真的切換才重建
            SetStartModeData();
    }
#endif // GATE(W906-FRW-S158-AMKORKR)

    SetWorkParameter();                                                         // golden :28634
    fMain->UpdateMainOperateMode();                                             // golden :28635（真本體 forms/fMain_OperateMode.cpp:139：lastdata.dat、加熱器繼電器、ATC7）

    fYieldMonitoring->ReadFile();                                               // golden :28637（移植樹 uYieldMonitoring.cpp:2676）
    fSCKART->AccessFile(true);                                                  //Steven 20161025 (wei) : SCK ART function   // golden :28638（forms/fSCKART.cpp:151）

    //ChungHung 20130910 alter for SCK can close site by Index
    fMain->ShowTestHeadComp(false);                                             //ChungHung 20130910 alter for SCK can close site by Index   // golden :28641（移植樹 forms/fMain.cpp 空殼）
    if(IniConfig.bA09_ByArmCloseSite)                                           // golden :28642
    {
        fTestCategory->ShowTestCategory(0);
        fTestCategory->ShowTestCategory(1);
    }

    const bool bE43Changed = bBackupState!=IniConfig.bE43AutoCleanUseHotplate;
    if(bE43Changed)                                                             // golden :28648 if(bBackupState!=IniConfig.bE43AutoCleanUseHotplate)
    {
        FileRW_Cleaning_LoadAutoCleanData();                                    // golden :28650 fCleaning->LoadAutoCleanData()
    }

    const bool bStartModeChanged =
       ((USE_AUTO_RETEST==eartInstall  && bRefreshStartMode0!=IniConfig.bA10_AutoReTest)     ||
        (CosFunction.bHaveFIFOMode     && bRefreshStartMode1!=IniConfig.bI37_EnableFIFOMode) ||                         //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
        (CosFunction.bCanDisableQAMode && bRefreshStartMode2!=IniConfig.bA51EnableEQCMode)   ||                         //Steven 20170209 : 修正開關ART時候,要重新整理Start Mode
        (IniConfig.bUseAutoSiteMapping && bRefreshStartMode3!=IniConfig.bI21EnableASM));                                //Steven 20220919 : 針對Auto Site Map開關整理Start Mode
    if(bStartModeChanged)                                                       // golden :28653-28656
        fMain->SetStartModeData();                                              // golden :28657 SetStartModeData()（TfMain 成員；RunStartMode.cpp:1298）

    const std::string ran = std::string("SetWorkParameter + UpdateMainOperateMode + fYieldMonitoring->ReadFile + fSCKART->AccessFile(true)") +
                            (bE43Changed ? " + LoadAutoCleanData (E43 changed)" : " (E43 unchanged: no LoadAutoCleanData)") +
                            (bStartModeChanged ? " + SetStartModeData (A10/I37/A51/I21 changed)" : " (start-mode keys unchanged: no SetStartModeData)") +
                            " (golden V912 main.cpp:28634-28657)";
    CloseTailRan("W906_Main_sbConfigurationClickTail", ran.c_str());
    return nullptr;
}
