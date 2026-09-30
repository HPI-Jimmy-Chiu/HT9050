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
static AnsiString s_asPEModelCaption = "PE Model OFF";  static int s_iB10APEModelVisible = -1;   //AI(W906-EVB10A) 20260929 [W906]: CC-E10 golden TfConfiguration::sbExitClick（cConfiguration.cpp:6386）`fMain->sbPEModel->Visible=cbC12->Checked;` 設的值（-1＝還沒設過，照下面 FormShow 的式子；本檔檔尾 W906_Main_SetPEModelVisible）；接在同一行

// golden main.cpp:11261-11262（TfMain::FormShow）：sbPEModel->Visible 的條件。按鈕看不見，使用者就按不到。
static bool W906_Main_PEModelVisible()
{
    if (s_iB10APEModelVisible >= 0) { return s_iB10APEModelVisible != 0; }  return (CosFunction.bUsePEModelFunction &&                                  //Ifor 20160825 add PE 工程模式 By CosFunction   //AI(W906-EVB10A) 20260929 [W906]: golden sbPEModel->Visible 最後一次被指派的值：開機 FormShow（:11261，這個式子）之後只有 Configuration 的 Exit／Save（sbExitClick :6386＝cbC12->Checked，不看 CosFunction）會改；接在同一行
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
    { void W906_EvB6_Hp3CaptureBegin(); void W906_EvB6_Hp3CaptureEnd(); W906_EvB6_Hp3CaptureBegin(); FileRW_Cleaning_LoadAutoCleanData(); W906_EvB6_Hp3CaptureEnd(); }   //ChungHung 20131120 AutoClean use Hotplate1   // golden :28424 fCleaning->LoadAutoCleanData()（還沒開機會印一行並跳過）  AI(W906-EVB6) 20260928 [W906]: R107 前後收下 golden 的 ShowMyMessage（ELMessage），交給存檔回覆（本檔檔尾 R107 段）；接在同一行，不移動行號
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

// ===========================================================================
//  AI(W906-EVB6) 20260928 [W906]: 批次 B6＋B9 —— 主畫面（golden TfMain）的圖示、按鈕、子頁事件
//    派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B6／B9、第四節 M-2～M-20、R107。
//    裁決：RULINGS_20260926 S167「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」；
//      Q47（Run Mode 圖示）、Q48（溫度圖示）「我們做」；S169（主畫面 Site 格＋控制鈕）「c++部分只需要做到事件觸發,
//      例如按了home, 只是 bNeedHome=true, 實際動作可以先列表, 後面通知Jimmy進行接上」。
//    golden 一律 V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950；本段行號 20260928 逐一重讀過）。
//    WS 入口：tools/wb_serve.cpp St01 那一行（act.main.peModel 同一行）的一個分支 → W906_Main_EvB6Op（本段最後）；
//      頁面 D:\HT9045\web\page\ht9045_main_st01_ev.js。
//
//  每支都自己持 FormLock（同本檔上面三支）；回傳 JSON {op, executed, guard, detail, golden, ...}。*ok＝「有照 golden 跑完
//    （含 golden 自己 return 的那幾條）或是 get」；參數錯、golden 按不到（Enabled／Visible）＝ false（頁面顯示原因）。
//
//  ⚠ 會碰到的機台狀態／檔案（逐支列在各段的註解裡；這裡是總表）：
//    M-2 Run Mode：LastSet.iRealDummy、MMPlate1／2 的 Tray 資料與 iPickPlate 等指標（非 REALLY 時，golden 先清再檢查）；
//        CosFunction.bLastSetInSetUpFile 時 <配方>\TestMode.Data（SaveTestMode）；fMain->UpdateMainOperateMode 真本體
//        （D:\HT9045\system\lastdata.dat、加熱器繼電器、ATC7 指令，forms/fMain_OperateMode.cpp:139）；事件紀錄 MES2148／2149／2150。
//    M-3 溫度：fMain->ChangeTempMode(10,…) 真本體（Jimmy 的 MainTempMode.cpp：Temperature.iMachineTempMode、LastSet.iTemperature、
//        TestMode.Data、ATC7 指令、fTemp_Set->ReadTempFile）。
//    M-8 Light：SW[SwCCDLight].On()／Off() —— 實體 DO（CCD 腔體燈；燈，不是互鎖、不會動機構）。
//    M-15 Index 扭力：COM2->WriteIndexTorqueSetting —— 透過 RS232 寫 Index 伺服驅動器的扭力設定（實體設備），並把
//        fAllMotorHome 設成 false（golden：寫完要重新回原點）。只在機台停著時（golden SystemStart 守衛）。
//    M-17：MOT[i].bCheckEncoderEveryTime（記憶體）。M-9／M-10／M-16：記憶體。
//    B9（M-6／M-7）：只登記事件（S169），不動機台；CLEAN OUT 例外（已翻好：cCleanOut.cpp:64，照 golden 接上）。
// ===========================================================================
#include "Motor/mymotor.h"      // AI(W906-EVB6) 20260928 [W906]: MOT[]（InitEmptyTray、Motor->Enable／PServoAlarmOn、bCheckEncoderEveryTime）
#include "myswitch.h"           // AI(W906-EVB6) 20260928 [W906]: SW[]（SwCCDLight）
#include "LastSet.h"            // AI(W906-EVB6) 20260928 [W906]: LastSet.iRealDummy／bBigFan／iTemperature（FileRW/MainClose.cpp:101 同樣 include）
#include "atester_shims.h"      // AI(W906-EVB6) 20260928 [W906]: COM2（TCOM2Shim：bCCDDummyRum、Read/WriteIndexTorqueSetting，本體 rs232.cpp:322／:334）
#include <cstdlib>              // AI(W906-EVB6) 20260928 [W906]: std::strtol（M-15）
#include <chrono>               // AI(W906-EVB6) 20260928 [W906]: B9 事件的存活時間（steady_clock）
#include <deque>                // AI(W906-EVB6) 20260928 [W906]: B9 事件佇列

// AI(W906-EVB6) 20260928 [W906]: 只宣告要用的（全域範圍；理由同本檔 :67-78 —— aHotPlateSubstrate.h／csystem.h／cAuthority.h／canary_support.h
//   會帶進兩個 TMyKitSuck〔陷阱 #3〕或與 cMyDB.h 的 RecordProcess 預設引數重複宣告）。不帶預設引數。
extern int  iPickPlate[2],  iPickPlateX[2],  iPickPlateY[2];                     // aHotPlateSubstrate.h:775（golden ainarm2.h:62）
extern int  iPlacePlate[2], iPlacePlateX[2], iPlacePlateY[2];                    // aHotPlateSubstrate.h:776（golden ainarm2.h:63）
extern bool authMainForm[12];                                                   // cAuthority.h:56（本體 cAuthority.cpp:119；golden cAuthority.h）
extern bool bAuthCriticalPara[26];                                              // cAuthority.h:64
bool HasAnyICInMachine();                                                       // csystem.h:109（本體 csystem.cpp:13373）
int  Barcode_Reader(int Barcode);                                               // BarcodeReader.h:111（本體 BarcodeReader.cpp:445；同 FileRW/MainClose.cpp:134）
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr, AnsiString errPart);   // canary_support.h:66（golden note.h:466；wb_serve 裡是網頁的阻塞框）
int  ShowMyMessageBox_YES_NO(AnsiString S1, AnsiString S2, AnsiString S3);     // canary_support.h:209（golden mymessbox.h:55）
namespace filerw { void SessionBegin(const std::string& answersJson); std::string SessionJson(); }   // FileRW/_EditList.h:147-148（R107；本檔不 include 它，理由同 :74）

namespace {

// ---- 小工具（本段共用）--------------------------------------------------------
struct EvB6Payload {
    cJSON* root = nullptr;
    bool   bad  = false;
    explicit EvB6Payload(const std::string& s) {
        root = cJSON_Parse(s.empty() ? "{}" : s.c_str());
        bad = (root == nullptr || !cJSON_IsObject(root));
    }
    ~EvB6Payload() { if (root) cJSON_Delete(root); }
    EvB6Payload(const EvB6Payload&) = delete;
    EvB6Payload& operator=(const EvB6Payload&) = delete;
    std::string Str(const char* k, const char* dflt) const {
        const cJSON* j = root ? cJSON_GetObjectItemCaseSensitive(root, k) : nullptr;
        return (j && cJSON_IsString(j) && j->valuestring) ? std::string(j->valuestring) : std::string(dflt);
    }
    bool Int(const char* k, int* v) const {
        const cJSON* j = root ? cJSON_GetObjectItemCaseSensitive(root, k) : nullptr;
        if (!(j && cJSON_IsNumber(j))) return false;
        *v = j->valueint;
        return true;
    }
    bool Bool(const char* k, bool* v) const {
        const cJSON* j = root ? cJSON_GetObjectItemCaseSensitive(root, k) : nullptr;
        if (!(j && cJSON_IsBool(j))) return false;
        *v = cJSON_IsTrue(j) != 0;
        return true;
    }
};

std::string EvB6Refuse(const char* guard, const std::string& detail, const char* golden)
{
    webbridge::JsonWriter w;
    w.BeginObject().Key("executed").Bool(false).Key("guard").String(guard).Key("detail").String(detail);
    if (golden) w.Key("golden").String(golden);
    w.EndObject();
    return w.Str();
}

void EvB6Todos(webbridge::JsonWriter& w, const std::vector<std::string>& todo)
{
    w.Key("todo").BeginArray();
    for (std::size_t i = 0; i < todo.size(); ++i) w.String(todo[i]);
    w.EndArray();
}

const char* RunModeName(int m)                                                  // golden NewRecordProcess 的字樣（main.cpp:29862-29867）
{
    if (m == REALLY) return "REALLY";
    if (m == DUMMY)  return "DUMMY";
    return "TRAY ONLY";
}
AnsiString RunModeCaption(int m)                                                // golden TfMain::LoadRunModePicture（main.cpp:12903-12921）的 palRunMode_1->Caption
{
    if (m == DUMMY)    return "No Device/ No Tray";
    if (m == HAS_TRAY) return "No Device";
    return "Normal";
}

// golden 的 Amkor／ATK UDP 訊息（fAutomation->AmkorSendMessage，golden automation.cpp:2302）移植樹沒有（同 S100／S88）
void EvB6AmkorTodo(std::vector<std::string>* todo, const char* where)
{
    if (CosFunction.bAmkorFunction || CUSTOMER_CODE == CC_QUALCOMM)            // golden AmkorSendMessage 第一道條件；其他客戶 golden 什麼都不送
        todo->push_back(std::string(where) + " fAutomation->AmkorSendMessage (ATK site-map UDP, bAmkorFunction/CC_QUALCOMM) -- not ported");
}

}  // namespace

// ===========================================================================
//  M-2　golden TfMain::imgRunModeClick（main.cpp:29796-29805）→ TfMain::RunICModeChange（:29807-29871）—— 主畫面 Run Mode 圖示 ▣
//    按得到：golden ChangeLevelAttr :13061 imgRunMode->Enabled=authMainForm[8]（Security_new.def [Main]，cAuthority.cpp:460）。
//    按下：:29798 SystemStart return；:29801 Insufficient(11)（權限不足時 golden 跳 WAR1676）；→ RunICModeChange()。
//  ⚠ golden 的順序（照翻，不修）：非 REALLY（Dummy／Tray Only）時，**先**清兩個加熱盤的 Tray 資料與取放指標（:29809-29835），
//    **後**才檢查機台內有沒有 IC（:29837）—— 所以 Dummy 模式下加熱盤上的「假料」一定會被清掉，就算最後因為別處有 IC 而沒換成。
//    這是 golden 的設計（Dummy 模式的加熱盤資料是模擬的），不是本檔發明。
//  D-005 T15（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D005_TFMAIN_TESTMODE_WRITERS_20260927.md）：本支是 TestMode.Data 的寫入者之一。
// ===========================================================================
// 回 0＝golden 某一道 return（*stop 說是哪一道）；1＝換好了。
static int W906_Main_RunICModeChange(bool bDefineMode, int iMode, const char** stop)
{
    if(LastSet.iRealDummy!=REALLY)
    {
        MOT[MMPlate1].InitEmptyTray("RunICModeChange");                         // golden __FUNC__
        MOT[MMPlate2].InitEmptyTray("RunICModeChange");
        if(HotPlateForm.iPlateSelect==1)                                        //Hotplate 1
        {
            iPickPlate[0]=1;
            iPickPlate[1]=1;
            iPlacePlate[0]=1;
            iPlacePlate[1]=1;
        }
        else                                                                    //if(HotPlateForm.iPlateSelect==2)                                 //Hotplate 2(Only) && Hotplate 1&2
        {
            iPickPlate[0]=0;
            iPickPlate[1]=0;
            iPlacePlate[0]=0;
            iPlacePlate[1]=0;
        }
        iPickPlateX[0]=0;
        iPickPlateY[0]=0;
        iPlacePlateX[0]=0;
        iPlacePlateY[0]=0;
        iPickPlateX[1]=0;
        iPickPlateY[1]=0;
        iPlacePlateX[1]=0;
        iPlacePlateY[1]=0;
    }

    if(fMain->CheckCanChangeRealDummy()==false ||                               //Steven 20240719 : && --> ||   （移植樹 cMainStatus.cpp:323）
       HasICUnderMachine()==true ||
       HasAnyICInMachine()==true)                                               //Steven 20240719 : 機台內有IC不可以換模式
    {
        ShowErrorMessage("MES1646", 0, MMSystem, false, "RunModeClick");        //Must finish [Clean out]!!
        *stop = "ic-in-machine";
        return 0;
    }

    if(bDefineMode)
        LastSet.iRealDummy=iMode;
    else
        LastSet.iRealDummy++;

    if(LastSet.iRealDummy>REALLY)
        LastSet.iRealDummy=DUMMY;

    if(CosFunction.bLastSetInSetUpFile)                                         //Steven 20111019
    {
        TestMode.iRunMode=LastSet.iRealDummy;
        SaveTestMode();
    }

    fMain->UpdateMainOperateMode();                                             // golden :29859（真本體 forms/fMain_OperateMode.cpp:139）
    fMain->LoadRunModePicture();                                                // golden :29860（門面計數；圖片／字樣由頁面照 LoadRunModePicture 的字樣顯示）

    if(LastSet.iRealDummy==REALLY)                                              //Steven 20140815
        NewRecordProcess("MES2148", "VVVV  Run Mode : REALLY  VVVV");
    else if(LastSet.iRealDummy==DUMMY)
        NewRecordProcess("MES2149", "XXXX  Run Mode : DUMMY  XXXX");
    else
        NewRecordProcess("MES2150", "XXXX  Run Mode : TRAY ONLY  XXXX");

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.SwitchRunMode);                                  // 9     切換 Real / Dummy Mode
    return 1;
}

