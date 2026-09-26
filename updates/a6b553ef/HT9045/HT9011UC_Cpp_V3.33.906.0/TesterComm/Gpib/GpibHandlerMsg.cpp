// ===========================================================================
//  TesterComm/Gpib/GpibHandlerMsg.cpp -- TSerialPoll::OnMyCopyMsg(), the bridge-side dispatcher for every
//  Handler -> bridge MSG_CMD_* packet.
//
//  AI(W906-GB-P1) 20260926: faithful translation of golden
//  D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp:3139-3917 (Big5/cp950 -> UTF-8), per
//  TesterComm/Gpib/TRANSLATION_RULES.md.  The body is golden text line by line: every else-if branch and its order,
//  the `#ifndef DEBUG` CloseGpib pre-check, the unreachable second MSG_CMD_CloseGpib / MSG_CMD_GetSiteOnOff
//  branches, and the nested SendMSG_CMD(MSG_CMD_PickLoad) / echo SendMessage exactly where golden has them (the
//  mailbox pumps while waiting, like SendMessage).  Deviations, each marked //AI(W906-GB-P1) in place:
//    * `__fastcall` dropped (rule 3).
//    * `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` -> `GHandler2Gpib = reinterpret_cast<MV*>(...)`,
//      `this->Handle` -> `reinterpret_cast<HWND>(this)`, `Close()` -> `RequestClose("OnMyCopyMsg")` (+ golden
//      `return`), `SendMessage(HMountWnd, WM_COPYDATA, ...)` -> `PostToHandler(pcp)` (rule 4).
//    * `fDummyART->spbStopART->Click()` / `fDummyART->spbAutoRetest->Click()` -> direct calls of the OnClick
//      handlers golden DummyArt.dfm wires to them (spbStopARTClick / spbAutoRetestClick): vclcompat
//      TControl::Click() is an offline no-op.  VCL TControl::Click fires OnClick without an Enabled check and
//      without touching Down, so the direct call is the same behaviour.
//    * `slCmdList->Strings[iRecvCommand]` into sprintf -> `AnsiString(...)` (StringsProxy through a variadic).
//    * `sBarCode->Strings[i]==0` -> `==AnsiString(0)` (keeps BCB6's == "0" overload resolution).
//  Everything this body calls is declared in GpibBridge.h (members WriteLog, WriteLastDataFile, ReadLastDataFile,
//  MyGPIBWrite, SendMSG_CMD, ShowSimulateItem, Save_Log, InitialBarcodeList, InitHANA_ART, PostToHandler,
//  RequestClose; globals fRS232Main / fDummyART; ibstop / ibrsv from GpibDriver.h).  MSG_CMD_* come from
//  MessageDef.h (all 125 used here checked against golden GPIB MessageDef.cpp: same values); CC_KYEC_XILINX and
//  eAMD / eIntel / eStandard (enum e2DIDFormat) from MachineType.h, via MessageDef.h.
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace gpibbridge {

using std::vector;   // golden Main.cpp `using namespace std` -- keeps `vector<Byte>()` below as golden wrote it

//------------------------------------------------------------------------------
void TSerialPoll::OnMyCopyMsg(TMessage &msg)
{
    AnsiString buffer;
    bool BackIsRun;
    bool bNeedReStart=false;                                                    //Ifor 20200924 add: AMD BarCode 模式不同需重新啟動GPIB程式
    AnsiString sGbibTask;
    sGbibTask.sprintf("%04d", iGbibTask);
    AnsiString s1="";                                                           //JerryYang 20160630 RS232加入GPIB
    int iMachineState, n, cTemp1[17], aDecade[17];                              //JerryYang 20160630 RS232加入GPIB
    PCOPYDATASTRUCT P;
    AnsiString Str;
    P=(PCOPYDATASTRUCT) msg.LParam;

    //AI(W906-GB-P1) 20260926: golden `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` (BCB idiom: iSendCommand
    //  is MV's first member, so this re-seats the pointer at the packet).  P->lpData is the mailbox payload the
    //  engine wrapped in a COPYDATASTRUCT; valid for the duration of this call only, exactly like WM_COPYDATA.
    GHandler2Gpib = reinterpret_cast<MV*>(P->lpData);
    iRecvCommand=GHandler2Gpib->iSendCommand;
#ifndef DEBUG
    if(iRecvCommand==MSG_CMD_CloseGpib)                                         //wei 20150408 Add Close GPIB Command
    {
        WriteLog("Handler ==> Close GPIB");
        if(GHandler2Gpib->bCloseGpib)                                           //Steven 20150812 : for 不正常關閉GPIB
        {
            RequestClose("OnMyCopyMsg");                                    //AI(W906-GB-P1) 20260926: golden Close()
            return;
        }
    }
#endif

    if(GHandler2Gpib->HandlerHwnd!=HMountWnd ||
       GHandler2Gpib->GpibHwnd!=reinterpret_cast<HWND>(this))                      //AI(W906-GB-P1) 20260926: golden this->Handle; the Handler fills GpibHwnd with reinterpret_cast<HWND>(SerialPoll)
    {
        WriteLog("GPIB : No handler window, close GPIB, _OnMyCopyMsg_");
        RequestClose("OnMyCopyMsg");                                    //AI(W906-GB-P1) 20260926: golden Close()
        return;
    }

    if(iRecvCommand==MSG_CMD_NONE || iRecvCommand==MSG_CMD_ChangeGpib)          //Steven 20180420 : 不能每次都更換Bin模式  //Steven 20191008 : 修正GPIBBin
    {
        iBinSelect=GHandler2Gpib->GPIBBin;                                      //kevin 20140305 for 256 bin
        if(iBinSelect!=oldiBinSelect)
        {
            oldiBinSelect=iBinSelect;
            StatusBar1->Panels->Items[5]->Text=IntToStr(iBinSelect)+" Bin";     //kevin 20140318
            WriteLastDataFile();
            WriteLog(AnsiString("Handler ==> CHANGE Support BIN : 0 ~ "+AnsiString(iBinSelect-1)));    //Steven 20141212 : Add GPIB Log
        }
    }

    bCatalystSimpleGPIB=false;
    if(iRecvCommand==MSG_CMD_CatalystSimpleGPIB)
    {
        WriteLog("Handler ==> Catalyst Simple GPIB");                           //Steven 20141212 : Add GPIB Log
        WriteLastDataFile();                                                    //Steven 20161006 : 確保設定的資料有存起來
        bCatalystSimpleGPIB=true;
    }
    else if(iRecvCommand==MSG_CMD_TimeOutSkip)                                  //Steven 20150304 : Add GPIB LOG
    {
        WriteLog("Handler ==> Test Time Out - SKIP");
    }
    else if(iRecvCommand==MSG_CMD_TimeOutRetryWait)
    {
        WriteLog("Handler ==> Test Time Out - Retry and Wait Result");
    }
    else if(iRecvCommand==MSG_CMD_TimeOutRetrySend)
    {
        WriteLog("Handler ==> Test Time Out - Retry and Resend SOT");
    }
    else if(iRecvCommand==MSG_CMD_HandlerHomeStart)
    {
        WriteLog("Handler ==> Home Start");
    }
    else if(iRecvCommand==MSG_CMD_HandlerHomeFinish)
    {
        WriteLog("Handler ==> Home Finish");
        bHasSiteMapping=false;
        iHasNEXTSTEP2=2;                                                        //0:TC Mode 1:TJ Mode 2:Auto Switch TC
    }
    else if(iRecvCommand==MSG_CMD_SiteMap)                                      //JerryYang 20160308 site map, add for Maxim_Philippine
    {
        asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        bHasSiteMapping=true;
        WriteLog(AnsiString("Handler ==> Site Mapping : "+asATC_SiteMapping));

        sSiteMap.sprintf("%s", GHandler2Gpib->Message);                         //Ifor 20260203 add: for Maxim_Philippine
        fRS232Main->ShowCommData("[Handler ==> SiteMap]"+sSiteMap,  vector<Byte>());

        //Steven 20260428 : SJSM 現場發現 Handler 新版本把 SiteMap 資料只填在 Message 欄位，
        //                  舊欄位 UseSiteMapData 為空。若 UseSiteMapData 空但 Message 有值，採用 Message
        if(asATC_SiteMapping=="" && sSiteMap!="")
        {
            asATC_SiteMapping=sSiteMap;
            WriteLog("Handler ==> Site Mapping fallback to Message: "+asATC_SiteMapping);
        }
    }
    else if(iRecvCommand==MSG_CMD_Arm1Down)
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            iATCUseChannel[i]=GHandler2Gpib->Site[i];
        }
        iTestMode=GHandler2Gpib->iLotStatus;
        iCurrentArm=1;
        asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        bHasSiteMapping=true;
        WriteLog("\r\nHandler ==> Arm 1 Down");
    }
    else if(iRecvCommand==MSG_CMD_Arm2Down)
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            iATCUseChannel[i]=GHandler2Gpib->Site[i];
        }
        iTestMode=GHandler2Gpib->iLotStatus;
        iCurrentArm=2;
        asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        bHasSiteMapping=true;
        WriteLog("\r\nHandler ==> Arm 2 Down");
    }
    else if(iRecvCommand==MSG_CMD_ContactTestArm1)
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            iATCUseChannel[i]=GHandler2Gpib->Site[i];
        }
        iTestMode=GHandler2Gpib->iLotStatus;
        iCurrentArm=1;                                                          //wei 20160908
        if(strlen(GHandler2Gpib->UseSiteMapData)>0)                            //Steven 20260428 : Contact Test時也更新SiteMapping(安全網，避免SiteMap指令遺漏)
            asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        WriteLog("\r\nHandler ==> Contact Test Arm 1");
    }
    else if(iRecvCommand==MSG_CMD_ContactTestArm2)
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            iATCUseChannel[i]=GHandler2Gpib->Site[i];
        }
        iTestMode=GHandler2Gpib->iLotStatus;
        iCurrentArm=2;                                                          //wei 20160908
        if(strlen(GHandler2Gpib->UseSiteMapData)>0)                            //Steven 20260428 : Contact Test時也更新SiteMapping(安全網，避免SiteMap指令遺漏)
            asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        WriteLog("\r\nHandler ==> Contact Test Arm 2");
    }
    else if(iRecvCommand==MSG_CMD_ContactTestAbort)
    {
        WriteLog("\r\nHandler ==> Abort Contact Test");
    }
    else if(iRecvCommand==MSG_CMD_OverDrive)                                    //Steven 20151207 : OverDrive for TSMC
    {
        ibstop(noncontroller);                                                  //jou 20230907 : 修正0x59測試機收不到的問題
        WriteLog("\r\nHandler ==> OverDrive OK, SRQ:0x59");
        ibrsv(noncontroller, 0x59);
    }
    else if(iRecvCommand==MSG_CMD_ReContact)                                    //Steven 20151207 : Recontact for TSMC
    {
        ibstop(noncontroller);                                                  //jou 20230907 : 修正0x59測試機收不到的問題
        WriteLog("\r\nHandler ==> ReContact OK, SRQ:0x59");
        ibrsv(noncontroller, 0x59);
    }
    else if(iRecvCommand==MSG_CMD_DeviceMapSRQ)                                 //Steven 20191009 : Device Map for Qualcomm
    {
        if(GHandler2Gpib->iLotStatus==193)
        {
            WriteLog("\r\nHandler ==> Device map function -- START --, SRQ:0xC1");
            if(bSimulate)
            {
                Str="PICKLOAD2;1,7,0;2,0,0;3,1,0;4,2,0;5,3,0;6,4,0;7,5,0;8,6,0;";
                SendMSG_CMD(MSG_CMD_PickLoad, Str);
            }
            else
            {
                ibrsv(noncontroller, 0xC1);
            }
        }
        else if(GHandler2Gpib->iLotStatus==85)
        {
            WriteLog("\r\nHandler ==> Device map function -- TEST OK --, SRQ:0x55");
            ibrsv(noncontroller, 0x55);
        }
        else
        {
            if(GHandler2Gpib->iLotStatus==591)
                WriteLog("\r\nHandler ==> Device map function -- RECONTACT OK --, SRQ:0x59");
            else if(GHandler2Gpib->iLotStatus==592)
                WriteLog("\r\nHandler ==> Device map function -- PLACELOAD OK --, SRQ:0x59");
            else
                WriteLog("\r\nHandler ==> Device map function -- TRAY FEED OK --, SRQ:0x59");

            ibrsv(noncontroller, 0x59);
        }
    }
    else if(iRecvCommand==MSG_CMD_CloseGpib)                                    //wei 20150408 Add Close GPIB Command
    {
        WriteLog("\r\nHandler ==> Close GPIB (DEBUG MODE)");
    }
    else if(iRecvCommand==MSG_CMD_ChangeGpib)                                   //wei 20150409 Add Close GPIB Command
    {
        WriteLog("\r\nHandler ==> Change GPIB Setting Start");

        bSimulate=GHandler2Gpib->bSimulate;
        ShowSimulateItem(bSimulate);
        if(bSimulate)
        {
            pnlHanaART->Enabled=true;
            WriteLog("Handler ==> Simulate GPIB");
        }
        else
        {
            if(LastSet.bTryHANA_ART==false)                                     //Steven 20250414 : HANA ART Function, 手動測試用
                pnlHanaART->Enabled=false;
            else
                pnlHanaART->Enabled=true;
            WriteLog("Handler ==> Normal GPIB");
        }

        bNeedInital=GHandler2Gpib->bTimeOutProcess;
        GpibAddress=GHandler2Gpib->GpibAddress;
        if(GpibAddress!=oldGpibAddress)
        {
            LastSet.GpibAddress=GpibAddress;
            StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
            WriteLog(AnsiString("Handler ==> CHANGE ADDR : "+AnsiString(LastSet.GpibAddress)).c_str());  //Steven 20141212 : Add GPIB Log
            oldGpibAddress=LastSet.GpibAddress;                                 //wei 20150629   oldGpibAddress會導致GpibAddress不會更改
        }

        bGpibMode=GHandler2Gpib->bGpibMode;
        if(bGpibMode)
            WriteLog("Handler ==> GPIB MODE");
        else
            WriteLog("Handler ==> Other MODE");

        iBinSelect=GHandler2Gpib->GPIBBin;
        WriteLog(AnsiString("Handler ==> Test Bin Count : "+AnsiString(iBinSelect)));

        WriteLog("Handler ==> Change GPIB Setting End");
        WriteLastDataFile();                                                    //Steven 20161006 : 確保設定的資料有存起來
    }
    else if(iRecvCommand==MSG_CMD_EnableBarCode)
    {
        LastSet.bUseBarcodeFunction=true;
        WriteLog("Handler ==> Enable Bar Code Command");
        WriteLastDataFile();                                                    //Steven 20161006 : 確保設定的資料有存起來
    }
    else if(iRecvCommand==MSG_CMD_DisableBarCode)
    {
        LastSet.bUseBarcodeFunction=false;
        WriteLog("Handler ==> Disable Bar Code Command");
        WriteLastDataFile();                                                    //Steven 20161006 : 確保設定的資料有存起來
    }
    else if(iRecvCommand==MSG_CMD_LotStatus)                                    //wei 20150409 Add Close GPIB Command
    {
        //jou 2015-09-21 Auto Retest function
        iLotMode=GHandler2Gpib->iLotStatus;
        if(iLotMode==2)
            WriteLog("Handler ==> FT Lot Start");
        else if(iLotMode==8)
            WriteLog("Handler ==> FT Lot End");
        else if(iLotMode==4)
            WriteLog("Handler ==> RT Lot Start");
        else if(iLotMode==10)
            WriteLog("Handler ==> RT Lot End");
        else if(iLotMode==12)                                                   //wei 20170911 add Reset 狀態
            WriteLog("Handler ==> GPIB Reset");
    }
    else if(iRecvCommand==MSG_CMD_TesterMode)
    {
        LastSet.iTesterMode=GHandler2Gpib->GPIBBin;
        if(LastSet.iTesterMode==InterfaceType_ADVAN_Type1)
            WriteLog("Handler ==> Change tester mode ADVAN_Type1");
        else if(LastSet.iTesterMode==InterfaceType_256Bin)
            WriteLog("Handler ==> Change tester mode 256 BIN");
        else if(LastSet.iTesterMode==InterfaceType_16Bin)
            WriteLog("Handler ==> Change tester mode 16 BIN");
        else if(LastSet.iTesterMode==InterfaceType_32Bin)
            WriteLog("Handler ==> Change tester mode 32 BIN");
        else if(LastSet.iTesterMode==InterfaceType_SPEA_Type)
            WriteLog("Handler ==> Change tester mode SPEA Type");
        else if(LastSet.iTesterMode==InterfaceType_16BinGS)
            WriteLog("Handler ==> Change tester mode 16 BIN GS");
        else if(LastSet.iTesterMode==InterfaceType_32BinGS)
            WriteLog("Handler ==> Change tester mode 32 BIN GS");
        else if(LastSet.iTesterMode==InterfaceType_15BinT6577)                  //Steven 20181214 : ASE-CL add T6577
            WriteLog("Handler ==> Change tester mode 15 BIN T6577");
        else if(LastSet.iTesterMode==InterfaceType_15BinQorvo)                  //Steven 20201022 : For RFMD
            WriteLog("Handler ==> Change tester mode 15 BIN Qorvo protocol.");
        else if(LastSet.iTesterMode==InterfaceType_Delta_Castle)                //Ifor 20200603 add: 93K CTH Tester
            WriteLog("Handler ==> Change tester mode Hygon");                   //Ifor 20201127 add:Hygon客戶要求顯示 Hygon方便辨識
        else
            WriteLog("Handler ==> Unkown Test Bin Mode");

        if(LastSet.iTesterMode!=InterfaceType_15BinQorvo)                       //Steven 20201022 : For RFMD
        {
            LastSet.bConfigureSRQ=false;
            LastSet.bHaveContactorInfo=false;
        }
        else
        {
            LastSet.iTesterType=1;                                              //Steven 20250218 : QROVO強迫為1
        }
        bA10_3_Enable=GHandler2Gpib->bTimeOutProcess;                           //Jimmychiu 20231205 : 借用變數bTimeOutProcess，當作A10-3開啟判斷
        //asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
        AnsiString sTmp=AnsiString(GHandler2Gpib->UseSiteMapData);              //RogerYang 20260430 : 只在 UseSiteMapData 有值時才覆蓋
        if(sTmp!="" && sTmp!="XXX")
            asATC_SiteMapping=sTmp;
        WriteLastDataFile();
    }
    else if(iRecvCommand==MSG_CMD_RetestFlag)                                   //jou 2015-09-21 Auto Retest function
    {
        Str.sprintf("Handler ==> Retest Flag : %s", GHandler2Gpib->Message);
        WriteLog(Str);
        strncpy(cGpibARTData, GHandler2Gpib->Message, sizeof(cGpibARTData));
        bECHO_FlagReset=true;
    }
    else if(iRecvCommand==MSG_CMD_RCMD)                                         //jou 2015-09-21 Auto Retest function
    {
        Str.sprintf("Handler ==> RCMD : %s", GHandler2Gpib->Message);
        WriteLog(Str);

        strncpy(cGpibARTData, GHandler2Gpib->Message, sizeof(cGpibARTData));
        bECHO_FlagRCMD=true;
    }
    else if(iRecvCommand==MSG_CMD_SVID)                                         //jou 2015-09-21 Auto Retest function
    {
        Str.sprintf("Handler ==> SVID : %s", GHandler2Gpib->Message);
        WriteLog(Str);
        strncpy(cGpibARTData, GHandler2Gpib->Message, sizeof(cGpibARTData));
        bECHO_FlagSVID=true;
    }
    else if(iRecvCommand==MSG_CMD_ECID)                                         //jou 2015-09-21 Auto Retest function
    {
        Str.sprintf("Handler ==> ECID : %s", GHandler2Gpib->Message);
        WriteLog(Str);
        strncpy(cGpibARTData, GHandler2Gpib->Message, sizeof(cGpibARTData));
        bECHO_FlagECID=true;
    }
    else if(iRecvCommand==MSG_CMD_SCKART_LOTRTCLEAR ||
            iRecvCommand==MSG_CMD_SCKART_LOTCLEAR   ||
//            iRecvCommand==MSG_CMD_SCKART_INPUTQTY   ||
            iRecvCommand==MSG_CMD_SCKART_QTY)                                   //Steven 20161025 : SCK ART function
    {
        DummyArtMSG.sprintf("%s", GHandler2Gpib->Message);                      //Steven 20170413 : Add ART simulator
        bMessageFromHandler=true;
        Str.sprintf("Handler ==> %s", GHandler2Gpib->Message);
        WriteLog(Str);
        if(LastSet.bDummyART==false)                                            //Steven 20180824 : Semi ART
        {
            MyGPIBWrite(GHandler2Gpib->Message, sGbibTask);
            Str.sprintf("Talk: %s", GHandler2Gpib->Message);
            WriteLog(Str);
        }
    }
    else if(iRecvCommand==MSG_CMD_SCKART_LOTSTATUS)
    {
        DummyArtStsMSG.sprintf("%s", GHandler2Gpib->Message);                   //Steven 20170413 (wei) : Add ART simulator
        bStsMessageFromHandler=true;
        Str.sprintf("Handler ==> %s", GHandler2Gpib->Message);
        WriteLog(Str);
        if(LastSet.bDummyART==false)                                            //Steven 20180824 : Semi ART
        {
            MyGPIBWrite(GHandler2Gpib->Message, sGbibTask);
            Str.sprintf("Talk: %s", GHandler2Gpib->Message);
            WriteLog(Str);
        }
        labStatus->Caption=AnsiString(GHandler2Gpib->Message);
    }
    else if(iRecvCommand==MSG_CMD_SCKART_RunDummy)                              //Steven 20180824 : Semi ART
    {
        LastSet.bDummyART=GHandler2Gpib->bSimulate;                             //Steven 20190702 : Fixed for SEMI ART
        fDummyART->spbStopARTClick(fDummyART->spbStopART);                     //AI(W906-GB-P1) 20260926: vclcompat Click() is a no-op; call the golden OnClick handler directly
        if(GHandler2Gpib->bSimulate)                                            //Steven 20190703 :
        {
            WriteLog("Handler ==> Start Dummy ART");
            LastSet.iLotCount=GHandler2Gpib->GPIBBin;
            LastSet.sLotID=AnsiString(GHandler2Gpib->Message);
            Str.sprintf("Handler ==> Lot Count: %d", GHandler2Gpib->GPIBBin);
            WriteLog(Str);
            Str.sprintf("Handler ==> Lot ID: %s", LastSet.sLotID);
            WriteLog(Str);
            fDummyART->rgTestType->ItemIndex=1;
            fDummyART->cbStepByStep->Checked=false;
            fDummyART->spbAutoRetestClick(fDummyART->spbAutoRetest);           //AI(W906-GB-P1) 20260926: vclcompat Click() is a no-op; call the golden OnClick handler directly
        }
        else
        {
            WriteLog("Handler ==> Start Normal ART");
        }
        WriteLastDataFile();
        ReadLastDataFile();
    }
    else if(iRecvCommand==MSG_CMD_QRA)                                          //Steven 20241004 : Qorvo check ART enable
    {
        if(GHandler2Gpib->iStatus[0]==0)
        {
            WriteLog("Handler ==> ART function Disabled");
            buffer.sprintf("QRA:0");
        }
        else
        {
            WriteLog("Handler ==> ART function Enabled");
            buffer.sprintf("QRA:1");
        }

        MyGPIBWrite(buffer, sGbibTask);
    }
    else if(iRecvCommand==MSG_CMD_State_Record)                                 //wei 20170911 (steven) State Record
    {
        WriteLog("Handler ==> State Record");
        Save_Log();
    }
    else if(iRecvCommand==MSG_CMD_EnableAMDFunction)
    {
//        LastSet.bUseAMDFunction=true;                                         //JerryYang 20200422 取消MSG_CMD_EnableAMDFunction
//        WriteLog("Handler ==> Enable AMD function");
//        WriteLastDataFile();
    }
    else if(iRecvCommand==MSG_CMD_DisableAMDFunction)
    {
//        LastSet.bUseAMDFunction=false;                                        //JerryYang 20200422 取消MSG_CMD_DisableAMDFunction
//        WriteLog("Handler ==> Disable AMD function");
//        WriteLastDataFile();
    }
    else if(iRecvCommand==MSG_CMD_2DIDFormat)                                   //JerryYang 20200422 2DID format
    {
        if(GHandler2Gpib->iStatus[0]==eAMD)                                     //AMD
        {
            if(LastSet.i2DIDFormat!=eAMD)                                       //Ifor 20200924 add: AMD BarCode 模式不同需重新啟動GPIB程式
            {
                bNeedReStart=true;
            }
            LastSet.i2DIDFormat=eAMD;
            WriteLog("Handler ==> Enable AMD function");
            WriteLastDataFile();
            if(bNeedReStart==true)                                              //Ifor 20200924 add: AMD BarCode 模式不同需重新啟動GPIB程式
            {
                RequestClose("OnMyCopyMsg");                                    //AI(W906-GB-P1) 20260926: golden Close()
                return;
            }
        }
        else if(GHandler2Gpib->iStatus[0]==eIntel)                              //Intel
        {
            LastSet.i2DIDFormat=eIntel;
            WriteLog("Handler ==> Intel 2DID format");
        }
        else
        {
            LastSet.i2DIDFormat=eStandard;
            WriteLog("Handler ==> Standard 2DID format");
        }
        WriteLastDataFile();
    }
    else if(iRecvCommand==MSG_CMD_ESC)                                          //Steven 20201022 : For RFMD
    {
        bESCIsOpen=true;
        WriteLog("Handler ==> Trigger 0x44 for ESC function");
    }
    else if(iRecvCommand==MSG_CMD_TempArm            ||
            iRecvCommand==MSG_CMD_TestArm            ||
            iRecvCommand==MSG_CMD_ContactForce       ||
            iRecvCommand==MSG_CMD_ActualTemp         ||
            iRecvCommand==MSG_CMD_Assign             ||
            iRecvCommand==MSG_CMD_StartMode          ||
            iRecvCommand==MSG_CMD_HandlerID          ||
            iRecvCommand==MSG_CMD_HandlerSiteMap     ||
            iRecvCommand==MSG_CMD_HandlerSoakTime    ||
            iRecvCommand==MSG_CMD_HandlerTemperature ||
            iRecvCommand==MSG_CMD_Force              ||
            iRecvCommand==MSG_CMD_BinMap             ||
            iRecvCommand==MSG_CMD_TestMode           ||
            iRecvCommand==MSG_CMD_GetNowAllTemp      ||
            iRecvCommand==MSG_CMD_ChkSetup           ||
            iRecvCommand==MSG_CMD_GetTestArmPos      ||
            iRecvCommand==MSG_CMD_GetTestArmEP       ||
            iRecvCommand==MSG_CMD_SetTemp            ||
            iRecvCommand==MSG_CMD_SetSoakTime        ||
            iRecvCommand==MSG_CMD_SetTJ              ||
            iRecvCommand==MSG_CMD_SetSiteMapData     ||
            iRecvCommand==MSG_CMD_SetAlarmSetup      ||
            iRecvCommand==MSG_CMD_SamSung_Tmp        ||                         //Steven 20191112 : 三星格式
            iRecvCommand==MSG_CMD_SamSung_Map        ||
            iRecvCommand==MSG_CMD_SamSung_Soak       ||
            iRecvCommand==MSG_CMD_SetTestTemp        ||
            iRecvCommand==MSG_CMD_SIGURD_CHKSTATUS   ||                         //KaiChen 20180910 ：Add GPIB CHKSTATUS?
            iRecvCommand==MSG_CMD_GETBINCATEGORY     ||                         //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
            iRecvCommand==MSG_CMD_SETUPFILENAME      ||                         //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
            iRecvCommand==MSG_CMD_SIGURD_HANDLERID   ||                         //KaiChen 20200507 ：Add GPIB HANDLERID?
            iRecvCommand==MSG_CMD_SGSETUP            ||                         //KaiChen 20190613 ：Add GPIB SGSETUP_
            iRecvCommand==MSG_CMD_SETSTARTMODE       ||                         //KaiChen 20180910 ：Add GPIB SetStartMode_
            iRecvCommand==MSG_CMD_CHECKLIST          ||                         //KaiChen 20190613 ：Add GPIB CHECKLIST?
            iRecvCommand==MSG_CMD_BINPOS             ||                         //KaiChen 20190706 ：Add GPIB BINPOS_
            iRecvCommand==MSG_CMD_SetSGFTP           ||                         //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
            iRecvCommand==MSG_CMD_GetSGFTP_STATUS    ||                         //Sam 20210329 : Add GPIB SGFTP_STATUS
            iRecvCommand==MSG_CMD_SetNONDOUBLEBIN    ||                         //Sam 20210329 : Add GPIB NONDOUBLEBIN_
            iRecvCommand==MSG_CMD_SetBINCOUNT        ||                         //Sam 20210329 : Add GPIB BINCOUNT_
            iRecvCommand==MSG_CMD_SetSGOSBIN         ||                         //Sam 20210406 : Add GPIB SGOSBIN_
            iRecvCommand==MSG_CMD_SetSGCONTFAIL      ||                         //Sam 20210422 : Add GPIB SGCONTFAIL_
            iRecvCommand==MSG_CMD_SETTESTERID        ||                         //Sam 20210617 : Add GPIB SETTESTERID
            iRecvCommand==MSG_CMD_GETTESTERID        ||                         //Sam 20210617 : Add GPIB SETTESTERID
            iRecvCommand==MSG_CMD_GetAutClean        ||                         //Sam 20220408 : Novatek 新增 AUTOCLEAN?
            iRecvCommand==MSG_CMD_ForcePerPinN       ||                         //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
            iRecvCommand==MSG_CMD_ContactHeight      ||                         //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
            iRecvCommand==MSG_CMD_YieldContinusFail  ||                         //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
            iRecvCommand==MSG_CMD_YieldSiteCompare   ||                         //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
            iRecvCommand==MSG_CMD_DUTStatus          ||                         //Sam 20220408 : Novatek 新增 DUTSTATUS?
            iRecvCommand==MSG_CMD_UPH                ||                         //Sam 20220408 : Novatek 新增 UPH?
            iRecvCommand==MSG_CMD_IndexCycleTime     ||                         //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
            iRecvCommand==MSG_CMD_TempOfs            ||                         //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
            iRecvCommand==MSG_CMD_TempRange          ||                         //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
            iRecvCommand==MSG_CMD_VACUUMAIR          ||                         //Sam 20220408 : Novatek 新增 VACUUMAIR?
            iRecvCommand==MSG_CMD_Get_All            ||                         //Sam 20220408 : Novatek 新增 SET_ALL?
            iRecvCommand==MSG_CMD_HandlerVersion     ||                         //Sam 20220408 : Novatek 新增 HANDLERVERSION?
            iRecvCommand==MSG_CMD_PPSELECT           ||                         //Richard 20220929 :Add for UTAC 讀檔
            iRecvCommand==MSG_CMD_ASKPPSELECT        ||                         //Richard 20220929 :Add for UTAC 讀檔 詢問Handler當前檔名
            iRecvCommand==MSG_CMD_SetBinMap          ||                         //Steven 20230210 : Set Bin Map.
            iRecvCommand==MSG_CMD_GETSHUTTLEMODE     ||                         //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
            iRecvCommand==MSG_CMD_SETMAXTEST         ||                         //Sam 20230201 : Add GPIB SETMAXTEST_
            iRecvCommand==MSG_CMD_GETMAXTEST         ||                         //Sam 20230201 : Add GPIB GETMAXTEST
            iRecvCommand==MSG_CMD_SETINITIALMAXTEST  ||                         //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
            iRecvCommand==MSG_CMD_GETINITIALMAXTEST  ||                         //Sam 20230201 : Add GPIB GETINITIALMAXTEST
            iRecvCommand==MSG_CMD_READYNEXTSHOT      ||                         //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
            iRecvCommand==MSG_CMD_NEXT2DID           ||                         //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
            iRecvCommand==MSG_CMD_SETAICCD           ||                         //Sam 20231108 : Add GPIB SETAICCD_
            iRecvCommand==MSG_CMD_ASIF_TJ_REQUEST    ||                         //Steven 20240903 : for MTK ASIF data
            iRecvCommand==MSG_CMD_ASIF_TJ_FB         ||                         //Steven 20240903 : for MTK ASIF data
            iRecvCommand==MSG_CMD_GETAICCD           ||                         //Sam 20240826 : Add GPIB GETAICCD?
            iRecvCommand==MSG_CMD_GetSiteOnOff       ||                         //JerryYang 20190627 回傳開關site狀態
            iRecvCommand==MSG_CMD_SETOSBIN           ||                         //Sam 20250115 : Add GPIB SETOSBIN_
            iRecvCommand==MSG_CMD_GETOSBIN           ||                         //Sam 20250115 : Add GPIB GETOSBIN?
            iRecvCommand==MSG_CMD_DUTCHK             ||                         //Steven 20250701 : for DOOSAN TESNA
            iRecvCommand==MSG_CMD_GetFFC             ||                         //Steven 20250701 : for Ampere
            iRecvCommand==MSG_CMD_GetTJFunction      ||
            iRecvCommand==MSG_CMD_GetPowerFollowing  ||
            iRecvCommand==MSG_CMD_SetSiteOnOff)
    {
        if((int)iRecvCommand>=slCmdList->Count)                                 //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
            Str.sprintf("Handler ==> CMD %d : %s", iRecvCommand, GHandler2Gpib->Message);
        else
            //AI(W906-GB-P1) 20260926: golden passes slCmdList->Strings[iRecvCommand] straight to %s.  vclcompat
            //  Strings[] is a StringsProxy, which the variadic pass-through conv<T> would hand to vsnprintf as an
            //  object (garbage); AnsiString(...) materialises the same text (cObserver.cpp:4945 (N5) precedent).
            Str.sprintf("Handler ==> %s : %s", AnsiString(slCmdList->Strings[iRecvCommand]), GHandler2Gpib->Message);

        if(iRecvCommand==MSG_CMD_Get_All)                                       //Sam 20220408 : Novatek 新增
            WriteLog("Handler ==> SEND_WORD_FILE_TEMP_VALUE");
        else
            WriteLog(Str);

        buffer.sprintf("%s", GHandler2Gpib->Message);
        MyGPIBWrite(buffer, sGbibTask);
    }
    else if(iRecvCommand==MSG_CMD_NONE)                                         //Steven 20141014 : Add GPIB Command
    {
        bHanaDummyTest=false;
        for(int i=0; i<TOTAL_SITE; i++)
        {
            iStart[i]=GHandler2Gpib->Site[i];
            MY_DUT_PAL[i]->cbSiteOn->Checked = iStart[i];
        }

        bSimulate=GHandler2Gpib->bSimulate;
        bNeedInital=GHandler2Gpib->bTimeOutProcess;
        GpibAddress=GHandler2Gpib->GpibAddress;
        iDoubleContact=GHandler2Gpib->iLotStatus;                               //Steven 20201024 : Fixed for double contact
        if(GpibAddress!=oldGpibAddress)
        {
            LastSet.GpibAddress=GpibAddress;
            StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
            WriteLastDataFile();
            WriteLog(AnsiString("Handler ==> CHANGE ADDR : "+AnsiString(LastSet.GpibAddress)));  //Steven 20141212 : Add GPIB Log
        }

        bGpibMode=GHandler2Gpib->bGpibMode;
        BackIsRun=GHandler2Gpib->IsTest;

        if(BackIsRun==false)                                                    //jou 2011-12-19 當Handler送出IsTest=false,Task=1;
        {
            WriteLog("Handler ==> HALT TEST");                                  //Steven 20141212 : Add GPIB Log
            iMainTask=1;
            iGbibTask=1;
            LastSet.bFULLSITES=false;                                           //kevin 20130516 //Steven 20150303 : 避免停止測試還收到Binon

            if(CustomerCode==CC_KYEC_XILINX && LastSet.bUseBarcodeFunction==true && LastSet.bSQR41==true)   //jou 20170407 (Steven) : Xilinx 要求Time out要送Barcode reject
            {
                if(sBarCode->CommaText!="" && sBarCode->Count==TOTAL_SITE)      //Steven 20151217 : Fixed for 2D code
                {
                    for(int i=0; i<TOTAL_SITE; i++)
                    {
                        //AI(W906-GB-P1) 20260926: golden `sBarCode->Strings[i]==0`.  BCB6 AnsiString has no ==(const char*)
                        //  overload, so 0 converts through AnsiString(int) and the test is == "0".  vclcompat has
                        //  operator==(const AnsiString&, const char*), which would take 0 as a NULL pointer and test
                        //  == "" instead; AnsiString(0) pins the BCB6 meaning (AnsiString(0) is "0").
                        if(sBarCode->Strings[i]==AnsiString(0))
                            sBarCode->Strings[i]="0";
                        else
                            sBarCode->Strings[i]="BARCODEREJECT";
                    }
                }

                WriteLog(AnsiString("Handler ==> CHANGE ADDR : "+AnsiString(LastSet.GpibAddress)));
            }
            else
            {
                InitialBarcodeList();
            }
            LastSet.bSQR41=false;                                               //Steven 20161025 : 確保設定的資料有存起來
            WriteLastDataFile();                                                //Steven 20161025 : 確保設定的資料有存起來
        }
        else
        {
            bESCIsOpen=false;
            if(iDoubleContact==0)                                               //Steven 20201024 : Fixed for double contact
                Str="Handler ==> START TEST";
            else
                Str="Handler ==> START TEST of Double Contact";

            WriteLog(Str);                                                      //Steven 20141212 : Add GPIB Log
            AnsiString buffer="";
            if(LastSet.bUseBarcodeFunction || LastSet.i2DIDFormat==eIntel)      //JerryYang 20200423 Intel 沒開2D要回覆N,N,N...,N
            {
                buffer=AnsiString(GHandler2Gpib->Message);
                sBarCode->CommaText=buffer;                                     //Ifor 20200924 add:AMD Barcode 資料改由GPIB 處理

                WriteLog(AnsiString("Handler ==> BARCODE : "+sBarCode->CommaText));

                if(sBarCode->CommaText!="" && sBarCode->Count==TOTAL_SITE)      //Steven 20151217 : Fixed for 2D code
                {
                    for(int i=0; i<TOTAL_SITE; i++)
                    {
                        MY_DUT_PAL[i]->labOcr->Caption=sBarCode->Strings[(TOTAL_SITE-1)-i];
                    }
                }
                else
                {
                    WriteLog("Barcode count error when get from handler!!");    //Steven 20151217 : Fixed for 2D code
                    for(int i=0; i<TOTAL_SITE; i++)
                    {
                        MY_DUT_PAL[i]->labOcr->Caption="ERROR";
                    }
                }
            }
            else
            {
                InitialBarcodeList();
            }
            asATC_SiteMapping=AnsiString(GHandler2Gpib->UseSiteMapData);
            WriteLog(AnsiString("Handler ==> Site Mapping : "+asATC_SiteMapping));
            bHasSiteMapping=true;
        }
        LastSet.bHasBarCode=false;
        IsTest=BackIsRun;

        if(bFind)
        {
            GGpib2Handler.iCommand=MSG_CMD_NONE;                                //Steven 20141014 : Add GPIB Command
            COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
            pcp->dwData=0;
            pcp->cbData=sizeof(GGpib2Handler);
            pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;               //2012-10-11    Dell Fix
            GGpib2Handler.Result[0]='E';
            GGpib2Handler.Result[1]='C';
            GGpib2Handler.Result[2]='H';
            GGpib2Handler.Result[3]='O';
            PostToHandler(pcp);                                                 //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
            dwStart=GetTickCount();
            delete pcp;
        }
    }
    else if(iRecvCommand==MSG_CMD_MachineState)                                 //JerryYang 20160630 RS232加入GPIB //JerryYang 20151109 add for力成 回傳機台狀態
    {
        for(int i=16; i>=0; i--)                                                //JerryYang 20151109 0~16,共17個bit
        {
            s1+=IntToStr(GHandler2Gpib->iStatus[i]);
        }

        n=1;
        iMachineState=0;
        for(int i=0; i<17; i++)                                                 //二進制轉十進制
        {
            if(i>0)
            {
                n=n*2;
            }
            cTemp1[i]=GHandler2Gpib->iStatus[i];
            cTemp1[i]=cTemp1[i]*n;
            iMachineState=iMachineState+cTemp1[i];
        }
        sMachineStateDecade=IntToStr(iMachineState);
        iMacStateStrLength=sMachineStateDecade.Length();
        WriteLog("Handler ==> Machine State : "+s1);
    }
    else if(iRecvCommand==MSG_CMD_TesterBin)                                    //JerryYang 20160308 add for Maxim_Philippine回傳各Bin數量
    {
        sTestBinCount.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> TesterBinCount : "+sTestBinCount);
        fRS232Main->ShowCommData("[Handler ==> TesterBinCount]"+sTestBinCount,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_SoakTime)                                     //JerryYang 20160308 add for Maxim_Philippine回傳加熱時間
    {
        sSoakTime.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> SoakTime : "+sSoakTime);
        fRS232Main->ShowCommData("[Handler ==> SoakTime]"+sSoakTime,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_JamCode)                                      //JerryYang 20160308 add for Maxim_Philippine回傳Jam code
    {
        sJamCode.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> JamCode : "+sJamCode);
        fRS232Main->ShowCommData("[Handler ==> JamCode]"+sJamCode,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_AllMassTemp)                                  //JerryYang 20160330 add for Maxim_Philippine回傳All Mass Temp
    {
        sAllMassTemp.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> AllMassTemp : "+sAllMassTemp);
        fRS232Main->ShowCommData("[Handler ==> AllMassTemp]"+sAllMassTemp,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_HanderIDRS232)                                //JerryYang 20160330 add for Maxim_Philippine回傳All Mass Temp
    {
        sHandlerID.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> HandlerID : "+sHandlerID);
        fRS232Main->ShowCommData("[Handler ==> HandlerID]"+sHandlerID,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_GetSiteOnOff)
    {
        sSiteOnOff.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> SiteEnable : "+sSiteOnOff);
        fRS232Main->ShowCommData("[Handler ==> SiteEnable]"+sSiteOnOff,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_GetNumOfSites)
    {
        sNumOfSites.sprintf("%s", GHandler2Gpib->Message);
//        WriteLog("Handler ==> NumOfSites : "+sNumOfSites);
        fRS232Main->ShowCommData("[Handler ==> NumOfSites]"+sNumOfSites,  vector<Byte>(), true);
    }
    else if(iRecvCommand==MSG_CMD_RUN_HANA_ART)                                 //JimmyChiu 20241023 HANA ART Function
    {
        LastSet.bRunHANA_ART=(GHandler2Gpib->iLotStatus==1);
        InitHANA_ART();                                                         //Steven 20250414 : HANA ART Function
        bHanaDummyTest=false;
    }
    else if(iRecvCommand==MSG_CMD_HANA_ART)                                     //JimmyChiu 20241023 HANA ART Function
    {
        if(GHandler2Gpib->iLotStatus==1)                                        //send number
        {
            iMacStateStrLength=GHandler2Gpib->iStatus[0];                       //Steven 20250414 : HANA ART Function
            if(iMacStateStrLength==HANA_ART_SMILL::DUMMYTEST_START_SRQ0x42)
            {
                for(int i=0; i<TOTAL_SITE; i++)
                {
                    iStart[i]=GHandler2Gpib->Site[i];
                    MY_DUT_PAL[i]->cbSiteOn->Checked=iStart[i];
                }
                WriteLog("Handler ==> HANA ART : Dummy Start Test");
                IsTest=true;
                bSimulate=false;
                bNeedInital=false;
                bHanaDummyTest=true;
            }
            else
            {
                sSiteOnOff.sprintf("Handler ==> HANA ART: %s 0x%s", GHandler2Gpib->Message, IntToHex(iMacStateStrLength, 2));
                WriteLog(sSiteOnOff);
                ibstop(noncontroller);                                          //jou 20230907 : 修正0x59測試機收不到的問題
                ibrsv(noncontroller, iMacStateStrLength);
            }
        }
        else if(GHandler2Gpib->iLotStatus==2)                                   //send char
        {
            sSiteOnOff.sprintf("Handler ==> HANA ART: %s", GHandler2Gpib->Message);
            WriteLog(sSiteOnOff);
            //AI(W906-GB-P1) 20260926: golden quirk kept: char[2048] != "" compares ADDRESSES (always true; g++ -Waddress
            //  may warn).  Same in BCB6; harmless because buffer stays "" when Message is empty.
            if(GHandler2Gpib->Message!="")
                buffer.sprintf("%s", GHandler2Gpib->Message);
            MyGPIBWrite(buffer, sGbibTask);
        }
        else
        {
            sSiteOnOff.sprintf("Handler ==> HANA ART: Wrong Command!", GHandler2Gpib->Message);
            WriteLog(sSiteOnOff);
        }
    }
    else
    {
        if((int)iRecvCommand>=slCmdList->Count)                                 //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
            Str.sprintf("Handler ==> CMD %d : %s", iRecvCommand, GHandler2Gpib->Message);
        else
            //AI(W906-GB-P1) 20260926: golden passes slCmdList->Strings[iRecvCommand] straight to %s.  vclcompat
            //  Strings[] is a StringsProxy, which the variadic pass-through conv<T> would hand to vsnprintf as an
            //  object (garbage); AnsiString(...) materialises the same text (cObserver.cpp:4945 (N5) precedent).
            Str.sprintf("Handler ==> %s : %s", AnsiString(slCmdList->Strings[iRecvCommand]), GHandler2Gpib->Message);
        WriteLog(Str);
        buffer.sprintf("%s\r\n", GHandler2Gpib->Message);
//        MyGPIBWrite(buffer, sGbibTask);                                       //Steven 20250919 : Mark
    }
}

//------------------------------------------------------------------------------

}  // namespace gpibbridge
