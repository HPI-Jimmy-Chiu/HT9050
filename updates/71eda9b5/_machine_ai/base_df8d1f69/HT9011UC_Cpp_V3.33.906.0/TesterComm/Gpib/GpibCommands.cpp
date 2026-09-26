// ===========================================================================
//  TesterComm/Gpib/GpibCommands.cpp -- TSerialPoll command dispatch of the GPIB bridge (H9046_32GPIB).
//
//  AI(W906-GB-P1) 20260926: faithful translation of golden
//  D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp:5004-6581 (Big5/cp950 -> UTF-8), per
//  TesterComm/Gpib/TRANSLATION_RULES.md:
//    DoOverDrive (:5004)  DoRecontact (:5037)  ProcessStatusStringRFMD (:5062)  ProcessStatusStringRCMD (:5125)
//    ProcessStatusString (:5141-6427, the whole if/else dispatcher, every customer / tester-mode branch)
//    InitialBarcodeList (:6429)  SetParameter (:6456)
//  Bodies are golden text line by line, branch order unchanged (the order is load-bearing: "SETSITEMAP " before
//  "SETSITEMAP", "*IDN?" before "*IDN", "PPSELECT?" before "PPSELECT", "PAUSE_01" before "PAUSE", "STATUS" after
//  every "...STATUS?" ...).  SetParameter is kept exactly as golden and tests exactly the line it is handed: golden
//  TestGPIB calls it at 0001/0200/0400 with `buffer` right after the in-place strupr (raw case only on the 0001
//  SGSETUP_ path, Main.cpp:1573-1581), never at 0600; the dispatch tests S (upper-cased at 0001/0200/0400, raw at
//  0600 and at 7200 for the RFMD table) and uses the raw-case BS only in ECHOCODE: / DEVICETEMP.
//  Deviations, each marked //AI(W906-GB-P1) in place:
//    * `__fastcall` dropped (SetParameter).
//    * SendMessage(HMountWnd, WM_COPYDATA, ...) -> PostToHandler(pcp) (DoOverDrive, DoRecontact); in DoOverDrive
//      the golden delete-before-send is reordered to send-then-delete (use-after-free is UB here).
//    * vclcompat StringsProxy / CommaTextProxy wrapped in AnsiString(...) where golden hands them to sprintf or calls
//      an AnsiString member on them (the printf-family forwards non-AnsiString objects raw through '...').
//    * TComm::WriteCommData(char*, Word): const_cast on AnsiString::c_str() (BCB6 c_str() returns char*).
//    * SOFTBIN: golden's 544-byte strncpy across cReturn+GpibStatus+GpibData kept through the object's bytes and
//      clamped to that span.
//  Not here (other bridge files): the globals and every other TSerialPoll member these bodies call (MyGPIBWrite,
//  SendMSG_CMD*, WriteLog, WriteLastDataFile, SendMode, Check2Dsum ...).  CC_* customer codes, eTestMode
//  (SingleSite/DualSite/QualSite1X4/QualSite2X2) and e2DIDFormat (eAMD/eIntel) come from MachineType.h via
//  MessageDef.h; MSG_CMD_* from MessageDef.h; ibrsv from GpibDriver.h.
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace gpibbridge {

//------------------------------------------------------------------------------
void TSerialPoll::DoOverDrive(AnsiString Msg, AnsiString Task)                  //Steven 20151207 : OverDrive for TSMC
{
    int iPosition;
    AnsiString Str;
    Str=Msg.SubString(10, Msg.Length());
    iPosition=atoi(Str.c_str());

    if(iPosition>300 || iPosition<-300)
    {
        WriteLog("OverDrive Fail, SRQ:0x53");
        ibrsv(noncontroller, 0x53);
    }
    else
    {
        COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
        pcp->dwData=0;
        pcp->cbData=sizeof(GGpib2Handler);

        GGpib2Handler.iCommand=MSG_CMD_OverDrive;
        GGpib2Handler.GPIBBin=iPosition;
        pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

        if((int)GGpib2Handler.iCommand>=slCmdList->Count)                       //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
            Str.sprintf("GPIB to Handler <== CMD %d : %d", GGpib2Handler.iCommand, GGpib2Handler.GPIBBin);
        else
            Str.sprintf("GPIB to Handler <== %s : %d", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]), GGpib2Handler.GPIBBin);  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
        WriteLog(Str);
//        delete pcp;                                                             //AI(W906-GB-P1) 20260926: moved below PostToHandler, see next comment

        //AI(W906-GB-P1) 20260926: golden runs `delete pcp;` and THEN
        //  `SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);` -- a use-after-free that worked under BCB6
        //  because the freed block still held dwData/cbData/lpData when SendMessage read it.  PostToHandler dereferences
        //  pcp, so keeping the order would be UB in C++; the golden intent (send this packet, then free it) is kept.
        PostToHandler(pcp);                                                     // golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
        delete pcp;
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::DoRecontact(AnsiString Msg, AnsiString Task)                  //Steven 20151207 : Recontact for TSMC
{
    int iCount;
    AnsiString Str;
    Str=Msg.SubString(10, Msg.Length());
    iCount=atoi(Str.c_str());

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=MSG_CMD_ReContact;
    GGpib2Handler.GPIBBin=iCount;
    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

    if((int)GGpib2Handler.iCommand>=slCmdList->Count)                           //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
        Str.sprintf("GPIB to Handler <== CMD %d : %d", GGpib2Handler.iCommand, GGpib2Handler.GPIBBin);
    else
        Str.sprintf("GPIB to Handler <== %s : %d", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]), GGpib2Handler.GPIBBin);  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
    WriteLog(Str);

    PostToHandler(pcp);                                                         // golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