static std::string W906_Main_RunModeOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "get");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    const int before = LastSet.iRealDummy;
    const bool enabled = authMainForm[8];                                       // golden ChangeLevelAttr :13061
    const char* stop = nullptr;
    int rc = -1;
    if (op == "click") {
        if (!enabled)
            return EvB6Refuse("disabled", "golden main.cpp:13061 imgRunMode->Enabled=authMainForm[8] is false (Security_new.def [Main]) -- the operator cannot click it",
                              "V912 main.cpp:29796 imgRunModeClick");
        if(SystemStart)                                                         // golden :29798
            stop = "system-running";
        else if(fSecurity->Insufficient(11)==false)                             //jou 981207 權限控制   // golden :29801（不足時 golden 跳 WAR1676）
            stop = "not-authorized";
        else
            rc = W906_Main_RunICModeChange(false, 0, &stop);                    //Steven 20220616 : IC Run Mode變更   // golden :29804
        std::printf("act.main.runMode click -> %s  iRealDummy %d -> %d\n", stop ? stop : "changed", before, LastSet.iRealDummy);
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:29796-29871 imgRunModeClick -> RunICModeChange");
    if (op == "click") {
        w.Key("executed").Bool(rc == 1);
        w.Key("guard").String(stop ? stop : "");
        if (stop && std::string(stop) == "ic-in-machine")
            w.Key("detail").String("golden :29837-29843 CheckCanChangeRealDummy／HasICUnderMachine／HasAnyICInMachine：機台內有 IC，跳 MES1646（Must finish [Clean out]!!）");
        w.Key("wrote").BeginArray();
        if (rc == 1) {
            if (CosFunction.bLastSetInSetUpFile) w.String("<配方>\\TestMode.Data [TestMode] Running Mode（SaveTestMode；InitialOK 才寫）");
            w.String("system\\lastdata.dat（fMain->UpdateMainOperateMode 真本體）");
        }
        w.EndArray();
    }
    w.Key("enabled").Bool(enabled);
    w.Key("before").Number((wb_int64)before);
    w.Key("realDummy").Number((wb_int64)LastSet.iRealDummy);
    w.Key("mode").String(RunModeName(LastSet.iRealDummy));
    w.Key("caption").String(RunModeCaption(LastSet.iRealDummy).c_str());       // golden palRunMode_1->Caption
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-3　golden TfMain::Panel42Click（main.cpp:22392-22449）—— 主畫面溫度圖示 🌡️（DFM imgTempOnOff 的 OnClick，main.dfm:4055）
//    按得到：golden UpdateMainOperateMode :13407／:13434／:13454 imgTempOnOff->Enabled=authMainForm[6]。
//    本體 ChangeTempMode(10,…)＝Hot↔Ambient 切換，是 Jimmy 已翻的 MainTempMode.cpp（不動；D-005 T13）。
//    客戶分支照 golden：HiSilicon（KYEC_LEE 要 PE 模式；其他要 FormHS 獨立密碼 —— 移植樹沒有 TFormHS，當成密碼失敗、不切換）、
//    Barcode_Reader(bcTemperature)（KYEC）、CC_ASE_CL（確認框＋DoPassword —— 移植樹 TfMain 沒有 DoPassword，當成失敗，同
//    JsonBridge/actions/MainTesterConnect.cpp E1 的做法）。fail-safe：客戶專屬的密碼段做不到時「不切換」。
// ===========================================================================
static std::string W906_Main_TempModeOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "get");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    const bool enabled = authMainForm[6];
    const int tmBefore = Temperature.iMachineTempMode, ltBefore = LastSet.iTemperature;
    const char* stop = nullptr;
    int changeRet = -1;
    std::vector<std::string> todo;
    if (op == "click") {
        if (!enabled)
            return EvB6Refuse("disabled", "golden main.cpp:13407/13434/13454 imgTempOnOff->Enabled=authMainForm[6] is false -- the operator cannot click it",
                              "V912 main.cpp:22392 Panel42Click");
        int ret=0;
        do {
            if(fSecurity->Insufficient(7)==false)                               //jou 981207 權限控制   // golden :22395
                { stop = "not-authorized"; break; }

            if(CosFunction.bHiSiliconFunction==true)                            //Ifor 20170905 :海思專版可使用獨立密碼修改溫度
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE)                                  //Ifor 20170905 :京元海思專版使用PE模式修改溫度
                {
                    if(bEnablePEModel==false)
                        { stop = "pe-model-off"; break; }
                }
                else
                {
#if 0 // GATE(W906-EVB6-HS) golden main.cpp:22409 -- TFormHS（HS_Function.h）移植樹沒有（SCK_ART_Remainder.h:466 記過）。VERBATIM
                    if(FormHS->CheckIndependentPassWord())
                    {
                        return;
                    }
#endif
                    stop = "independent-password-not-ported";                   // [W906] fail-safe：做不到獨立密碼 ＝ 不切換
                    break;
                }
            }

            if(Barcode_Reader(bcTemperature)==0)                                // 20140103 wei KYEC Barcode Reader   // golden :22416
                { stop = "barcode-reader"; break; }

            if(CUSTOMER_CODE==CC_ASE_CL)                                        //JerryYang 20250120 : add
            {
                ret=ShowMyMessageBox_YES_NO("Sure to change the hot mode?", "確定要切換加熱模式?", "");                                      //JerryYang 20250120 : add
                if(ret==2)
                    { stop = "cancelled"; break; }
                else
                {
#if 0 // GATE(W906-EVB6-PW) golden main.cpp:22430 -- DoPassword() 不是 TfMain 門面的成員（同 MainTesterConnect.cpp E1、FileRW/TestIF_File.cpp:2576）。VERBATIM
                    if(DoPassword()==false)                                     //Steven 20101124
                    {
                        return;
                    }
#endif
                    stop = "password-not-ported";                               // [W906] fail-safe：沒有密碼框 ＝ 密碼失敗
                    break;
                }
            }

            changeRet = fMain->ChangeTempMode(10, true, bRefreshFunction);      // golden :22437（本體 MainTempMode.cpp，Jimmy）
            EvB6AmkorTodo(&todo, "golden main.cpp:22438");                      // golden :22438 fAutomation->AmkorSendMessage(1)
            fMain->ShowTestHeadComp(false);                                     //ChungHung 20130910 alter for SCK can close site by Index   // golden :22439（移植樹空殼）

            if(IniConfig.bA09_ByArmCloseSite)                                   //ChungHung 20130910 alter for SCK can close site by Index
            {
                fTestCategory->ShowTestCategory(0);
                fTestCategory->ShowTestCategory(1);
            }

            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.SwitchTemperature);
        } while (false);
        std::printf("act.main.tempMode click -> %s  ChangeTempMode=%d  iMachineTempMode %d -> %d  LastSet.iTemperature %d -> %d\n",
                    stop ? stop : "ran", changeRet, tmBefore, Temperature.iMachineTempMode, ltBefore, LastSet.iTemperature);
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:22392-22449 Panel42Click -> ChangeTempMode(10) (MainTempMode.cpp)");
    if (op == "click") {
        w.Key("executed").Bool(stop == nullptr);
        w.Key("guard").String(stop ? stop : "");
        if (stop == nullptr) w.Key("changeTempModeReturn").Number((wb_int64)changeRet);   // ChangeTempMode 自己 return 1 的時候（運轉中、機台有 IC…）golden 也不告訴按鈕
        EvB6Todos(w, todo);
    }
    w.Key("enabled").Bool(enabled);
    w.Key("machineTempModeBefore").Number((wb_int64)tmBefore);
    w.Key("machineTempMode").Number((wb_int64)Temperature.iMachineTempMode);  // 0=Hot 1=Ambient 2=ATC 3=AmbientHot（tag temp.mode 同一個值）
    w.Key("lastSetTemperature").Number((wb_int64)LastSet.iTemperature);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-8　golden TfMain::spbLightClick（main.cpp:26780-26811）＋ TfMain::LightOn（:26772-26778）—— 主畫面 Light 鈕
//  ⚠ spbLight->Tag／dtLightOnTime 在 golden 是 TfMain 的一份狀態，spbLightClick 與 csystem.cpp ProcessCCDLight（自動關燈）共用。
//    移植樹 ProcessCCDLight 用的是 csystem.cpp:13096-13098 的檔內 static（W7G5_spbLight_Tag／W7G5_dtLightOnTime），外面碰不到，
//    而且 ProcessCCDLight 目前沒有呼叫端（golden 在 Timer2Timer 呼叫）⇒ 這裡另外一份（s_iSpbLightTag），兩份在 Jimmy 接上
//    Timer2Timer 之前不會互相影響；接上時要合成一份（交件清單）。
//  ⚠ SW[SwCCDLight].On()／Off() 是實體 DO（TMySwitch::On 先記 OutValue 再看 Enable 才真的輸出，myswitch.cpp:74）：
//    CCD 腔體照明燈。不是互鎖、不會讓機構動。畫面的 Light 字樣由 tag light.off（SW[SwCCDLight].OutValue）顯示。
// ===========================================================================
namespace {
int        s_iSpbLightTag = 0;                                                  // golden spbLight->Tag（main.dfm:2351 沒設 ⇒ 0）
AnsiString s_asSpbLightCaption = "Light OFF";                                   // golden main.dfm:2356 Caption = 'Light OFF'
TDateTime  s_dtLightOnTime = 0;                                                 // golden main.h:1222 dtLightOnTime（FormShow :10658 也設 Now()）
}  // namespace

static void W906_Main_LightOn()                                                 // golden TfMain::LightOn :26772
{
    s_iSpbLightTag=1;                                                           // golden: spbLight->Tag=1;
    s_asSpbLightCaption="Light ON";                                             // golden: spbLight->Caption="Light ON";
    SW[SwCCDLight].On();
    s_dtLightOnTime=Now();                                                      // golden: dtLightOnTime=Now();
}

// 回 true＝跑完；false＝golden 的 return（CC_ASE_CL 運轉中不能開燈）。
static bool W906_Main_spbLightClick()
{
    if(s_iSpbLightTag==0)                                                       // golden: spbLight->Tag==0
    {
        if(REAL_TIME_CCD==true && COM2->bCCDDummyRum==false)                    //Steven 20110907 : RTC燈光控制
        {
            if(SystemStart==false)
            {
                W906_Main_LightOn();
            }
            else
            {
                SW[SwCCDLight].Off();
                s_iSpbLightTag=0;
                s_asSpbLightCaption="Light OFF";
            }
        }
        else
        {
            if(CUSTOMER_CODE==CC_ASE_CL && SystemStart==true)                   //KaiHuang 20200728 : For ASE-CL A168 產品不能開燈測試
                return false;

            W906_Main_LightOn();
        }
    }
    else
    {
        SW[SwCCDLight].Off();
        s_iSpbLightTag=0;
        s_asSpbLightCaption="Light OFF";
    }
    return true;
}

