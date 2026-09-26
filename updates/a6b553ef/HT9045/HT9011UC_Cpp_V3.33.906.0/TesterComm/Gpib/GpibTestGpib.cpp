// ===========================================================================
//  TesterComm/Gpib/GpibTestGpib.cpp -- TSerialPoll::TestGPIB(), the GPIB device-side state machine.
//
//  AI(W906-GB-P1) 20260926: faithful translation of golden
//  D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp:1458-3080 (Big5/cp950 -> UTF-8), per
//  TesterComm/Gpib/TRANSLATION_RULES.md.  The body is golden text line by line (every customer branch, the
//  function-local statics, `static int &Task=iGbibTask`, `goto TEST_GPIB` and the cross-case `goto GoToLabel`
//  into case 400 are all kept exactly as golden).  Deviations, each marked //AI(W906-GB-P1) in place:
//    * `__fastcall` dropped (rule 3).
//    * buffer/tempbuffer are StrLength+1 bytes (golden StrLength): golden `buffer[ibcnt]=0` after a full
//      iBufferCount (== StrLength) read writes one past the end.
//    * MY_DUT_PAL[i] -> MY_DUT_PAL.at(i) so the golden try/catch(...) still catches a missing panel.
//    * strupr: file-local helper below (Borland RTL strupr semantics).
//  Not here: golden Main.cpp:1456-1457 `HTimer WaitRequestDelay; HTimer FRWaitRequestDelay;` are defined by
//  another bridge file (declared extern in GpibBridge.h), as are every TSerialPoll member this body calls
//  (UpdateLed, WriteLog, SetParameter, ProcessStatusString*, MyGPIBWrite, SendMSG_CMD*, CheckGSBINONString,
//  InitialBarcodeList, TestGPIBForCastle) and the free function CheckBINONString (Main.cpp:991).
//  NI calls (ibfind/ibrsc/ibpad/ibtmo/ibwait/ibrd/ibrsv/ibwrt/ibstop, ibsta/ibcnt) are the gpibbridge::
//  wrappers of GpibDriver.h.  CC_ASE_JP / CC_KYEC_LEE come from MachineType.h and eAMD (enum e2DIDFormat)
//  from MachineType.h:1164, both via MessageDef.h.
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace gpibbridge {

//AI(W906-GB-P1) 20260926: golden calls Borland RTL `strupr(char*)` (string.h): converts 'a'..'z' to upper case
//  IN PLACE and returns its argument.  MinGW's <string.h> hides the OLDNAMES strupr under -std=c++17 (tree probe:
//  EJ1N/TextProcess.cpp:74-79 "strupr was not declared").  C-locale semantics as BCB6 ran it: ASCII 'a'..'z' only,
//  not MBCS aware (a Big5 trail byte in 0x61..0x7A is upper-cased too, same as Borland).  File-local, so the
//  golden `strupr(buffer)` text is kept; unqualified lookup inside namespace gpibbridge finds this one first.
static char* strupr(char* s)
{
    if(s==NULL)
        return s;
    for(char* p=s; *p!='\0'; ++p)
    {
        if(*p>='a' && *p<='z')
            *p=static_cast<char>(*p-'a'+'A');
    }
    return s;
}
//------------------------------------------------------------------------------
// golden Main.cpp:1456-1457 -- defined elsewhere in the bridge (GpibBridge.h: extern HTimer WaitRequestDelay,
// FRWaitRequestDelay):
//HTimer WaitRequestDelay;
//HTimer FRWaitRequestDelay;                                                      //Steven 20120112 : 進FR?沒有回應會卡死
int TSerialPoll::TestGPIB()
{
    if(LastSet.iTesterMode==InterfaceType_Delta_Castle)                         //Ifor 20200519 add: Castle Tester // 2015.08.29 , Joye , Castle
    {
        return TestGPIBForCastle();
    }

//    bool bCloseSiteHaveBinErr=false;                                          //Steven 20141016 : 關Site不能有Bin       //Steven 20170214 (wei): 改成全域變數
    //AI(W906-GB-P1) 20260926: golden `buffer[StrLength], tempbuffer[StrLength]`.  ibrd() reads up to iBufferCount (2560 ==
    //   StrLength) bytes and every read is followed by `buffer[ibcnt]=0` / `tempbuffer[ibcnt]=0`, so a full
    //   2560-byte read writes index 2560 -- one past the end (UB in C++; in BCB6 it hit the next static).  The
    //   +1 keeps the golden intent (NUL-terminate the text just read) without the overrun.  The golden
    //   StrLength / iBufferCount uses below are unchanged; sizeof(buffer) now includes the extra byte, which
    //   is only ever 0, so the strncpy(..., sizeof(...)) copies below end on the same text.
    static char buffer[StrLength+1], tempbuffer[StrLength+1];
    static int &Task=iGbibTask, i;
    int Mask, iLength, iEndChar, iCommomPos;
    bool bHasLrLn=false;
    bool bHadCommon=false;
    char *pstr,C;
    char tstr[TOTAL_SITE][16];
    static AnsiString S,SiteString,BS,asTemp;
    static bool bEchoNG=false;
    static bool bFirstIn=true;
    bool bIsTesterError;
    AnsiString asBuffer="", asCheckBuffer="";                                   //KaiChen 20181025 ：Add GPIB SETUPFILENAME_ 保留小寫
    static bool bErrorFlag=false;
    static int iErrorFlag=0;
    static int iBinonCount=0;
    static int iBackupFRTask=1;                                                 //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
    AnsiString cAsstring="";
    int iChange=0;
    int data;
    int iFullSiteTimeOut;                                                       //ChungHung 20141029 add for SCK want to enable/disable and setting time

    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life.  Golden: every Handler
    //   RunTestProgram relaunches the exe, so these start at their initialisers each life (see g_bridgeLife in
    //   GpibBridge.h).  Task is bound to the global iGbibTask (ResetBridgeGlobals resets it); i is loop scratch.
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        memset(buffer, 0, sizeof(buffer));
        memset(tempbuffer, 0, sizeof(tempbuffer));
        i=0;
        S=""; SiteString=""; BS=""; asTemp="";
        bEchoNG=false;
        bFirstIn=true;
        bErrorFlag=false;
        iErrorFlag=0;
        iBinonCount=0;
        iBackupFRTask=1;
    }

    TEST_GPIB:
    UpdateLed();

    switch(Task)
    {
        case 1:
            bEchoNG=false;
            iRCMDBackupTask=1;                                                  //JerryYang 20190226 RCMD command
            if(oldGpibAddress!=GpibAddress)
            {
                noncontroller=ibfind("gpib0");                                  // Open a session to the GPIB board
                if(noncontroller<0)
                {
                    if(bSimulate==false)
                        WriteLog("0001 Error Open GPIB0");
                    return 2;
                }
                oldGpibAddress=GpibAddress;
                LastSet.GpibAddress=GpibAddress;
                StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
                ibrsc(noncontroller, 0);                                        // Release system control
                ibpad(noncontroller, GpibAddress);                              // Change primary address from 0 to GpibAddress
                ibtmo(noncontroller, LastSet.iTimeOut);
                if(IsTest)
                {
                    Task=100;
                    break;
                }
            }

            if(bESCIsOpen)                                                      //Steven 20201022 : For RFMD
            {
                bESCIsOpen=false;
                Task=7000;
                break;
            }

            if(IsTest==false)                                                   //1015 lee
            {
                if(IsTestDelay.Off()==false)                                    //20220616 wei 次數計數改為時間延遲
                {
                    break;
                }

                ibwait(noncontroller, 0);                                       // Update Status variable
                UpdateLed();
                if(ibsta&LACS)                                                  //if LTX ,sned first all system will very slow // Wait until non-controller is listener and ATN line is dropped.
                {
                    ibrd(noncontroller, buffer, iBufferCount);                  // Read data bytes
                    strncpy(tempbuffer, buffer, sizeof(tempbuffer));            //Steven 20110608
                    if(ibcnt==0)
                    {
                        buffer[ibcnt]=0;
                        if((ibsta&0x04) && iLotMode!=0)
                        {
                            bMessageFromHandler=true;                           //Steven 20170413 (wei) : Add ART simulator
                            if(LastSet.bDummyART==false)
                            {
                                if(LastSet.iTesterMode==InterfaceType_15BinQorvo)
                                {
                                    WriteLog("0001 SRQ:0x48");                  //Steven 20241004 : For Qorvo ART
                                    ibrsv(noncontroller, 0x48);
                                }
                                else
                                {
                                    WriteLog("0001 SRQ:0xC0");
                                    ibrsv(noncontroller, 0xC0);
                                }
                            }
                            else
                            {
                                WriteLog("Dummy FT ==> SRQ:0xC0");
                            }
                            iLotModeGPIB=iLotMode;
                            LastSet.bSRQC0=true;
                            iLotMode=0;
                        }
                        break;
                    }

                    asBuffer.sprintf("Byte=%d     str=%s", ibcnt, buffer);
                    asCheckBuffer.sprintf("%s", buffer);
                    mmoBINON->Lines->Add(asBuffer);
                    buffer[ibcnt]=0;
                    BS=buffer;

                    if(asCheckBuffer.Pos("SGSETUP_")==1)                        //KaiChen 20181025 ：Add GPIB SETUPFILENAME_ 保留小寫
                    {
                        pstr=buffer;
                    }
                    else
                    {
                        pstr=strupr(buffer);
                    }
                    SetParameter(buffer);                                       //wei 20151127 設定參數

                    S="0001 LISTEN:"+AnsiString(pstr);
                    WriteLog(S);
                    strncpy(buffer, pstr, sizeof(buffer));
                    S=buffer;
                    if(S.Pos("FR?")==1)
                    {
                        asShowMemo="0001";
                        iBackupFRTask=1;                                        //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                        FRWaitRequestDelay.SetSecAndOn(5);                      //Steven 20120112 : 進FR?沒有回應會卡死

                        LastSet.bSRQC0=false;                                   //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagRCMD=false;                                //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagSVID=false;                                //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagECID=false;                                //jou 2015-09-21 Auto Retest function
                        Task=1000;
                        goto TEST_GPIB;
//                        break;
                    }
                    else if(ProcessStatusStringRFMD(S, BS, "0001")>=0)          //Steven 20201022 : For RFMD
                    {
                        if(bMustWaitESC==true && bESCIsOpen==false)
                        {
                            IsTest=false;
                            Task=1;
                        }
                        break;
                    }
                    else if(ProcessStatusString(S, BS, "0001"))                 //Steven 20150713 : 整合部分COMMAND
                    {
                        break;
                    }
                    else if(ProcessStatusStringRCMD(S, BS, "0001"))             //JerryYang 20190226 RCMD command
                    {
                        iRCMDBackupTask=Task;
                        Task=2000;
                    }
                    else if(S.Pos("BINON")==1)
                    {
                        //Steven 20110608 Start
                        strncpy(buffer, tempbuffer, sizeof(buffer));
                        Task=400;

                        if(LastSet.bSQR41)                                      //Steven 20110914
                        {
                            IsTest=true;
                            iMainTask=100;
                        }
                        else
                        {
                            if(LastSet.bBinonEcho)                              //jou 2011-12-02 工程模式測試機會直接送BINON回來，如果不處理，測試機會一直送一直送 //Steven 20111219
                            {
                                IsTest=true;
                                iMainTask=100;
                            }
                            else
                            {
                                IsTest=false;
                                break;
                            }
                        }

                        if(iHasNEXTSTEP2==1)                                    //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
                        {
                            iHasNEXTSTEP2=2;                                    //0:TC Mode 1:TJ Mode 2:Auto Switch TC
                        }
                        goto GoToLabel;
                    }
                    else if(S.Pos("FULLSITES?")==1)
                    {
                        Task=300;
                        bFirstIn=true;
                        LastSet.bFULLSITES=true;                                //kevin 20130516
                        iMainTask=1;
                        break;
                    }
                    else if(LastSet.bFlagRCMD==false && S.Pos("RCMD:")!=0)      //jou 2015-09-21 Auto Retest function
                    {
                        asShowMemo="0001";
                        LastSet.bFlagRCMD=true;
                        bECHO_FlagRCMD=false;
                        strncpy(GGpib2Handler.cReturn, S.c_str(), sizeof(GGpib2Handler.cReturn));
                        SendMSG_CMD(MSG_CMD_RCMD);
                        Task=2000;
                        break;
                    }
                    else if(LastSet.bFlagSVID==false && S.Pos("SVID:")!=0)      //jou 2015-09-21 Auto Retest function
                    {
                        asShowMemo="0001";
                        LastSet.bFlagSVID=true;
                        bECHO_FlagSVID=false;
                        strncpy(GGpib2Handler.cReturn, S.c_str(), sizeof(GGpib2Handler.cReturn));
                        SendMSG_CMD(MSG_CMD_SVID);
                        Task=3000;
                        break;
                    }
                    else if(LastSet.bFlagECID==false && S.Pos("ECID:")!=0)      //jou 2015-09-21 Auto Retest function
                    {
                        asShowMemo="0001";
                        LastSet.bFlagECID=true;
                        bECHO_FlagECID=false;
                        strncpy(GGpib2Handler.cReturn, S.c_str(), sizeof(GGpib2Handler.cReturn));
                        SendMSG_CMD(MSG_CMD_ECID);
                        Task=4000;
                        break;
                    }
                    else if(S.Pos("RETESTFLAG")!=0)                             //jou 2015-09-21 Auto Retest function
                    {
                        bECHO_FlagReset=false;
                        asShowMemo="0001";
                        strncpy(GGpib2Handler.cReturn, S.c_str(), sizeof(GGpib2Handler.cReturn));
                        SendMSG_CMD(MSG_CMD_RetestFlag);
                        Task=6000;
                        break;
                    }
                    else if(LastSet.bRunHANA_ART==true && S!="")                //JimmyChiu 20241023 HANA ART Function
                    {                                                           //Steven 20250414 : HANA ART Function
                        asShowMemo="0001";
                        SendMSG_CMD(MSG_CMD_HANA_ART, S);
                        break;
                    }
                    else
                    {
                        if(LastSet.i2DIDFormat==eAMD)                           //JerryYang 20200422 2DID format選項改用下拉選單
                        {
                            MyGPIBWrite("1", "0001");
                        }
                    }
                }
                else if(iLotMode!=0)
                {
                    bMessageFromHandler=true;                                   //Steven 20170413 (wei) : Add ART simulator
                    if(LastSet.bDummyART==false)
                    {
                        if(LastSet.iTesterMode==InterfaceType_15BinQorvo)
                        {
                            WriteLog("0001 SRQ:0x48");                          //Steven 20241004 : For Qorvo ART
                            ibrsv(noncontroller, 0x48);
                        }
                        else
                        {
                            WriteLog("0001 SRQ:0xC0");
                            ibrsv(noncontroller, 0xC0);
                        }
                    }
                    else
                    {
                        WriteLog("Dummy FT ==> SRQ:0xC0");
                    }
                    iLotModeGPIB=iLotMode;
                    LastSet.bSRQC0=true;
                    iLotMode=0;
                }
                break;
            }

            bMustWaitESC=false;
            if(bESCIsOpen)                                                      //Steven 20201022 : For RFMD
            {
                bESCIsOpen=false;
                Task=7000;
                break;
            }

            Task=100;
//jou 980818 add speed
//            break;
        case 100:                                                               //jou 2011-11-24 GPIB斷線就不應該送出測試訊號
            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
            UpdateLed();

            if(iDoubleContact &&                                                //Steven 20220830 : 修正Double Contact
               LastSet.iTesterMode!=InterfaceType_15BinQorvo &&
               CustomerCode!=CC_ASE_JP)                                         //Sam 20220817 : ASE_JP DoubleContact 指令要另外丟
            {
                if(iDoubleContact==1)
                {
                    WriteLog("0100 SRQ:0x42");
                    ibrsv(noncontroller, 0x42);
                }
                else if(iDoubleContact==2)                                      //Joseph 20220830 (Jason,Maxwu) 新增0x43 Double Contact S //
                {
                    WriteLog("0100 SRQ:0x43");
                    ibrsv(noncontroller, 0x43);
                }
                else if(iDoubleContact==3)                                      //JerryYang 20230721 : double contact送0xC1
                {
                    WriteLog("0100 SRQ:0xC1");
                    ibrsv(noncontroller, 0xC1);
                }
            }
            else
            {
                if(LastSet.bRunHANA_ART==true &&                                //Steven 20250414 : HANA ART Function
                   bHanaDummyTest==true)
                {
                    WriteLog("0100 Dummy Test SRQ:0x42");
                    ibrsv(noncontroller, 0x42);
                }
                else
                {
                    WriteLog("0100 SRQ:0x41");
                    ibrsv(noncontroller, 0x41);
                }
            }

            if(cbFullSiteTimeOut->Checked)                                      //ChungHung 20141029 add for SCK want to enable/disable and setting time
            {
                iFullSiteTimeOut=atoi(cbbFullsiteTimeOut->Text.c_str());
                TimerFullSite.SetSecAndOn(iFullSiteTimeOut);
                //TimerFullSite.SetSecAndOn(5);                                 //Steven 20141016 : FullSite的Test Time Out
            }

            LastSet.bSQR41=true;                                                //Steven 20110909
            LastSet.bFULLSITES=false;                                           //kevin 20130516
            GGpib2Handler.bOneCycle=false;                                      //jou 2015-10-05 增加 0x41 之後的 "ONECYCLE" 指令支援

            LastSet.bSRQC0=false;                                               //jou 2015-09-21 Auto Retest function
            LastSet.bFlagRCMD=false;                                            //jou 2015-09-21 Auto Retest function
            LastSet.bFlagSVID=false;                                            //jou 2015-09-21 Auto Retest function
            LastSet.bFlagECID=false;                                            //jou 2015-09-21 Auto Retest function

            if(bCatalystSimpleGPIB)
            {
                Task=400;
                break;                                                          //jou 980818 start : add speed
            }
            else
            {
                Task=200;
            }
        case 200:
            ibwait(noncontroller, 0);                                           // Update Status variable
            if((ibsta&LACS) && (!(ibsta&ATN)))                                  // Wait until non-controller is listener and ATN line is dropped.
            {
                ibrd(noncontroller, buffer, iBufferCount);                      // Read data bytes
                strncpy(tempbuffer, buffer, sizeof(tempbuffer));                //Steven 20110608
                buffer[ibcnt]=0;
                BS=buffer;                                                      //Eliot 2010_1203
                pstr=strupr(buffer);
                SetParameter(buffer);                                           //wei 20151127 設定參數
                S="0200 LISTEN:"+AnsiString(pstr);
                WriteLog(S);
                strncpy(buffer, pstr, sizeof(buffer));
                S=buffer;
                if(S.Pos("FULLSITES?")==1)
                {
                    WriteLog("0200 SRQ:0x00");
                    ibrsv(noncontroller, 0x00);
                    ZeroMemory(bCheckDiodeThermal, sizeof(bCheckDiodeThermal)); //Eliot 20190617
                    data=0;

                    for(i=0; i<TOTAL_SITE; i++)
                    {
                        data<<=1;
                        data|=iStart[31-i];
                    }

                    if(LastSet.bUpperCase)                                      //Eliot 2010_1129 start
                    {
                        asTemp.sprintf("%08x", data);
                        asTemp=asTemp.UpperCase();
                        if(CustomerCode==CC_ASE_JP)                             //Sam 20220817 : ASE_JP DoubleContact 指令要另外丟
                        {
                            if(iDoubleContact>0)
                                GpibString.sprintf("Fullsites %s;R\r\n", asTemp);
                            else
                                GpibString.sprintf("Fullsites %s;T\r\n", asTemp);
                        }
                        else if(LastSet.bHaveContactorInfo)                     //Steven 20201022 : For RFMD
                        {
                            GpibString.sprintf("Fullsites %s;CONTACTOR %d,%s\r\n", asTemp, iCurrentArm, asTemp);
                        }
                        else
                        {
                            GpibString.sprintf("Fullsites %s\r\n", asTemp);
                        }
                    }
                    else
                    {
                        if(CustomerCode==CC_ASE_JP)                             //Sam 20220817 : ASE_JP DoubleContact 指令要另外丟
                        {
                            if(iDoubleContact>0)
                                GpibString.sprintf("Fullsites %08x;R\r\n", data);
                            else
                                GpibString.sprintf("Fullsites %08x;T\r\n", data);
                        }
                        else if(LastSet.bHaveContactorInfo)                     //Steven 20201022 : For RFMD
                        {
                            GpibString.sprintf("Fullsites %08x;CONTACTOR %d,%08x\r\n", data, iCurrentArm, data);
                        }
                        else
                        {
                            GpibString.sprintf("Fullsites %08x\r\n", data);
                        }
                    }

                    if(data==0)
                    {
                        IsTest=false;
                        Task=1;
                        return 0;
                    }
                    LastSet.bFULLSITES=true;                                    //kevin 20130516
                    bFirstIn=true;
                    Task=300;
                    break;
                }
                else if(S.Pos("FR?")==1)
                {
                    asShowMemo="0200";
                    iBackupFRTask=200;                                          //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                    Task=1000;
                    FRWaitRequestDelay.SetSecAndOn(5);                          //Steven 20120112 : 進FR?沒有回應會卡死
                    goto TEST_GPIB;
//                    break;
                }
                else if(ProcessStatusStringRFMD(S, BS, "0200")>=0)              //Steven 20201022 : For RFMD
                {
                    if(bMustWaitESC==true && bESCIsOpen==false)
                    {
                        IsTest=false;
                        Task=1;
                    }
                    break;
                }
                else if(ProcessStatusString(S, BS, "0200"))                     //Steven 20150713 : 整合部分COMMAND
                {
                    break;
                }
                else if(ProcessStatusStringRCMD(S, BS, "0200"))                 //JerryYang 20190226 RCMD command
                {
                    iRCMDBackupTask=Task;
                    Task=2000;
                }
                else if(S.Pos("BINON")==1)
                {
                    strncpy(buffer, tempbuffer, sizeof(buffer));                //Steven 20110608 Start
                    Task=400;
                    if(LastSet.bSQR41)                                          //Steven 20110914
                    {
                        iBinonCount=0;
                        IsTest=true;
                        iMainTask=100;
                    }
                    else
                    {
                        if(LastSet.bBinonEcho)                                  //Steven 20111219
                        {
                            iBinonCount=0;
                            IsTest=true;
                            iMainTask=100;
                        }
                        else
                        {
                            IsTest=false;
                            break;
                        }
                    }

                    if(iHasNEXTSTEP2==1)                                        //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
                    {
                        iHasNEXTSTEP2=2;                                        //0:TC Mode 1:TJ Mode 2:Auto Switch TC
                    }
                    goto GoToLabel;
                }
                else if(S.Pos("ONECYCLE")!=0)                                   //jou 2015-10-05 增加 0x41 之後的 "ONECYCLE" 指令支援
                {
                    asShowMemo="0200";
                    Task=5000;
                }
                else if(LastSet.bRunHANA_ART==true && S!="")                    //JimmyChiu 20241023 HANA ART Function
                {                                                               //Steven 20250414 : HANA ART Function
                    asShowMemo="0200";
                    SendMSG_CMD(MSG_CMD_HANA_ART, S);
                    break;
                }
                else
                {
                    Task=200;                                                   // error echo
                    break;
                }
            }
            else if(cbFullSiteTimeOut->Checked && TimerFullSite.Off())          //ChungHung 20141029 add for SCK want to enable/disable and setting time   //Steven 20141016 : FullSite的Test Time Out
            {
                SendMSG_CMD(MSG_CMD_NoFullSiteRespon);
                IsTest=false;                                                   //ChungHung 20141017 fix FullSite Problem
                Task=1;
                return 4;                                                       //ChungHung 20141031 3改為4
            }
            break;
        case 300:
            if(bFirstIn)                                                        //Steven 20110920
            {
                WaitRequestDelay.SetSecAndOn(10);                               //Steven 20170717 (Jou) : 改成10秒, 因為ASE-M的測試機是1.5sec後才會傳送資料
                bFirstIn=false;
            }
            bEchoNG=false;
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            if((ibsta&TACS) && (!(ibsta&ATN)))                                  // If addressed to talk, send the response "I am a talker"
//            {
            if(MyGPIBWrite(GpibString, "0300"))                                 //Steven 20250926 : 變更GPIB write流程
            {
                if(LastSet.bSQR41)                                              //Steven 20110914
                {
                    Task=400;
                }
                else
                {
                    Task=1;
                    break;
                }
            }
            else
            {
                if(WaitRequestDelay.Off())                                      //Steven 20110920
                {
                    WriteLog("Wait to reply Fullsites Time Out!");              //Steven 20120222 : 加上記錄
                    if(LastSet.bSQR41)                                          //Steven 20120222 : 有在測試的話,不能回到Task1
                    {
                        Task=400;
                    }
                    else
                    {
                        Task=1;
                    }
//                    Task=1;                                                   //Steven 20150713 : mark
                }
                break;
            }
        case 400:
            bNeedRetest=false;
            ibwait(noncontroller, 0);                                           // Update Status variable
            if((ibsta&LACS) && (!(ibsta&ATN)))                                  // Wait until non-controller is listener and ATN line is dropped.
            {
                ZeroMemory(buffer, StrLength);                                  //Steven 20160215 : 先清空記憶體再讀取資料, 避免舊資料出現
                ibrd(noncontroller, buffer, iBufferCount);                      // Read data bytes
                buffer[ibcnt]=0;
                GoToLabel:

                BS=buffer;                                                      // 保留大小寫
//                BS="BINON:00000000,00000000,00000000,00000000;";

                if(BS=="")                                                      //jou 2011-01-24 start : 如遇到空字串自動過濾掉
                {
                    Task=200;
                    break;
                }

                pstr=strupr(buffer);
                SetParameter(buffer);                                           //wei 20151127 設定參數
//                pstr="BINON:00000000,00000000,00000000,00000000;";

                S="0400 LISTEN:"+AnsiString(BS);
                WriteLog(S);

                S=pstr;
                if(S.Pos("BINON")==1)
                {
                    strncpy(GGpib2Handler.cReturn, pstr, sizeof(GGpib2Handler.cReturn));
                    ZeroMemory(bCheckDiodeThermal, sizeof(bCheckDiodeThermal)); //Ifor 20190528 : add Diode Thermal Check
                    S=BS;

                    bErrorFlag=false;
                    iErrorFlag=0;
                    bTempHasReady=false;                                        //Ifor 20190125 : add
                    if(GPIBVersionCheck>5 &&
                       (LastSet.iTesterMode==InterfaceType_16BinGS ||
                        LastSet.iTesterMode==InterfaceType_32BinGS))            //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
                    {
                        bErrorFlag=CheckGSBINONString(S);
                    }
                    else
                    {
                        bErrorFlag=!CheckBINONString(pstr);
                        if(bErrorFlag==true)
                        {
                            for(i=0; i<TOTAL_SITE; i++)
                            {
                                result[i]=9999;
                            }

                            WriteLog("Tester ==> CHECK BINON STRING LENGTH ERROR");  //Steven 20141212 : Add GPIB Log
                        }
                        else
                        {
                            //BINON:00000000,00000000,00888888,88888888;\n
                            //01234567890123456789012345678901234567890
                            //          1   1     2  22     3 33
                            if(LastSet.iTesterMode==InterfaceType_15BinT6577)   //Steven 20181214 : ASE-CL add T6577
                            {
                                for(i=0; i<8; i++)
                                {
                                    //0~7
                                    C=pstr[33+i];
                                    if(     C>='0' && C<='9')   result[7-i]=C-'0';
                                    else if(C>='B' && C<='G')   result[7-i]=C-'B'+10;
                                    else if(C=='A')             result[7-i]=0;
                                    else
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=1;
                                    }

                                    //8~15
                                    C=pstr[24+i];
                                    if(     C>='0' && C<='9')   result[15-i]=C-'0';
                                    else if(C>='B' && C<='G')   result[15-i]=C-'B'+10;
                                    else if(C=='A')             result[15-i]=0;
                                    else
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=2;
                                    }

                                    //16~23
                                    C=pstr[15+i];
                                    if(     C>='0' && C<='9')   result[23-i]=C-'0';
                                    else if(C>='B' && C<='G')   result[23-i]=C-'B'+10;
                                    else if(C=='A')             result[23-i]=0;
                                    else
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=3;
                                    }

                                    //24~31
                                    C=pstr[6+i];
                                    if(     C>='0' && C<='9')   result[31-i]=C-'0';
                                    else if(C>='B' && C<='G')   result[31-i]=C-'B'+10;
                                    else if(C=='A')             result[31-i]=0;
                                    else
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=4;
                                    }
                                }
                            }
                            else if(LastSet.iTesterMode==InterfaceType_15BinQorvo)     //Steven 20201022 : Add Qorvo protocol
                            {
                                for(i=0; i<8; i++)
                                {
                                    //0~7
                                    C=pstr[33+i];
                                    if(     C>='0' && C<='9')   result[7-i]=C-'0';
                                    else if(C=='A')             result[7-i]=999;
                                    else if(C>='B' && C<='G')   result[7-i]=C-'B'+10;
                                    else
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //8~15
                                    C=pstr[24+i];
                                    if(     C>='0' && C<='9')   result[15-i]=C-'0';
                                    else if(C=='A')             result[15-i]=999;
                                    else if(C>='B' && C<='G')   result[15-i]=C-'B'+10;
                                    else
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //16~23
                                    C=pstr[15+i];
                                    if(     C>='0' && C<='9')   result[23-i]=C-'0';
                                    else if(C=='A')             result[23-i]=999;
                                    else if(C>='B' && C<='G')    result[23-i]=C-'B'+10;
                                    else
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //24~31
                                    C=pstr[6+i];
                                    if(     C>='0' && C<='9')   result[31-i]=C-'0';
                                    else if(C=='A')             result[31-i]=999;
                                    else if(C>='B' && C<='G')   result[31-i]=C-'B'+10;
                                    else
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }
                                }
                            }
                            else if(iBinSelect<=16)                             //0-15  bin
                            {
                                for(i=0; i<8; i++)
                                {
                                    //0~7
                                    C=pstr[33+i];
                                    if(     C>='0' && C<='9')   result[7-i]=C-'0';
                                    else if(C>='A' && C<='F')   result[7-i]=C-'A'+10;
                                    else
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //8~15
                                    C=pstr[24+i];
                                    if(     C>='0' && C<='9')   result[15-i]=C-'0';
                                    else if(C>='A' && C<='F')   result[15-i]=C-'A'+10;
                                    else
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //16~23
                                    C=pstr[15+i];
                                    if(     C>='0' && C<='9')   result[23-i]=C-'0';
                                    else if(C>='A' && C<='F')   result[23-i]=C-'A'+10;
                                    else
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }

                                    //24~31
                                    C=pstr[6+i];
                                    if(     C>='0' && C<='9')   result[31-i]=C-'0';
                                    else if(C>='A' && C<='F')   result[31-i]=C-'A'+10;
                                    else
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=5;
                                    }
                                }
                            }
                            else if(iBinSelect==17)                             //16  bin
                            {
                                for(i=0; i<8; i++)
                                {
                                    //0~7
                                    C=pstr[33+i];
                                    if(     C>='0' && C<='9')   result[7-i]=C-'0';
                                    else if(C>='A' && C<='G')   result[7-i]=C-'A'+10;
                                    else
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=6;
                                    }

                                    //8~15
                                    C=pstr[24+i];
                                    if(     C>='0' && C<='9')   result[15-i]=C-'0';
                                    else if(C>='A' && C<='G')   result[15-i]=C-'A'+10;
                                    else
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=6;
                                    }

                                    //16~23
                                    C=pstr[15+i];
                                    if(     C>='0' && C<='9')   result[23-i]=C-'0';
                                    else if(C>='A' && C<='G')   result[23-i]=C-'A'+10;
                                    else
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=6;
                                    }

                                    //24~31
                                    C=pstr[6+i];
                                    if(     C>='0' && C<='9')   result[31-i]=C-'0';
                                    else if(C>='A' && C<='G')   result[31-i]=C-'A'+10;
                                    else
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=6;
                                    }
                                }
                            }
                            else if(iBinSelect==33)                             //32  bin
                            {
                                for(i=0; i<8; i++)
                                {
                                    //0~7
                                    C=pstr[33+i];
                                    if(     C>='0' && C<='9')   result[7-i]=C-'0';
                                    else if(C>='A' && C<='W')   result[7-i]=C-'A'+10;
                                    else
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=7;
                                    }

                                    //8~15
                                    C=pstr[24+i];
                                    if(     C>='0' && C<='9')   result[15-i]=C-'0';
                                    else if(C>='A' && C<='W')   result[15-i]=C-'A'+10;
                                    else
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=7;
                                    }

                                    //16~23
                                    C=pstr[15+i];
                                    if(     C>='0' && C<='9')   result[23-i]=C-'0';
                                    else if(C>='A' && C<='W')   result[23-i]=C-'A'+10;
                                    else
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=7;
                                    }

                                    //24~31
                                    C=pstr[6+i];
                                    if(     C>='0' && C<='9')   result[31-i]=C-'0';
                                    else if(C>='A' && C<='W')   result[31-i]=C-'A'+10;
                                    else
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=7;
                                    }
                                }
                            }
                            else
                            {
                                int k=0, k1=0, k2=0, k3=0;
                                for(i=0; i<8; i++)
                                {
                                    cAsstring="";
                                    //0~7
                                    k=i*3;
                                    for(int j=0; j<3; j++)
                                    {
                                        k1= 81+j+k;
                                        cAsstring+=pstr[k1];
                                    }
                                    try
                                    {
                                        iChange=StrToInt(cAsstring);
                                        result[7-i]=iChange;
                                    }
                                    catch(...)
                                    {
                                        result[7-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=8;
                                    }
                                    cAsstring="";

                                    //8~15
                                    for(int j=0; j<3; j++)
                                    {
                                        k2= 56+j+k;
                                        cAsstring+=pstr[k2];
                                    }
                                    try
                                    {
                                        iChange=StrToInt(cAsstring);
                                        result[15-i]=iChange;
                                    }
                                    catch(...)
                                    {
                                        result[15-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=8;
                                    }
                                    cAsstring="";

                                    //16~23
                                    for(int j=0; j<3; j++)
                                    {
                                        k3= 31+j+k;
                                        cAsstring+=pstr[k3];
                                    }

                                    try
                                    {
                                        iChange=StrToInt(cAsstring);
                                        result[23-i]=iChange;
                                    }
                                    catch(...)
                                    {
                                        result[23-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=8;
                                    }
                                    cAsstring="";

                                    //24~31
                                    for(int j=0; j<3; j++)
                                    {
                                        k3= 6+j+k;
                                        cAsstring+=pstr[k3];
                                    }

                                    try
                                    {
                                        iChange=StrToInt(cAsstring);
                                        result[31-i]=iChange;
                                    }
                                    catch(...)
                                    {
                                        result[31-i]=9999;
                                        bErrorFlag=true;
                                        iErrorFlag=8;
                                    }
                                }
                            }

                            if(bErrorFlag)
                            {
                                WriteLog(AnsiString("Tester ==> CHECK BINON CHARACTER ERROR - ")+AnsiString(iErrorFlag));  //Steven 20141212 : Add GPIB Log
                            }
                        }
                    }

                    if(iHasNEXTSTEP2==1)                                        //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
                    {
                        iHasNEXTSTEP2=2;                                        //0:TC Mode 1:TJ Mode 2:Auto Switch TC
                    }
                    bCloseSiteHaveBinErr=false;
                    for(i=0; i<TOTAL_SITE; i++)                                 //Steven 20141016 : 關Site不能有Bin
                    {
                        if(iStart[i]==0)                                        //kevin 20141212
                        {
                            if(result[i]==0 || result[i]==10)                   //Kevin 20141212 : 測式機沒有ic回 BIN 10
                            {
                                result[i]=0;
//                                bCloseSiteHaveBinErr=false;                   //Steven 20170214 (wei) mark
                            }
                            else
                            {
                                bCloseSiteHaveBinErr=true;
                                break;
                            }
                        }
                    }

                    for(i=0; i<TOTAL_SITE; i++)
                    {
                        ZeroMemory(tstr[i], 16);                                //Steven 20160215 : 先清空記憶體再讀取資料, 避免舊資料出現
                        if(iStart[i]==1)
                        {
                            if(bCloseSiteHaveBinErr==false)                     //Steven 20141016 : 關Site不能有Bin   //Steven 20141024 : true --> false
                            {
                                sprintf(tstr[i], "%4d", result[i]);
                            }
                            else
                            {
                                strncpy(tstr[i], "9999", sizeof(tstr[i]));
                                result[i]=9999;                                 //Steven 20170214 (wei): Add
                            }
                        }
                        else
                        {
                            strncpy(tstr[i], "----", sizeof(tstr[i]));
                        }
                    }

                    if(bCloseSiteHaveBinErr && LastSet.bSQR41==true)            //Steven 20141016 : 關Site不能有Bin   //Steven 20141225 : 有在測試的時候才判斷
                    {
                        WriteLog("Tester ==> CLOSE SITE HAS BIN ERROR");        //Steven 20141212 : Add GPIB Log
                        SendMSG_CMD(MSG_CMD_CloseSiteHaveBin);                  //Steven 20231017 : GPIB flow error need alarm
                        if(LastSet.bBinonEcho)                                  //Steven 20150302 : 修正 CLOSE SITE HAS BIN 時，也要回覆ECHO給Tester
                        {
                            iBinonCount=0;
                            HT_TACS_ATN.SetSecAndOn(LastSet.iTACS_ATNTimeOut);  //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
                            htDelay.SetMSAndOn(500);
                            Task=500;
                            break;
                        }
                        else
                        {
                            Task=9999;
                            return 2;
                        }
                    }

                    if(LastSet.bSQR41==false)                                   //Steven 20170214 (wei): Add
                    {
                        WriteLog("Tester ==> BINON WITHOUT 0x41 ERROR");        //Steven 20170214 : Add GPIB Log
                        SendMSG_CMD(MSG_CMD_BinonWithout0x41);                  //Steven 20231017 : GPIB flow error need alarm
                        if(LastSet.bBinonEcho)
                        {
                            iBinonCount=0;
                            HT_TACS_ATN.SetSecAndOn(LastSet.iTACS_ATNTimeOut);  //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
                            htDelay.SetMSAndOn(500);
                            Task=500;
                            break;
                        }
                        else
                        {
                            Task=9999;
                            return 2;
                        }
                    }

                    if(LastSet.bUseBarcodeFunction &&
                       LastSet.bHasBarCode==false)                              //Steven 20150713 : for 2D Code
                    {
                        SendMSG_CMD(MSG_CMD_BarCodeFlowErr);
                        WriteLog("Tester ==> Without BARCODE? COMMAND ERROR!");
                    }
                    else if(LastSet.bUseBarcodeFunction==false &&
                            LastSet.bHasBarCode==true)                          //jou 20211029 : 增加 No Open Barcode Function alarm
                    {
                        SendMSG_CMD(MSG_CMD_BarcodeOFF);                        //wei 20161024 No Open Barcode Function
                        WriteLog("Handler ==> Barcode Function is Disabled!");
                    }

                    bIsTesterError=false;
                    for(i=0; i<TOTAL_SITE; i++)
                    {
                        if(iStart[i]==1)
                        {
                            if(LastSet.iTesterMode==InterfaceType_15BinQorvo && result[i]==999) //Steven 20201022 : For RFMD
                            {
                                bNeedRetest=true;
                            }
                            else
                            {
                                if(result[i]<0 || result[i]>iBinSelect)         //kevin 20140328 bin 0 使用
                                    bIsTesterError=true;
                            }
                        }
//                        else
//                        {
//                            if(result[i]!=0)                                  //Steven 20141016 : 關Site不能有Bin
//                            {
//                                bIsTesterError=true;
//                                bErrorFlag=true;
//                            }
//                        }
                    }

                    if(bIsTesterError)
                    {
                        WriteLog("Tester ==> BIN IS <0 OR >MAX_BIN ERROR");     //Steven 20141212 : Add GPIB Log
                        for(i=0; i<TOTAL_SITE; i++)
                        {
                            if(iStart[i]==1)
                            {
                                if(iBinSelect<=15)                              //kevin 20140305   15 bin
                                    result[i]=16;
                                else
                                    result[i]=256;
                            }
                        }
                    }
                    else if(bNeedRetest)                                        //Steven 20201022 : For RFMD
                    {
                        WriteLog("Tester ==> Need Replunge");
                    }

                    try
                    {
                        for(i=0; i<TOTAL_SITE; i++)
                        {
                            //AI(W906-GB-P1) 20260926: golden MY_DUT_PAL[i].  BCB6 `catch(...)` also caught the VCL
                            //   EAccessViolation of a missing panel; std::vector operator[] out of range is UB
                            //   and throws nothing, so .at() is used to let the golden catch(...) below see it.
                            MY_DUT_PAL.at(i)->plSite->Caption=AnsiString(tstr[i]);
                        }
                    }
                    catch(...)
                    {
                        WriteLog("MY_DUT_PAL->plSite->Caption ERROR");
                    }

                    if(bCatalystSimpleGPIB)
                    {
                        //IsTest=false;
                        Task=1;
                        InitialBarcodeList();                                   //jou 20170407 (Steven) : 模擬時,GPIB return需清空Barcode
                        return 1;
                    }

                    if(LastSet.bStringLength)
                    {
                        if(bErrorFlag==false)
                        {
                            iBinonCount=0;
                            HT_TACS_ATN.SetSecAndOn(LastSet.iTACS_ATNTimeOut);  //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
                            htDelay.SetMSAndOn(500);
                            Task=500;
                        }
                        else
                        {
                            Task=9999;
                            InitialBarcodeList();                               //jou 20170407 (Steven) : 字串長度錯誤時,GPIB return需清空Barcode
                            return 1;
                        }
                    }
                    else
                    {
                        iBinonCount=0;                                          //Steven 20150713
                        HT_TACS_ATN.SetSecAndOn(LastSet.iTACS_ATNTimeOut);      //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
                        htDelay.SetMSAndOn(500);
                        Task=500;
                        break;
                    }
                }
                else if(S.Pos("FULLSITES?")==1)                                 //Steven 20120213 : 如果在等Binon卻收到Fullsite?會Hang Up
                {
                    Task=300;
                    bFirstIn=true;
                    LastSet.bFULLSITES=true;                                    //kevin 20130516
                    //IsTest=true;
                    iMainTask=1;
                    break;
                }
                else if(S.Pos("FR?")==1)
                {
                    asShowMemo="0400";
                    iBackupFRTask=400;                                          //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                    Task=1000;
                    FRWaitRequestDelay.SetSecAndOn(5);                          //Steven 20120112 : 進FR?沒有回應會卡死
                    goto TEST_GPIB;
//                    break;
                }
                else if(ProcessStatusStringRFMD(S, BS, "0400")>=0)              //Steven 20201022 : For RFMD
                {
                    if(bMustWaitESC==true && bESCIsOpen==false)
                    {
                        IsTest=false;
                        Task=1;
                    }
                    break;
                }
                else if(ProcessStatusString(S, BS, "0400"))                     //Steven 20150713 : 整合部分COMMAND
                {
                    break;
                }
                else if(ProcessStatusStringRCMD(S, BS, "0400"))                 //JerryYang 20190226 RCMD command
                {
                    iRCMDBackupTask=Task;
                    Task=2000;
                }
                else if(LastSet.bRunHANA_ART==true && S!="")                    //JimmyChiu 20241023 HANA ART Function
                {                                                               //Steven 20250414 : HANA ART Function
                    asShowMemo="0400";
                    SendMSG_CMD(MSG_CMD_HANA_ART, S);
                    break;
                }
                else if(S=="")
                {
                    Task=200;
                    break;
                }
            /*  else                                                            //Kevin 20110331 : 不符條件的字串會項賽
                {
                    Task=100;
                    break;
                } */
            }
            break;
        case 500:
            iBinonCount++;
            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
            if((ibsta&TACS) && (!(ibsta&ATN)))                                  // If addressed to talk, send the response "I am a talker"
            {
                //S="BINON:00000000,00000000,00000000,00000011\r\n";
                //   12345678901234567890123456789012345678901
                //         12345678901234567890123456789012345678
                // S=S.SubString(7,S.Length()-6);
                //kevin 20110411    S=S.SubString(7,36);
                if(GPIBVersionCheck>5 &&
                   (LastSet.iTesterMode==InterfaceType_16BinGS ||
                    LastSet.iTesterMode==InterfaceType_32BinGS))                //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
                {
                    S=S.SubString(7, S.Length());
                    S="ECHO:"+S;
                }
                else
                {
                    bHasLrLn=false;
                    bHadCommon=false;
                    iCommomPos=S.Pos(";");
                    if(iCommomPos!=0)
                        bHadCommon=true;
                    iEndChar=S.Pos("\r\n");                                     //Steven 20101124 : for T2000
                    if(iEndChar!=0)
                        bHasLrLn=true;

                    if(iBinSelect<=16)                                          //0-15  bin
                    {
                        if(bHasLrLn && bHadCommon)                              //Steven 20230410 : 根據結尾字元決定長度
                            S=S.SubString(7, 38);
                        else if(bHasLrLn)
                            S=S.SubString(7, 37);
                        else if(bHadCommon)
                            S=S.SubString(7, 36);
                        else
                            S=S.SubString(7, 35);

                        #ifdef KINGMAX
                            S="ECHO :"+S;
                        #else
                            S="ECHO:"+S;
                        #endif
                    }
                    else
                    {
                        S=S.SubString(7, S.Length());
//                        S="ECHO:"+S;                                          //Steven 20230809 : 會連發兩次ECHO
                        #ifdef KINGMAX
                            S="ECHO :"+S;
                        #else
                            S="ECHO:"+S;
                        #endif
                    }
                }
                asBuffer=S;
//                ibwrt(noncontroller, asBuffer.c_str(), asBuffer.Length());    // Send data across the bus.
//                S="0500 TALK:"+asBuffer;
//                WriteResult(S);
                MyGPIBWrite(asBuffer, "0500");
                Task=600;
//jou 980818 start : add speed
            }
            else if(htDelay.Off() &&                                            //Steven 20210723 : 加上確認測試機是不是還在送資料
                    (ibsta&LACS) && !(ibsta&ATN))                               // Wait until non-controller is listener and ATN line is dropped.
            {
                WriteLog("GPIB ==> TACS and ATN Check Fail (ECHO BINON), Try ibrd");    //Steven 20211030 : 修改ECHO BINON的Time out retry機制
                ibtmo(noncontroller, 1);
                ibrd(noncontroller, tempbuffer, iBufferCount);                  // Read data bytes
                if(iBufferCount!=0)
                {
                    tempbuffer[ibcnt]=0;
                    pstr=strupr(tempbuffer);
                    asBuffer="0500 LISTEN:"+AnsiString(pstr);                   //Steven 20220304 : S --> asBuffer
                }
                else
                {
                    asBuffer="0500 ibrd error";
                }
                WriteLog(asBuffer);                                             //Steven 20220304 : S --> asBuffer
                ibtmo(noncontroller, LastSet.iTimeOut);
                htDelay.SetMSAndOn(400);
//                Task=400;
            }
            else if(HT_TACS_ATN.Off())                                          //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
            {
                WriteLog("GPIB ==> TACS and ATN Check Time Out (ECHO BINON)");
                if(CustomerCode==CC_KYEC_LEE)                                   //Ifor 20180917 (Steven) : Add KYEC 要求Time Out後直接分ErrBin (同HT0745)
                {
                    for(i=0; i<TOTAL_SITE; i++)
                    {
                        result[i]=9999;
                    }
                    noncontroller=ibfind("gpib0");                              // Open a session to the GPIB board
                    ibrsc(noncontroller, 0);                                    // Release system control
                    ibpad(noncontroller, GpibAddress);                          // Change primary address from 0 to GpibAddress
                    ibtmo(noncontroller, LastSet.iTimeOut);
                    AnsiString StrErr="Tester time up error";
                    sprintf(GGpib2Handler.cReturn, "%s", StrErr.c_str());
                    WriteLog(" ");

                    return 1;
                }
                else
                {
                    Task=400;
                    break;
                }
            }
            break;
        case 600:
            ibwait(noncontroller, 0);                                           // Update Status variable
            if((ibsta&LACS) && (!(ibsta&ATN)))                                  // Wait until non-controller is listener and ATN line is dropped.
            {
                ibrd(noncontroller, buffer, iBufferCount);                      // Read data bytes
                strncpy(tempbuffer, buffer, sizeof(tempbuffer));                //Steven 20110608
                buffer[ibcnt] = 0;

                BS=buffer;                                                      // 保留大小寫

                if(BS=="")                                                      //jou 2011-01-24 start : 如遇到空字串自動過濾掉
                {
                    break;
                }

                S="0600 LISTEN:"+AnsiString(BS);
                WriteLog(S);
                S=buffer;
                if(S.Pos("ECHOSTOP")==1)                                        //ChungHung 20130326 add
                {
                    noncontroller = ibfind ("gpib0");                           // Open a session to the GPIB board
                    if(noncontroller<0)
                    {
                        if(bSimulate==false)
                            WriteLog("0600 Error Open GPIB0");
                        return 2;
                    }
                    ibrsc(noncontroller, 0);                                    // Release system control
                    ibpad(noncontroller, GpibAddress);                          // Change primary address from 0 to GpibAddress
                    ibtmo(noncontroller, LastSet.iTimeOut);

                    Task=1;
                    WriteLog("");

                    return 3;
                }
                else if(S.Pos("ECHOOK")==1)
                {
                    noncontroller = ibfind ("gpib0");                           // Open a session to the GPIB board
                    if(noncontroller<0)
                    {
                        if(bSimulate==false)
                            WriteLog("0600 Error Open GPIB0");
                        return 2;
                    }
                    ibrsc(noncontroller, 0);                                    // Release system control
                    ibpad(noncontroller, GpibAddress);                          // Change primary address from 0 to GpibAddress
                    ibtmo(noncontroller, LastSet.iTimeOut);
                    iCurrentArm=0;                                              //Ifor 20180117 : add 測試結束清除

                    if(LastSet.i2DIDFormat==eAMD)                               //JerryYang 20200422 2DID format選項改用下拉選單
                    {
                        WriteLog("0600 SRQ:0x00");                              // 2016.06.30 , Joye , AMD ECHOOK 0x00
                        ibrsv(noncontroller, 0x00);                             // 2016.06.30 , Joye , AMD ECHOOK 0x00
                    }
                    Task=1;

                    if(S.Pos("ECHOOK:ONECYCLE")==1)                             //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
                        GGpib2Handler.bOneCycle=true;

                    InitialBarcodeList();                                       //Steven 20151223 : ATK說做完就要清掉
                    LastSet.bHasBarCode=false;

                    return 1;
                }
                else if(S.Pos("BINON")==1)
                {
                    strncpy(buffer, tempbuffer, sizeof(buffer));
                    Task=400;
                    if(LastSet.bSQR41)                                          //Steven 20110914
                    {
                        IsTest=true;
                        iMainTask=100;
                    }
                    else
                    {
                        if(LastSet.bBinonEcho)                                  //Steven 20111219
                        {
                            IsTest=true;
                            iMainTask=100;
                        }
                        else
                        {
                            IsTest=false;
                            break;
                        }
                    }
                    if(iHasNEXTSTEP2==1)                                        //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode
                    {
                        iHasNEXTSTEP2=2;                                        //0:TC Mode 1:TJ Mode 2:Auto Switch TC
                    }
                    goto GoToLabel;
                }
                else if(S.Pos("ECHONG")==1)                                     //1201
                {
                    if(bEchoNG==false)                                          //jou 2013-06-07 ECHONG 重新再讀一次 BINON
                    {
                        bEchoNG=true;
                        Task=400;
                        break;
                    }
                    else                                                        //jou 2013-06-07 ECHONG 第二次必須直接顯示GPIB ERROR
                    {
                        LastSet.bSQR41=false;
                        LastSet.bFULLSITES=false;
                        LastSet.bSRQC0=false;                                   //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagRCMD=false;                                //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagSVID=false;                                //jou 2015-09-21 Auto Retest function
                        LastSet.bFlagECID=false;                                //jou 2015-09-21 Auto Retest function
                        return 2;
                    }
                }
                else if(S.Pos("FR?")==1)
                {
                    asShowMemo="0600";
                    iBackupFRTask=600;                                          //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                    Task=1000;
                    FRWaitRequestDelay.SetSecAndOn(5);                          //Steven 20120112 : 進FR?沒有回應會卡死
                    goto TEST_GPIB;
//                    break;
                }
                else if(ProcessStatusStringRFMD(S, BS, "0600")>=0)              //Steven 20201022 : For RFMD
                {
                    if(bMustWaitESC==true && bESCIsOpen==false)
                    {
                        IsTest=false;
                        Task=1;
                    }
                    break;
                }
                else if(ProcessStatusString(S, BS, "0600"))                     //Steven 20150713 : 整合部分COMMAND
                {
                    break;
                }
                else if(ProcessStatusStringRCMD(S, BS, "0600"))                 //JerryYang 20190226 RCMD command
                {
                    iRCMDBackupTask=Task;
                    Task=2000;
                }
                else if(LastSet.bRunHANA_ART==true && S!="")                    //JimmyChiu 20241023 HANA ART Function
                {                                                               //Steven 20250414 : HANA ART Function
                    asShowMemo="0600";
                    SendMSG_CMD(MSG_CMD_HANA_ART, S);
                    break;
                }
            }
            break;
        case 1000:
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();

            if(IsTest)
                asBuffer.sprintf("%s", szRunState);
            else
                asBuffer.sprintf("%s", szIdelState);

            if(MyGPIBWrite(asBuffer, asShowMemo))                               //Steven 20250926 : 變更GPIB write流程
            {
                Task=iBackupFRTask;                                             //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                iBackupFRTask=1;

                LastSet.bFlagRCMD=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagSVID=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagECID=false;                                        //jou 2015-09-21 Auto Retest function
            }
            else if(FRWaitRequestDelay.Off())                                   //Steven 20120112 : 進FR?沒有回應會卡死
            {
                Task=iBackupFRTask;                                             //jou 2011-12-21 中途問FR?不能回到Task=1,因為這樣會重送0x41,測試機summery會多
                iBackupFRTask=1;
                WriteLog("FR Request Time Out!");                               //Steven 20110131 mark
            }
            break;
        case 2000:                                                              //RCMD
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();
//            if(ibsta&TACS)                                                      //if LTX ,sned first all system will very slow
//            {
                if(bECHO_FlagRCMD==true)                                        //jou 2015-09-21 Auto Retest function
                {
                    if(MyGPIBWrite(S, asShowMemo))
                    {
                        Task=iRCMDBackupTask;                                   //JerryYang 20190226 RCMD command
                        bECHO_FlagRCMD=false;
                        LastSet.bFlagRCMD=false;
                    }
                }
