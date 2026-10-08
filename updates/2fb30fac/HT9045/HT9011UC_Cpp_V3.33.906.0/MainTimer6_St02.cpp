// ===========================================================================
//  MainTimer6_St02.cpp -- E-TM-011 (POOL-3, St02): golden TfMain::Timer6Timer, the SECS/GEM run-check wait after a remote START.
//  AI(W906-ETM011) 20261008 (St02-E).  golden 913 main.cpp:32478-32727 (= 0618 main.cpp:31299-31548, whitespace-normalised equal),
//  translated in order; golden main.dfm Timer6: Enabled = False, OnTimer = Timer6Timer, Interval default 1000.  MainTimersSt02.cpp
//  binds it to the timer table entry fMain->Timer6 (forms/fMain_Timers.h:29) and keeps it off until an opener turns it on --
//  golden 913 main.cpp:6469 / :6479 / :6504 (0618 :6182 / :6192 / :6217) = WebStart.cpp:3684 / :3695 / :3720, and csystem.cpp:11049.
//
//  What runs:
//    SECS branch (:32489-:32500): bEnable_SECS_GEM && bRCMDStart && bPhysicalStart -> count; past iN07RunCheckAlarmTime ->
//      PhysicalStart off, timer off, WAR16110 (AlarmCodeCatalog.cpp:1174), bSECSGEMAlarm off (the key lock).
//    else (:32714-:32725): count=0; with SoftStart and SystemStart both false the wait was interrupted -> unlock, PhysicalStart off,
//      RecordProcess("Run check was interrupted!"); timer off.
//  Closed in place (#if 0, golden text kept), the else-if conditions stay so golden's branch order holds:
//    N25 (:32503-:32540) CC_ChipMos_ZHUBEI auto start: customer branch, and fFTPClient->N25_ReadAutoStartFileFromFTP / iAutoStartTask /
//      tAutoStartTimer / tAutoStartTimeOutTimer are not ported (the opener WebStart.cpp:3720 is still MARKED).
//    N29 (:32544-:32712) GM-test "Important Parameter.csv" check: ImpParaCheck and the compare tables are not ported (same opener).
//  One deviation: golden's TfMain member Timer6 is spelled fMain->Timer6 here (a free function, as MainTimer8.cpp).
// ===========================================================================
#include "MachineType.h"
#include "cmydef.h"           // InitialOK, SoftStart, SystemStart, bPhysicalStart, bSECSGEMAlarm, CUSTOMER_CODE, MMSystem
#include "Config.h"           // IniConfig (bEnable_SECS_GEM, bRCMDStart, iN07RunCheckAlarmTime, bN25_1_EnableStartControl, bN29_...)
#include "canary_support.h"   // ShowErrorMessage / RecordProcess
#include "forms/fMain.h"      // fMain->Timer6