static std::string W906_Main_LightOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "get");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    bool ran = false;
    if (op == "click") {
        if (REAL_TIME_CCD==true && COM2 == nullptr)                             // [W906] golden :26784 只在 REAL_TIME_CCD 時讀 COM2（短路）；移植樹沒建 COM2 就不能照 golden 判斷
            return EvB6Refuse("forms-not-created", "COM2 (TCOM2Shim) is null -- golden :26784 reads COM2->bCCDDummyRum when REAL_TIME_CCD", "V912 main.cpp:26780 spbLightClick");
        ran = W906_Main_spbLightClick();
        std::printf("act.main.light click -> %s  Tag=%d  SW[SwCCDLight].OutValue=%d Enable=%d\n",
                    ran ? s_asSpbLightCaption.c_str() : "ASE-CL running: golden return", s_iSpbLightTag,
                    (int)SW[SwCCDLight].OutValue, (int)SW[SwCCDLight].Enable);
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:26780-26811 spbLightClick (LightOn :26772)");
    if (op == "click") {
        w.Key("executed").Bool(ran);
        w.Key("guard").String(ran ? "" : "ase-cl-running");
        if (!ran) w.Key("detail").String("golden :26799-26800 CC_ASE_CL && SystemStart：A168 產品不能開燈測試");
    }
    w.Key("tag").Number((wb_int64)s_iSpbLightTag);
    w.Key("caption").String(s_asSpbLightCaption.c_str());
    w.Key("outValue").Bool(SW[SwCCDLight].OutValue != 0);                       // 下命令的狀態（tag light.off 讀同一個）
    w.Key("ioEnable").Bool(SW[SwCCDLight].Enable);                              // false ＝ IO 表沒載入，沒有真的輸出
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-9　golden TfMain::spbFanClick（main.cpp:26763-26770）—— 主畫面 FAN 鈕：LastSet.bBigFan 反相、字樣跟著換。
//    看得到：golden Timer2Timer :21651-21658 spbFan->Visible=(AccessLevel>=LevelSet.AccessLevel[105])（每一拍重算）。
//    ⚠ 真正開關風扇的是 golden Timer2Timer :21665 SW[SwBigFan].OnOff(LastSet.bBigFan) —— 不在按鈕裡，移植樹的 Timer2 也還沒有
//      這一行（全樹 SwBigFan 只有 cinitial.cpp:1196 的名字）⇒ 按了只改 LastSet.bBigFan（golden 同），風扇要等 Jimmy 接 Timer2（交件清單）。
//    字樣：golden FormShow :10621-10624 與本支同一條規則（bBigFan ⇒ "FAN OFF"，否則 "FAN ON"）。
// ===========================================================================
static AnsiString FanCaption() { return LastSet.bBigFan ? "FAN OFF" : "FAN ON"; }   // golden :26766-26769／:10621-10624

static std::string W906_Main_FanOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "get");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    const bool visible = (AccessLevel>=LevelSet.AccessLevel[105]);             // golden Timer2Timer :21651-21658
    if (op == "click") {
        if (!visible)
            return EvB6Refuse("hidden", "golden main.cpp:21651-21658: spbFan->Visible=(AccessLevel>=LevelSet.AccessLevel[105]) is false -- the operator cannot press it",
                              "V912 main.cpp:26763 spbFanClick");
        LastSet.bBigFan=!LastSet.bBigFan;                                       // golden :26765
        std::printf("act.main.fan click -> LastSet.bBigFan=%d (%s); SW[SwBigFan] is driven by golden Timer2Timer :21665, not ported\n",
                    (int)LastSet.bBigFan, FanCaption().c_str());
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:26763-26770 spbFanClick");
    if (op == "click") {
        w.Key("executed").Bool(true);
        std::vector<std::string> todo;
        todo.push_back("golden main.cpp:21665 Timer2Timer SW[SwBigFan].OnOff(LastSet.bBigFan) is not ported -- the fan output does not follow yet (Jimmy)");
        EvB6Todos(w, todo);
    }
    w.Key("visible").Bool(visible);
    w.Key("bigFan").Bool(LastSet.bBigFan);
    w.Key("caption").String(FanCaption().c_str());
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-10　golden TfMain::btnViewClick（main.cpp:8689-8701）—— 右邊功能狀態清單的收合／展開鈕
//    bShowFunc：golden TfMain 建構 :1506 設 true；本支反相；:8692-8699 換圖示（純畫面）；:8700 MainFormSizeToEpson(true)
//    重排版面（ShowFunctions :9121-9152：bShowFunc 時每一項寬 200，否則 50）—— 版面由頁面照這兩個數字做。
//    按得到：golden ChangeMainFormWitdh :35706-35713 btnView->Enabled=!IniConfig.bG23DiasbleFuncStatusView。
// ===========================================================================
namespace { bool s_bShowFunc = true; }                                         // golden main.cpp:1506 bShowFunc=true;

static std::string W906_Main_FuncViewOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "get");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    const bool enabled = !IniConfig.bG23DiasbleFuncStatusView;                  // golden :35706-35713
    if (op == "click") {
        if (!enabled)
            return EvB6Refuse("disabled", "golden main.cpp:35706-35708: IniConfig.bG23DiasbleFuncStatusView -> btnView->Enabled=false",
                              "V912 main.cpp:8689 btnViewClick");
        s_bShowFunc=!s_bShowFunc;                                               // golden :8691
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String("V912 main.cpp:8689-8701 btnViewClick (ShowFunctions :9121-9152)");
    if (op == "click") w.Key("executed").Bool(true);
    w.Key("enabled").Bool(enabled);
    w.Key("showFunc").Bool(s_bShowFunc);
    w.Key("itemWidth").Number((wb_int64)(s_bShowFunc ? 200 : 50));             // golden ShowFunctions :9136-9143
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-16　golden TfMain::palHP2ViewClick（main.cpp:31875-31878）—— Heater View 的「HOTPLATE 2」表頭（後門）：
//    IniConfig.bStartProductOnLine=false（只改記憶體）。
//  ⚠ golden 的怪處（照翻）：V912 全樹沒有任何地方讀 IniConfig.bStartProductOnLine（20260928 grep golden *.cpp「.bStartProductOnLine」：
//    只有本行與 CosFunction.cpp:730／:4149 兩行註解；kevin 20180517 已改用 bI40_bStartProductOnLine）⇒ 這個後門在 V912 按了沒有效果。
//  rgHotplateShowMessage（同一頁的單選）：golden 本體 :35150 整段註解掉 → 不做（派工單 M-16）。
// ===========================================================================
static std::string W906_Main_HP2ViewOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "click");
    if (op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    IniConfig.bStartProductOnLine=false;                                        //kevin 20140412 後門   // golden :31877
    std::printf("act.main.hp2View click -> IniConfig.bStartProductOnLine=false (memory only; golden V912 has no reader of it)\n");
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op).Key("executed").Bool(true);
    w.Key("golden").String("V912 main.cpp:31875-31878 palHP2ViewClick");
    w.Key("startProductOnLine").Bool(IniConfig.bStartProductOnLine);
    w.Key("note").String("golden V912 沒有任何地方讀 IniConfig.bStartProductOnLine（已被 bI40_bStartProductOnLine 取代）：按了沒有效果，golden 同");
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-17　golden TfMain::cbCheckEncoderEveryTimeClick（main.cpp:29548-29560）—— Motor View「Check Encoder Every Time」勾選框
//  ⚠ golden 的怪處（照翻）：不看勾選狀態 —— 勾與取消勾都把「有伺服警報輸入」的馬達設成每次都檢查編碼器，沒有地方把它清回 false
//    （golden FormShow :9099 只把勾選框的 Checked 設回 false，MOT[].bCheckEncoderEveryTime 不動）。要不要改是 Steven／Jimmy 的決定。
//  [W906] MOT[i].Motor 是 NULL 的軸跳過（golden 每一軸都有 Motor；移植樹沒建的軸 deref 會讓 wb_serve 當掉）。
// ===========================================================================
static std::string W906_Main_CheckEncoderOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "click");
    if (op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    int nSet = 0, nNull = 0;
    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        if (MOT[i].Motor == nullptr) { ++nNull; continue; }                     // [W906] 見上
        if(MOT[i].Motor->Enable==true && MOT[i].Motor->PServoAlarmOn==1)
        {
            MOT[i].bCheckEncoderEveryTime=true;
            ++nSet;
        }
    }
    #ifdef ASE_KaohSiung
    RecordProcess("Check Encoder Every Time is selected.");
    #endif
    bool checked = false;
    const bool hasChecked = p.Bool("checked", &checked);
    std::printf("act.main.checkEncoder click (checked=%s) -> %d motor(s) set bCheckEncoderEveryTime=true (%d without Motor)\n",
                hasChecked ? (checked ? "1" : "0") : "?", nSet, nNull);
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op).Key("executed").Bool(true);
    w.Key("golden").String("V912 main.cpp:29548-29560 cbCheckEncoderEveryTimeClick");
    w.Key("motorsSet").Number((wb_int64)nSet);
    w.Key("note").String("golden 不看勾選狀態：勾或取消勾都只會把有伺服警報輸入（PServoAlarmOn）的軸設成每次檢查編碼器，不會清回來");
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-15　golden TfMain::btnSetZ1Click／btnSetZ2Click（main.cpp:22635-22651）、btnReadZ1Click／btnReadZ2Click（:22653-22661）
//    —— Comm View「Torque」分頁的 Set Z1／Z2、Read Z1／Z2。
//    Set：:22637 SystemStart return → COM2->WriteIndexTorqueSetting(z, edtSetZ?->Text)（rs232.cpp:334：Panasonic／Mitsubishi 驅動器，
//      HP 通訊卡那一半在 rs232.cpp 閘著）→ fAllMotorHome=false（寫完要重新回原點）。
//    Read：COM2->ReadIndexTorqueSetting(z)（rs232.cpp:322，golden 沒有 SystemStart 守衛）；讀回的值由收包那一段寫進 edtReadZ?／edTorue?，
//      那幾格移植樹沒有 tag 送 → 頁面看不到讀回值（交件清單）。
//    值的範圍：golden 輸入框 edtSetZ1Click :26813-26823 的小鍵盤（Mitsubishi 0..100，否則 0..300，整數）—— 網頁鍵盤可以被繞過，這裡重查。
//  ⚠ 寫的是實體伺服驅動器的扭力設定（RS232），會讓機台要求重新回原點。只在機台停著時做（golden 同）；要上機驗。
// ===========================================================================
static std::string W906_Main_IndexTorqueOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "");
    int z = -1;
    if ((op != "set" && op != "read") || !p.Int("z", &z) || (z != 0 && z != 1))
        return EvB6Refuse("bad-payload", "要 {\"op\":\"set\"|\"read\",\"z\":0|1[,\"text\":\"<整數>\"]}（z 0＝Z1、1＝Z2）", nullptr);
    FormLockGuard lock;
    if (COM2 == nullptr)
        return EvB6Refuse("forms-not-created", "COM2 (TCOM2Shim) is null", "V912 main.cpp:22635-22661");
    const char* golden = (op == "set") ? (z == 0 ? "V912 main.cpp:22635 btnSetZ1Click" : "V912 main.cpp:22644 btnSetZ2Click")
                                       : (z == 0 ? "V912 main.cpp:22653 btnReadZ1Click" : "V912 main.cpp:22658 btnReadZ2Click");
    if (op == "set") {
        const std::string text = p.Str("text", "");
        const int maxV = (INDEX_DRIVER_TYPE==Mitsubishi_DRIVER) ? 100 : 300;   // golden edtSetZ1Click :26815-26822
        char* end = nullptr;
        const long v = text.empty() ? -1 : std::strtol(text.c_str(), &end, 10);
        if (text.empty() || (end && *end != '\0') || v < 0 || v > maxV)
            return EvB6Refuse("out-of-range", "golden edtSetZ1Click :26813-26823 小鍵盤：整數 0.." + std::to_string(maxV) + "；收到 \"" + text + "\"", golden);
        if(SystemStart)                                                         // golden :22637／:22646
            return EvB6Refuse("system-running", "golden :22637 if(SystemStart) return;", golden);

        COM2->WriteIndexTorqueSetting(z, AnsiString(text.c_str()));             //Steven 20210223 : 整合Torque存取
        fAllMotorHome=false;
        std::printf("act.main.indexTorque set Z%d=%s -> COM2->WriteIndexTorqueSetting, fAllMotorHome=false (golden %s)\n", z + 1, text.c_str(), golden);
    } else {
        COM2->ReadIndexTorqueSetting(z);                                        //Steven 20210223 : 整合Torque存取
        std::printf("act.main.indexTorque read Z%d -> COM2->ReadIndexTorqueSetting (golden %s)\n", z + 1, golden);
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op).Key("z").Number((wb_int64)z).Key("executed").Bool(true);
    w.Key("golden").String(golden);
    w.Key("allMotorHome").Bool(fAllMotorHome);
    std::vector<std::string> todo;
    if (op == "read") todo.push_back("the read-back value (golden edtReadZ1/edtReadZ2/edTorue0/edTorue1, written by the COM receive path) has no tag -- the page cannot show it");
    EvB6Todos(w, todo);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  M-18　golden TfMain::StringGrid2DblClick（main.cpp:25128-25145）—— Comm View 的 StringGrid2（吸嘴真空時間表）雙擊。