//            }
            break;
        case 3000:                                                              //SVID
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();
//            if(ibsta&TACS)                                                      //if LTX ,sned first all system will very slow
//            {
                if(bECHO_FlagSVID==true)                                        //jou 2015-09-21 Auto Retest function
                {
                    if(MyGPIBWrite(S, asShowMemo))
                    {
                        Task=1;
                        bECHO_FlagSVID=false;
                        LastSet.bFlagSVID=false;
                    }
                }
//            }
            break;
        case 4000:                                                              //ECID
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();
//            if(ibsta&TACS)                                                      //if LTX ,sned first all system will very slow
//            {
                if(bECHO_FlagECID==true)                                        //jou 2015-09-21 Auto Retest function
                {
                    if(MyGPIBWrite(S, asShowMemo))
                    {
                        Task=1;
                        bECHO_FlagECID=false;
                        LastSet.bFlagECID=false;
                    }
                }
//            }
            break;
        case 5000:                                                              //jou 2015-10-05 增加 0x41 之後的 "ONECYCLE" 指令支援
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();
//            if(ibsta&TACS)                                                      //if LTX ,sned first all system will very slow
//            {
                asBuffer="ONECYCLEOK";
                if(MyGPIBWrite(asBuffer, asShowMemo))
                {
                    GGpib2Handler.bOneCycle=true;
                    Task=200;
                }