namespace ht9045 {

// golden 913 main.cpp:32478 `void __fastcall TfMain::Timer6Timer(TObject *Sender)`
void W906_Timer6Timer()
{
    int iResult=0;
    static int count=0;
    AnsiString sSiteMap="", sText="";   (void)iResult; (void)sSiteMap; (void)sText;   // used only in the closed N25 / N29 bodies
    static bool bTimerRunning=false;
    if(InitialOK==false || bTimerRunning==true)
        return;

    bTimerRunning=true;

    if(IniConfig.bEnable_SECS_GEM==true && IniConfig.bRCMDStart==true && bPhysicalStart==true)
    {
        count++;
        if(count>IniConfig.iN07RunCheckAlarmTime)
        {
            count=0;
            bPhysicalStart=false;
            fMain->Timer6->Enabled=false;
            ShowErrorMessage("WAR16110", 0, MMSystem, false, "Main--Timer6");   //JerryYang 20170817 (Steven) SPIL景桂要求run check time out要上報
            bSECSGEMAlarm=false;                                                //Ifor 20151208 :解除按鍵Lock
        }
    }
    else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI && IniConfig.bN25_1_EnableStartControl)                                    //Steven 20210413 : 南茂的自動Start功能
    {
#if 0 // GATE (N25) -- golden 913 :32503-32540: CC_ChipMos_ZHUBEI auto start -- customer branch; fFTPClient->N25_ReadAutoStartFileFromFTP / iAutoStartTask / tAutoStartTimer / tAutoStartTimeOutTimer not ported
        switch(iAutoStartTask)
        {
            case 1:
                iResult=fFTPClient->N25_ReadAutoStartFileFromFTP();
                if(iResult==1)
                {
                    SoftStart=true;
                    bSecsGemCanStart=false;
                    bSECSGEMAlarm=false;                                        //Ifor 20151208 :解除按鍵Lock
                    bHasSaveSet=false;                                          //Ifor 20151208 :清除設定檔變更旗標
                    bPhysicalStart=true;
                    Timer6->Enabled=false;
                }
                else if(iResult==-1)
                {
                    bPhysicalStart=false;
                    Timer6->Enabled=false;
                }
                else
                {
                    tAutoStartTimer.SetSecAndOn(60);
                    iAutoStartTask=100;
                }
                break;
            case 100:
                if(tAutoStartTimer.Off())
                {
                    iAutoStartTask=1;
                }
                break;
        }

        if(iResult==0 && tAutoStartTimeOutTimer.Off())
        {
            bPhysicalStart=false;
            Timer6->Enabled=false;
            ShowMyMessage("Auto Start Check Time Out!", "自動Start檢查超出設定時間!");
        }
#endif // GATE (N25)
    }
    else if(IniConfig.bN29_ParameterCheckForGMTest==true)                       //Steven 20220311 : GM Test工作檔比對功能
    {
#if 0 // GATE (N29) -- golden 913 :32544-32712: GM-test Important Parameter.csv check -- ImpParaCheck and the compare tables not ported; its opener (WebStart.cpp:3720) is still MARKED
        switch(iAutoStartTask)
        {
            case 1:
                tAutoStartTimer.SetSecAndOn(60);
                if(!DirectoryExists(IniConfig.sN29_FilePath))
                {
                    iAutoStartTask=100;
                    break;
                }

                if(!FileExists(IniConfig.sN29_FilePath+"\\Important Parameter.csv"))
                {
                    iAutoStartTask=200;
                }

                if(ImpParaCheck->Count<=1)
                {
                    try
                    {
                        ImpParaCheck->LoadFromFile(IniConfig.sN29_FilePath+"\\Important Parameter.csv");
                        iAutoStartTask=400;
                    }
                    catch(...)
                    {
                        iAutoStartTask=300;
                    }
                }
                else
                {
                    iAutoStartTask=400;
                }
                break;
            case 100:
                bPhysicalStart=false;
                Timer6->Enabled=false;
                ShowMyMessage("Important parameter check directory not exist!", IniConfig.sN29_FilePath);
                break;
            case 200:
                bPhysicalStart=false;
                Timer6->Enabled=false;
                ShowMyMessage("Important parameter check file not exist!", IniConfig.sN29_FilePath);
                break;
            case 300:
                bPhysicalStart=false;
                Timer6->Enabled=false;
                ShowMyMessage("Important parameter read file fail!", IniConfig.sN29_FilePath);
                break;
            case 400:
                iResult=-1;
                for(int i=1; i<ImpParaCheck->Count; i++)
                {
                    ImpParaCheckRecipe->Clear();
                    ImpParaCheckRecipe->CommaText=ImpParaCheck->Strings[i];
                    if(ImpParaCheckRecipe->Count!=4)
                    {
                        continue;
                    }
                    else if(iResult==-1 && ImpParaCheckRecipe->Strings[1]==cbSetupFileName->Text)
                    {
                        sText=ImpParaCheckRecipe->Text;
                        iResult=1;
                        sSiteMap="";
                        double dTemp=atof(ImpParaCheckRecipe->Strings[3].c_str());
                        if((LastSet.iTemperature==Tempture_Ambient && dTemp!=25) ||
                           (LastSet.iTemperature!=Tempture_Ambient && dTemp!=Temperature.fWorkTemperBase))
                        {
                            iResult=-2;
                        }
                        else
                        {
                            if(TestIF_File.iTestMode==SingleSite)
                                sSiteMap="1X1_";
                            else if(TestIF_File.iTestMode==DualSite)
                                sSiteMap="2X1_";
                            else if(TestIF_File.iTestMode==TriSite1X3)
                                sSiteMap="3X1_";
                            else if(TestIF_File.iTestMode==QualSite1X4)
                                sSiteMap="4X1_";
                            else if(TestIF_File.iTestMode==DualSite2x1)
                                sSiteMap="1X2_";
                            else if(TestIF_File.iTestMode==QualSite2X2 ||
                                    TestIF_File.iTestMode==QualSite2X2N)
                                sSiteMap="2X2_";
                            else if(TestIF_File.iTestMode==_6Site2X3 ||
                                    TestIF_File.iTestMode==_6Site2X3N)          //Steven 20220425 : 2X3NN Mode
                                sSiteMap="3X2_";
                            else if(TestIF_File.iTestMode==_8Site2X4 ||
                                    TestIF_File.iTestMode==_8Site2X4N)          //Wei 20231211 : 2X4NN Mode
                                sSiteMap="4X2_";
                            else if(TestIF_File.iTestMode==_10Site2X5)
                                sSiteMap="5X2_";
                            else if(TestIF_File.iTestMode==_12Site2X6)
                                sSiteMap="6X2_";
                            else if(TestIF_File.iTestMode==_16Site2X8)
                                sSiteMap="8X2_";
                            else if(TestIF_File.iTestMode==_16Site4X4)
                                sSiteMap="4X4_";
                            else if(TestIF_File.iTestMode==_32Site4X8N)
                                sSiteMap="8X4_";
                            else if(TestIF_File.iTestMode==_32Site4X8M)
                                sSiteMap="8X4_";

                            int iCt=0;
                            for(int x=0; x<TestSocket.iShtRow; x++)
                            {
                                for(int y=0; y<TestSocket.iShtCol; y++)
                                {
                                    if(iCt==0)
                                    {
                                        iCt++;
                                        if(TestIF_File.iSiteMap[x][y]<0)
                                            sSiteMap+=AnsiString("0");
                                        else
                                            sSiteMap+=AnsiString(TestIF_File.iSiteMap[x][y]);
                                    }
                                    else
                                    {
                                        if(TestIF_File.iSiteMap[x][y]<0)
                                            sSiteMap+=AnsiString(":0");
                                        else
                                            sSiteMap+=AnsiString(":")+AnsiString(TestIF_File.iSiteMap[x][y]);
                                    }
                                }
                            }

                            if(sSiteMap!=ImpParaCheckRecipe->Strings[2])
                            {
                                iResult=-3;
                            }
                        }
                    }
                }

                if(iResult==1)
                {
                    SoftStart=true;
                    bSecsGemCanStart=false;
                    bSECSGEMAlarm=false;                                        //Ifor 20151208 :解除按鍵Lock
                    bHasSaveSet=false;                                          //Ifor 20151208 :清除設定檔變更旗標
                    bPhysicalStart=true;
                    Timer6->Enabled=false;
                }
                else if(iResult==-1)
                {
                    bPhysicalStart=false;
                    Timer6->Enabled=false;
                    ShowMyMessage("Important parameter check, recipe not found in list!", cbSetupFileName->Text);
                }
                else if(iResult==-2)
                {
                    bPhysicalStart=false;
                    Timer6->Enabled=false;
                    ShowMyMessage("Important parameter check, temperature setting error!", sText);                      //Steven 20220523 : modify
                }
                else if(iResult==-3)
                {
                    bPhysicalStart=false;
                    Timer6->Enabled=false;
                    ShowMyMessage("Important parameter check, site map setting error!", sText);                         //Steven 20220523 : modify
                }
                break;
        }

        if(tAutoStartTimer.Off())
        {
            bPhysicalStart=false;
            Timer6->Enabled=false;
            ShowMyMessage("Important Parameter Check Time Out!", "");
        }
#endif // GATE (N29)
    }
    else
    {
        count=0;

        if(SoftStart==false && SystemStart==false)                              //JerryYang 20250303 : fix run check沒有跳Timeout
        {
            bSECSGEMAlarm=false;
            bPhysicalStart=false;
            RecordProcess("Run check was interrupted!");
        }
        fMain->Timer6->Enabled=false;
    }
    bTimerRunning=false;
}

}  // namespace ht9045
