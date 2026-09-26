// =============================================================================
//  forms/fMain_ATCSiteUse.cpp  --  TfMain::ChangeATCSiteUse（golden 本體）
//
//  AI(W906-OPMODE) 20260926: 新檔，只放這一個成員函式的本體。
//
//  來源  golden HT9011UC_Code_V3.33.906.0_20260618\main.cpp:13129-13942
//        （814 行；宣告在 golden main.h:1355，移植樹 forms/fMain.h:562）。
//        由 906 的文字翻譯：Big5 以 cp950 解碼，原作者的中文註解原樣帶過來。
//        V912（HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13652-14465）
//        的同一個函式逐行相同（去掉行尾空白後 0 行差異）—— 沒有 912 差異。
//
//  範圍  使用者 20260922 裁決「都要，全部動作都要執行，此專案就是要上線的」：
//        整個函式照翻，包含所有 ATC 命令（ATCInterfaceForm->OnLine／SetRunATC／
//        ATCChillerSwitch／OpenChannel／CloseChannel／SendCommToATC7、
//        ATC_60_SYS.SetChannelUse、fTemp_Set->ControlATC60AirFlow），不加安全閘。
//        #if 0 只用在「相依在移植樹裡不存在」的那幾句（下面 G1-G5）。
//
//  函式內的 static（golden 原樣，:13131-13137）：bFirstIn、bBackupATC[]、
//        bEnableATC[]、SITE_Ch[8]。它們活到行程結束、所有 TfMain 實例共用一份
//        （golden 只有一個 fMain）。InitialOK==false 時只清 bBackupATC／bEnableATC，
//        不重置 bFirstIn —— 照 golden。⚠ 測試多次呼叫時狀態會跨呼叫保留。
//
//  GATES（全部是 TODO(W906-OPMODE)，每一條都是缺相依，不是安全閘）
//    G1 golden :13155-13156
//         if(Tri_Temp_Machine==1 && bManualDefrost_Start == true && fTemp_Set->fShow==true) return;
//       ⇒ 主迴圈審查後改成照翻（不閘）：fTemp_Set->fShow 由 W906_FormFShow("fTemp_Set", …) 回答（成員 OR 網頁視窗總表，
//       csystem.h:440；WebStart.cpp:1945 的 START 互鎖對同一個 fTemp_Set 用同一套）。第一版整句閘掉＝永遠不提早 return，
//       而 golden 的手動除霜只能從設定頁啟動，所以 golden 在除霜期間幾乎一定會在這裡 return（審查 minor）。
//    G2 golden :13508（只在 #ifndef DEBUG_ATC 下）
//         if(ATC_InterfaceForm->IsConnect()==true)
//       全域 ATC_InterfaceForm 是 acarry_shims.h:115 的 TATC_InterfaceFormShim*，
//       只有 iATC_MODE_TYPE 一個成員；IsConnect() 只存在於
//       forms/fATCHandlerSide.h:783 的 TATC_InterfaceForm 類別，那個類別沒有全域實例。
//       ⇒ 主迴圈審查後改：條件的 #else 臂是 if(false)（第一版只閘條件行，出貨組態整塊無條件執行＝把不存在的連線當成已連線）。
//       移植樹沒有 ATC 連線（TATC_InterfaceForm::IsConnect() 回 IsConnectFlag，建構時 false，連線路徑 GATE(NET)），
//       所以「沒連線 ⇒ golden 整塊跳過」是這裡唯一可能的狀態；DEBUG_ATC 組態照 golden 不檢查、整塊執行。
//    G3 golden :13921  ATC_InterfaceForm->EnablesChannel(TriTemperature_TotalChannel, bATC_EnablesChannel);
//    G4 golden :13929  ATC_InterfaceForm->EnablesChannel(iATC_Use_Heat_Count, bUse);
//    G5 golden :13932  ATC_InterfaceForm->EnablesChannel(iATC_Use_Heat_Count, bUse);（#ifndef DEBUG_ATC）
//       同 G2 的原因：shim 沒有 EnablesChannel()（fATCHandlerSide.h:843 那個類別才有，
//       而且本體在那裡也是 GATE (NET/WIDGET)）。bATC_EnablesChannel[] 的備份照寫，
//       只有「送給新 ATC 系統開關頻道」的命令沒有送出。
//    搜尋方式：Grep 工具掃移植樹 *.{h,cpp}（IsConnect／EnablesChannel／
//       ATC_InterfaceForm／TATC_InterfaceForm\s*\*），全域實例只有 acarry_shims.cpp:73 的 shim。
//
//  照 golden 保留、沒有改的 golden 行為（只回報）
//    * :13916-13919  for(int i=32; i<TriTemperature_TotalChannel; i++) bATC_EnablesChannel[i]=...
//      bATC_EnablesChannel 在 golden 與移植樹都宣告成 [32]（golden cmydef.h:4093／
//      移植 cmydef.h:4109）；TriTemperature_TotalChannel 由 Gerneral.ini [System]
//      讀入（golden database.cpp:1512）。它大於 32 時是越界寫入 —— golden 本身的缺陷。
//    * :13335  SendCommToATC7(ATC_SET_TEMP, Temperature.fWorkTemperBase, "") 把 double
//      傳給 AnsiString 參數，靠 AnsiString(double)（vclcompat/AnsiString.h:77），同 BCB6。
//    * :13806  NN_2Row 迴圈外層用 FTestSuck.iShtCol 當列數上限（不是 iShtRow）。
//    * :13327-13328  S／szDir 只寫不讀（照 golden 保留）。
//
//  執行期相依（不是閘，是風險）
//    fTemp_Set 是 uTemp_Set.cpp:224 的裸指標（沒有初值），只有 tools/wb_serve.cpp:3111
//    會 new。其他執行檔走到 eNewATCSystem 分支的 iSiteToATC 或 :13940 的
//    ControlATC60AirFlow 會解參考 NULL —— golden 同樣不檢查，照翻。
//
//  連結  本檔尚未加進 CMakeLists.txt（整併時處理）。它用到 ht9045_sm 的
//    ATCInterfaceForm（ATC/ATCInterface.cpp）、ATC_InterfaceForm（acarry_shims.cpp）、
//    fTemp_Set（uTemp_Set.cpp）、IsNNMode（cinitial.cpp）、TestSocket／FTestSuck，
//    以及 ht9045_core 的 GetLastOpenFN／GetRecipeFileName（common.cpp）。
//    forms/fMain.cpp 所在的 ht9045_forms 不連 ht9045_sm，要放哪個 library 由整併決定。
// =============================================================================
#include "forms/fMain.h"            // TfMain（fMain.h:562 宣告 ChangeATCSiteUse）
#include "csystem.h"          // W906_FormFShow（G1）
#include "MachineType.h"            // ATC_HEAD_COUNT、DEBUG_ATC（SOFT_SIMULTE 組態）、eATC*／eTestMode／tc*／NN_1Row／NN_2Row
#include "cmydef.h"                 // InitialOK、Tri_Temp_Machine、bDefrostKeepATCTemp、bManualDefrost_Start、ATC_SYSTEM、bRunATC、
                                    // bTestSiteUse、bATC_EnablesChannel、iATC_Use_Heat_Count、TriTemperature_TotalChannel、
                                    // bUT150Install、iTriggerBoostFunction、Tempture_Hot
