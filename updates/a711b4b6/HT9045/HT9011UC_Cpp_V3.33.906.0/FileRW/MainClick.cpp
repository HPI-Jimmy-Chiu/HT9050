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
//    （closed）」與「真的寫了檔（SaveSetupFile）」之後跑本函式（同 FileRW/Temperature.cpp S88 MainTempOffsetTail 的做法，時機待 Steven）。
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
    fMain->UpdateMainOperateMode();                                             // golden :28478（移植樹門面計數 stub，forms/fMain.cpp）

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