//    ⚠ golden 怪處（照翻）：Task List 的 sgTaskList 也掛同一支 OnDblClick／OnSelectCell（main.dfm:16279-16280）——
//      在 Task List 雙擊，一樣用那一格的欄列（iStringGrid2Col／Row）判斷、一樣跳出 StringGrid2 的右鍵選單 PopupMenu4。
//    這支只回答「選單出不出來」（四條守衛照 golden 在伺服器重查，不信任前端）；選單三項（main.dfm:17317-17331）：
//      RecordSuckRealTimeVacuumOnOffTime（:25097）、ModifySuckVacuumOnOffTime（:25147）—— 改吸嘴的真空時間（TMyKitSuck），沒翻
//      （交件清單）；SetToDefineValue1（:26672）—— 已翻：act.main.setToDefineValue（本檔 S100-c）。
// ===========================================================================
static std::string W906_Main_Sg2DblClickOp(const EvB6Payload& p, bool* ok)
{
    int row = -1, col = -1;
    if (!p.Int("row", &row) || !p.Int("col", &col))
        return EvB6Refuse("bad-payload", "要 {\"row\":int,\"col\":int,\"grid\":\"StringGrid2\"|\"sgTaskList\"}（雙擊的那一格）", nullptr);
    FormLockGuard lock;
    const char* guard = nullptr;
    const char* detail = nullptr;
    if(SystemStart==true)                                                       // golden :25131
        { guard = "system-running"; detail = "golden main.cpp:25131: running -- the popup menu does not open"; }
    else if(row==0  || row==5 || row==10 || row==15 || row==20 || row>21)       // golden :25134-25136
        { guard = "bad-cell"; detail = "golden main.cpp:25134-25136: header/separator row or row>21 -- the popup menu does not open"; }
    else if(col<3 || col==5)                                                    // golden :25138-25139
        { guard = "bad-cell"; detail = "golden main.cpp:25138-25139: col<3 or col==5 -- the popup menu does not open"; }
    else if(AccessLevel<iDefSupervisorLevel)                                    //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel   // golden :25140
        { guard = "not-authorized"; detail = "golden main.cpp:25140: AccessLevel<iDefSupervisorLevel -- the popup menu does not open"; }
    if (ok) *ok = true;                                                         // 選單不出來也是 golden 的正常結果
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("golden").String("V912 main.cpp:25128-25145 StringGrid2DblClick (sgTaskList shares it, main.dfm:16279)");
    w.Key("grid").String(p.Str("grid", "StringGrid2"));
    w.Key("row").Number((wb_int64)row).Key("col").Number((wb_int64)col);
    w.Key("popup").Bool(guard == nullptr);
    if (guard) w.Key("guard").String(guard).Key("detail").String(detail);
    w.Key("items").BeginArray();
    w.BeginObject().Key("name").String("RecordSuckRealTimeVacuumOnOffTime").Key("caption").String("Record Suck Real Time Vacuum On/Off Time")
     .Key("ported").Bool(false).Key("golden").String("main.cpp:25097").EndObject();
    w.BeginObject().Key("name").String("ModifySuckVacuumOnOffTime").Key("caption").String("Modify Suck Vacuum On/Off Time")
     .Key("ported").Bool(false).Key("golden").String("main.cpp:25147").EndObject();
    w.BeginObject().Key("name").String("SetToDefineValue1").Key("caption").String("Set To Define Value")
     .Key("ported").Bool(true).Key("cmd").String("act.main.setToDefineValue").Key("golden").String("main.cpp:26672").EndObject();
    w.EndArray();
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  B9 M-7　主畫面 6 顆控制鈕（main.dfm:10623-10791 TBtnPanel）—— S169：C++ 只做到「事件觸發」，實際動作列給 Jimmy。
//    HOME：已翻 —— WS main.home（Jimmy 816ce9b2，origin/main 的 tools/wb_serve.cpp 檔尾 W906_MainHomeCommand）；本段不重做，頁面直接送它。
//    CLEAN OUT：已翻 —— TfMain::BtnCleanOutClick／CleanOut（cCleanOut.cpp:64-67，golden main.cpp:4395-4458）；這裡照 golden 接上。
//    RESET（:7655 BtnResetClick → Reset）、ONE CYCLE（:4469 BtnOneCycleClick）、TRAY FEED（:14467 BtnTrayEndClick →
//    InitialTrayFeedTask）、ALARM RESET（:22867 BtnAlarmResetClick）：只登記「按了」（port-only 事件佇列，一下一筆，3 秒沒被取走就丟掉），
//    由 Jimmy 在 tick 裡用 W906_Main_TakeCtlButtonEvent() 取走後跑 golden 本體（宣告 FileRW/MainClickTail.h 檔尾）。
//    按得到：golden TfMain::ProcessKeyFlush（main.cpp:4228-4286，Timer1 每 250 ms）管 Enabled —— 按下當下照同一條規則算
//      （BtnOneCycle 不在它的清單裡：一直按得到，golden 本體自己擋 fAllMotorHome／iOneCycle）。
// ===========================================================================
namespace {
struct CtlBtnDef { const char* name; const char* golden; };
const CtlBtnDef kCtlBtns[] = {
    {"reset",      "V912 main.cpp:7655 BtnResetClick -> Reset(\"BtnResetClick\") (:7660)"},
    {"oneCycle",   "V912 main.cpp:4469 BtnOneCycleClick"},
    {"trayFeed",   "V912 main.cpp:14467 BtnTrayEndClick -> InitialTrayFeedTask(\"BtnTrayEndClick\")"},
    {"alarmReset", "V912 main.cpp:22867 BtnAlarmResetClick"},
};
const int kCtlBtnN = (int)(sizeof(kCtlBtns) / sizeof(kCtlBtns[0]));
// 按了還沒被取走的每一下（S169 的「事件旗標」；一下一筆、記按下的時間）。
// [W906] 存活時間：golden 按下當場就跑本體，不會晚跑 —— 取用端（Jimmy 的 tick，500 ms 一拍）超過 kEvTtlMs 還沒取走的一下直接丟掉，
//   免得接上取用端之後，很久以前按的 RESET／ONE CYCLE 在第一拍突然執行。
typedef std::chrono::steady_clock EvClock;
const long long kEvTtlMs = 3000;
std::deque<EvClock::time_point> s_ctlBtnPending[kCtlBtnN];
const std::size_t kCtlBtnCap = 8;                                               // [W906] 存活時間內最多記 8 下（golden 每一下都會跑一次本體）
void EvExpire(std::deque<EvClock::time_point>* q)
{
    const EvClock::time_point now = EvClock::now();
    while (!q->empty() && std::chrono::duration_cast<std::chrono::milliseconds>(now - q->front()).count() > kEvTtlMs) q->pop_front();
}

int CtlBtnIndex(const std::string& name)
{
    for (int i = 0; i < kCtlBtnN; ++i) if (name == kCtlBtns[i].name) return i;
    return -1;
}
// golden ProcessKeyFlush :4239-4280。name：reset／trayFeed／alarmReset／cleanOut／home／oneCycle。
bool CtlBtnEnabled(const std::string& name)
{
    if (name == "oneCycle") return true;                                        // 不在 Ptr[]／Enabled 清單裡
    if(SystemStart)                                                             // :4239-4246
        return false;
    if (name == "home") return true;                                            // :4278
    if(fAllMotorHome==false || iOneCycle)                                       // :4249
    {
        if (name == "reset")
            return IniConfig.bO01_ResetNeedClearAndCheckHP ? (fAllMotorHome != false) : false;   // :4254-4269
        return false;                                                           // CleanOut／TrayEnd :4251-4252、AlarmReset :4271
    }
    return true;                                                                // :4273-4276
}
}  // namespace

// AI(W906-EVB6) 20260928 [W906]: 給 Jimmy 的取用點（S169）。回 true＝有一下待處理、已經取走一下（計數減一）。
bool W906_Main_TakeCtlButtonEvent(const char* button)
{
    const int i = button ? CtlBtnIndex(button) : -1;
    if (i < 0) return false;
    FormLockGuard lock;
    EvExpire(&s_ctlBtnPending[i]);
    if (s_ctlBtnPending[i].empty()) return false;
    s_ctlBtnPending[i].pop_front();
    return true;
}

static std::string W906_Main_CtlButtonOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "click");
    const std::string button = p.Str("button", "");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    const int bi = CtlBtnIndex(button);
    if (op == "click" && bi < 0)
        return EvB6Refuse("bad-payload", "button 要是 reset／oneCycle／trayFeed／alarmReset（HOME 送 main.home，CLEAN OUT 送 act.main.cleanOut）；收到 \"" + button + "\"", nullptr);
    FormLockGuard lock;
    if (op == "click") {
        if (!CtlBtnEnabled(button))
            return EvB6Refuse("disabled", std::string("golden ProcessKeyFlush main.cpp:4228-4286: this button is Enabled=false now (SystemStart=") +
                              (SystemStart ? "1" : "0") + " fAllMotorHome=" + (fAllMotorHome ? "1" : "0") + " iOneCycle=" + std::to_string(iOneCycle) + ")",
                              kCtlBtns[bi].golden);
        EvExpire(&s_ctlBtnPending[bi]);
        if (s_ctlBtnPending[bi].size() >= kCtlBtnCap)
            return EvB6Refuse("queue-full", "已經有 " + std::to_string(kCtlBtnCap) + " 下還沒被處理（實際動作還沒接：S169，交給 Jimmy）", kCtlBtns[bi].golden);
        s_ctlBtnPending[bi].push_back(EvClock::now());
        std::printf("act.main.ctlButton %s -> event raised (pending %u, kept %lld ms); the golden body (%s) is not run here (S169, Jimmy)\n",
                    button.c_str(), (unsigned)s_ctlBtnPending[bi].size(), kEvTtlMs, kCtlBtns[bi].golden);
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    if (op == "click") {
        w.Key("executed").Bool(true).Key("button").String(button).Key("golden").String(kCtlBtns[bi].golden);
        w.Key("eventOnly").Bool(true);
        w.Key("note").String("S169：C++ 只登記「按了」，golden 本體（實際動作）還沒接，交給 Jimmy（W906_Main_TakeCtlButtonEvent）");
    }
    w.Key("pending").BeginObject();
    for (int i = 0; i < kCtlBtnN; ++i) { EvExpire(&s_ctlBtnPending[i]); w.Key(kCtlBtns[i].name).Number((wb_int64)s_ctlBtnPending[i].size()); }
    w.EndObject();
    w.Key("eventTtlMs").Number((wb_int64)kEvTtlMs);
    w.Key("enabled").BeginObject();
    for (int i = 0; i < kCtlBtnN; ++i) w.Key(kCtlBtns[i].name).Bool(CtlBtnEnabled(kCtlBtns[i].name));
    w.Key("cleanOut").Bool(CtlBtnEnabled("cleanOut")).Key("home").Bool(CtlBtnEnabled("home"));
    w.EndObject();
    w.EndObject();
    return w.Str();
}

// CLEAN OUT：golden BtnCleanOutClick 的全部內容就是 CleanOut("BtnCleanOutClick")（cCleanOut.cpp:64-67）—— 直接呼叫同一支
//   CleanOut，只為了拿回它的 bool（golden BtnCleanOutClick 丟掉回傳值）。
static std::string W906_Main_CleanOutOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "click");
    if (op != "click") return EvB6Refuse("bad-op", op, nullptr);
    FormLockGuard lock;
    const char* golden = "V912 main.cpp:4395 BtnCleanOutClick -> CleanOut(\"BtnCleanOutClick\") (port cCleanOut.cpp:64-67, from 906 :4264-4330)";
    if (!CtlBtnEnabled("cleanOut"))
        return EvB6Refuse("disabled", std::string("golden ProcessKeyFlush main.cpp:4239-4252: BtnCleanOut->Enabled=false now (SystemStart=") +
                          (SystemStart ? "1" : "0") + " fAllMotorHome=" + (fAllMotorHome ? "1" : "0") + " iOneCycle=" + std::to_string(iOneCycle) + ")", golden);
    const int coBefore = iCleanOut;
    const bool r = fMain->CleanOut("BtnCleanOutClick");                         // ＝ fMain->BtnCleanOutClick(NULL)
    std::printf("act.main.cleanOut -> CleanOut returned %d  iCleanOut %d -> %d\n", (int)r, coBefore, iCleanOut);
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op).Key("executed").Bool(r);
    w.Key("golden").String(golden);
    if (!r) w.Key("guard").String("golden-refused").Key("detail").String("golden CleanOut returned false (906 :4296-4310: one-cycle running or not homed, and a tray/IC is still in the loader/plates)");
    w.Key("cleanOutBefore").Number((wb_int64)coBefore).Key("cleanOut").Number((wb_int64)iCleanOut);
    std::vector<std::string> todo;
    todo.push_back("V912 main.cpp:4401-4405 (ATK AMR: fAGV->IsATK_AMR() && iATKFixFullTask!=eAtkFFIdle -> refuse) is newer than the 906 body in cCleanOut.cpp -- not in the port (Jimmy)");
    EvB6Todos(w, todo);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  B9 M-6　golden TfMain::mtDutOnOffMouseUp（main.cpp:29932-30616，685 行）—— 主畫面 Test Site 格子。S169：只登記「點了哪一格」。
//    這裡照 golden 開頭的幾道 return 先查（按不到就不登記）：:29935 SystemStart、:29942 Insufficient(10)（不足時 golden 跳 WAR1676）、
//    :29945-29954 CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart && bAuthCriticalPara[3]、:29956 ConvertIndexCells==-1
//    （網頁送的是格子座標，換成「超出網頁格子」）。後面整支（開關 site、寫 TestMode.Data／lastdata.dat、ATC site、Auto Site Mapping…）
//    與 ShowTestHeadComp1（:23253-24232）是 Jimmy 的（新題 N-1；本檔 (P8-b) W906_Main_DutOnOff_SaveATC7Channels 是其中 :30329-30374）。
//  ⚠ 座標：網頁格子是 tag 契約的 8×4（WebBridgeTags.cpp site.arm{a}.s{n}：col＝x，arm＝y/2＋1，row＝y%2，即
//    LastSet.bUseTestSocket[arm-1][row][col]）；golden mtDutOnOff 的 XItem／YItem 由 ShowTestHeadComp1 依測試模式設 ——
//    Jimmy 取用時要把 (x,y) 換成 golden 的 X／Y（交件清單）。
// ===========================================================================
namespace {
struct SiteClick { int x, y; EvClock::time_point at; };
std::deque<SiteClick> s_siteClicks;                                             // 點了還沒被取走的格子（先進先出；存活時間同上 kEvTtlMs）
const std::size_t kSiteClickCap = 32;
void SiteExpire()
{
    const EvClock::time_point now = EvClock::now();
    while (!s_siteClicks.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(now - s_siteClicks.front().at).count() > kEvTtlMs)
        s_siteClicks.pop_front();
}
}  // namespace