#include "cprod.h"                  // Temperature、TestIF、TestIF_File
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction.bEnable_1x3Kit
#include "LastSet.h"                // LastSet.iTemperature／bUseTestSocket
#include "common.h"                 // GetLastOpenFN、GetRecipeFileName
#include "cinitial.h"               // IsNNMode（golden cinitial.h:60）
#include "aHotPlateSubstrate.h"     // TestSocket、FTestSuck（經 mykitsuck.h，golden MyKitSuck.h）
#include "ATC/ATCInterface.h"       // ATCInterfaceForm（「舊」ATC 介面）、ATC_RUN／ATC_STOP／ATC_SET_TEMP／ATC_CH_ENABLED
#include "acarry_shims.h"           // ATC_InterfaceForm（「新」ATC 介面的 shim，只有 iATC_MODE_TYPE）
#include "forms/fATCHandlerSide.h"  // ATC_TYPE_60（golden ATC_Handler_Side.h 的 #define）
#include "forms/fTemp_Set.h"        // fTemp_Set（iSiteToATC、ControlATC60AirFlow、fShow）
#include "forms/fLotInfo.h"         // fLotInfo->aldATCPower

//------------------------------------------------------------------------------
void TfMain::ChangeATCSiteUse()                                                 //Steven 20120523 : ATC
{
    static bool bFirstIn=true;                                                  //Steven 20120719 : ATC加入保護, 不要重複送出開關Site命令
    static bool bBackupATC[ATC_HEAD_COUNT];
    static bool bEnableATC[ATC_HEAD_COUNT];
    bool bUse[ATC_HEAD_COUNT];
    bool bUseArm1PnPMode=false;                                                 //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
    static int SITE_Ch[8] = {tcShuttle1, tcShuttle2, tcHotPlate1, tcHotPlate2,  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                             tcShuttle3, tcShuttle4, tcHotPlate3, tcHotPlate4};

    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true)
    {
        bUseArm1PnPMode=true;                                                   //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
    }

    if(InitialOK==false)
    {
        ZeroMemory(bBackupATC, sizeof(bBackupATC));
        ZeroMemory(bEnableATC, sizeof(bEnableATC));
        return;
    }
    ZeroMemory(bUse, sizeof(bUse));

    if(Tri_Temp_Machine==1 && bDefrostKeepATCTemp==true)                        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        return;

    //AI(W906-OPMODE) 20260926: 照翻（主迴圈審查後改）。fTemp_Set->fShow 在這個架構裡由 W906_FormFShow 回答＝成員值 OR 網頁視窗總表
    //   （csystem.h:440；wb_serve 由 WebMotorAccessLive.cpp:1196 裝 W906_FormFShowHook）—— WebStart.cpp:1945 的 START 互鎖對同一個 fTemp_Set 用的就是這一套。
    //   golden 只有溫控設定頁的兩顆除霜鈕會把 bManualDefrost_Start 設成 true（uTemp_Set.cpp btnDefrostStartClick／btn_DefrostAllUseStartClick），
    //   所以手動除霜期間 golden 幾乎一定在這裡提早 return（審查指出第一版整句閘掉＝永遠不 return）。
    if(Tri_Temp_Machine==1 && bManualDefrost_Start == true && W906_FormFShow("fTemp_Set", fTemp_Set->fShow)==true)   //AI(W906-OPMODE) 20260926: golden `fTemp_Set->fShow==true`  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        return;
    // 2011.04.22 , Joye , ATC ---------------------
    if(ATC_SYSTEM==eATCHonPrecType &&
       (TestIF_File.iTestMode==DualSite ||
        TestIF_File.iTestMode==SingleSite))                                     //Steven 20120410 : Hontech ATC
    {
        if(Temperature.bATCActiveCooling==true)
        {
            if(bFirstIn)
            {
                ATCInterfaceForm->OnLine();
                ATCInterfaceForm->SetRunATC(true);
                ATCInterfaceForm->ATCChillerSwitch(true);
                bRunATC=true;                                                   //ChungHung 20160118 add for Hisi V102
            }

            if(IniConfig.bA09_ByArmCloseSite==false)                            //ChungHung 20130910 alter for SCK can close site by Index start
            {
                if(Temperature.bATCActiveCooling==true &&
                   (bTestSiteUse[0][0][0]==true ||
                    bTestSiteUse[1][0][0]==true))                               //JerryYang 20171017 (Steven) 修正ATC2.0 只開arm2不會讀取溫度的問題
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       TestIF_File.iShuttleMode==1)
                    {
                        if(TestIF_File.iShuttle_Sel==0)                         //Front Arm Only
                        {
                            bEnableATC[0]=true;
                            bEnableATC[2]=false;
                        }
                        else if(TestIF_File.iShuttle_Sel==1)                    //Rear Arm Only
                        {
                            bEnableATC[0]=false;
                            bEnableATC[2]=true;
                        }
                    }
                    else
                    {
                        bEnableATC[0]=true;
                        bEnableATC[2]=true;
                    }
                }
                else
                {
                    bEnableATC[0]=false;
                    bEnableATC[2]=false;
                }

                if(Temperature.bATCActiveCooling==true &&
                   (bTestSiteUse[0][0][1]==true ||
                    bTestSiteUse[1][0][1]==true) &&
                   TestIF_File.iTestMode==DualSite)                             //JerryYang 20171017 (Steven) 修正ATC2.0 只開arm2不會讀取溫度的問題
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       TestIF_File.iShuttleMode==1)
                    {
                        if(TestIF_File.iShuttle_Sel==0)                         //Front Arm Only
                        {
                            bEnableATC[1]=true;
                            bEnableATC[3]=false;
                        }
                        else if(TestIF_File.iShuttle_Sel==1)                    //Rear Arm Only
                        {
                            bEnableATC[1]=false;
                            bEnableATC[3]=true;
                        }
                    }
                    else
                    {
                        bEnableATC[1]=true;
                        bEnableATC[3]=true;
                    }
                }
                else
                {
                    bEnableATC[1]=false;
                    bEnableATC[3]=false;
                }
            }
            else
            {
                if(Temperature.bATCActiveCooling==true)
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       TestIF_File.iShuttleMode==1)
                    {
                        if(TestIF_File.iShuttle_Sel==0)                         //Front Arm Only
                        {
                            bEnableATC[0]=bTestSiteUse[0][0][0];
                            bEnableATC[2]=false;
                        }
                        else if(TestIF_File.iShuttle_Sel==1)                    //Rear Arm Only
                        {
                            bEnableATC[0]=false;
                            bEnableATC[2]=bTestSiteUse[1][0][0];
                        }
                    }
                    else
                    {
                        bEnableATC[0]=bTestSiteUse[0][0][0];
                        bEnableATC[2]=bTestSiteUse[1][0][0];
                    }
                }
                else
                {
                    bEnableATC[0]=false;
                    bEnableATC[2]=false;
                }

                if(Temperature.bATCActiveCooling==true &&
                   TestIF_File.iTestMode==DualSite)
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       TestIF_File.iShuttleMode==1)
                    {
                        if(TestIF_File.iShuttle_Sel==0)                         //Front Arm Only
                        {
                            bEnableATC[1]=bTestSiteUse[0][0][1];
                            bEnableATC[3]=false;
                        }
                        else if(TestIF_File.iShuttle_Sel==1)                    //Rear Arm Only
                        {
                            bEnableATC[1]=false;
                            bEnableATC[3]=bTestSiteUse[1][0][1];
                        }
                    }
                    else
                    {
                        bEnableATC[1]=bTestSiteUse[0][0][1];
                        bEnableATC[3]=bTestSiteUse[1][0][1];
                    }
                }
                else
                {
                    bEnableATC[1]=false;
                    bEnableATC[3]=false;
                }
            }
            //ChungHung 20130910 alter for SCK can close site by Index end
            // ---------------------------------------------------------
            if(bFirstIn)
            {
                for(int i=0; i<4; i++)
                {
                    bBackupATC[i]=!bEnableATC[i];
                }
                bFirstIn=false;
            }

            if(ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect->Value==true)
            {
                for(int i=0; i<4; i++)
                {
                    if(bBackupATC[i]!=bEnableATC[i])
                    {
                        if(bEnableATC[i])
                            ATCInterfaceForm->OpenChannel(i);
                        else
                            ATCInterfaceForm->CloseChannel(i);
                        bBackupATC[i]=bEnableATC[i];
                    }
                }
            }
            //Ifor 20160602 備份 ATC 2.0 System Site Setting Status
            for(int i=0; i<ATC_HEAD_COUNT; i++)
            {
                bATC_EnablesChannel[i]=bEnableATC[i];
            }
        }
        else if(Temperature.bATC70Active==true)                                 //Ifor 20160317 Add ATC7.0 Change Site Use
        {
            AnsiString S=GetLastOpenFN();
            AnsiString szDir="";
            AnsiString asData1="", asData2="";

            if(LastSet.iTemperature==Tempture_Hot)                              //Steven 20151111 : For ATC7.0 修改判斷式
            {
                if(fLotInfo->aldATCPower->Value==true)
                {
                    ATCInterfaceForm->SendCommToATC7(ATC_SET_TEMP, Temperature.fWorkTemperBase, "");
                    ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");
                }
            }

            szDir=GetRecipeFileName("Temperature.Data");
            if(TestIF_File.iShuttleMode==0)                                     //Dual Shuttle
            {
                switch(TestIF_File.iTestMode)
                {
                    case DualSite:                                              //1x2    //Ifor 20160314 修正ATC 7.0 Dual Shuttle 關Site 異常
                        Temperature.bATC7ChannelEnabled[0] = bTestSiteUse[0][0][0];                                     //Ifor 20160829 Fix 重複寫擋
                        Temperature.bATC7ChannelEnabled[1] = bTestSiteUse[0][0][1];
                        Temperature.bATC7ChannelEnabled[2] = bTestSiteUse[1][0][0];
                        Temperature.bATC7ChannelEnabled[3] = bTestSiteUse[1][0][1];
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
                            Temperature.bATC7ChannelEnabled[0] = bTestSiteUse[0][0][0];                                 //Ifor 20160829 Fix 重複寫擋
                            Temperature.bATC7ChannelEnabled[1] = bTestSiteUse[0][0][1];
                            Temperature.bATC7ChannelEnabled[2] = false;
                            Temperature.bATC7ChannelEnabled[3] = false;
                        }
                        else if(TestIF_File.iShuttle_Sel==1)
                        {
                            Temperature.bATC7ChannelEnabled[0] = false;         //Ifor 20160829 Fix 重複寫擋
                            Temperature.bATC7ChannelEnabled[1] = false;
                            Temperature.bATC7ChannelEnabled[2] = bTestSiteUse[1][0][0];
                            Temperature.bATC7ChannelEnabled[3] = bTestSiteUse[1][0][1];
                        }
                        break;
                }
            }

            asData1.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[0], Temperature.bATC7ChannelEnabled[1]);
            asData2.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[2], Temperature.bATC7ChannelEnabled[3]);
            if(fLotInfo->aldATCPower->Value==true)
            {
                ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, asData1, asData2);
            }
        }
        else
        {
            if(fLotInfo->aldATCPower->Value==true)
            {
                ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
            }
            else
            {
                bFirstIn=true;
                ATCInterfaceForm->SetRunATC(false);
                bRunATC=false;                                                  //ChungHung 20160118 add for Hisi V102
                ATCInterfaceForm->iStopATCChillerType=1;
            }
        }
    }
    else
    {
        if(ATC_SYSTEM==eATCHonPrecType)
        {
            if(ATCInterfaceForm->IsOnLine()==true)
            {
                ATCInterfaceForm->SetRunATC(false);
                ATCInterfaceForm->iStopATCChillerType=1;
                bRunATC=false;                                                  //ChungHung 20160118 add for Hisi V102
            }
            else if(fLotInfo->aldATCPower->Value==true)
            {
                ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
            }
        }
    }

    AnsiString sATC_1="", sATC_2="";
    if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    {
        if(IniConfig.bA09_ByArmCloseSite==false)
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(bTestSiteUse[0][i][j])
                    {                                                           //Ifor 20151028 修正ATC3.0 開關Site異常問題
                        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                                         //Index 1
                            sATC_1+=",1";
                        else
                            sATC_1+=",0";

                        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                                         //Index 2
                            sATC_2+=",1";
                        else
                            sATC_2+=",0";
                    }
                    else
                    {
                        sATC_1+=",0";
                        sATC_2+=",0";
                    }
                }
            }
            sATC_1+=sATC_2;
            ATCInterfaceForm->ATC_60_SYS.SetChannelUse(sATC_1, 16);
        }
        else
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(TestIF_File.iShuttleMode==0)
                    {
                        if(bTestSiteUse[0][i][j])
                        {
                            sATC_1+=",1";
                        }
                        else
                        {
                            sATC_1+=",0";
                        }

                        if(bTestSiteUse[1][i][j])
                        {
                            sATC_2+=",1";
                        }
                        else
                        {
                            sATC_2+=",0";
                        }
                    }
                    else
                    {
                        if(TestIF_File.iShuttle_Sel==0)
                        {
                            if(bTestSiteUse[0][i][j])
                            {
                                sATC_1+=",1";
                            }
                            else
                            {
                                sATC_1+=",0";
                            }
                            sATC_2+=",0";
                        }
                        else
                        {
                            if(bTestSiteUse[1][i][j])
                            {
                                sATC_2+=",1";
                            }
                            else
                            {
                                sATC_2+=",0";
                            }
                            sATC_1+=",0";
                        }
                    }
                }
            }
            sATC_1+=sATC_2;
            ATCInterfaceForm->ATC_60_SYS.SetChannelUse(sATC_1, 16);
        }
    }
    else if(ATC_SYSTEM==eNewATCSystem)                                          //Ifor 20151230 :add New ATC Interface Change ATC Site Use
    {
        #ifndef DEBUG_ATC
        //AI(W906-OPMODE) 20260926: 全域 ATC_InterfaceForm 是 shim（acarry_shims.h:115），沒有 IsConnect() ⇒ 條件換成「沒連線」（#else 那一臂；見檔頭 G2）
#if 0 // TODO(W906-OPMODE): ATC_InterfaceForm->IsConnect() missing -- global is TATC_InterfaceFormShim* (acarry_shims.h:115, only iATC_MODE_TYPE); IsConnect exists only on forms/fATCHandlerSide.h:783 TATC_InterfaceForm, which has no global instance (Grep *.{h,cpp}) -- golden main.cpp:13508
        if(ATC_InterfaceForm->IsConnect()==true)                                //Ifor 20160503 尚未與atc系統連線 不設定 atc site use
#else
        if(false)   //AI(W906-OPMODE) 20260926: 主迴圈審查後改 —— 第一版只閘條件，出貨組態因此整塊無條件執行。移植樹沒有 ATC 連線（TATC_InterfaceForm::IsConnect() 回 IsConnectFlag，建構時 false、連線路徑 GATE(NET)，forms/fATCHandlerSide.cpp:176／:355），這裡唯一可能的值就是「沒連線」⇒ golden 整塊跳過；保留區塊讓它照樣被編譯檢查
#endif
        #endif
        {
            //Ifor 20160513 修改ATC ATC Site Mapping 方向
            //      ARM1                    ARM2
            //0 2 4 6 8 10 12 14    16 18 20 22 24 26 28 30
            //1 3 5 7 9 11 13 15    17 19 21 23 25 27 29 31
            bool bCheck32Heater=false;

            if((TestIF_File.iTestMode==_12Site2X6    ||
                TestIF_File.iTestMode==_10Site2X5    ||                         //wei 20190614 10 site
                TestIF_File.iTestMode==_16Site2X8    ||
                TestIF_File.iTestMode==_16Site4X4    ||                         //kevin 20190604 add
                TestIF_File.iTestMode==_32Site4X8N)  &&
                TestIF_File.bUse32Heater==false)
            {
                bCheck32Heater=true;
            }

            if(bCheck32Heater==true)                                            //Ifor 20170814 (wei) add HT9045AT Kit 1對2 加熱
            {
                int iATCUseSite=iATC_Use_Heat_Count/2;
                if(IniConfig.bL17HeadHeaterOnWhenCloseSite)                     //kevin 20190628 add  關site need add Hot
                {
                    if(TestIF_File.iTestMode==_12Site2X6 ||
                       TestIF_File.iTestMode==_10Site2X5)
                    {
                        bUse[0]=true;
                        bUse[1]=true;
                        bUse[2]=true;
                        bUse[3]=true;
                        bUse[4]=true;
                        bUse[5]=true;

                        if(bUseArm1PnPMode==true)
                        {
                            bUse[iATCUseSite]   = bUse[0];
                            bUse[iATCUseSite+1] = bUse[1];
                            bUse[iATCUseSite+2] = bUse[2];
                            bUse[iATCUseSite+3] = bUse[3];
                            bUse[iATCUseSite+4] = bUse[4];
                            bUse[iATCUseSite+5] = bUse[5];
                        }
                        else
                        {
                            bUse[iATCUseSite]   = true;
                            bUse[iATCUseSite+1] = true;
                            bUse[iATCUseSite+2] = true;
                            bUse[iATCUseSite+3] = true;
                            bUse[iATCUseSite+4] = true;
                            bUse[iATCUseSite+5] = true;
                        }
                    }
                    else if(TestIF_File.iTestMode==_16Site2X8 ||
                            TestIF_File.iTestMode==_32Site4X8N ||
                            TestIF_File.iTestMode==_16Site4X4 )                 //kevin 20190604 add
                    {
                        bUse[0]=true;
                        bUse[1]=true;
                        bUse[2]=true;
                        bUse[3]=true;
                        bUse[4]=true;
                        bUse[5]=true;
                        bUse[6]=true;
                        bUse[7]=true;

                        if(bUseArm1PnPMode==true)
                        {
                            bUse[iATCUseSite]   = bUse[0];
                            bUse[iATCUseSite+1] = bUse[1];
                            bUse[iATCUseSite+2] = bUse[2];
                            bUse[iATCUseSite+3] = bUse[3];
                            bUse[iATCUseSite+4] = bUse[4];
                            bUse[iATCUseSite+5] = bUse[5];
                            bUse[iATCUseSite+6] = bUse[6];
                            bUse[iATCUseSite+7] = bUse[7];
                        }
                        else
                        {
                            bUse[iATCUseSite]   = true;
                            bUse[iATCUseSite+1] = true;
                            bUse[iATCUseSite+2] = true;
                            bUse[iATCUseSite+3] = true;
                            bUse[iATCUseSite+4] = true;
                            bUse[iATCUseSite+5] = true;
                            bUse[iATCUseSite+6] = true;
                            bUse[iATCUseSite+7] = true;
                        }
                    }
                }
                else
                {
                    if(TestIF_File.iTestMode==_12Site2X6 ||
                       TestIF_File.iTestMode==_10Site2X5)
                    {
                        bUse[0]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1]);
                        bUse[1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1]);
                        bUse[2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3]);
                        bUse[3]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3]);
                        bUse[4]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5]);
                        bUse[5]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5]);

                        if(bUseArm1PnPMode==true)
                        {
                            bUse[iATCUseSite]   = bUse[0];
                            bUse[iATCUseSite+1] = bUse[1];
                            bUse[iATCUseSite+2] = bUse[2];
                            bUse[iATCUseSite+3] = bUse[3];
                            bUse[iATCUseSite+4] = bUse[4];
                            bUse[iATCUseSite+5] = bUse[5];
                        }
                        else
                        {
                            bUse[iATCUseSite]   =(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1]);
                            bUse[iATCUseSite+1] =(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1]);
                            bUse[iATCUseSite+2] =(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3]);
                            bUse[iATCUseSite+3] =(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3]);
                            bUse[iATCUseSite+4] =(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5]);
                            bUse[iATCUseSite+5] =(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5]);
                        }
                    }
                    else if(TestIF_File.iTestMode==_16Site2X8 ||
                            TestIF_File.iTestMode==_32Site4X8N ||
                            TestIF_File.iTestMode==_16Site4X4)                  //kevin 20190604 add
                    {
                       if(ATC_InterfaceForm->iATC_MODE_TYPE==61)                //Ztex 2023.12.31 for HT-1032 AT
                        {
                            if(TestIF.iTestMode==_16Site4X4)
                            {
                                bUse[0]             =LastSet.bUseTestSocket[0][2][0];
                                bUse[1]             =LastSet.bUseTestSocket[0][3][0];
                                bUse[2]             =LastSet.bUseTestSocket[0][2][1];
                                bUse[3]             =LastSet.bUseTestSocket[0][3][1];
                                bUse[4]             =LastSet.bUseTestSocket[0][2][2];
                                bUse[5]             =LastSet.bUseTestSocket[0][3][2];
                                bUse[6]             =LastSet.bUseTestSocket[0][2][3];
                                bUse[7]             =LastSet.bUseTestSocket[0][3][3];

                                bUse[16]            =LastSet.bUseTestSocket[0][0][0];
                                bUse[17]            =LastSet.bUseTestSocket[0][1][0];
                                bUse[18]            =LastSet.bUseTestSocket[0][0][1];
                                bUse[19]            =LastSet.bUseTestSocket[0][1][1];
                                bUse[20]            =LastSet.bUseTestSocket[0][0][2];
                                bUse[21]            =LastSet.bUseTestSocket[0][1][2];
                                bUse[22]            =LastSet.bUseTestSocket[0][0][3];
                                bUse[23]            =LastSet.bUseTestSocket[0][1][3];
                            }
                            else if(TestIF.iTestMode==_16Site2X8)
                            {
                                bUse[0]             =LastSet.bUseTestSocket[0][0][0];
                                bUse[1]             =LastSet.bUseTestSocket[0][1][0];
                                bUse[2]             =LastSet.bUseTestSocket[0][0][1];
                                bUse[3]             =LastSet.bUseTestSocket[0][1][1];
                                bUse[4]             =LastSet.bUseTestSocket[0][0][2];
                                bUse[5]             =LastSet.bUseTestSocket[0][1][2];
                                bUse[6]             =LastSet.bUseTestSocket[0][0][3];
                                bUse[7]             =LastSet.bUseTestSocket[0][1][3];

                                bUse[8]             =LastSet.bUseTestSocket[0][0][4];
                                bUse[9]             =LastSet.bUseTestSocket[0][1][4];
                                bUse[10]            =LastSet.bUseTestSocket[0][0][5];
                                bUse[11]            =LastSet.bUseTestSocket[0][1][5];
                                bUse[12]            =LastSet.bUseTestSocket[0][0][6];
                                bUse[13]            =LastSet.bUseTestSocket[0][1][6];
                                bUse[14]            =LastSet.bUseTestSocket[0][0][7];
                                bUse[15]            =LastSet.bUseTestSocket[0][1][7];

                                bUse[16]            =LastSet.bUseTestSocket[1][0][0];
                                bUse[17]            =LastSet.bUseTestSocket[1][1][0];
                                bUse[18]            =LastSet.bUseTestSocket[1][0][1];
                                bUse[19]            =LastSet.bUseTestSocket[1][1][1];
                                bUse[20]            =LastSet.bUseTestSocket[1][0][2];
                                bUse[21]            =LastSet.bUseTestSocket[1][1][2];
                                bUse[22]            =LastSet.bUseTestSocket[1][0][3];
                                bUse[23]            =LastSet.bUseTestSocket[1][1][3];

                                bUse[24]            =LastSet.bUseTestSocket[1][0][4];
                                bUse[25]            =LastSet.bUseTestSocket[1][1][4];
                                bUse[26]            =LastSet.bUseTestSocket[1][0][5];
                                bUse[27]            =LastSet.bUseTestSocket[1][1][5];
                                bUse[28]            =LastSet.bUseTestSocket[1][0][6];
                                bUse[29]            =LastSet.bUseTestSocket[1][1][6];
                                bUse[30]            =LastSet.bUseTestSocket[1][0][7];
                                bUse[31]            =LastSet.bUseTestSocket[1][1][7];
                            }
                        }
                        else
                        {
                            bUse[0]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1]);
                            bUse[1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1]);
                            bUse[2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3]);
                            bUse[3]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3]);
                            bUse[4]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5]);
                            bUse[5]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5]);
                            bUse[6]=(LastSet.bUseTestSocket[0][0][6] || LastSet.bUseTestSocket[0][0][7]);
                            bUse[7]=(LastSet.bUseTestSocket[0][1][6] || LastSet.bUseTestSocket[0][1][7]);

                            if(bUseArm1PnPMode==true)
                            {
                                bUse[iATCUseSite]   = bUse[0];
                                bUse[iATCUseSite+1] = bUse[1];
                                bUse[iATCUseSite+2] = bUse[2];
                                bUse[iATCUseSite+3] = bUse[3];
                                bUse[iATCUseSite+4] = bUse[4];
                                bUse[iATCUseSite+5] = bUse[5];
                                bUse[iATCUseSite+6] = bUse[6];
                                bUse[iATCUseSite+7] = bUse[7];
                            }
                            else
                            {
                                bUse[iATCUseSite]   =(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1]);
                                bUse[iATCUseSite+1] =(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1]);
                                bUse[iATCUseSite+2] =(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3]);
                                bUse[iATCUseSite+3] =(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3]);
                                bUse[iATCUseSite+4] =(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5]);
                                bUse[iATCUseSite+5] =(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5]);
                                bUse[iATCUseSite+6] =(LastSet.bUseTestSocket[1][0][6] || LastSet.bUseTestSocket[1][0][7]);
                                bUse[iATCUseSite+7] =(LastSet.bUseTestSocket[1][1][6] || LastSet.bUseTestSocket[1][1][7]);
                            }
                        }
                    }
                }
            }
            else if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)                                       //wei 20240617 Multi Zone
            {
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(bTestSiteUse[0][i][j])
                        {
                            if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                                     //Index 1
                            {
                                for(int k=0; k<4; k++)
                                {
                                    if(Temperature.bZoneTempEnable[k])
                                    {
                                        bUse[k]=true;
                                    }
                                    else
                                    {
                                        bUse[k]=false;
                                    }
                                }
                            }
                        }

                        if(bTestSiteUse[1][i][j])
                        {
                            if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                                     //Index 2
                            {
                                for(int k=0; k<4; k++)
                                {
                                    if(Temperature.bZoneTempEnable[k])
                                    {
                                        bUse[k+iATC_Use_Heat_Count/2]=true;
                                    }
                                    else
                                    {
                                        bUse[k+iATC_Use_Heat_Count/2]=false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if(IsNNMode()==NN_1Row)                                         //Steven 20241126 : 修正ATC溫度顯示
                {
                    if(CosFunction.bEnable_1x3Kit && TestIF_File.bUse1x3SiteKit)                                        //KevinCheng 20260109 : 頻道顯示
                    {
                        bUse[0]             =LastSet.bUseTestSocket[0][0][0];
                        bUse[4]             =LastSet.bUseTestSocket[0][0][1];
                        bUse[16]             =LastSet.bUseTestSocket[1][0][0];
                        bUse[20]             =LastSet.bUseTestSocket[1][0][1];
                    }
                    else
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bTestSiteUse[0][0][j])
                            {
                                if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                                 //Index 1
                                    bUse[fTemp_Set->iSiteToATC[0][0][j]]=true;
                            }

                            if(bTestSiteUse[1][0][j])
                            {
                                if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                                 //Index 2
                                    bUse[fTemp_Set->iSiteToATC[1][0][j]]=true;
                            }
                        }
                    }
                }
                else if(IsNNMode()==NN_2Row)
                {
                    for(int i=0; i<FTestSuck.iShtCol; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bTestSiteUse[0][i][j])
                            {
                                if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                                 //Index 1
                                    bUse[fTemp_Set->iSiteToATC[0][i][j]]=true;
                            }

                            if(bTestSiteUse[1][i][j])
                            {
                               if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                                  //Index 2
                                    bUse[fTemp_Set->iSiteToATC[1][i][j]]=true;
                            }
                        }
                    }
                }
                else if(TestIF_File.iTestMode==DualSite && TestIF_File.bUse1x3SiteKit && CosFunction.bEnable_1x3Kit)    //KevinCheng 20260109 : 頻道顯示
                {
                    bUse[0]             =LastSet.bUseTestSocket[0][0][0];
                    bUse[4]             =LastSet.bUseTestSocket[0][0][1];
                    bUse[16]             =LastSet.bUseTestSocket[1][0][0];
                    bUse[20]            =LastSet.bUseTestSocket[1][0][1];
                }
                else
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(IniConfig.bL17HeadHeaterOnWhenCloseSite)         //kevin 20190628 add  關site need add Hot
                            {
                                if(TestIF_File.iTestMode==_8Site2X4 && TestIF_File.bOctal_16Kit)                        //JerryYang 20230204 : 修正8 site mode使用16site SLK溫度offset異常
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                             //Index 1
                                        bUse[fTemp_Set->iSiteToATC[0][i][j+2]]=true;

                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                             //Index 2
                                        bUse[fTemp_Set->iSiteToATC[1][i][j+2]]=true;
                                }
                                else
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                             //Index 1
                                        bUse[fTemp_Set->iSiteToATC[0][i][j]]=true;

                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                             //Index 2
                                        bUse[fTemp_Set->iSiteToATC[1][i][j]]=true;
                                }
                            }
                            else if(TestIF_File.iTestMode==_8Site2X4 && TestIF_File.bOctal_16Kit)                       //JerryYang 20230204 : 修正8 site mode使用16site SLK溫度offset異常
                            {
                                if(bTestSiteUse[0][i][j])
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                             //Index 1
                                        bUse[fTemp_Set->iSiteToATC[0][i][j+2]]=true;
                                }

                                if(bTestSiteUse[1][i][j])
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                             //Index 2
                                        bUse[fTemp_Set->iSiteToATC[1][i][j+2]]=true;
                                }
                            }
                            else
                            {
                                if(bTestSiteUse[0][i][j])
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                             //Index 1
                                        bUse[fTemp_Set->iSiteToATC[0][i][j]]=true;
                                }

                                if(bTestSiteUse[1][i][j])
                                {
                                    if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) ||
                                       TestIF_File.iShuttleMode==0 ||
                                       bUseArm1PnPMode==true)                   //Index 2
                                    {
                                        bUse[fTemp_Set->iSiteToATC[1][i][j]]=true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(bUseArm1PnPMode==true)                                           //Ifor 20190815 : Arm1 Pick Place 無安裝加熱片 不加熱
            {
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        bUse[fTemp_Set->iSiteToATC[1][i][j]]=bUse[fTemp_Set->iSiteToATC[0][i][j]];

                        if(TestIF_File.bArm1UseHeat==false)
                        {
                            bUse[fTemp_Set->iSiteToATC[0][i][j]]=false;
                        }
                    }
                }
            }

            if(Tri_Temp_Machine==1)                                             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                for(int i=0; i<iATC_Use_Heat_Count; i++)
                {
                    bATC_EnablesChannel[i]=bUse[i];
                }

                //AI(W906-OPMODE) 20260926: 照 golden —— bATC_EnablesChannel 只有 [32]（cmydef.h:4109），TriTemperature_TotalChannel>32 時下面是越界寫入；golden 缺陷，只回報不改（見檔頭）
                for(int i=32; i<TriTemperature_TotalChannel; i++)
                {
                    bATC_EnablesChannel[i]=bUT150Install[SITE_Ch[i-32]];
                }

                //AI(W906-OPMODE) 20260926: 閘掉送給新 ATC 系統的 EnablesChannel —— shim 沒有這個成員；bATC_EnablesChannel[] 的備份照寫（見檔頭 G3）