int TSerialPoll::ProcessStatusStringRFMD(AnsiString Str, AnsiString BS, AnsiString Task)     //Steven 20201022 : For RFMD
{
    int iResult=-1;
    AnsiString S, cCommData;

    if(Str.Pos("QRM?")==1)
    {
        if(LastSet.bUseBarcodeFunction)
            cCommData.sprintf("QRM:1");
        else
            cCommData.sprintf("QRM:0");

        MyGPIBWrite(cCommData, Task);
        iResult=0;
    }
    else if(Str.Pos("QRX?")==1 ||
            Str.Pos("QRA?")==1)                                                 //Steven 20241004 : Qorvo check ART enable
    {
        SendMSG_CMD(MSG_CMD_QRA);
        iResult=1;
    }
    else if(Str.Pos("REQUEST,CHECKEMPTY")==1)
    {
        SendMSG_CMD_ESC(MSG_CMD_ESC, 1);
        iResult=2;
    }
    else if(Str.Pos("CONFIGURE,SRQ=Y")==1)
    {
        cCommData.sprintf("CONFIGURE,SRQ=Y:OK");
        MyGPIBWrite(cCommData, Task);
        LastSet.bConfigureSRQ=true;
        iResult=1;
    }
    else if(Str.Pos("CONFIGURE,FULLSITES?=Y")==1)
    {
        cCommData.sprintf("CONFIGURE,FULLSITES=Y:OK");
        MyGPIBWrite(cCommData, Task);
        LastSet.bHaveContactorInfo=true;
        iResult=1;
        bMustWaitESC=true;
        bESCIsOpen=false;
    }
    else if(Str.Pos("CONFIGURE,CONTACTOR=Y")==1)
    {
        cCommData.sprintf("CONFIGURE,CONTACTOR=Y:OK\r\n");
        MyGPIBWrite(cCommData, Task);
        LastSet.bHaveContactorInfo=true;
        iResult=1;
        bMustWaitESC=true;
        bESCIsOpen=false;
    }
    else if(Str.Pos("CONFIGURE,CONTACTOR=N")==1)
    {
        cCommData.sprintf("CONFIGURE,CONTACTOR=N:OK\r\n");
        MyGPIBWrite(cCommData, Task);
        LastSet.bHaveContactorInfo=false;
        iResult=1;
        bMustWaitESC=true;
        bESCIsOpen=false;
    }
    return iResult;
}
//------------------------------------------------------------------------------
bool TSerialPoll::ProcessStatusStringRCMD(AnsiString Str, AnsiString BS, AnsiString Task)     //JerryYang 20190226 RCMD command
{
    bool bResult=false;
    if(LastSet.bFlagRCMD==false && Str.Pos("RCMD:")!=0)                         //jou 2015-09-21 Auto Retest function
    {
        asShowMemo=Task+"TALK:";
        WriteLog("GPIB to Handler <== "+Str);
        LastSet.bFlagRCMD=true;
        bECHO_FlagRCMD=false;
        strncpy(GGpib2Handler.cReturn, Str.c_str(), sizeof(GGpib2Handler.cReturn));
        SendMSG_CMD(MSG_CMD_RCMD);
        bResult=true;
    }
    return bResult;
}
//------------------------------------------------------------------------------
bool TSerialPoll::ProcessStatusString(AnsiString Str, AnsiString BS, AnsiString Task)     //Steven 20150713 : 整合部分COMMAND
{
    bool bResult=false;
    AnsiString S, S2, cCommData;
    AnsiString sPcName, sVersion, sBarCodeMsg, sSiteMapData="";
    AnsiString str1, str2;
    char cmd[128];
    TStringList *SL;
    int iSendUseSite[TOTAL_SITE];
    double ATCNowTJTemp[4];
    ZeroMemory(ATCNowTJTemp, sizeof(ATCNowTJTemp));

    if(Str.Pos("HOSTNAME?")==1)
    {
        sPcName.sprintf("%s", PcName);
        MyGPIBWrite(sPcName, Task);
        bResult=true;
    }
    else if(Str.Pos("QRC?")==1)                                                 //Steven 20201022 : For RFMD
    {
        if(LastSet.bSQR41==false)                                               //jou 20170407 (Steven) : 沒有0x41不應該送出Barcode
            InitialBarcodeList();

        sBarCode_ASE_CL->Clear();

        for(int i=0; i<32; i++)                                                 //不支援32site
        {
            if(LastSet.bUseBarcodeFunction==false)
                sBarCode_ASE_CL->Add("NS");
            if(sBarCode->Strings[31-i]=="0" || sBarCode->Strings[31-i]=="")
                sBarCode_ASE_CL->Add("@");
            else if(AnsiString(sBarCode->Strings[31-i]).AnsiPos("ERROR")!=0)  //AI(W906-GB-P1) 20260926: StringsProxy has no AnsiPos (cObserver.cpp:1500 precedent)
                sBarCode_ASE_CL->Add("#");
            else if(sBarCode->Strings[31-i]=="NULL")
                sBarCode_ASE_CL->Add("$");
            else
                sBarCode_ASE_CL->Add(sBarCode->Strings[31-i]);
        }
        sBarCodeMsg.sprintf("QRC:%s\r\n", AnsiString(sBarCode_ASE_CL->CommaText));  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
        sBarCodeMsg=StringReplace(sBarCodeMsg, "\"",  "", TReplaceFlags()<<rfReplaceAll);   //Steven 20210520 : 去除2DID的雙引號
        MyGPIBWrite(sBarCodeMsg, Task);
        LastSet.bHasBarCode=true;
        bResult=true;
    }
    else if(Str.Pos("TEMPOK")==1)                                               //Steven 20250915 : 資料錯誤
    {
        bResult=true;
    }
    else if(Str.Pos("TEMPNG")==1)                                               //Steven 20250915 : 資料錯誤
    {
        SendMSG_CMD(MSG_CMD_ECHONG, Str);
        bResult=true;
    }
    else if(Str.Pos("SOAKOK")==1)                                               //Steven 20250915 : 資料錯誤
    {
        bResult=true;
    }
    else if(Str.Pos("SOAKNG")==1)                                               //Steven 20250915 : 資料錯誤
    {
        SendMSG_CMD(MSG_CMD_ECHONG, Str);
        bResult=true;
    }
    else if(Str.Pos("DUTCHKOK")==1)                                             //Steven 20250915 : 資料錯誤
    {
        bResult=true;
    }
    else if(Str.Pos("DUTCHKNG")==1)                                             //Steven 20250915 : 資料錯誤
    {
        SendMSG_CMD(MSG_CMD_ECHONG, Str);
        bResult=true;
    }
    else if(Str.Pos("ASIF_TJ_EFUSED")==1)                                       //Steven 20240903 : for MTK ASIF data
    {
        SendMSG_CMD(MSG_CMD_ASIF_TJ_EFUSED, Str);
        bResult=true;
    }
    else if(Str.Pos("ASIF_TJ_REQUEST")==1)                                      //Steven 20240903 : for MTK ASIF data
    {
        SendMSG_CMD(MSG_CMD_ASIF_TJ_REQUEST);
        bResult=true;
    }
    else if(Str.Pos("ASIF_TJ_FB")==1)                                           //Steven 20240903 : for MTK ASIF data
    {
        SendMSG_CMD(MSG_CMD_ASIF_TJ_FB);
        bResult=true;
    }
    else if(Str.Pos("BARCODE?")==1)                                             //Steven 20150713 : Add 2D code
    {
        if(CustomerCode==CC_KYEC_XILINX)
        {
            if(LastSet.bUseBarcodeFunction)
            {
                if(LastSet.bSQR41==true)                                        //jou 20170407 (Steven) : 沒有0x41不應該送出Barcode
                    sBarCodeMsg.sprintf("BARCODE:%s;", AnsiString(sBarCode->CommaText));  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
                else
                    sBarCodeMsg="ECHOCODENG";

                LastSet.bHasBarCode=true;
                MyGPIBWrite(sBarCodeMsg, Task);
                bResult=true;
            }
            else
            {
                SendMSG_CMD(MSG_CMD_BarcodeOFF);                                //wei 20161024 No Open Barcode Function
                WriteLog("Handler ==> No Open Barcode Function!");
            }
        }
        else
        {
            if(LastSet.bSQR41==false)                                           //jou 20170407 (Steven) : 沒有0x41不應該送出Barcode
                InitialBarcodeList();

            if(LastSet.bUseBarcodeFunction)                                     //Steven 20211109 : 增加無開啟Barcode卻收到Barcode的alarm
            {
                sBarCodeMsg.sprintf("BARCODE:%s;", AnsiString(sBarCode->CommaText));  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
                MyGPIBWrite(sBarCodeMsg, Task);
                LastSet.bHasBarCode=true;
            }
            else
            {
                if(LastSet.bAlarmWhen2DFuncMisMatch &&                          //Steven 20211109 : 當測試機問BARCODE?, 但是機台沒有開功能時, 要跳ALARM
                   LastSet.i2DIDFormat!=eIntel)                                 //JerryYang 20230721 : Intel沒開2DID也要正常回傳
                {
                    SendMSG_CMD(MSG_CMD_BarcodeOFF);                            //wei 20161024 No Open Barcode Function
                    WriteLog("Handler ==> No Open Barcode Function!");
                }
                else
                {
                    sBarCodeMsg.sprintf("BARCODE:%s;", AnsiString(sBarCode->CommaText));  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
                    MyGPIBWrite(sBarCodeMsg, Task);
                    LastSet.bHasBarCode=true;
                }
            }
            bResult=true;
        }
    }
    else if(Str.Pos("GET2DID?")==1)                                             //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    {
        if(LastSet.bSQR41==false)                                               //jou 20170407 (Steven) : 沒有0x41不應該送出Barcode
            InitialBarcodeList();

        sBarCode_ASE_CL->Clear();

        for(int i=0; i<32; i++)
        {
            sBarCode_ASE_CL->Add(sBarCode->Strings[31-i]);
        }
        sBarCodeMsg.sprintf("%s,", AnsiString(sBarCode_ASE_CL->CommaText));  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
        MyGPIBWrite(sBarCodeMsg, Task);
        LastSet.bHasBarCode=true;
        bResult=true;
    }
    else if(Str.Pos("RESUME_01")==1)                                            //Steven 20220517 : Add for GIGA
    {
        SendMSG_CMD(MSG_CMD_RESUME);
        str2.sprintf("SETTINGOK");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("RESUME")==1)                                               //Steven 20201022 : For RFMD
    {
        SendMSG_CMD(MSG_CMD_RESUME);
        bResult=true;
    }
    else if(Str.Pos("TESTALARM")==1)                                            //Steven 20201022 : For RFMD
    {
        SendMSG_CMD(MSG_CMD_TestAlarm);
        bResult=true;
    }
    else if(Str.Pos("LOTORDER")==1)                                             //jou 2015-09-21 Auto Retest function
    {
        strncpy(GGpib2Handler.cReturn, Str.c_str(), sizeof(GGpib2Handler.cReturn));
        SendMSG_CMD(MSG_CMD_LotStatus);
        bResult=true;
    }
    else if(Str.Pos("SRQKIND?")==1)
    {
        DummyArtMSG=AnsiString(iLotModeGPIB);                                   //Steven 20170413 (wei) : Add ART simulator
        str1.sprintf("SRQKIND %s\r\n", AnsiString(iLotModeGPIB));
        if(LastSet.bDummyART==false)                                            //Steven 20180824 : Semi ART
        {
            MyGPIBWrite(str1, "0001");
        }
        else
        {
            str1.sprintf("SRQKIND 2\r\n");
            S.printf("Dummy FT ==> %s", str1);
            WriteLog(S);
        }
        if(LastSet.bSRQC0==true)                                                //Steven 20170110
        {
            LastSet.bSRQC0=false;
            LastSet.bFlagRCMD=false;
            LastSet.bFlagSVID=false;
            LastSet.bFlagECID=false;
            iLotMode=0;
        }
        bResult=true;
        SerialPoll->bMessageFromHandler=true;                                   //Steven 20170413 (wei) : Add ART simulator
    }
    else if(Str.Pos("ECHOCODE:")==1)                                            //Steven 20150713 : Add 2D code
    {
        if(BS.AnsiPos(sBarCode->CommaText)==10)
        {
            sBarCodeMsg="ECHOCODEOK";
        }
        else
        {
            sBarCodeMsg="ECHOCODENG";
        }
        MyGPIBWrite(sBarCodeMsg, Task);
        bResult=true;
    }
    else if(Str.Pos("VERSION?")==1)
    {
        if(bA10_3_Enable==true)                                                 //Jimmychiu 20231205 : 借用變數bTimeOutProcess，當作A10-3開啟判斷
        {
            if(LastSet.iTesterMode==InterfaceType_16BinGS ||
               LastSet.iTesterMode==InterfaceType_32BinGS)
                sVersion.sprintf("%s\r\n", "ADVANTEST M4871 Rev.1.P3 FULLSITEB");
            else
                sVersion.sprintf("%s\r\n", "IFUNT200 Version B17");
        }
        else
        {
            sVersion.sprintf("%s\r\n", CheckAndReadIniData(asGeneralPath,  "SystemSetup", "Version", AnsiString("IFUNT200 Version B17")));  //Steven 20170717 (Jou) : VERSION?命令改成傳送前讀檔案
        }
        MyGPIBWrite(sVersion, Task);
        bResult=true;
    }
    else if(Str.Pos("CHKMATCH?")==1 ||
            Str.Pos("CHKMACH?")==1)                                             // 2010.02.25 , Joye , Check Hontech Machine
    {
        if(bRETURN_GPIB_VERSION==true)                                          //wei 20151125 GPIB VERSION
        {
            MyGPIBWrite(sGPIBVersion, Task);
        }
        else
        {
            MyGPIBWrite(szMachineType, Task);
        }
        bResult=true;
    }
    else if(Str.Pos("*IDN?")==1)                                                //Steven 20150430 : for Eagle tester in Maxim Thailand
    {
        if(LastSet.i2DIDFormat==eAMD)                                           //JerryYang 20200422 2DID format
        {

            sVersion.sprintf("1 Hontech,ROGERS,0,Rogers %s", GPIBVersion);      //Ifor 20190603 : add 新增Hontech identification
        }
        else
        {
            sVersion.sprintf("HONTECH");
        }
        MyGPIBWrite(sVersion, Task);
        bResult=true;
    }
    else if(Str.Pos("SETTEMP?")==1)
    {
        SendMSG_CMD(MSG_CMD_HandlerTemperature);
        bResult=true;
    }
    else if(Str.Pos("SETTEMP +")==1 || Str.Pos("SETTEMP_")==1)
    {
        SendMSG_CMD(MSG_CMD_SetTemp, TempStr);                                  // SetHandlerTemperature();
        bResult=true;
    }
    else if(Str.Pos("SETTESTTEMP +")==1)
    {
        SendMSG_CMD(MSG_CMD_SetTestTemp, TempStr);                              // SetTestTemperature();
        bResult=true;
    }
    else if(Str.Pos("SETSOAK?")==1)                                             // 2010.02.25 , Joye , Get/Set Soak Time {
    {
        SendMSG_CMD(MSG_CMD_HandlerSoakTime);
        bResult=true;
    }
    else if(Str.Pos("SETSOAK ")==1 ||
            Str.Pos("SETSOAK_")==1)                                             //KaiChen 20181129 ：Add GPIB SETSOAK_
    {
        SendMSG_CMD(MSG_CMD_SetSoakTime, TempStr);                              //SetHandlerSoakTime();
        bResult=true;
    }                                                                           // 2010.02.25 , Joye , Get/Set Soak Time }
    else if(Str.Pos("SETSITEMAP ")==1 ||                                        //wei 20151126 Add SETSITEMAP  Command
            Str.Pos("SETSITEMAP_")==1)                                          //KaiChen 20181129 ：Add GPIB SETSITEMAP_
    {
        SendMSG_CMD(MSG_CMD_SetSiteMapData, Sitemapstr);                        //SetSiteMapData();
        bResult=true;
    }
    else if(Str.Pos("DUTCHK?")==1)                                              //Steven 20250701 : for DOOSAN TESNA
    {
        SendMSG_CMD(MSG_CMD_DUTCHK);
        bResult=true;
    }
    //AI(W906-GB-P1) 20260926: mixed-case patterns kept verbatim ("GetFFC?", "GetTJFunction?", "GetPowerFollowing?",
    //  "Contact Force?", "SetSiteOnOff:").  Golden TestGPIB hands this function the strupr'ed line at 0001/0200/0400
    //  (Main.cpp:1579/1821/2031), where they can never match, and the raw-case line at 0600 (Main.cpp:2782 `S=buffer`,
    //  no strupr, no SetParameter), where they can -- a golden quirk, not a translation loss.
    else if(Str.Pos("GetFFC?")==1)                                              //Steven 20250701 : for Ampere
    {
        SendMSG_CMD(MSG_CMD_GetFFC);
        bResult=true;
    }
    else if(Str.Pos("GetTJFunction?")==1)
    {
        SendMSG_CMD(MSG_CMD_GetTJFunction);
        bResult=true;
    }
    else if(Str.Pos("GetPowerFollowing?")==1)
    {
        SendMSG_CMD(MSG_CMD_GetPowerFollowing);
        bResult=true;
    }
    else if(Str.Pos("SETSITEMAP")==1 ||
            Str.Pos("SITEMAP?")==1)                                             // 2011.01.11 , Joye , Get Site Map {
    {
        SendMSG_CMD(MSG_CMD_HandlerSiteMap);
        bResult=true;
    }
    //Steven 20110613 End: Corn GPIB
    else if(Str.Pos("HANDLERID?")==1)                                           //jou 2012-07-19 for Dialog
    {
        if(iUseGPIBFormat)                                                      //Steven 20140920 : For 矽格阮瑋民的要求,不能用Handler ID
        {
            MyGPIBWrite(szHadnlerID, Task);
        }
        bResult=true;
    }
    else if(Str.Pos("HANDLER ID?")==1 ||
            Str.Pos("ID?")==1)                                                  //Steven 20211122 : Hana Micron加入ID?
    {
        SendMSG_CMD(MSG_CMD_HandlerID);
        bResult=true;
    }
    else if(Str.Pos("START MODE?")==1)                                          //ChungHung alter 20130510 command for NS
    {
        SendMSG_CMD(MSG_CMD_StartMode);
        bResult=true;
    }
    else if(Str.Pos("ASSIGN?")==1)                                              //ChungHung alter 20130510 command for NS
    {
        SendMSG_CMD(MSG_CMD_Assign);
        bResult=true;
    }
    else if(Str.Pos("ACTUALTEMP?")==1)                                          //ChungHung alter 20130510 command for NS
    {
        SendMSG_CMD(MSG_CMD_ActualTemp);
        bResult=true;
    }
    else if(Str.Pos("CONTACT FORCE?") ||                                        //ChungHung alter 20130510 command for NS
            Str.Pos("Contact Force?"))                                          //jou 2016-03-30 修正TSMC contact force異常
    {
        SendMSG_CMD(MSG_CMD_ContactForce);
        bResult=true;
    }
    else if(Str.Pos("TEST ARM?")==1)
    {
        cCommData.sprintf("%d\r\n", iCurrentArm);
        MyGPIBWrite(cCommData, Task);
        bResult=true;
    }
    else if(Str.Pos("TEMPARM?")==1)
    {
        SendMSG_CMD(MSG_CMD_TempArm);
        bResult=true;
    }
    else if(Str.Pos("FORCE?")==1)
    {
        SendMSG_CMD(MSG_CMD_Force);
        bResult=true;
    }
    else if(Str.Pos("BINMAP?")==1)                                              //ChungHung 20150217 add for SCK request
    {
        SendMSG_CMD(MSG_CMD_BinMap);
        bResult=true;
    }
    else if(Str.Pos("BINMAP_")==1)                                              //Steven 20230210 : Set Bin Map.
    {
        SendMSG_CMD(MSG_CMD_SetBinMap, Str);
        bResult=true;
    }
    else if(Str.Pos("TESTMODE?")==1)                                            //ChungHung 20150217 add for SCK request
    {
        SendMSG_CMD(MSG_CMD_TestMode);
        bResult=true;
    }
    else if(Str.Pos("DEVICETEMP")==1 ||                                         //ChungHung 20150122 add for TSMC Use Tj
            Str.Pos("SETTESTOFFSET_")==1)
    {
        SendMSG_CMD(MSG_CMD_SetTJ, Str);                                        // SetTempFormTJ(Str.c_str());
        S="ECHO "+BS;
        MyGPIBWrite(S, Task);
        bResult=true;
    }
    else if(Str.Pos("GETNOWALLTEMP?")==1)                                       // Frank 20150801 Add GetNowAllTemp? Command
    {
        SendMSG_CMD(MSG_CMD_GetNowAllTemp);
        bResult=true;
    }
    else if(Str.Pos("OVERDRIVE")==1)                                            //Steven 20151207 : OverDrive for TSMC
    {
        DoOverDrive(Str, Task);
        bResult=true;
    }
    else if(Str.Pos("RECONTACT")==1)                                            //Steven 20151207 : Recontact for TSMC
    {
        DoRecontact(Str, Task);
        bResult=true;
    }
    else if(Str.Pos("PICKLOAD")==1)                                             //Steven 20191009 : Device Map for Qualcomm
    {
        SendMSG_CMD(MSG_CMD_PickLoad, Str);
        S="ECHO:"+Str;                                                          //Steven 20210416 : QTI要求回傳ECHO
        MyGPIBWrite(S, Task);
        bResult=true;
    }
    else if(Str.Pos("PLACETOLOAD")==1)                                          //Steven 20191009 : Device Map for Qualcomm
    {
        SendMSG_CMD(MSG_CMD_PlaceLoad);
        bResult=true;
    }
    else if(Str.Pos("TRAYFEED")==1)                                             //Steven 20191009 : Device Map for Qualcomm
    {
        SendMSG_CMD(MSG_CMD_TrayFeed);
        bResult=true;
    }
    else if(Str.Pos("CHKSETUP?")==1)                                            //wei 20151125 Add CHKSETUP? Command
    {
        SendMSG_CMD(MSG_CMD_ChkSetup);
        bResult=true;
    }
    else if(Str.Pos("SGSETUP_")==1)                                             //KaiChen 20190613 ：Add GPIB SGSETUP_
    {
        SendMSG_CMD(MSG_CMD_SGSETUP, Sitemapstr);                               //        SetHandlerSETUP();
        bResult=true;
    }
    else if(Str.Pos("BINPOS_")==1)                                              //KaiChen 20190706 ：Add GPIB BINPOS_
    {
        SendMSG_CMD(MSG_CMD_BINPOS, Sitemapstr);                                //        SetHandlerBINPOS();
        bResult=true;
    }
    else if(Str.Pos("SETUP_")==1)                                               //wei 20151125 Add SETUP_ Command
    {
        SendMSG_CMD(MSG_CMD_SetAlarmSetup, Sitemapstr);                         //        SetCHKSETUP();
        bResult=true;
    }
    else if(Str.Pos("CHECKLIST?")==1)                                           //KaiChen 20190613 ：Add GPIB CHECKLIST?
    {
        SendMSG_CMD(MSG_CMD_CHECKLIST);
        bResult=true;
    }
    else if(Str.Pos("TESTARMPOS?")==1)                                          //wei 20160122  TestArmPos
    {
        SendMSG_CMD(MSG_CMD_GetTestArmPos);
        bResult=true;
    }
    else if(Str.Pos("TESTARMEP?")==1)                                           //wei 20160122 TestArmEP
    {
        SendMSG_CMD(MSG_CMD_GetTestArmEP);
        bResult=true;
    }
    else if(Str.Pos("LOTCLEAR?")==1)                                            //Steven 20161025 : SCK ART function
    {
        SendMSG_CMD(MSG_CMD_SCKART_LOTCLEAR);
        bResult=true;
    }
    else if(Str.Pos("LOTRETESTCLEAR?")==1)
    {
        SendMSG_CMD(MSG_CMD_SCKART_LOTRTCLEAR);
        bResult=true;
    }
    else if(Str.Pos("TMP?")==1)                                                 //Steven 20191112 : 三星格式
    {
        SendMSG_CMD(MSG_CMD_SamSung_Tmp);
        bResult=true;
    }
    else if(LastSet.bRunHANA_ART==false &&                                      //Steven 20250414 : HANA ART Function
            Str.Pos("MAP?")==1)                                                 //Steven 20191112 : 三星格式
    {
        SendMSG_CMD(MSG_CMD_SamSung_Map);
        bResult=true;
    }
    else if(LastSet.bRunHANA_ART==false &&                                      //Steven 20250414 : HANA ART Function
            Str.Pos("SOAK?")==1)                                                //Steven 20191112 : 三星格式
    {
        SendMSG_CMD(MSG_CMD_SamSung_Soak);
        bResult=true;
    }
    else if(Str.Pos("CHKSTATUS?")==1)                                           //KaiChen 20180910 ：Add GPIB CHKSTATUS?
    {
        SendMSG_CMD(MSG_CMD_SIGURD_CHKSTATUS);
        bResult=true;
    }
    else if(Str.Pos("GETBINCATEGORY?")==1)                                      //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
    {
        SendMSG_CMD(MSG_CMD_GETBINCATEGORY);
        bResult=true;
    }
    else if(Str.Pos("SETUPFILENAME?")==1)                                       //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
    {
        SendMSG_CMD(MSG_CMD_SETUPFILENAME);
        bResult=true;
    }
    else if(Str.Pos("SETSTARTMODE_")==1)                                        //KaiChen 20180910 ：Add GPIB SetStartMode_
    {
        SendMSG_CMD(MSG_CMD_SETSTARTMODE, Sitemapstr);                          //        SetSetStartMode();
        bResult=true;
    }
    else if(Str.Pos("SGFTP_STATUS")==1)                                         //Sam 20210329 : Add GPIB SGFTP_STATUS
    {
        SendMSG_CMD(MSG_CMD_GetSGFTP_STATUS);
        bResult=true;
    }
    else if(Str.Pos("SGFTP_")==1)                                               //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
    {
        SendMSG_CMD(MSG_CMD_SetSGFTP, Sitemapstr);                              //        SetSGFTP();
        bResult=true;
    }
    else if(Str.Pos("NONDOUBLEBIN_")==1)                                        //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
    {
        SendMSG_CMD(MSG_CMD_SetNONDOUBLEBIN, Sitemapstr);                       //        SetNONDOUBLEBIN();
        bResult=true;
    }
    else if(Str.Pos("BINCOUNT_")==1)                                            //Sam 20210329 : Add GPIB BINCOUNT_
    {
        SendMSG_CMD(MSG_CMD_SetBINCOUNT, Sitemapstr);                           //        SetBINCOUNT();
        bResult=true;
    }
    else if(Str.Pos("SGOSBIN_")==1)                                             //Sam 20210406 : Add GPIB SGOSBIN_
    {
        SendMSG_CMD(MSG_CMD_SetSGOSBIN, Sitemapstr);                            //        SetSGOSBIN();
        bResult=true;
    }
    else if(Str.Pos("SGCONTFAIL_")==1)                                          //Sam 20210422 : Add GPIB SGCONTFAIL_
    {
        SendMSG_CMD(MSG_CMD_SetSGCONTFAIL, Sitemapstr);                         //        SetSGCONTFAIL();
        bResult=true;
    }
    else if(Str.Pos("SETTESTERID_")==1)                                         //Sam 20210617 : Add GPIB SETTESTERID
    {
        SendMSG_CMD(MSG_CMD_SETTESTERID, Sitemapstr);                           //        SetTesterID();
        bResult=true;
    }
    else if(Str.Pos("SETTESTERID?")==1)                                         //Sam 20210617 : Add GPIB SETTESTERID
    {
        SendMSG_CMD(MSG_CMD_GETTESTERID);
        bResult=true;
    }
    else if(Str.Pos("GETSHUTTLEMODE?")==1)                                      //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
    {
        SendMSG_CMD(MSG_CMD_GETSHUTTLEMODE);
        bResult=true;
    }
    else if(Str.Pos("SETMAXTEST_")==1)                                          //Sam 20230201 : Add GPIB SETMAXTEST_
    {
        SendMSG_CMD(MSG_CMD_SETMAXTEST, Sitemapstr);
        bResult=true;
    }
    else if(Str.Pos("GETMAXTEST?")==1)                                          //Sam 20230201 : Add GPIB GETMAXTEST
    {
        SendMSG_CMD(MSG_CMD_GETMAXTEST);
        bResult=true;
    }
    else if(Str.Pos("SETINITIALMAXTEST_")==1)                                   //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
    {
        SendMSG_CMD(MSG_CMD_SETINITIALMAXTEST, Sitemapstr);
        bResult=true;
    }
    else if(Str.Pos("GETINITIALMAXTEST?")==1)                                   //Sam 20230201 : Add GPIB GETINITIALMAXTEST
    {
        SendMSG_CMD(MSG_CMD_GETINITIALMAXTEST);
        bResult=true;
    }
    else if(Str.Pos("AUTOCLEAN?")==1)                                           //Sam 20220408 : Novatek 新增 AUTOCLEAN?
    {
        SendMSG_CMD(MSG_CMD_GetAutClean);
        bResult=true;
    }
    else if(Str.Pos("DEVICEFORCEPERPIN?")==1)                                   //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
    {
        SendMSG_CMD(MSG_CMD_ForcePerPinN);
        bResult=true;
    }
    else if(Str.Pos("ARMCONTACTHIGHVALUE?")==1)                                 //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
    {
        SendMSG_CMD(MSG_CMD_ContactHeight);
        bResult=true;
    }
    else if(Str.Pos("YIELDCONTINUESFAIL?")==1)                                  //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
    {
        SendMSG_CMD(MSG_CMD_YieldContinusFail);
        bResult=true;
    }
    else if(Str.Pos("YIELDSITEUNBALANCE?")==1)                                  //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
    {
        SendMSG_CMD(MSG_CMD_YieldSiteCompare);
        bResult=true;
    }
    else if(Str.Pos("DUTSTATUS?")==1)                                           //Sam 20220408 : Novatek 新增 DUTSTATUS?
    {
        SendMSG_CMD(MSG_CMD_DUTStatus);
        bResult=true;
    }
    else if(Str.Pos("UPH?")==1)                                                 //Sam 20220408 : Novatek 新增 UPH?
    {
        SendMSG_CMD(MSG_CMD_UPH);
        bResult=true;
    }
    else if(Str.Pos("INDEXCYCLETIME?")==1)                                      //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
    {
        SendMSG_CMD(MSG_CMD_IndexCycleTime);
        bResult=true;
    }
    else if(Str.Pos("GETTEMPOFFSET?")==1)                                       //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
    {
        SendMSG_CMD(MSG_CMD_TempOfs);
        bResult=true;
    }
    else if(Str.Pos("GETTEMPERATURETOLERANCE?")==1)                             //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
    {
        SendMSG_CMD(MSG_CMD_TempRange);
        bResult=true;
    }
    else if(Str.Pos("VACUUMAIR?")==1)                                           //Sam 20220408 : Novatek 新增 VACUUMAIR?
    {
        SendMSG_CMD(MSG_CMD_VACUUMAIR);
        bResult=true;
    }
    else if(Str.Pos("SET_ALL?")==1)                                             //Sam 20220408 : Novatek 新增 SET_ALL?
    {
        SendMSG_CMD(MSG_CMD_Get_All);
        bResult=true;
    }
    else if(Str.Pos("HANDLERVERSION?")==1)                                      //Sam 20220408 : Novatek 新增 HANDLERVERSION?
    {
        SendMSG_CMD(MSG_CMD_HandlerVersion);
        bResult=true;
    }
    else if(Str.Pos("SETAICCD_")==1)                                            //Sam 20231108 : Add GPIB SETAICCD_
    {
        SendMSG_CMD(MSG_CMD_SETAICCD, Sitemapstr);
        bResult=true;
    }
    else if(Str.Pos("GETAICCD?")==1)                                            //Sam 20240826 : Add GPIB GETAICCD?
    {
        SendMSG_CMD(MSG_CMD_GETAICCD);
        bResult=true;
    }
    else if(Str.Pos("GETSITEONOFF?")==1)                                        //JerryYang 20190627 回傳開關site狀態
    {
        SendMSG_CMD(MSG_CMD_GetSiteOnOff);
        bResult=true;
    }
    else if(Str.Pos("SETOSBIN_")==1)                                            //Sam 20250115 : Add GPIB SETOSBIN_
    {
        SendMSG_CMD(MSG_CMD_SETOSBIN, Sitemapstr);
        bResult=true;
    }
    else if(Str.Pos("GETOSBIN?")==1)                                            //Sam 20250115 : Add GPIB GETOSBIN?
    {
        SendMSG_CMD(MSG_CMD_GETOSBIN);
        bResult=true;
    }
    else if(Str.Pos("INPUTQTY")==1)
    {
        if(LastSet.iTesterType==1)
        {
            S2.sprintf("SETTINGOK");
            Str=Str.SubString(9, Str.Length());
            SL=new TStringList();
            SL->CommaText=Str;
            if(SL->Count>=3)
            {
                SendMSG_CMD_INPUTQTY(MSG_CMD_SCKART_INPUTQTY, SL->Strings[1], atoi(SL->Strings[0].c_str()), SL->Strings[2]);
                labLotID->Caption=SL->Strings[1];
                labLotCount->Caption=SL->Strings[0];
            }
            else if(SL->Count>=2)
            {
                SendMSG_CMD_INPUTQTY(MSG_CMD_SCKART_INPUTQTY, SL->Strings[1], atoi(SL->Strings[0].c_str()));
                labLotID->Caption=SL->Strings[1];
                labLotCount->Caption=SL->Strings[0];
            }
            else
            {
                SendMSG_CMD_INPUTQTY(MSG_CMD_SCKART_INPUTQTY, AnsiString(" "), atoi(Str.c_str()));
                labLotID->Caption=AnsiString(" ");
                labLotCount->Caption=Str;
            }
            SL->Clear();                                                        //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
            delete SL;
        }
        else
        {
            Str=Str.SubString(9, Str.Length());
            SL=new TStringList();
            SL->CommaText=Str;
            if(SL->Count>=2)
            {
                SendMSG_CMD_INPUTQTY(MSG_CMD_SCKART_INPUTQTY, SL->Strings[0], atoi(SL->Strings[1].c_str()));
                labLotID->Caption=SL->Strings[0];
                labLotCount->Caption=SL->Strings[1];
            }
            else
            {
                SendMSG_CMD_INPUTQTY(MSG_CMD_SCKART_INPUTQTY, SL->Strings[0], 0);
                labLotID->Caption=SL->Strings[0];
                labLotCount->Caption=0;
            }
            SL->Clear();                                                        //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
            delete SL;
            S2.sprintf("ECHOQTY%s", Str);
            DummyArtMSG=S2;                                                     //Steven 20170413 (wei) : Add ART simulator
            bMessageFromHandler=true;
        }

        if(LastSet.bDummyART==false)
        {
            MyGPIBWrite(S2, Task);
        }
        bResult=true;
    }
    else if(Str.Pos("LOTSTATUS?")==1)
    {
        if(LastSet.iTesterMode!=InterfaceType_15BinQorvo)                       //Steven 20250218 : QROVO強迫為1
        {
            LastSet.iTesterType=0;
        }

        WriteLastDataFile();
        SendMSG_CMD(MSG_CMD_SCKART_LOTSTATUS);
        bResult=true;
    }
    else if(Str.Pos("ALARM?")==1)
    {
        SendMSG_CMD(MSG_CMD_SCKART_Alarm);
        if(FileExists("D:\\HT9045_Log\\Alarm.txt"))
        {
            SL=new TStringList();
            SL->LoadFromFile("D:\\HT9045_Log\\Alarm.txt");
            MyGPIBWrite(SL->CommaText, Task);
            SL->Clear();                                                        //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
            delete SL;
        }
        else
        {
            MyGPIBWrite("ALARM:", Task);
        }
        bResult=true;
    }
    else if(Str.Pos("QTY?")==1)
    {
        SendMSG_CMD(MSG_CMD_SCKART_QTY);
        bResult=true;
    }
    else if(Str.Pos("INITIAL?")==1)
    {
        SendMSG_CMD(MSG_CMD_SCKART_INITIAL);
        bResult=true;
    }
    else if(Str.Pos("SRQMASK")==1)
    {
        LastSet.iTesterType=1;
        WriteLastDataFile();
        SendMSG_CMD(MSG_CMD_SCKART_SRQMASK);
        bResult=true;
    }
    else if(Str.Pos("AUTO_CLEAN")==1)                                           //wei 20180326
    {
        SendMSG_CMD(MSG_CMD_Auto_Clean);
        str2.sprintf("SETTINGOK");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("STOP_01")==1)                                              //Steven 20220517 : Add for GIGA
    {
        SendMSG_CMD(MSG_CMD_Stop_01);
        str2.sprintf("SETTINGOK");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("PAUSE_01")==1)                                             //Steven 20220517 : Add for GIGA
    {
        SendMSG_CMD(MSG_CMD_Pause01);
        str2.sprintf("SETTINGOK");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("PPSELECT?")==1)                                            //Richard 20220929 :Add for UTAC 讀檔
    {
        SendMSG_CMD(MSG_CMD_ASKPPSELECT);
        bResult=true;
    }
    else if(Str.Pos("PPSELECT")==1 && Str.Pos("PPSELECT?")!=1)
    {
        SendMSG_CMD(MSG_CMD_PPSELECT, Str);
        bResult=true;
    }
    else if(Str.Pos("PAUSE")==1)                                                //wei 20180326
    {
        SendMSG_CMD(MSG_CMD_Pause);                                             //KaiChen 20180913 ：Add GPIB PAUSE
        if(CustomerCode==CC_SIGURD_HUKOU        || CustomerCode==CC_SIGURD_PeiXing ||
           CustomerCode==CC_SIGURD_ChungXing    || CustomerCode==CC_SIGURD_SUZHOU)
        {
            str2.sprintf("SETTINGOK>PAUSE");
        }
        else
        {
            str2.sprintf("SETTINGOK");
        }
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("SETHANDLERDOPAUSE")==1)                                    //Jimmychiu 20231102 : GPIB Command SETHANDLERDOPAUSE
    {
        SendMSG_CMD(MSG_CMD_Pause);
        bResult=true;
    }
    else if(Str.Pos("ECHOOK:ONECYCLE")==1)                                      //KaiChen 20181114 ：Add GPIB ECHOOK:ONECYCLE
    {
        SendMSG_CMD(MSG_CMD_ECHOOK_ONECYCLE);
        str2.sprintf("SETTINGOK");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("ONECYCLE")!=0)                                             //KaiChen 20180910 ：Add GPIB ONECYCLE
    {
        SendMSG_CMD(MSG_CMD_ONECYCLE, sOneCycleMsg);                            //Sam 20221103 : OneCycle 完後顯示訊息
        if(CustomerCode==CC_SIGURD_HUKOU        || CustomerCode==CC_SIGURD_PeiXing ||
           CustomerCode==CC_SIGURD_ChungXing    || CustomerCode==CC_SIGURD_SUZHOU)
        {
            str2.sprintf("SETTINGOK>PAUSE");
        }
        else
        {
            str2.sprintf("SETTINGOK");
        }
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    //Ifor 20180117 : add AMD Command
    //==>
    else if(Str.Pos("*IDN")==1)                                                 //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("1 Delta_Design,SUMMIT,0,Summit 4.2.2");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("IDENTIFY")==1)                                             //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("Summit 4.2.2");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("WHICH")==1)                                                //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("Summit028T");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("NUMTESTSITES")==1)                                         //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("1");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("STATUS")==1)                                               //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("4112");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("SYSTEMMODE 1")==1)                                         //2016-06-06    Dell    for AMD Test
    {
        str2.sprintf("1");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(Str.Pos("RUNPROFILE")==1)                                           // 2016.07.01 , Joye , AMD Run ProFile
    {
        if(Task=="0001")
        {
            SendMSG_CMD(MSG_CMD_AMDNextStep1);
            str2.sprintf("1");
            MyGPIBWrite(str2, Task);
            bResult=true;
        }
        else if(Task=="0400")
        {
            SendMSG_CMD(MSG_CMD_AMDNextStep1);
            str2.sprintf("1");
            MyGPIBWrite(str2, Task);
            for(int i=0; i<TOTAL_SITE; i++)
            {
                if(iStart[i]==1)
                {
                    iSendUseSite[i]=0;
                }
                else
                {
                    iSendUseSite[i]=-1;
                }
            }

            if(iCurrentArm==1)                                                  //Handler ==> Arm 1 Down
            {
                bCheckTC[0]=true;
                if(rgController->ItemIndex==1)
                {
                    SendMode(0,0);
                }
                else if(rgController->ItemIndex==0)
                {
                    str1.sprintf("1038,4,%d,%d,-1,-1", iSendUseSite[0], iSendUseSite[1]);
                    CommAMD->WriteCommData(const_cast<char*>(str1.c_str()), str1.Length());  //AI(W906-GB-P1) 20260926: BCB6 c_str() is char*
                    S.sprintf("%s H2A:  %s", Task, str1);
                    WriteLog(S);

                    if(chkTempReady->Checked==true)                             //ifor 20181227 : add 不等ATC回覆Temp Ready
                        bTempHasReady=true;
                    else
                        bTempHasReady=false;
                }
            }

            if(iCurrentArm==2)                                                  //Handler ==> Arm 1 Down
            {
                bCheckTC[1]=true;
                if(rgController->ItemIndex==1)
                {
                    SendMode(1,0);
                }
                else if(rgController->ItemIndex==0)
                {
                    str1.sprintf("1038,4,-1,-1,%d,%d", iSendUseSite[0], iSendUseSite[1]);
                    CommAMD->WriteCommData(const_cast<char*>(str1.c_str()), str1.Length());  //AI(W906-GB-P1) 20260926: BCB6 c_str() is char*
                    S.sprintf("%s H2A: %s", Task, str1);
                    WriteLog(S);

                    if(chkTempReady->Checked==true)                             //ifor 20181227 : add 不等ATC回覆Temp Ready
                        bTempHasReady=true;
                    else
                        bTempHasReady=false;
                }
            }
            bTempHasReady=true;                                                 //Ifor 20250701 : 溫度轉換完成
            bResult=true;
        }
    }
    else if(Str.Pos("SENDERROR")==1)                                            // 2016.07.04 , Joye , AMD Send Error
    {
        int iWitchArm=0;
        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            iWitchArm=0;
        }
        else if(iCurrentArm==2)
        {
            iWitchArm=2;
        }
        else
        {
            iWitchArm=999;
        }

        if(iWitchArm<999)
        {
            //(1)   Both site fail:               “11 DUT OUT OF GUARDBAND”
            //(2)   Both site pass:               “00 NO_ERROR”
            //(3)   Site1 Fail and Site2 pass:    “10 DUT OUT OF GUARDBAND”
            //(4)   Site1 Pass and Site2 Fail:    “01 DUT OUT OF GUARDBAND”
            if(bCheckDiodeThermal[0+iWitchArm]==true ||
               bCheckDiodeThermal[1+iWitchArm]==true)
            {                                                                   //Ifor 20180202 NO_ERROR =>1 NO_ERROR
                S2.sprintf("%d%d DUT OUT OF", bCheckDiodeThermal[0+iWitchArm], bCheckDiodeThermal[1+iWitchArm]);
            }
            else
            {
                S2="00 NO_ERROR";
            }
            MyGPIBWrite(S2, Task);
        }
        bResult=true;
    }
    else if(Str.Pos("TESTPARTS")==1)                                            //2016-06-06    Dell    for AMD Test
    {
        MyGPIBWrite("1", Task);
        bResult=true;
    }
    else if(Str.Pos("READDIODE")==1)                                            //Ifor 20180202 : add
    {
        int iWitchArm=0;
        //AI(W906-GB-P1) 20260926: FindWindow kept (GpibBridge.h rule).  Golden asks whether the Handler main window is the
        //  HT505 build; beside an HT9045 Handler that is NULL, and in-process (no BCB6 TfMain) it is NULL as well, so the
        //  first branch runs exactly as golden ran on HT9045.  Kept verbatim below: ATCNowTJTemp has 4 slots, so the
        //  `i<iATC_Use_Heat_Count/2` loop assumes <=8 ATC controllers (golden NEXTSTEP note: 八台控制器).
        if(FindWindow("TfMain", "HT505")==NULL)
        {
            if(iCurrentArm==1)                                                  //Handler ==> Arm 1 Down
            {
                iWitchArm=0;
            }
            else if(iCurrentArm==2)                                             //Handler ==> Arm 2 Down
            {
                 iWitchArm=iATC_Use_Heat_Count/2;
            }

            for(int i=0; i<iATC_Use_Heat_Count/2; i++)
            {
                if(iATCUseChannel[i+iWitchArm]==1)
                {
                    if(dBySiteTJ[i+iWitchArm]<-20 ||                            //Ifor 20250701 : 支援負溫
                       dBySiteTJ[i+iWitchArm]>200)
                    {
                         ATCNowTJTemp[i]=999;
                    }
                    else
                    {
                        ATCNowTJTemp[i]=dBySiteTJ[i+iWitchArm];
                    }
                }
                else
                {
                    ATCNowTJTemp[i]=-200;
                }
            }
        }
        else
        {
            for(int i=0; i<iATC_Use_Heat_Count; i++)
            {
                if(dBySiteTJ[i]<-80 || dBySiteTJ[i]>200)
                {
                    ATCNowTJTemp[i]=999;
                }
                else
                {
                    ATCNowTJTemp[i]=dBySiteTJ[i];
                }
                if(i>=3)
                    break;
            }
        }

        if(iTestMode==SingleSite)
        {
            S2.sprintf("1 %5.2f", ATCNowTJTemp[0]);
        }
        else if(iTestMode==DualSite)
        {
            S2.sprintf("1 %5.2f %5.2f", ATCNowTJTemp[0], ATCNowTJTemp[1]);
        }
        else if(iTestMode==QualSite2X2 || iTestMode==QualSite1X4)
        {
            S2.sprintf("1 %5.2f %5.2f% 5.2f %5.2f", ATCNowTJTemp[0], ATCNowTJTemp[1], ATCNowTJTemp[2], ATCNowTJTemp[3]);
        }

        MyGPIBWrite(S2, Task);
        bResult=true;
    }
    else if(Str.Pos("NEXTSTEP ")==1)                                            //Ifor 20220617 Note: 僅支援Qual Site 以下(八台控制器)
    {
        bNEXTSTEP_CMD=true;
        iSwitchTJDelay=5;                                                       //Ifor 20210303 add:收到TC/TJ Switch 訊號延遲5次不讀取溫度
        SleepEx(1, false);
        MyGPIBWrite("1", Task);
        int i;
        S2=Str;
        S2=StringReplace(S2, "\r", "", TReplaceFlags()<<rfReplaceAll);
        S2=StringReplace(S2, "\n", "", TReplaceFlags()<<rfReplaceAll);          //Ifor 20230426 add:移除換行符號
        if(asATC_SiteMapping!="XXX")                                            //Ifor 20220322 add:避免沒有收到Site Mapping 資料導致易常
        {                                                                       //Ifor 20210903 add:有開Site的地方才送切換訊號
            TStringList *sDataList;                                             //JerryYang 20210824
            sDataList=new TStringList();
            sDataList->Clear();
            sDataList->CommaText=asATC_SiteMapping;
            int iATCSite[4]={0, 0, 0, 0};

            ZeroMemory(iSendUseSite, sizeof(iSendUseSite));                     //Ifor 20220322 add:避免資料沒更新

            if(iStart[0]==0 && iStart[1]==0 && iStart[2]==0 && iStart[3]==0)    //Ifor 20220617 避免Contact Mode 測試機Debug 時無開關Site資料無法切換TJ模式
            {
                for(i=0; i<4; i++)
                {
                    iStart[i]=1;
                }
            }

            for(i=0; i<4; i++)                                                  //Ifor 20220120 add:依據ATC Site Map 切換TC/TJ Mode
            {
                if(S2.Length()==10)                                             //Ifor 20230426 add:避免測試機送單Site資料導致模式切換異常
                {
                    iSendUseSite[i]=atoi(Str.SubString(10,1).c_str())-1;
                    if(iSendUseSite[i]==3)                                      //Ifor 20230426 add  新增 NEXTSTEP 4 控溫模式
                        iSendUseSite[i]=2;
                }
                else
                {
                    if(i<sDataList->Count)                                      //Ifor 20220322 add:資料長度不符導致發生異常
                    {
                        iATCSite[i]=atoi(sDataList->Strings[i].c_str())-1;
                        //AI(W906-GB-P1) 20260926: kept verbatim: `(mode-1) && iStart[..]` is a LOGICAL and, so
                        //  iSendUseSite[i] is 0/1 here, never the mode (the `==3` fix-up below cannot fire on this path);
                        //  a site-map entry of "0" gives iATCSite[i]=-1 and reads iStart[-1] (golden reads out of bounds too).
                        iSendUseSite[i] =((atoi(Str.SubString(10+iATCSite[i],1).c_str())-1) && (iStart[iATCSite[i]]));  //Ifor 20220722 Fix iATCSite[i] => [i]

                        if(iSendUseSite[i]==3)                                  //Ifor 20230426 add  新增 NEXTSTEP 4 控溫模式
                            iSendUseSite[i]=2;
                    }
                }
            }

            sDataList->Clear();
            delete sDataList;
        }
        else                                                                    //Ifor 20220617 add:沒有Site map資料依據測試機命令切換TJ Mode
        {
            for(i=0; i<4; i++)
            {
                iSendUseSite[i]=atoi(Str.SubString(10+i, 1).c_str())-1;
                if(iSendUseSite[i]==3)                                          //Ifor 20230426 add  新增 NEXTSTEP 4 控溫模式
                    iSendUseSite[i]=2;
            }
        }

        if(iSendUseSite[0]==1 || iSendUseSite[1]==1 ||iSendUseSite[2]==1 ||iSendUseSite[3]==1)
        {
            SendMSG_CMD(MSG_CMD_AMDNextStep2);
            iHasNEXTSTEP2=1;                                                    //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
        }
        else
        {
            SendMSG_CMD(MSG_CMD_AMDNextStep1);
            iHasNEXTSTEP2=0;                                                    //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
        }

        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            if(iATC_Use_Heat_Count>4)                                           //Ifor 20201023 add: ATC 控制器數量
                sprintf(cmd, "1038,8,%d,%d,%d,%d,-1,-1,-1,-1", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
            else
                sprintf(cmd, "1038,4,%d,%d,-1,-1", iSendUseSite[0], iSendUseSite[1]);
        }
        else if(iCurrentArm==2)                                                 //Handler ==> Arm 1 Down
        {
            if(iATC_Use_Heat_Count>4)                                           //Ifor 20201023 add: ATC 控制器數量
                sprintf(cmd, "1038,8,-1,-1,-1,-1,%d,%d,%d,%d", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
            else
                sprintf(cmd, "1038,4,-1,-1,%d,%d",iSendUseSite[0],iSendUseSite[1]);
        }

        if(iSendUseSite[0]==0 && iSendUseSite[1]==0 && iSendUseSite[2]==0 && iSendUseSite[3]==0)
        {
            ZeroMemory(bCheckDiodeThermal, sizeof(bCheckDiodeThermal));         //Ifor 20190528 : add Diode Thermal Check
        }

        if(iCurrentArm==1 || iCurrentArm==2)
        {
            CommAMD->WriteCommData(cmd,strlen(cmd));
            S.sprintf("%s H2A: %s", Task, cmd);
            WriteLog(S);
        }

        if(chkTempReady->Checked==true)                                         //ifor 20181227 : add 不等ATC回覆Temp Ready
            bTempHasReady=true;
        else
            bTempHasReady=false;
        bResult=true;
    }
    else if(Str.Pos("016GETBARCODENUMBER025")==1)                               //Ifor 20190304 : add AMD 2D Barcode 回覆資料
    {
        AnsiString AsSendText="";
        TStringList *sList;

        sList=new TStringList();
        sList->CommaText=sBarCode->CommaText;                                   //Ifor 20200924 add:AMD Barcode 資料改由GPIB 處理
        for(int i=0; i<4; i++)
        {
            if(sList->Strings[31-i]=="0")
            {
                AsSendText=AsSendText+"NA";
            }
            else
            {
                AsSendText=AsSendText+sList->Strings[31-i];
            }
            if(i==3)
                AsSendText=AsSendText+"#";
            else
                AsSendText=AsSendText+",";
        }
        sList->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete sList;
        sBarCode->CommaText=Check2Dsum(AsSendText);

        if(LastSet.bUseBarcodeFunction==false || LastSet.bSQR41==false)         //Contact 未按下 0X41 回覆未開啟 Barcode
        {
            if(iTestMode==SingleSite)
            {
                sBarCode->CommaText=Check2Dsum("BARCODEDISABLED,NA,NA,NA#");
            }
            else if(iTestMode==DualSite)
            {
                sBarCode->CommaText=Check2Dsum("BARCODEDISABLED,BARCODEDISABLED,NA,NA#");
            }
            else if(iTestMode==QualSite2X2 || iTestMode==QualSite1X4)           //Ifor 20210125 add:
            {
                sBarCode->CommaText=Check2Dsum("BARCODEDISABLED,BARCODEDISABLED,BARCODEDISABLED,BARCODEDISABLED#");
            }
        }
        sBarCodeMsg.sprintf("%s\r\n", AnsiString(sBarCode->CommaText));         //Ifor 20190923 Fix GPIB 回覆命令無 \r\n  //AI(W906-GB-P1) 20260926: StringsProxy/CommaTextProxy -> AnsiString for the varargs sprintf (rule 5)
        MyGPIBWrite(sBarCodeMsg, Task);
        LastSet.bHasBarCode=true;
        bResult=true;
    }
    else if(Str.Pos("READTEMP")!=0)                                             //Ifor 20201030 add:讀取TC溫度
    {
        if(iATC_Use_Heat_Count>4)
        {
            S2.sprintf("1 %5.2f %5.2f% 5.2f %5.2f %5.2f %5.2f% 5.2f %5.2f", dBySiteTC[0], dBySiteTC[1], dBySiteTC[2], dBySiteTC[3], dBySiteTC[4], dBySiteTC[5], dBySiteTC[6], dBySiteTC[7]);
        }
        else
        {
            S2.sprintf("1 %5.2f %5.2f% 5.2f %5.2f", dBySiteTC[0], dBySiteTC[1], dBySiteTC[2], dBySiteTC[3]);
        }
        MyGPIBWrite(S2, Task);
        bResult=true;
    }
    else if(Str.Pos("READDAQ")!=0)                                              //Ifor 20201030 add:讀取TJ溫度
    {
        if(iATC_Use_Heat_Count>4)
        {
            S2.sprintf("1 %5.2f %5.2f% 5.2f %5.2f %5.2f %5.2f% 5.2f %5.2f", dBySiteTJ[0], dBySiteTJ[1], dBySiteTJ[2], dBySiteTJ[3], dBySiteTJ[4], dBySiteTJ[5], dBySiteTJ[6], dBySiteTJ[7]);
        }
        else
        {
            S2.sprintf("1 %5.2f %5.2f% 5.2f %5.2f", dBySiteTJ[0], dBySiteTJ[1], dBySiteTJ[2], dBySiteTJ[3]);
        }
        MyGPIBWrite(S2, Task);
        bResult=true;
    }
    else if(Str.Pos("SOFTBIN")==1)                                              //Steven 20220120 : Amlogic需要收SBIN
    {
        GGpib2Handler.GPIBBin=Str.Length();
//        strncpy(GGpib2Handler.cReturn, Str.c_str(), GGpib2Handler.GPIBBin);    //AI(W906-GB-P1) 20260926: golden line, see below
        //AI(W906-GB-P1) 20260926: golden copies the whole SOFTBIN line starting at cReturn[256] and deliberately runs
        //  on into GpibStatus[32] + GpibData[256]: the `<544` guard below is exactly 256+32+256, i.e. golden treats
        //  cReturn..GpibData as one 544-byte buffer (and, as in golden, the copy happens BEFORE the guard).  Writing past
        //  a member array is UB in C++, and for GPIBBin>=544 golden also ran over GPIBBin/bOneCycle and past the end of
        //  GGpib2Handler.  Intent kept: the same strncpy into the same contiguous span, addressed through the object's
        //  bytes and clamped to the span.  Lines shorter than 544 give byte-identical packets (no NUL written, exactly as
        //  strncpy with n==strlen); lines of 544+ are not sent in either version (golden could also clobber GPIBBin).
        {
            static_assert(offsetof(VM, GpibStatus)==offsetof(VM, cReturn)+sizeof(GGpib2Handler.cReturn) &&
                          offsetof(VM, GpibData)==offsetof(VM, GpibStatus)+sizeof(GGpib2Handler.GpibStatus),
                          "VM cReturn/GpibStatus/GpibData must be contiguous (golden SOFTBIN 544-byte span)");
            const size_t kSoftBinSpan=sizeof(GGpib2Handler.cReturn)+sizeof(GGpib2Handler.GpibStatus)+sizeof(GGpib2Handler.GpibData);
            size_t nSoftBin=(size_t)GGpib2Handler.GPIBBin;
            if(nSoftBin>kSoftBinSpan)
                nSoftBin=kSoftBinSpan;
            strncpy(reinterpret_cast<char*>(&GGpib2Handler)+offsetof(VM, cReturn), Str.c_str(), nSoftBin);
        }
        if(GGpib2Handler.GPIBBin<544)
        {
            SendMSG_CMD(MSG_CMD_SBIN);
            str2.sprintf("ECHO%s", Str);
            MyGPIBWrite(str2, Task);
        }
        bResult=true;
    }
    else if(Str.Pos("TOGGLE TC2")!=0)                                           //Ifor 20230103 add: TC2 控溫模式
    {
        SleepEx(1, false);
        MyGPIBWrite("1", Task);

        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            if(iATC_Use_Heat_Count>4)                                           //Ifor 20201023 add: ATC 控制器數量
                sprintf(cmd, "1038,8,2,2,2,2,-1,-1,-1,-1");
            else
                sprintf(cmd, "1038,4,2,2,-1,-1");
        }
        else if(iCurrentArm==2)                                                 //Handler ==> Arm 1 Down
        {
            if(iATC_Use_Heat_Count>4)                                           //Ifor 20201023 add: ATC 控制器數量
                sprintf(cmd, "1038,8,-1,-1,-1,-1,2,2,2,2");
            else
                sprintf(cmd, "1038,4,-1,-1,2,2");
        }

        if(iCurrentArm==1 || iCurrentArm==2)
        {
            CommAMD->WriteCommData(cmd, strlen(cmd));
            S.sprintf("%s H2A: %s", Task, cmd);
            WriteLog(S);
        }
        bResult=true;
    }
    else if(Str.Pos("READYNEXTSHOT?")>0)                                        //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
    {
        SendMSG_CMD(MSG_CMD_READYNEXTSHOT);
        bResult=true;
    }
    else if(Str.Pos("NEXT2DID?")>0)                                             //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
    {
        SendMSG_CMD(MSG_CMD_NEXT2DID);
        bResult=true;
    }
    else if(Str.Pos("SetSiteOnOff:")==1)                                        //JimmyChiu 20250715 : Auto site on/off by GPIB
    {
        SendMSG_CMD(MSG_CMD_SetSiteOnOff, Sitemapstr);
        bResult=true;
    }
    return bResult;
}
//------------------------------------------------------------------------------
void TSerialPoll::InitialBarcodeList()
{
    if(sBarCode->Count==MAX_SITE_COUNT)
    {
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            sBarCode->Strings[i]="0";
        }
    }
    else
    {
        sBarCode->Clear();
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            sBarCode->Add("0");
        }
    }

    if(MY_DUT_PAL.size()!=0)
    {
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            MY_DUT_PAL[i]->labOcr->Caption="0";
        }
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::SetParameter(AnsiString str)                                  //wei 20151127 設定參數
{
    AnsiString ascCommandBuffer="";

    ascCommandBuffer=str;
    if(ascCommandBuffer.Pos("SETTEMP +")==1)
    {
        ascCommandBuffer.Delete(1, 9);
        TempStr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETTEMP_")==1)                                //KaiChen 20181129 ：Add GPIB SETTEMP_
    {
        ascCommandBuffer.Delete(1, 8);
        TempStr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETSOAK_")==1)                                //KaiChen 20181129 ：Add GPIB SETSOAK_
    {
        ascCommandBuffer.Delete(1, 8);
        TempStr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETSOAK")==1)
    {
        ascCommandBuffer.Delete(1, 7);
        TempStr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETSITEMAP_")==1)                             //KaiChen 20181129 ：Add GPIB SETSITEMAP_
    {
        ascCommandBuffer.Delete(1, 11);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETSITEMAP ")==1)
    {
        ascCommandBuffer.Delete(1, 11);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SGSETUP_")==1)                                //KaiChen 20190613 ：Add GPIB SGSETUP_
    {
        ascCommandBuffer.Delete(1, 8);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETUP")==1)
    {
        ascCommandBuffer.Delete(1, 6);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETTESTTEMP +")==1)
    {
        ascCommandBuffer.Delete(1, 13);
        TempStr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("BINPOS_")==1)                                 //KaiChen 20190706 ：Add GPIB BINPOS_
    {
        ascCommandBuffer.Delete(1, 7);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETSTARTMODE_")==1)                           //KaiChen 20180910 ：Add GPIB SetStartMode_
    {
        ascCommandBuffer.Delete(1, 13);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SGFTP_")==1)                                  //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
    {
        ascCommandBuffer.Delete(1, 6);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("NONDOUBLEBIN_")==1)                           //Sam 20210329 : Add GPIB NONDOUBLEBIN_
    {
        ascCommandBuffer.Delete(1, 13);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("BINCOUNT_")==1)                               //Sam 20210329 : Add GPIB BINCOUNT_
    {
        ascCommandBuffer.Delete(1, 9);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SGOSBIN_")==1)                                //Sam 20210406 : Add GPIB SGOSBIN_
    {
        ascCommandBuffer.Delete(1, 8);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SGCONTFAIL_")==1)                             //Sam 20210422 : Add GPIB SGCONTFAIL_
    {
        ascCommandBuffer.Delete(1, 11);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETTESTERID_")==1)                            //Sam 20210617 : Add GPIB SETTESTERID
    {
        ascCommandBuffer.Delete(1, 12);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("ONECYCLE_")==1 ||
            ascCommandBuffer.Pos("ONECYCLE")==1)                                //Sam 20221103 : OneCycle 完後顯示訊息
    {
        if(ascCommandBuffer.Pos("ONECYCLE_")==1)                                //OneCycle 完成需要顯示訊息
        {
            ascCommandBuffer.Delete(1, 9);
            sOneCycleMsg=ascCommandBuffer;
        }
        else                                                                    //一般 OneCycle
        {
            sOneCycleMsg="";
        }
    }
    else if(ascCommandBuffer.Pos("SETMAXTEST_")==1)                             //Sam 20230201 : Add GPIB SETMAXTEST_
    {
        ascCommandBuffer.Delete(1, 11);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETINITIALMAXTEST_")==1)                      //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
    {
        ascCommandBuffer.Delete(1, 18);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SETAICCD_")==1)                               //Sam 20231108 : Add GPIB SETAICCD_
    {
        ascCommandBuffer.Delete(1, 9);
        Sitemapstr=ascCommandBuffer;
    }
    else if(ascCommandBuffer.Pos("SetSiteOnOff:")==1)                           //JimmyChiu 20250715 : Auto site on/off by GPIB
    {
        AnsiString sfilter="SetSiteOnOff:";
        ascCommandBuffer.Delete(1, sfilter.Length());
        Sitemapstr=ascCommandBuffer;
    }
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