// AI(W906-EVB6) 20260928 [W906]: 給 Jimmy 的取用點（S169）。回 true＝取走最早的一格（網頁格子座標，見上）。
bool W906_Main_TakeSiteClickEvent(int* x, int* y)
{
    FormLockGuard lock;
    SiteExpire();
    if (s_siteClicks.empty()) return false;
    if (x) *x = s_siteClicks.front().x;
    if (y) *y = s_siteClicks.front().y;
    s_siteClicks.pop_front();
    return true;
}

static std::string W906_Main_SiteClickOp(const EvB6Payload& p, bool* ok)
{
    const std::string op = p.Str("op", "click");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    int x = -1, y = -1;
    if (op == "click" && (!p.Int("x", &x) || !p.Int("y", &y)))
        return EvB6Refuse("bad-payload", "要 {\"x\":0..7,\"y\":0..3}（主畫面 SitePanel 的格子）", nullptr);
    FormLockGuard lock;
    const char* golden = "V912 main.cpp:29932 mtDutOnOffMouseUp";
    if (op == "click") {
        if(SystemStart)                                                         // golden :29935
            return EvB6Refuse("system-running", "golden :29935 if(SystemStart) return;", golden);
        if(fSecurity->Insufficient(10)==false)                                  //jou 981207 權限控制   // golden :29942
            return EvB6Refuse("not-authorized", "golden :29942 fSecurity->Insufficient(10)==false（權限項目 [10]）", golden);
        if(CosFunction.bLotStartLockCriticalPara)                               //JerryYang 20220311 : ATP鎖定Critical parameter
        {
            if(RunInfo.bLotStart)
            {
                if(bAuthCriticalPara[3])
                    return EvB6Refuse("critical-para-locked", "golden :29945-29954 bLotStartLockCriticalPara && RunInfo.bLotStart && bAuthCriticalPara[3]", golden);
            }
        }
        if (x < 0 || x >= 8 || y < 0 || y >= 4)                                 // golden :29956 ConvertIndexCells==-1 的網頁版
            return EvB6Refuse("bad-cell", "格子在網頁 8×4 以外（golden :29956 ConvertIndexCells==-1 return）", golden);
        SiteExpire();
        if (s_siteClicks.size() >= kSiteClickCap)
            return EvB6Refuse("queue-full", "已經有 " + std::to_string(kSiteClickCap) + " 格還沒被處理（實際開關 site 還沒接：S169，交給 Jimmy）", golden);
        SiteClick c = {x, y, EvClock::now()};
        s_siteClicks.push_back(c);
        std::printf("act.main.siteClick x=%d y=%d (arm %d row %d col %d) -> event raised (pending %u); golden body not run here (S169, Jimmy)\n",
                    x, y, y / 2 + 1, y % 2, x, (unsigned)s_siteClicks.size());
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op).Key("golden").String(golden);
    if (op == "click") {
        w.Key("executed").Bool(true).Key("eventOnly").Bool(true);
        w.Key("x").Number((wb_int64)x).Key("y").Number((wb_int64)y);
        w.Key("arm").Number((wb_int64)(y / 2 + 1)).Key("row").Number((wb_int64)(y % 2)).Key("col").Number((wb_int64)x);
        w.Key("note").String("S169：C++ 只登記點了哪一格，golden 本體（開關 site、存檔、ATC、Site Map）還沒接，交給 Jimmy（W906_Main_TakeSiteClickEvent）");
    }
    SiteExpire();
    w.Key("pending").Number((wb_int64)s_siteClicks.size());
    w.Key("eventTtlMs").Number((wb_int64)kEvTtlMs);
    w.EndObject();
    return w.Str();
}

// ===========================================================================
//  R107　HP-3 尾段（上面 W906_Main_sbPlateFormClickTail，golden main.cpp:28424 fCleaning->LoadAutoCleanData）裡 golden 的
//    ShowMyMessage（C 路轉成 filerw::ELMessage，例 golden AutoClean/uCleaning.cpp:457「The site Y-pitch can not use arm 2 for auto clean!!」）
//    以前記進上一次 C 路 session 的殘留、網頁看不到。decisions-pending R107 照通則做 B：放進存檔回覆。
//    做法：LoadAutoCleanData 前後各呼叫一次下面兩支（同一行，見 :796），把這段期間的 messages／todo 收下來；
//    呼叫端（產生檔 FileRW/HotPlateForm_File.cpp SaveFlow，設定在 tools/formbridge/TfHotPlate.py saveFlowAfter）用
//    W906_Main_Hp3TailTakeMessage／W906_Main_Hp3TailTakeTodo 取走、放進 J.Message／J.Todo（form.save 的 ack.messages／ack.todo）。
//    A 形狀（form.save）沒有 filerw session，SessionBegin 只清掉上一次 C 路請求的殘留（不會在任何 ack 出現的東西）。
// ===========================================================================
namespace {
std::vector<std::pair<std::string, std::string> > s_hp3Msgs;
std::vector<std::string> s_hp3Todos;
}  // namespace

void W906_EvB6_Hp3CaptureBegin()
{
    s_hp3Msgs.clear();
    s_hp3Todos.clear();
    filerw::SessionBegin("");
}

void W906_EvB6_Hp3CaptureEnd()
{
    cJSON* root = cJSON_Parse(filerw::SessionJson().c_str());
    if (root == nullptr) return;
    const cJSON* ms = cJSON_GetObjectItemCaseSensitive(root, "messages");
    for (const cJSON* m = ms ? ms->child : nullptr; m; m = m->next) {
        const cJSON* en = cJSON_GetObjectItemCaseSensitive(m, "en");
        const cJSON* zh = cJSON_GetObjectItemCaseSensitive(m, "zh");
        s_hp3Msgs.push_back(std::make_pair(std::string((en && cJSON_IsString(en) && en->valuestring) ? en->valuestring : ""),
                                           std::string((zh && cJSON_IsString(zh) && zh->valuestring) ? zh->valuestring : "")));
        std::printf("close-tail W906_Main_sbPlateFormClickTail -> golden message: %s / %s (R107: into the form.save reply)\n",
                    s_hp3Msgs.back().first.c_str(), s_hp3Msgs.back().second.c_str());
    }
    const cJSON* ts = cJSON_GetObjectItemCaseSensitive(root, "todo");
    for (const cJSON* t = ts ? ts->child : nullptr; t; t = t->next)
        if (cJSON_IsString(t) && t->valuestring) s_hp3Todos.push_back(t->valuestring);
    cJSON_Delete(root);
}

// 取走一筆；*en／*zh／*todo 指到的字串在下一次呼叫同一支之前有效（FileRW/MainClickTail.h 只放不帶型別的宣告，理由見該檔檔頭）。
bool W906_Main_Hp3TailTakeMessage(const char** en, const char** zh)
{
    static std::pair<std::string, std::string> s_last;
    if (s_hp3Msgs.empty()) return false;
    s_last = s_hp3Msgs.front();
    s_hp3Msgs.erase(s_hp3Msgs.begin());
    if (en) *en = s_last.first.c_str();
    if (zh) *zh = s_last.second.c_str();
    return true;
}

bool W906_Main_Hp3TailTakeTodo(const char** todo)
{
    static std::string s_last;
    if (s_hp3Todos.empty()) return false;
    s_last = s_hp3Todos.front();
    s_hp3Todos.erase(s_hp3Todos.begin());
    if (todo) *todo = s_last.c_str();
    return true;
}

// ===========================================================================
//  WS 入口（tools/wb_serve.cpp St01 那一行的一個分支）：cmd → 上面各支。value＝JSON 字串。
// ===========================================================================
std::string W906_Main_EvB6Op(const std::string& cmd, const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    EvB6Payload p(payloadJson);
    if (p.bad) return EvB6Refuse("bad-payload", "value 不是 JSON 物件", nullptr);
    if (fMain == nullptr || fSecurity == nullptr || fTestCategory == nullptr)
        return EvB6Refuse("forms-not-created", "fMain／fSecurity／fTestCategory 有一個是 NULL", nullptr);
    if (cmd == "act.main.runMode")      return W906_Main_RunModeOp(p, ok);      // M-2
    if (cmd == "act.main.tempMode")     return W906_Main_TempModeOp(p, ok);     // M-3
    if (cmd == "act.main.light")        return W906_Main_LightOp(p, ok);        // M-8
    if (cmd == "act.main.fan")          return W906_Main_FanOp(p, ok);          // M-9
    if (cmd == "act.main.funcView")     return W906_Main_FuncViewOp(p, ok);     // M-10
    if (cmd == "act.main.indexTorque")  return W906_Main_IndexTorqueOp(p, ok);  // M-15
    if (cmd == "act.main.hp2View")      return W906_Main_HP2ViewOp(p, ok);      // M-16
    if (cmd == "act.main.checkEncoder") return W906_Main_CheckEncoderOp(p, ok); // M-17
    if (cmd == "act.main.sg2DblClick")  return W906_Main_Sg2DblClickOp(p, ok);  // M-18
    if (cmd == "act.main.siteClick")    return W906_Main_SiteClickOp(p, ok);    // B9 M-6
    if (cmd == "act.main.ctlButton")    return W906_Main_CtlButtonOp(p, ok);    // B9 M-7
    if (cmd == "act.main.cleanOut")     return W906_Main_CleanOutOp(p, ok);     else if (cmd == "act.main.ftrt") { std::string W906_Main_FtRtOp(const std::string&, bool*); return W906_Main_FtRtOp(payloadJson, ok); }   // B9 M-7 CLEAN OUT  //AI(W906-B8-M11) 20260930 [W906]: B8 M-11 主畫面 FT／RT 小方塊（golden palFTClick／palRTClick → DoFTRTClick，本體檔尾）；同一行附加
    return EvB6Refuse("unknown-action", cmd, nullptr);
}

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]: 事件批次 B10 part a —— 設定視窗「真的開／關」那一下（頁面表的邊緣）→ 那個表單的 golden 程式。
//    派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B10、第四節 B10 表（YM-3、SU-9、OS-6、CT-3b、CC-E10）。
//    裁決：Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」；20260929「照 BCB 的邏輯」。
//    表與規則：FileRW/WindowEdgeTails.h（純邏輯，ctest EvB10A_Edges）；各表單的本體在各自的 FileRW/<結構>.cpp 檔尾。
//    呼叫端：tools/wb_serve.cpp:4389 頁面表的邊緣掛勾（W906_PageTableEdgeHookSet 那個 lambda；在 C 路清「開過了」W906_EditPageWindowClosed
//      之前先跑 ⇒ 各結構用 filerw::PageShownNow 問得到「這一次開過、FormClose 還沒跑」）。wb_serve 主迴圈那一條執行緒、不在任何 WS 指令裡
//      （同 WebTeachLeave.h W906_WindowEdgeRegister 的說明）：沒有回覆可以帶 ⇒ golden 的 ELMessage／ELTodo 逐行印到主控台。
//    本段自己持 FormLock（可重入）；golden 丟例外接住、印一行，不讓它跑出主迴圈。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"   // AI(W906-EVB10A) 20260929 [W906]
#include <exception>                  // AI(W906-EVB10A) 20260929 [W906]

namespace {
void EvB10ADumpSession(const char* form)
{
    cJSON* sj = cJSON_Parse(filerw::SessionJson().c_str());
    if (!sj) return;
    const cJSON* ms = cJSON_GetObjectItemCaseSensitive(sj, "messages");
    for (const cJSON* m = (ms && cJSON_IsArray(ms)) ? ms->child : nullptr; m; m = m->next) {
        const cJSON* en = cJSON_GetObjectItemCaseSensitive(m, "en");
        const cJSON* zh = cJSON_GetObjectItemCaseSensitive(m, "zh");
        std::printf("[EVB10A] %s golden message: %s%s%s\n", form, (en && cJSON_IsString(en)) ? en->valuestring : "",
                    (zh && cJSON_IsString(zh) && *zh->valuestring) ? " / " : "", (zh && cJSON_IsString(zh)) ? zh->valuestring : "");
    }
    const cJSON* td = cJSON_GetObjectItemCaseSensitive(sj, "todo");
    for (const cJSON* t = (td && cJSON_IsArray(td)) ? td->child : nullptr; t; t = t->next)
        if (cJSON_IsString(t)) std::printf("[EVB10A] %s todo: %s\n", form, t->valuestring);
    cJSON_Delete(sj);
}
}  // namespace