#if 0 // TODO(W906-OPMODE): ATC_InterfaceForm->EnablesChannel() missing -- global is TATC_InterfaceFormShim* (acarry_shims.h:115, only iATC_MODE_TYPE); EnablesChannel exists only on forms/fATCHandlerSide.h:843 TATC_InterfaceForm (no global instance, body GATE NET/WIDGET) -- golden main.cpp:13921
                ATC_InterfaceForm->EnablesChannel(TriTemperature_TotalChannel, bATC_EnablesChannel);
#endif
            }
            else
            {
                for(int i=0; i<iATC_Use_Heat_Count; i++)                        //Ifor 20160513 備份 New ATC System Site Setting Status
                {
                    bATC_EnablesChannel[i]=bUse[i];
                }
                //AI(W906-OPMODE) 20260926: 閘掉送給新 ATC 系統的 EnablesChannel —— shim 沒有這個成員；bATC_EnablesChannel[] 的備份照寫（見檔頭 G4）
#if 0 // TODO(W906-OPMODE): ATC_InterfaceForm->EnablesChannel() missing -- global is TATC_InterfaceFormShim* (acarry_shims.h:115, only iATC_MODE_TYPE); EnablesChannel exists only on forms/fATCHandlerSide.h:843 TATC_InterfaceForm (no global instance, body GATE NET/WIDGET) -- golden main.cpp:13929
                ATC_InterfaceForm->EnablesChannel(iATC_Use_Heat_Count, bUse);
#endif
            }
            #ifndef DEBUG_ATC
            //AI(W906-OPMODE) 20260926: 閘掉送給新 ATC 系統的 EnablesChannel —— shim 沒有這個成員；bATC_EnablesChannel[] 的備份照寫（見檔頭 G5）