//            }
            break;
        case 6000:                                                              //jou 2015-09-21 Auto Retest function
//            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
//            UpdateLed();
//            if(ibsta&TACS)                                                      //if LTX ,sned first all system will very slow
//            {
                if(bECHO_FlagReset==true)
                {
                    if(MyGPIBWrite(S, asShowMemo))
                    {
                        Task=1;
                        bECHO_FlagReset=false;
                    }
                }
//            }
            break;
        case 7000:                                                              //Steven 20201022 : For RFMD
            ibstop(noncontroller);
            WriteLog("7000 SRQ:0x44");
            ibrsv(noncontroller, 0x44);
            Task=7100;
            break;
        case 7100:
            asBuffer="CHECKEMPTY\r\n";
            MyGPIBWrite(asBuffer, "7100");
            Task=7200;
            break;
        case 7200:
            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
            UpdateLed();
            if(ibsta&LACS)
            {
                ZeroMemory(buffer, StrLength);                                  //Steven 20160215 : 先清空記憶體再讀取資料, 避免舊資料出現
                ibrd(noncontroller, buffer, iBufferCount);                      // Read data bytes
                strncpy(tempbuffer, buffer, sizeof(tempbuffer));                //Steven 20110608
                buffer[ibcnt]=0;

                BS=buffer;                                                      // 保留大小寫

                if(BS=="")                                                      //jou 2011-01-24 start : 如遇到空字串自動過濾掉
                {
                    break;
                }

                S="7200 LISTEN:"+AnsiString(BS);
                WriteLog(S);
                S=buffer;

                if(S.Pos("ECHOOK")==1)
                {
                    SendMSG_CMD_ESC(MSG_CMD_ESC, 2);
                    Task=1;
                    return 5;

                }
                else if(S.Pos("ECHONG")==1)
                {
                    SendMSG_CMD_ESC(MSG_CMD_ESC, 0);
                    Task=1;
                    return 5;
                }
                else
                {
                    data=ProcessStatusStringRFMD(S, BS, "7200");
                    if(data==1)
                    {
                        SendMSG_CMD_ESC(MSG_CMD_ESC, 2);
                        Task=1;
                        return 5;
                    }
                    else if(data==0)                                            //QRM?
                    {

                    }
                    else
                    {
                        Task=1;
                    }
                }
            }
            break;
    }
    return 0;
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