void W906_EvB10A_WindowEdge(const char* goldenForm, bool open)
{
    typedef const char* (*EdgeFn)(bool);
    static const EdgeFn kFn[] = {                                               // 順序＝FileRW/WindowEdgeTails.h evb10a::kRows
        &FileRW_YieldMonitoring_WindowEdge,                                     // fYieldMonitoring  YM-3
        &FileRW_Setup_WindowEdge,                                               // fSetup            SU-9（✕；Exit 走 form.event）
        &FileRW_Offset_WindowEdge,                                              // fOffSet           OS-6
        &FileRW_Contact_WindowEdge,                                             // fContact          CT-3b
        &FileRW_IniConfig_WindowEdge,                                           // fConfiguration    CC-E10
        // AI(W906-EVB10C) 20260929 [W906]：B10c（golden FormClose；表 FileRW/WindowEdgeTails.h 第 6～19 列）
        &FileRW_Speed_WindowEdge,                                               // fSpeed            cSpeed.cpp:1272
        &FileRW_LdUld_WindowEdge,                                               // fLd_ULd           cLd_ULd.cpp:170
        &FileRW_TrayForm_WindowEdge,                                            // fTrayForm         cTrayForm.cpp:559
        &FileRW_TrayAssignment_WindowEdge,                                      // fTrayAssignment   cTrayAssignment.cpp:1325
        &FileRW_Temperature_WindowEdge,                                         // fTemp_Set         uTemp_Set.cpp:4196（✕；Exit 走 form.event TS-10）
        &FileRW_TTLCfg_WindowEdge,                                              // fDIOFrom          DIOInterFaceCFG.cpp:264
        &FileRW_QAMode_WindowEdge,                                              // fQAMode           QAMode.cpp:171
        &FileRW_BarCode_WindowEdge,                                             // fBarCode          BarCode.cpp:531
        &FileRW_VacuumUnit_WindowEdge,                                          // fVacuumUnit       VacuumUnit.cpp:304
        &FileRW_BinSelect_WindowEdge,                                           // fBinSel           cBinSel.cpp:2137
        &FileRW_HSys_WindowEdge,                                                // HandlerSystem     HandlerSys.cpp:1215
        &FileRW_CounterSel_WindowEdge,                                          // fCounterSel       cCounterSel.cpp:56
        &FileRW_StartCondition_WindowEdge,                                      // fStartCondition   cStartCondition.cpp:619
        &FileRW_Cleaning_WindowEdge,                                            // fCleaning         uCleaning.cpp:2106
    };
    static_assert(sizeof(kFn) / sizeof(kFn[0]) == evb10a::kRowCount, "FileRW/WindowEdgeTails.h kRows and this table must match");
    const int i = evb10a::Find(goldenForm);
    const bool running = SystemStart || SoftStart;
    const evb10a::Verdict v = evb10a::Decide(goldenForm, open, running);
    if (i < 0 || v == evb10a::kNotListed) return;
    if (v == evb10a::kSkipRunning) {
        std::printf("[EVB10A] %s closed while SystemStart=%d SoftStart=%d -> golden %s not run (same rule as R86: golden cannot open or close "
                    "this modal form while running -- V912 main.cpp:3970-3978 hides palSetup / palConfig; not run later either)\n",
                    goldenForm, (int)SystemStart, (int)SoftStart, evb10a::kRows[i].golden);
        std::fflush(stdout);
        return;
    }
    FormLockGuard lock;                                                         // 本檔 :81（golden 程式改全域、讀寫檔）
    filerw::SessionBegin("");
    std::string what;
    try {
        const char* w = kFn[i](open);
        what = w ? w : "";
    } catch (const std::exception& x) {
        what = std::string("golden ") + evb10a::kRows[i].golden + " threw: " + x.what();
    } catch (...) {
        what = std::string("golden ") + evb10a::kRows[i].golden + " threw a non-std exception";
    }
    std::printf("[EVB10A] %s %s -> %s\n", goldenForm, open ? "opened" : "closed", what.c_str());
    EvB10ADumpSession(goldenForm);
    std::fflush(stdout);
}

// CC-E10（golden TfConfiguration::sbExitClick cConfiguration.cpp:6386-6388，呼叫端 FileRW/IniConfig.cpp 檔尾）：
//   `fMain->sbPEModel->Visible=cbC12->Checked;` ⇒ 上面 W906_Main_PEModelVisible 從此回這個值（act.main.peModel 的 visible、click 的守衛都看它）。
//   ⚠ golden 怪處照翻：這一句不看 CosFunction.bUsePEModelFunction（開機 FormShow :11261 有看）—— 客戶沒有 PE 功能、但 [C12] 勾著時，
//   golden 關掉 Configuration 之後主畫面會出現 PE 鈕。
void W906_Main_SetPEModelVisible(bool bVisible)
{
    s_iB10APEModelVisible = bVisible ? 1 : 0;
}

// CC-E10：`if(cbC12->Checked==false && bEnablePEModel==true) fMain->sbPEModelClick(this);`（避免 PE 模式開著時功能被關掉）。
//   條件由呼叫端照 golden 判斷；這裡就是按一下 PE 鈕（上面 W906_Main_sbPEModelClick，golden main.cpp:33724，含 :33728 的守衛）。
//   golden 的主畫面三格 Enabled（edATCAmbientTemper／edWorkTemperBase／edSoakTime）沒有頁面可以套 → 印出來；todo 交給 filerw::ELTodo。
const char* W906_Main_PEModelClickFromConfig()
{
    std::vector<std::pair<std::string, bool> > ui;
    std::vector<std::string> todo;
    const int rc = W906_Main_sbPEModelClick(&ui, &todo);
    for (std::size_t k = 0; k < todo.size(); ++k) filerw::ELTodo(todo[k].c_str());
    for (std::size_t k = 0; k < ui.size(); ++k)
        std::printf("[EVB10A] fConfiguration sbExitClick -> fMain->sbPEModelClick: golden %s->Enabled=%d (main screen; no page to apply it)\n",
                    ui[k].first.c_str(), (int)ui[k].second);
    std::printf("act.main.peModel (from Configuration sbExitClick) -> %s (bHasEnteredPEModel=%d bEnablePEModel=%d)\n",
                rc == 0 ? "guard" : (rc == 1 ? "ON" : "OFF"), (int)bHasEnteredPEModel, (int)bEnablePEModel);
    return rc == 0 ? "sbPEModelClick returned at its golden guard (main.cpp:33728 Insufficient(128)==false && HasICUnderMachine()==false)"
                   : (rc == 1 ? "sbPEModelClick -> PE Model ON" : "sbPEModelClick -> PE Model OFF");
}

// ===========================================================================
//  AI(W906-B8-M11) 20260930 [W906]: B8 M-11 —— 主畫面 FT／RT 小方塊（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「M-11」；
//    Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden V912 main.cpp：palFTClick :30700-30703／palRTClick :30705-30708 → DoFTRTClick(bIsRT, bIsMan=true) :35752-35774
//    → FTClick :30710-30832／RTClick :30834-31001（回傳碼：FT 0／1／2／3／4／8、RT 0～8；DoFTRTClick 只在非 0 時 RecordProcess 一行）。
//    做什麼：改主畫面起動模式（cbRunStartMode → cbRunStartModeChange ＝ Jimmy 的 RunStartMode.cpp W906_CbRunStartModeChange，只呼叫）、
//    fLotInfo 的 Run Mode 字；SPIL 清 Yield 計數＋Clarn_Data；RT 在 [O21] 時清 Fail Bin 計數（fCounterClear->ClearCount）；2DID 白名單
//    （cbRunMode=="CORR"）時送 SECS Lot End（fLotInfo->sbSECSLotEndClick）；芯云送 SECS 事件。按下當下不動軸。
//  WS 入口：act.main.ftrt {"op":"get"}｜{"op":"click","tile":"FT"|"RT"}（tools/wb_serve.cpp St01 那一行 act.main.* 的條件、本檔 W906_Main_EvB6Op
//    同一行分派）；頁面 D:\HT9045\web\page\ht9045_main_st01_ev.js 的 palFT／palRT（main.html 是 Jimmy 的，不動）。
//  golden 點得到嗎（VCL：TPanel 看不見或停用就沒有 OnClick）：
//    Visible＝fMain->palFT／palRT->Visible（golden FormShow :9080-9096、SetRunStartMode :597-601 等；移植樹 SetRunStartMode 在 RunStartMode.cpp
//      照翻、維護同一個成員）。
//    Enabled＝golden ChangeLevelAttr :13036-13037 palFT->Enabled=cbRunStartMode->Enabled（CC_JCET 不改＝DFM 預設 True；運轉中那一臂不設 ⇒
//      留開始前最後一拍的值，按得到的話 FTClick／RTClick 第一行 SystemStart 回 1、DoFTRTClick 記一行 "FTClick fail! (1)"，照 golden）。
//  ⚠ cbRunStartMode->Enabled：golden 由 ChangeLevelAttr（Timer2Timer :21675 每一拍）依權限、機台狀態重算（:12937-13029：運轉中、[FT Bin＝RT Bin]
//    時手臂有料或 Z 不在安全位、Loader 有料、等級低於權限表第 12 項、CheckCanChangeRealDummy、Auto Clean／Index 任務、HasICUnderMachine／
//    HasAnyICInMachine ⇒ 鎖住）；FTClick :30766／RTClick :30911 也讀它。移植樹 TfMain::ChangeLevelAttr 是空殼（forms/fMain.h:241），
//    fMain->cbRunStartMode->Enabled 沒有人維護（vclcompat 預設 false）⇒ 在按下的當下照同一套規則算（FtRtStartModeEnabled，只讀、不寫回成員）。
//    這是「機台有料不准切 FT／RT」那一道互鎖：不信任前端，C++ 重算。golden「這一拍不改、留上一拍的值」的兩處（LastSet.iRunStartMode==
//    rsmAutoSiteMap、CC_SCC 且 Lot 已開始）：ASM 用同一套規則代替上一拍；SCC＋Lot 已開始＝golden SetLotStart 鎖住（uLotInfo；移植樹
//    forms/fLotInfo.cpp GATE W906-LOT-W1-MAINPANELS）⇒ false。
//  跟 golden 不同（寫明）：
//    * spbUserName->Caption（JCET 權限訊息）：TfMain 門面沒有（forms/fMain.h:1192）⇒ 用 fMain->cbUserSelect->Text（登入等級的字；密碼本模式
//      golden 顯示使用者名稱，那一份在 WebLogin.cpp 的匿名 namespace，本檔不連它 —— tests/test_main_ctlbuttons 整支編進本檔）。
//    * bRTContinueNeedClearCT（golden V912 cmydef.cpp:5788，RogerYang 20260703）：移植樹沒有、也沒有讀它的地方（golden 讀在 csystem.cpp:12863，
//      移植樹 0 筆）⇒ 放在本檔 W906_bRTContinueNeedClearCT，設了沒人讀（ack.todo 說明）。移植 csystem.cpp 那一段時搬回 cmydef、改回原名。
//    * palFTMouseDown／Up（:31420-31469）只改 BevelOuter（按下去的外觀）＝純畫面，沒做。
//    * 遠端（GPIB ChangeHandlerStartMode、TCP HTSET 317）也叫 DoFTRTClick(…, false)：移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:12162-12165、
//      :12177-12180、:17008-17010、:17016-17018 仍閘著（GATE REGISTER 12／A14；Command.cpp 不是 St01 的檔）。本段的 W906_Main_DoFTRTClick 可以直接接
//      （bForceRunStartModeEnabled＝golden 那幾行先把 cbRunStartMode->Enabled 設 true），另外登記。
//    * forms/fMain.cpp:513-514 的 TfMain::FTClick／RTClick 替身（Jimmy 的檔）沒改：本段是自由函式，不經過那兩個成員。
//  ⚠ 會碰到的機台狀態／檔案：LastSet.iRunStartMode 與 SetRunStartMode 的一整串（W906_CbRunStartModeChange → SetRunStartMode：起動模式、主畫面
//    燈號、BinSel ReadFile／SetWorkParameter —— test_tester_connect_rules 記過「SetRunStartMode 會寫機台檔」）；SPIL：Clarn_Data（計數、lastdata）；
//    [O21]：fCounterClear->ClearCount(ctFailBinCount)；2DID 白名單：Lot End（報表）；事件紀錄 RecordProcess（FTClick／RTClick fail）。
//  測試：ctest B8_M11_FtRt（tests/test_b8_m11_ftrt.cpp）。
// ===========================================================================
#include "aHotPlateSubstrate.h"        // AI(W906-B8-M11) 20260930 [W906]: InArmSuck／OutArmSuck（177 個 TU 用的那一份 TMyKitSuck，陷阱 #3；同 cMainStatus.cpp:26）、SetRunStartMode
#include "forms/fLotInfo.h"            // AI(W906-B8-M11) 20260930 [W906]: fLotInfo->cbRunMode／sbSECSLotEnd／sbSECSLotEndClick
#include "forms/fYieldMonitoring.h"    // AI(W906-B8-M11) 20260930 [W906]: fYieldMonitoring->ClearYieldCount（本體 uYieldMonitoring.cpp:1756）
#include "forms/fCounterClear.h"       // AI(W906-B8-M11) 20260930 [W906]: fCounterClear->ClearCount（本體 cCounterClear.cpp）

extern int iTestHeadMotorTask;                                                  // atester.h:169（golden atester.cpp:5346）
bool ShuttleHasIC();                                                            // csystem.h（本體 csystem.cpp:19097）
bool IndexHasIC();                                                              // csystem.h（本體 csystem.cpp:19127）
bool AllArmZIsSafe();                                                           // csystem.h（本體 csystem.cpp:18405）
void W906_CbRunStartModeChange();                                               // RunStartMode.cpp:1184（golden TfMain::cbRunStartModeChange main.cpp:23726，Jimmy）
void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString S3, bool Ok, bool bServoOff);   // canary_support.h:80（golden mymessbox.h:58；wb_serve 轉到網頁）
extern int        W906_ShowMyMessage_Count;                                    // canary_support.h:168
extern AnsiString W906_ShowMyMessage_LastS1;                                   // canary_support.h:167

bool W906_bRTContinueNeedClearCT = false;                                       // golden V912 cmydef.cpp:5788 bRTContinueNeedClearCT（見本段檔頭）
// 測試縫（port-only）：golden cbRunStartModeChange(this)。預設＝Jimmy 的真本體；ctest 換成記錄器（SetRunStartMode 會寫機台檔，ctest 不能跑它）。
void (*W906_FtRt_RunStartModeChangeFn)() = &W906_CbRunStartModeChange;