#if 0 // TODO(W906-OPMODE): ATC_InterfaceForm->EnablesChannel() missing -- global is TATC_InterfaceFormShim* (acarry_shims.h:115, only iATC_MODE_TYPE); EnablesChannel exists only on forms/fATCHandlerSide.h:843 TATC_InterfaceForm (no global instance, body GATE NET/WIDGET) -- golden main.cpp:13932
            ATC_InterfaceForm->EnablesChannel(iATC_Use_Heat_Count, bUse);
#endif
            #endif
        }
    }

    if(Temperature.bLBTempFunction && iTriggerBoostFunction==-1)
    {
        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
            fTemp_Set->ControlATC60AirFlow(0);
    }
}
//------------------------------------------------------------------------------
// =============================================================================
//  AI(W906-ATC1) 20260926: golden TfMain::FormShow main.cpp:9357-9361（逐行搬）—— NB2 R72 ATC-1／R73 抓到的當機
//
//  golden 開機在 InitialHandler（:9559）之前做這兩件事；移植樹從來沒做：
//    * InitialATC 全樹唯一的呼叫點（ATC/ATCInterface.cpp:233）在 #ifndef HANDLER_CONTROL_ATC 裡，而 MachineType.h:12 無條件定義它 ⇒ 死碼；
//      唯一的 ATC_SYS_PAL.push_back 在 InitialATC 裡（ATCInterface.cpp:341）⇒ 這個 vector 永遠是空的。
//    * ATCInterfaceForm->ATCIniPath 也從來沒設（forms/fATCHandlerSide.cpp:610 自己註明不碰無底線那一個）。
//  0926 OPMODE（7304dcef）把 ChangeATCSiteUse 從空替身換成本體、75b87a88 又接上 HOME 與三溫機 ⇒ USE_ATC_MODE=4（Hontech ATC）、
//  測試模式 Dual／Single 的機台，InitialOK 之後第一次換配方／登入／切運轉模式／按 HOME，就在 SetRunATC（ATCInterface.cpp:1134）或
//  OnLine→SetChillerTemperature（:533）對空 vector 取 [0] ⇒ 當機。
//  修好之後不會開始跟 ATC 通訊：真正收送的是 ATCWatchTimerTimer（:696），移植樹沒有人驅動它（另案）。
//  但 OnLine→LoadATCSystem 會照 golden 讀 D:\HT9045\system\ATC.ini（缺鍵補寫預設值）。
//  呼叫點：tools/wb_serve.cpp 開機，InitialHandler 之前（同一行）；ATC_SYSTEM 已由 LoadMachineConfig（database.cpp:800）讀好。
//  ctest：tests/test_atc_boot_init.cpp（AtcBootInit）。
// =============================================================================
void W906_BootInitialATC()                                                      // golden main.cpp:9357-9361（TfMain::FormShow 內）
{
    if (ATCInterfaceForm == 0) return;                                          // AI(W906-ATC1): the null guard is the port's
    if(ATC_SYSTEM==eATCHonPrecType)                                             //Steven 20120410 : Hontech ATC
    {
        ATCInterfaceForm->InitialATC(eATCHonPrecType, 4, "D:\\HT9045\\system\\ATC.ini");
    }
    ATCInterfaceForm->ATCIniPath="D:\\HT9045\\system\\ATC.ini";                 //Steven 20160604 : Add protect for ATC initial
}
//------------------------------------------------------------------------------