namespace {

// golden TfMain::ChangeLevelAttr（main.cpp:12937-13029）裡 cbRunStartMode->Enabled 那幾行（見本段檔頭）；*why＝最後一條鎖住的規則。
//   bAsIfStopped：不看 SystemStart、照停機那一臂算（小方塊用：golden :13036 palFT->Enabled 只在停機那一臂設，運轉中留著開始前最後一拍的值；
//   這裡用目前的機台狀態代替那一拍）
bool FtRtStartModeEnabled(std::string* why, bool bAsIfStopped = false)
{
    std::string w;
    bool en = false;
    if(SystemStart && !bAsIfStopped)                                            // :12937-12948
    {
        en=false; w="SystemStart (golden ChangeLevelAttr :12942)";
    }
    else
    {
        // :12959-12993（LastSet.iRunStartMode==rsmAutoSiteMap 時 golden 這一拍不改 —— 同一套規則代替上一拍的值，見檔頭）
        if(CUSTOMER_CODE==CC_SCC)
        {
            if(RunInfo.bLotStart==false)
                en=authMainForm[9];
            else
                { en=false; w="CC_SCC and the lot has started (golden SetLotStart locked it; ChangeLevelAttr :12963 leaves it)"; }
        }
        else if(IniConfig.bFTBin2RTBin)                                         //Steven 20210202 : 當Bin一樣時, One Cycle之後要可以切換FT/RT
        {
            if(InArmSuck.HasIC() || OutArmSuck.HasIC() || ShuttleHasIC() ||
               IndexHasIC() || AllArmZIsSafe()==false)
                { en=false; w="[FT Bin = RT Bin]: IC on an arm / shuttle / index, or an arm Z not safe (:12968-12971)"; }
            else
                en=authMainForm[9];
        }
        else
        {
            if(LastSet.iRealDummy==REALLY &&
               ((MOT[MMTrayY].fHasTray &&
                 MOT[MMTrayY].Tray.HasIC()) ||                                  //JerryYang 20170502 (Steven) loader有IC不能切換模式
                MOT[MMTrayY_Car].fHasTray))                                     //Steven 20210202 : 補漏洞, onecycle之後不能切模式
                { en=false; w="REALLY and the loader tray has IC / a tray on the tray car (:12980-12985)"; }
            else
                en=authMainForm[9];
        }
        if(!en && w.empty()) w="authMainForm[9] (Security_new.def [Main] start mode) is off (:12964/:12975/:12989)";

        if(AccessLevel<LevelSet.AccessLevel[12])                                // :12995-12999
            { en=false; w="AccessLevel below LevelSet.AccessLevel[12] (Start Mode Select) (:12995)"; }

        if(IniConfig.bNewResetFunction && bResetIsPressed)                      //Steven 20130625 : 新的Reset方式   // :13001
        {
        }
        else
        {
            if(fMain->CheckCanChangeRealDummy()==false)                         // :13006（cMainStatus.cpp:323 的活本體）
                { en=false; w="CheckCanChangeRealDummy()==false: IC on a plate / shuttle / index / arm (:13006)"; }

            if((iTestHeadMotorTask!=1 && fAllMotorHome==true) ||                //JerryYang 20160728 Auto clean未完成不能更改測試模式（golden 沒有括號，&& 先算，意思相同）
               bART_RT2RunNoChangeMode ||                                       //kevin 20150717  只退fail RT2 不能更改測試模式
               (bRunAutoClean && iDoAutoCleanTask!=1))
                { en=false; w="index test-head task running / ART RT2 no-change / auto clean running (:13012-13016)"; }

            if(HasICUnderMachine() || HasAnyICInMachine())                      // :13023（模式鎖住）
                { en=false; w="IC or tray still in the machine: HasICUnderMachine() || HasAnyICInMachine() (:13023)"; }
        }
    }
    if (why) *why = en ? std::string() : w;
    return en;
}

// golden `cbRunStartMode->Text=StartModeName[x];`：VCL TComboBox 設 Text ⇒ 選到同名那一項（ItemIndex 跟著；同 tools/wb_serve.cpp
//   W906_RunStartModeCommand 的做法。清單是空的——移植樹 SetStartModeData 空殼——就取 enum 編號，同 RunStartMode.cpp W906_RsmComboSyncText）
void FtRtSetStartModeText(const AnsiString& t)
{
    TfLotInfoRunMode* cb = fMain->cbRunStartMode;
    int pos = -1;
    if (cb->Items != 0 && cb->Items->GetCount() > 0) pos = cb->Items->IndexOf(t);
    else for (int i = 0; i < rsmRunModeTotal; ++i) if (StartModeName[i] == t) { pos = i; break; }
    cb->ItemIndex = pos;
    cb->Text = t;
}

// golden TfMain::FTClick（main.cpp:30710-30832）逐行；cbRunStartMode→fMain->cbRunStartMode、cbRunStartMode->Enabled（讀）→bRsmEnabled（見檔頭）
int W906_Main_FTClick(bool bMan, bool bRsmEnabled)                             //Steven 20210423 :修改FT/RT Click回覆動作
{
    if(SystemStart)                                                             //JerryYang 20170420 (Steven) 新增防護
        return 1;

    if(IniConfig.bSPILFunction && bCanRunSCKART)                                //JerryYang 20220923 : add
    {
    }

    if(CUSTOMER_CODE==CC_JCET && bMan==true)                                    //RogerYang 20260319 : 加入登入權限，且需要先完成onecycle
    {
        if(LastSet.iRealDummy==REALLY)
        {
            if(InArmSuck.HasIC()   ||
               OutArmSuck.HasIC()  ||
               ShuttleHasIC()      ||
               IndexHasIC()        )
            {
                ShowMyMessage("Please finish ONE CYCLE before Change Mode!", "請先完成ONE CYCLE再切換模式!", "", false, false);
                return 4;
            }
        }

//        if(DoPassword()==false)
//        {
//            return 8;
//        }
//        return 5;
        if(AccessLevel < LevelSet.AccessLevel[12])                              //RogerYang 20260530 : 用 Start Mode Select 同等級
        {
            AnsiString sMsg, sMsgCN;
            sMsg.sprintf("Current level \"%s\", please switch to higher level account.", fMain->cbUserSelect->Text.c_str());   // [W906] golden spbUserName->Caption（見檔頭）
            sMsgCN.sprintf("當前權限\"%s\"，請切換更高權限帳號。", fMain->cbUserSelect->Text.c_str());
            ShowMyMessage(sMsg, sMsgCN, "", false, false);
            return 8;
        }
    }

    if(IniConfig.bShowFTandRTButtonCanClick==true ||                            //Steven 20131224 : FT & RT Buttion 可以按
       IniConfig.bEnable_SECS_GEM==true ||                                      //Steven 20150604 : Add for SECS GEM
       CUSTOMER_CODE==CC_Murata)                                                //Steven 20200615 : Add Murata
    {
        if(IniConfig.bFTBin2RTBin)                                              //Steven 20210202 : 當Bin一樣時, One Cycle之後要可以切換FT/RT
        {
            if(InArmSuck.HasIC() ||
               OutArmSuck.HasIC() ||
               ShuttleHasIC() ||
               IndexHasIC() ||
               AllArmZIsSafe()==false)
            {
                return 2;
            }
        }
        else
        {
            if(iSecsGemSwitchFTRT==0 &&                                         //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功
               bRsmEnabled==false)                                              //JerryYang 20170328 (wei) 非ART模式時機台內有IC不能切換FT/RT   // [W906] golden cbRunStartMode->Enabled==false
                return 3;

            if(LastSet.iRealDummy==REALLY &&
               ((MOT[MMTrayY].fHasTray &&
                 MOT[MMTrayY].Tray.HasIC()) ||                                  //JerryYang 20170502 (Steven) loader有IC不能切換模式
                MOT[MMTrayY_Car].fHasTray))
                return 4;
        }

        if(RunInfo.bLotStart==true &&
           TestIF_File.bEnableBarCode==true &&
           TestIF_File.b2DIDAllowList==true &&
           fLotInfo->cbRunMode->Text=="CORR")                                   //JerryYang 20250428 : fix 2DID白名單
        {
            fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);
        }

        if(fMain->cbRunStartMode->Text==StartModeName[rsmAutoSiteMap])
        {
            FtRtSetStartModeText(StartModeName[rsmAutoSiteMap]);
        }
        else if(fMain->cbRunStartMode->Text==StartModeName[rsmContinuStart])
        {
            FtRtSetStartModeText(StartModeName[rsmContinuStart]);
        }
        else
        {
            if(HasAnyICInMachine())                                             //Steven 20210802 : 還有IC在機台裡面, 不能做initial start
                FtRtSetStartModeText(StartModeName[rsmContinuStart]);
            else
                FtRtSetStartModeText(StartModeName[rsmInitialStart]);
        }
        W906_FtRt_RunStartModeChangeFn();                                       // golden cbRunStartModeChange(this)

        if(IniConfig.bVTESTFunction==true)
        {
            fLotInfo->cbRunMode->Text="FT1";
        }
        else if(CUSTOMER_CODE==CC_SCC ||
                CUSTOMER_CODE==CC_SJ_Semiconductor ||
                CUSTOMER_CODE==CC_JCET)                                         //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
        {
        }
        else
        {
            fLotInfo->cbRunMode->Text="Normal";
        }
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20160926 Add 矽品台中、矽品蘇州,切FT/RT要清Yield count  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        fYieldMonitoring->ClearYieldCount();                                    //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
        if(CUSTOMER_CODE==CC_XINYUN)                                            //Steven 20220713 : for XinYun
            fMain->Clarn_Data(0, "SPIL_FTClick");
        else
            fMain->Clarn_Data(4, "SPIL_FTClick");
    }

    if(iSecsGemSwitchFTRT!=0)
        iSecsGemSwitchFTRT=2;                                                   //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功

    if(CUSTOMER_CODE==CC_XINYUN && IniConfig.bEnable_SECS_GEM==true)            //AI(secs-xinyun) 20260610 (RogerYang) : 芯云切FT通知Host CEID56
        EventReport(SECS_EVENT.TesterFT);

    return 0;
}

// golden TfMain::RTClick（main.cpp:30834-31001）逐行；替換同 FTClick
int W906_Main_RTClick(bool bMan, bool bRsmEnabled)                             //Steven 20210423 :修改FT/RT Click回覆動作
{
    if(SystemStart)                                                             //JerryYang (Steven) 20170420 新增防護
        return 1;

    if(IniConfig.bSPILFunction && bCanRunSCKART)                                //JerryYang 20220923 : add
    {
        return 7;
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  //RogerYang 20260319 : 加入登入權限，且需要先完成onecycle
    {
        if(bMan==true)
        {
            if(LastSet.iRealDummy==REALLY)
            {
                if(InArmSuck.HasIC()   ||
                   OutArmSuck.HasIC()  ||
                   ShuttleHasIC()      ||
                   IndexHasIC()        )
                {
                    ShowMyMessage("Please finish ONE CYCLE before Change Mode!", "請先完成ONE CYCLE再切換模式!", "", false, false);
                    return 4;
                }
            }

//            if(DoPassword()==false)
//            {
//                return 8;
//            }
            if(AccessLevel < LevelSet.AccessLevel[12])                          //RogerYang 20260530 : 用 Start Mode Select 同等級
            {
                AnsiString sMsg, sMsgCN;
                sMsg.sprintf("Current level \"%s\", please switch to higher level account.", fMain->cbUserSelect->Text.c_str());   // [W906] golden spbUserName->Caption（見檔頭）
                sMsgCN.sprintf("當前權限\"%s\"，請切換更高權限帳號。", fMain->cbUserSelect->Text.c_str());
                ShowMyMessage(sMsg, sMsgCN, "", false, false);
                return 8;
            }
        }

        if(IniConfig.bUseAutoSiteMapping && IniConfig.bI21EnableASM)
        {
            if(fMain->cbRunStartMode->Text==StartModeName[rsmAutoSiteMap])
            {
                if(fMain->CheckCanChangeRealDummy()==false ||                   //JerryYang 20190709 新增保護
                  (LastSet.iRealDummy==REALLY &&
                   ((MOT[MMTrayY].fHasTray &&                                   //JerryYang 20170502 (Steven) loader有IC不能切換模式
                     MOT[MMTrayY].Tray.HasIC()) ||                              //JerryYang 20170328 (wei) 非ART模式時機台內有IC不能切換FT/RT
                    MOT[MMTrayY_Car].fHasTray)))
                    return 2;

                if((iTestHeadMotorTask!=1 && fAllMotorHome==true) ||            // golden 沒有括號（&& 先算，意思相同）
                   bART_RT2RunNoChangeMode || bRunAutoClean)
                    return 3;

                SetRunStartMode(rsmCInitialRetest);
            }
        }
    }

    if(IniConfig.bShowFTandRTButtonCanClick==true ||                            //Steven 20131224 : FT & RT Buttion 可以按
       CUSTOMER_CODE==CC_Murata ||
       IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20150604 : Add for SECS GEM
    {
        if(IniConfig.bFTBin2RTBin)                                              //Steven 20210202 : 當Bin一樣時, One Cycle之後要可以切換FT/RT
        {
            if(InArmSuck.HasIC() ||
               OutArmSuck.HasIC() ||
               ShuttleHasIC() ||
               IndexHasIC() || AllArmZIsSafe()==false)
            {
                return 4;
            }
        }
        else
        {
            if(iSecsGemSwitchFTRT==0 &&                                         //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功
               bRsmEnabled==false)                                              //JerryYang 20170328 (wei) 非ART模式時機台內有IC不能切換FT/RT   // [W906] golden cbRunStartMode->Enabled==false
                return 5;

            if(LastSet.iRealDummy==REALLY &&
               ((MOT[MMTrayY].fHasTray &&
                 MOT[MMTrayY].Tray.HasIC()) ||
               MOT[MMTrayY_Car].fHasTray))                                      //JerryYang 20170502 (Steven) loader有IC不能切換模式
                return 6;
        }

        if(RunInfo.bLotStart==true &&
           TestIF_File.bEnableBarCode==true &&
           TestIF_File.b2DIDAllowList==true &&
           fLotInfo->cbRunMode->Text=="CORR")                                   //JerryYang 20250428 : fix 2DID白名單
        {
            fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);
        }

        if(IniConfig.bI50_EnableAutoSiteMappingTrigger==true &&
           IniConfig.bI50_RT==false)                                            //Steven 20231031 : RT不做ASM
        {
            if(fMain->cbRunStartMode->Text==StartModeName[rsmContinuStart] ||
               fMain->cbRunStartMode->Text==StartModeName[rsmContinuRetest])
            {
                FtRtSetStartModeText(StartModeName[rsmContinuRetest]);
            }
            else
            {
                if(HasAnyICInMachine())                                         //Steven 20210802 : 還有IC在機台裡面, 不能做initial start
                    FtRtSetStartModeText(StartModeName[rsmContinuRetest]);
                else
                    FtRtSetStartModeText(StartModeName[rsmCInitialRetest]);
            }
        }
        else
        {
            if(fMain->cbRunStartMode->Text==StartModeName[rsmAutoSiteMap])
            {
                FtRtSetStartModeText(StartModeName[rsmAutoSiteMap]);
            }
            else if(fMain->cbRunStartMode->Text==StartModeName[rsmContinuStart] ||
                    fMain->cbRunStartMode->Text==StartModeName[rsmContinuRetest])
            {
                FtRtSetStartModeText(StartModeName[rsmContinuRetest]);
            }
            else
            {
                if(HasAnyICInMachine())                                         //Steven 20210802 : 還有IC在機台裡面, 不能做initial start
                    FtRtSetStartModeText(StartModeName[rsmContinuRetest]);
                else
                    FtRtSetStartModeText(StartModeName[rsmCInitialRetest]);
            }
        }
        W906_FtRt_RunStartModeChangeFn();                                       // golden cbRunStartModeChange(this)
        if(CUSTOMER_CODE==CC_SCC ||                                             //Steven 20250530 : SCC要求Tray Feed的時候, 要按下Lot End
           CUSTOMER_CODE==CC_SJ_Semiconductor)
        {
            fLotInfo->cbRunMode->Text="RT1";
        }
        else if(CUSTOMER_CODE==CC_JCET)                                         //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能 不切換
        {
        }
        else
        {
            fLotInfo->cbRunMode->Text="RT";
        }

        if(IniConfig.bO21FTAfterTrayEndClearFailBinCount)                       //Frank 20241114 : Add
            fCounterClear->ClearCount(ctFailBinCount);

        if(fMain->cbRunStartMode->Text==StartModeName[rsmContinuRetest])        //AI(ht9045-clearcount-flow) 20260703 (RogerYang) : FT/RT切換進RT Continue也武裝(手動/Host切換路徑)
            W906_bRTContinueNeedClearCT=true;                                   // [W906] golden bRTContinueNeedClearCT（見檔頭）
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20160926 Add 矽品台中、矽品蘇州,切FT/RT要清Yield count  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        fYieldMonitoring->ClearYieldCount();                                    //RogerYang 20170704 (Steven) 中科說要清掉
        if(CUSTOMER_CODE==CC_XINYUN)                                            //Steven 20220713 : for XinYun
            fMain->Clarn_Data(0, "SPIL_RTClick");
        else
            fMain->Clarn_Data(3, "SPIL_RTClick");
    }

    if(iSecsGemSwitchFTRT!=0)
        iSecsGemSwitchFTRT=2;                                                   //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功

    if(CUSTOMER_CODE==CC_XINYUN && IniConfig.bEnable_SECS_GEM==true)            //AI(secs-xinyun) 20260610 (RogerYang) : 芯云切RT通知Host CEID57
        EventReport(SECS_EVENT.TesterRT);

    return 0;
}

const char* FtRtCodeText(bool bIsRT, int r)                                     // 回傳碼的意思（golden 行號；只給 ack 看）
{
    if (r == 0) return "golden returned 0";
    if (r == 1) return "SystemStart (golden :30712 / :30836)";
    if (!bIsRT) {
        if (r == 2) return "[FT Bin = RT Bin]: IC on an arm / shuttle / index, or an arm Z not safe (golden :30752-30761)";
        if (r == 3) return "start mode locked: cbRunStartMode->Enabled==false and no SECS/GEM switch pending (golden :30765-30767)";
        if (r == 4) return "REALLY and the loader tray has IC / a tray on the tray car, or JCET: ONE CYCLE not finished (golden :30769-30773 / :30728)";
        if (r == 8) return "JCET: AccessLevel below LevelSet.AccessLevel[12] (golden :30738)";
    } else {
        if (r == 2) return "JCET + ASM: CheckCanChangeRealDummy / loader has IC (golden :30878-30883)";
        if (r == 3) return "JCET + ASM: index task / ART RT2 / auto clean (golden :30885-30887)";
        if (r == 4) return "[FT Bin = RT Bin]: IC on an arm / shuttle / index, or an arm Z not safe, or JCET: ONE CYCLE not finished (golden :30898-30906 / :30855)";
        if (r == 5) return "start mode locked: cbRunStartMode->Enabled==false and no SECS/GEM switch pending (golden :30910-30912)";
        if (r == 6) return "REALLY and the loader tray has IC / a tray on the tray car (golden :30914-30918)";
        if (r == 7) return "SPIL + SCK ART running (golden :30839-30842)";
        if (r == 8) return "JCET: AccessLevel below LevelSet.AccessLevel[12] (golden :30864)";
    }
    return "unknown return code";
}

}  // namespace

// golden TfMain::DoFTRTClick（main.cpp:35752-35774）。回傳 FTClick／RTClick 的值（golden 是 void；多回傳只給 ack）。
//   bForceRunStartModeEnabled：golden 遠端路徑（Command.cpp ChangeHandlerStartMode :11302-11305／:11313-11316）呼叫前先
//   `cbRunStartMode->Enabled=true;`（Sam 20210715 切換模式不要卡權限）；主畫面的小方塊傳 false（照 ChangeLevelAttr 算）。
int W906_Main_DoFTRTClick(bool bIsRT, bool bIsMan, bool bForceRunStartModeEnabled)   //RogerYang 20260410 : 包起來，區分手動按下還是程式按下
{
    const bool bRsmEnabled = bForceRunStartModeEnabled || FtRtStartModeEnabled(nullptr);
    AnsiString Str;
    int iResult;
    if(bIsRT==false)    //FT
    {
        iResult=W906_Main_FTClick(bIsMan, bRsmEnabled);
        if(iResult!=0)
        {
            Str.sprintf("FTClick fail! (%d)", iResult);
            RecordProcess(Str);
        }
    }
    else                //RT
    {
        iResult=W906_Main_RTClick(bIsMan, bRsmEnabled);
        if(iResult!=0)
        {
            Str.sprintf("RTClick fail! (%d)", iResult);
            RecordProcess(Str);
        }
    }
    return iResult;
}

// WS act.main.ftrt（本檔 W906_Main_EvB6Op 同一行分派；自己持 FormLock）。*ok＝get，或 golden 的處理器有跑（含 golden 自己回非 0 的那幾條）；
//   小方塊點不到（看不見／停用，golden 沒有 OnClick）、參數錯＝false，回 {executed:false, guard, detail}。
std::string W906_Main_FtRtOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    EvB6Payload p(payloadJson);
    if (p.bad) return EvB6Refuse("bad-payload", "value 不是 JSON 物件", nullptr);
    const std::string op = p.Str("op", "get");
    const std::string tile = p.Str("tile", "");
    if (op != "get" && op != "click") return EvB6Refuse("bad-op", op, nullptr);
    if (op == "click" && tile != "FT" && tile != "RT") return EvB6Refuse("bad-payload", "tile 要是 FT 或 RT，收到 \"" + tile + "\"", nullptr);
    if (fMain == nullptr || fLotInfo == nullptr || fMain->cbRunStartMode == nullptr || fMain->palFT == nullptr || fMain->palRT == nullptr ||
        fLotInfo->cbRunMode == nullptr || fMain->cbUserSelect == nullptr || fYieldMonitoring == nullptr || fCounterClear == nullptr)
        return EvB6Refuse("forms-not-created", "fMain／fLotInfo／fYieldMonitoring／fCounterClear 或它們的元件有一個是 NULL", nullptr);
    FormLockGuard lock;
    std::string lockWhy;
    const bool rsmEnabled = FtRtStartModeEnabled(&lockWhy);
    std::string tileWhy = lockWhy;                                              // golden :13036：運轉中 ChangeLevelAttr 不改 palFT／palRT->Enabled（見 FtRtStartModeEnabled）
    const bool tileRule = SystemStart ? FtRtStartModeEnabled(&tileWhy, true) : rsmEnabled;
    const bool jcet = (CUSTOMER_CODE == CC_JCET);
    const bool ftEnabled = jcet ? true : tileRule;                               // golden :13031-13038（JCET 不改＝DFM 預設 True）
    const bool rtEnabled = ftEnabled;
    const bool ftVisible = fMain->palFT->Visible, rtVisible = fMain->palRT->Visible;
    const bool isRT = (tile == "RT");
    const char* golden = isRT ? "V912 main.cpp:30705 palRTClick -> DoFTRTClick(true, true) :35752 -> RTClick :30834"
                              : "V912 main.cpp:30700 palFTClick -> DoFTRTClick(false, true) :35752 -> FTClick :30710";
    if (op == "click") {
        if (!(isRT ? rtVisible : ftVisible))
            return EvB6Refuse("hidden", std::string("golden ") + (isRT ? "palRT" : "palFT") + "->Visible is false (FormShow :9080 [Show FT and RT Button] / "
                              "SetRunStartMode :597-601) -- the operator cannot click it", golden);
        if (!(isRT ? rtEnabled : ftEnabled))
            return EvB6Refuse("disabled", std::string("golden ChangeLevelAttr :13036-13037 ") + (isRT ? "palRT" : "palFT") +
                              "->Enabled=cbRunStartMode->Enabled, which is false now: " + tileWhy, golden);
    }
    const int rsmBefore = LastSet.iRunStartMode;
    const std::string captionBefore = fMain->cbRunStartMode->Text.c_str();
    const std::string runModeBefore = fLotInfo->cbRunMode->Text.c_str();
    const int msgBefore = W906_ShowMyMessage_Count;
    int r = -1;
    if (op == "click") {
        r = W906_Main_DoFTRTClick(isRT, true, false);                          // golden palFTClick／palRTClick
        std::printf("act.main.ftrt %s -> %s(%d)  iRunStartMode %d -> %d  cbRunStartMode '%s' -> '%s'  cbRunMode '%s' -> '%s'\n", tile.c_str(),
                    isRT ? "RTClick" : "FTClick", r, rsmBefore, LastSet.iRunStartMode, captionBefore.c_str(), fMain->cbRunStartMode->Text.c_str(),
                    runModeBefore.c_str(), fLotInfo->cbRunMode->Text.c_str());
    }
    if (ok) *ok = true;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("golden").String(golden);
    if (op == "click") {
        w.Key("tile").String(tile);
        w.Key("executed").Bool(r == 0);
        w.Key("result").Number((wb_int64)r);
        w.Key("guard").String(r == 0 ? "" : FtRtCodeText(isRT, r));
        w.Key("modeChanged").Bool(LastSet.iRunStartMode != rsmBefore);
        w.Key("iRunStartModeBefore").Number((wb_int64)rsmBefore);
        w.Key("captionBefore").String(captionBefore);
        w.Key("runModeBefore").String(runModeBefore);
        if (W906_ShowMyMessage_Count != msgBefore) w.Key("message").String(std::string(W906_ShowMyMessage_LastS1.c_str()));
        std::vector<std::string> todo;
        if (r == 0 && !(IniConfig.bShowFTandRTButtonCanClick || IniConfig.bEnable_SECS_GEM || CUSTOMER_CODE == CC_Murata))
            todo.push_back("golden changes the start mode only with [FT & RT button can click] / SECS-GEM / Murata (main.cpp:30748 / :30894) -- none is on, so only the SPIL / SECS tail ran");
        if (isRT && W906_bRTContinueNeedClearCT)
            todo.push_back("golden bRTContinueNeedClearCT=true (:30982) has no reader in the port (golden csystem.cpp:12863 not ported) -- kept in W906_bRTContinueNeedClearCT");
        EvB6Todos(w, todo);
    }
    w.Key("runStartModeEnabled").Bool(rsmEnabled);
    w.Key("runStartModeLock").String(lockWhy);
    w.Key("ftVisible").Bool(ftVisible).Key("rtVisible").Bool(rtVisible);
    w.Key("ftEnabled").Bool(ftEnabled).Key("rtEnabled").Bool(rtEnabled);
    w.Key("iRunStartMode").Number((wb_int64)LastSet.iRunStartMode);
    w.Key("caption").String(std::string(fMain->cbRunStartMode->Text.c_str()));
    w.Key("runMode").String(std::string(fLotInfo->cbRunMode->Text.c_str()));
    w.EndObject();
    return w.Str();
}
