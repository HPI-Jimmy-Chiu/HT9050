// ===========================================================================
//  TesterComm/Rs232/Rs232HandlerMsg.cpp -- TfRS232Main's Handler link: ProcessHandlerConnect, the two SendMSG_CMD
//  overloads, SendResultFinish, and OnMyCopyMsg (the RS232-side dispatcher for every Handler -> RS232 MSG_CMD_*).
//
//  AI(W906-GB-P4) 20260926: faithful translation of golden
//  D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\MainForm.cpp (Big5/cp950 -> UTF-8), per
//  TesterComm/Rs232/TRANSLATION_RULES.md.  Golden ranges, in file order:
//      580-654    TfRS232Main::ProcessHandlerConnect
//      655-683    TfRS232Main::SendMSG_CMD(int), TfRS232Main::SendMSG_CMD(int, AnsiString)
//      684-767    TfRS232Main::SendResultFinish
//      768-1387   TfRS232Main::OnMyCopyMsg
//  Bodies are golden text line by line: every else-if branch and its order (MSG_CMD_CloseGpib also sits in the
//  log-only list, reached when bCloseGpib is false or under DEBUG), the nested SendMSG_CMD(MSG_CMD_State_TTL) inside
//  MSG_CMD_TesterMode exactly where golden has it (the mailbox pumps while waiting, like SendMessage), the
//  second-board block of MSG_CMD_State_TTL that re-declares (shadows) the first block's locals, and the golden
//  quirks noted in place.  Deviations, each marked //AI(W906-GB-P4) in place:
//    * `__fastcall` dropped (rule 3).
//    * FindWindow("TfMain", <caption>) -> W906_FindHandlerWnd(): the Handler is attached while mailbox != NULL and
//      HMountWnd != NULL (Rs232Engine::Start sets HMountWnd to its mailbox token before FormCreate) (rule 4).
//    * SendMessage(HMountWnd, WM_COPYDATA, ...) -> PostToHandler(pcp); Close() -> RequestClose(<function>) (the
//      golden `return` after it kept); `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` ->
//      `GHandler2Gpib = reinterpret_cast<MV*>(P->lpData);`; this->Handle -> reinterpret_cast<HWND>(this)
//      (== Rs232Engine::BridgeWndToken()) (rule 4).
//    * ProcessHandlerConnect's three function-local statics re-armed per program life (rule 13).
//    * crc_chk(<char buffer>, ...) -> crc_chk(reinterpret_cast<unsigned char*>(<char buffer>), ...) (5 sites): golden
//      passes char* to the `unsigned char* data` parameter (cmydef.h:288), which BCB6 accepts with a
//      suspicious-pointer warning and g++ rejects.  Same bytes (Rs232Log.cpp does the same).
//    * HHandler2Gpib (MSG_CMD_2DIDFormat): a file-static copy of the RS232 program's own, never-written instance
//      (golden MessageDef.cpp:231), so the name cannot bind to the Handler's live ::HHandler2Gpib (see the note).
//  The golden build switches follow the include (rule 2): the `#ifndef DEBUG` blocks at golden :618 and :780 behave
//  as the release build unless RS232STD_GOLDEN_DEBUG=1.  None of these five functions has a SOFT_SIMULTE arm.
//  Not here (other RS232 files): ShowCommData / ShowCommDataHToR / ShowCommDataRToH, SendCommandToTester(_TTL),
//  Open/Close/Load/SaveSetup*, InitialBarcodeList, PostToHandler, RequestClose, crc_chk, and every global declared
//  extern in Rs232Bridge.h.
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

// golden MainForm.cpp:16-17, after its includes (see the Rs232Bridge.h banner)
#if RS232STD_GOLDEN_DEBUG
#define DEBUG
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif

// (these standard headers do not look at DEBUG / SOFT_SIMULTE)
#include <cstddef>      // offsetof (static_assert below)
#include <cstdlib>      // atoi (MSG_CMD_State_TTL)
#include <cstring>      // strcpy / strncpy

namespace rs232std {

//AI(W906-GB-P4) 20260926: golden FindWindow("TfMain", <Handler window caption>) (MainForm.cpp:596 / :609).  In-process
//   there is no Handler window to search: the Handler side is "found" while it is attached, i.e. while the engine owner
//   has set HMountWnd to its non-NULL token and the mailbox exists (Rs232Engine::Start, before FormCreate).  The caption
//   argument is ignored; it is kept so both golden call sites keep their shape.  Same helper as GpibCore.cpp's.
static HWND W906_FindHandlerWnd(const TfRS232Main* self, const char* /*caption*/)
{
    if(self!=NULL && self->mailbox!=NULL && HMountWnd!=NULL)
        return HMountWnd;
    return NULL;
}

//AI(W906-GB-P4) 20260926: golden SendMSG_CMD(int, AnsiString) strncpy's up to 544 bytes starting at VM::cReturn
//   ("Message最大是 256+32+256=544", MainForm.cpp:676): it deliberately runs on through GpibStatus[32] and GpibData[256],
//   which follow cReturn contiguously in VM (char arrays, no padding), and zero-pads the rest of that span.  Kept
//   byte-for-byte (the golden RS232 MessageDef.h:232-243 VM has the same layout as MessageDef.h); this assert pins the
//   layout that span relies on.  Same assert as GpibCore.cpp's.
static_assert(offsetof(VM, GpibData) + sizeof(VM::GpibData) - offsetof(VM, cReturn) == 544,
              "VM cReturn+GpibStatus+GpibData must be the contiguous 544-byte span golden SendMSG_CMD writes");

//AI(W906-GB-P4) 20260926: golden MSG_CMD_2DIDFormat (MainForm.cpp:855/:857) reads `HHandler2Gpib.iStatus[0]`, not
//   GHandler2Gpib->iStatus[0].  In the RS232 program HHandler2Gpib is its own MessageDef.cpp:231 instance of the
//   Handler-side packet, which nothing in RS232Standard ever writes (grep of the 902 folder: only :855/:857 read it;
//   Rs232Globals.cpp's banner "this program never reads them" misses these two reads),
//   so it is always zero and golden always logs "Standard 2DID" (eStandard == 0).  Rs232Bridge.h does not declare it,
//   and inside namespace rs232std the bare name would otherwise bind to the Handler's ::HHandler2Gpib -- the Handler's
//   live outgoing packet, read from the TesterComm thread.  This file-static, zero-initialised, never-written copy
//   keeps the golden outcome (a log line only).  Pure golden state: nothing to re-arm (rule 13).
static MV HHandler2Gpib;

void TfRS232Main::ProcessHandlerConnect()
{
    AnsiString str;
    static AnsiString Str1="";
    static bool bFirst=true;
    static int iFirstShowCount=0;                                               //Isaac 20210512 : 剛開啟RS232視窗要初始化，不要馬上送指令
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        Str1="";
        bFirst=true;
        iFirstShowCount=0;
    }

    if(HMountWnd==NULL)
    {
        TStringList *List=new TStringList();
        List->CommaText="HTXXXX,HT-9046,HT-9045,HT-9046LS,HT-9055,HT-502,HT-505,HT-1032AT,HT-1032,HT-7080,HT-9045_12Site,HT-9046LA,HT-9046CN,HT-9046AU,HT-9046CR,HT-9132LS,HT-9132,HT-9046";

        for(int i=0; i<List->Count; i++)                                        //Steven 20241001 : 改用For迴圈找機台標頭
        {
            if(HMountWnd==NULL)
            {
                //AI(W906-GB-P4) 20260926: golden FindWindow("TfMain", List->Strings[i].c_str()) (see W906_FindHandlerWnd)
                HMountWnd=W906_FindHandlerWnd(this, List->Strings[i].c_str());
                Str1=List->Strings[i];
            }
            else
            {
                break;
            }
        }
        List->Clear();
        delete List;
    }
    else
    {
        //AI(W906-GB-P4) 20260926: golden FindWindow("TfMain", Str1.c_str()) (see W906_FindHandlerWnd)
        HMountWnd=W906_FindHandlerWnd(this, Str1.c_str());
    }

    if(HMountWnd!=NULL)
    {
        bFind=true;
    }
    else
    {
        #ifndef DEBUG
            RequestClose("ProcessHandlerConnect");                             //AI(W906-GB-P4) 20260926: golden Close() (rule 4)
        #endif
       bFind=false;
       bFirst=false;                                                            //wei 20150617 Add version control
    }

    if(bFirst && bFind)                                                         //Steven 20141014 : 神盾測試模式
    {
        //AI(W906-GB-P4) 20260926: RS232Standard's first-connect sequence has no SleepEx (unlike GPIB Main.cpp:3122);
        //   none added.
        SendMSG_CMD(MSG_CMD_AskArmTestMode);

        GGpib2Handler.bOneCycle=false;
        GGpib2Handler.bError=false;
        GGpib2Handler.bEchoStop=false;
        GGpib2Handler.GPIBBin=0;
        SendMSG_CMD(MSG_CMD_Version, RS232Version);                             //wei 20150617 Add version control
        //AI(W906-GB-P4) 20260926: golden Str1 is the Handler caption FindWindow matched in the title loop above.  The
        //   engine attaches HMountWnd before the first call, so that loop never runs in-process and Str1 stays "" (log
        //   text only).  No fallback: the only Handler-model source in RS232Standard is CheckAndReadIniData(asHGeneralPath,
        //   "Version", "Model", ...), which writes the key back into the Handler's Gerneral.ini when it is missing.
        ShowCommData("[RS232]", "RS232 run with Handler", Str1);
        bFirst=false;
    }
    else if(bFind==false)
    {
        bFirst=true;
    }

    if(iFirstShowCount>-1)                                                      //Isaac 20210512 : 剛開啟RS232視窗要初始化，不要馬上送指令
    {
        iFirstShowCount++;
    }

    if(iFirstShowCount>2 && bFind==true)
    {
        iFirstShowCount=-999;                                                   //only send first time
        if(iUseRS232Mode>=InterfaceType_TTL)                                    //Isaac 20200903 :TTL RS232通訊
            SendMSG_CMD(MSG_CMD_State_TTL);
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::SendMSG_CMD(int CMD)
{
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;
    ShowCommDataRToH(CMD);                                                      //Steven 20240913 : 變更RS232 log記錄方式

    //AI(W906-GB-P4) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//---------------------------------------------------------------------------
void TfRS232Main::SendMSG_CMD(int CMD, AnsiString Message)           //wei 20150617 Add version control
{
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    //AI(W906-GB-P4) 20260926: 544-byte span kept exactly (see the static_assert at the top of this file).  Unlike
    //   the GPIB program, golden RS232 does not memset cReturn first; strncpy zero-pads the span after the text.
    strncpy(GGpib2Handler.cReturn, Message.c_str(), 544);                       //Message最大是 256+32+256=544     //Steven 20191009 : GpibStatus --> cReturn
    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;
    ShowCommDataRToH(CMD, Message);                                             //Steven 20240913 : 變更RS232 log記錄方式

    //AI(W906-GB-P4) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//---------------------------------------------------------------------------
void TfRS232Main::SendResultFinish()
{
    AnsiString S, Str="";
    bool bClosedSiteHaveBin=false;
    if(bFind && bManualTest==false)
    {
        if(iHasCE==iRS232Error)                                                 //Steven 20231205 : 判斷RS232流程是否異常
            GGpib2Handler.iCommand=MSG_CMD_BinonWithoutFullsite;
        else
            GGpib2Handler.iCommand=MSG_CMD_NONE;

        COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
        pcp->dwData=0;
        pcp->cbData=sizeof(GGpib2Handler);
        pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

        if(GGpib2Handler.iCommand==MSG_CMD_NONE)
        {
            for(int i=USE_SITE_COUNT-1; i>=0; i--)                              //jou 2015-03-23 use site
            {                                                                   //Steven 20171017 (wei) : Add Bin log to handler
                if(MY_DUT_PAL[i]->cbSiteOn->Checked==true)
                {
                    S.sprintf("%d", GGpib2Handler.Result[i]);                   //Isaac 20200903 :TTL RS232通訊
                    MY_DUT_PAL[i]->plSite->Caption=AnsiString(S);
                }
                else
                {
                    if(GGpib2Handler.Result[i]!=0 &&
                       GGpib2Handler.Result[i]!=999)                            //Steven 20240916 : 修正RS232 關site有bin
                    {
                        bClosedSiteHaveBin=true;
                    }
                    S="0";                                                      //Isaac 20200903 :0,->0
                    MY_DUT_PAL[i]->plSite->Caption=AnsiString(" ");
                }
                if(i!=0)                                                        //Isaac 20200903 :
                    S+=",";
                Str+=S;
            }
        }

        if(iHasCE==iRS232Error)
        {
            Str="";
            ShowCommData("[Tester]", "BA Without CE Flow!!");
            GGpib2Handler.iCommand=MSG_CMD_BinonWithoutFullsite;
            for(int i=USE_SITE_COUNT-1; i>=0; i--)
            {
                GGpib2Handler.Result[i]=999;
                MY_DUT_PAL[i]->plSite->Caption=AnsiString(999);
                S=AnsiString(999);
                if(i!=0)
                    S+=",";
                Str+=S;
            }
        }
        else if(bClosedSiteHaveBin==true)                                       //Steven 20240913 : Add close site have bin alarm
        {
            ShowCommData("[Tester]", "Closed Site Have Bin ERROR!!");

            if(iCheckClosedSiteHasBin==1)                                       //Steven 20241205 : 啟用關site有bin檢查
            {
                Str="";
                ShowCommData("[Tester]", "Set all bin to 999!!");
                GGpib2Handler.iCommand=MSG_CMD_CloseSiteHaveBin;
                for(int i=USE_SITE_COUNT-1; i>=0; i--)
                {
                    GGpib2Handler.Result[i]=999;
                    MY_DUT_PAL[i]->plSite->Caption=AnsiString(999);
                    S=AnsiString(999);
                    if(i!=0)
                        S+=",";
                    Str+=S;
                }
            }
        }

        ShowCommDataRToH(GGpib2Handler.iCommand, Str);
        //AI(W906-GB-P4) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp); (rule 4)
        PostToHandler(pcp);
        delete pcp;
    }
    bManualTest=false;
}
//---------------------------------------------------------------------------
void TfRS232Main::OnMyCopyMsg(TMessage &msg)
{
    bool BackIsRun=false;
    PCOPYDATASTRUCT P;
    P=(PCOPYDATASTRUCT) msg.LParam;
    unsigned int iRecvCommand;
    //AI(W906-GB-P4) 20260926: golden `&GHandler2Gpib->iSendCommand=(unsigned int *)P->lpData;` (BCB idiom: iSendCommand
    //   is MV's first member, so this re-seats the pointer at the packet).  P->lpData is the mailbox payload
    //   Rs232Engine::OnHandlerMessage wrapped in a COPYDATASTRUCT; valid for the duration of this call only, exactly
    //   like WM_COPYDATA.
    GHandler2Gpib = reinterpret_cast<MV*>(P->lpData);
    iRecvCommand=GHandler2Gpib->iSendCommand;
    AnsiString s1="";                                                           //JerryYang 20151109 先清除機台狀態
    int iMachineState, cTemp1[17], aDecade[17];                                 //JerryYang 20151109
    AnsiString sTTLState="";                                                    //Isaac 20200903 :TTL RS232通訊

#ifndef DEBUG
    if(iRecvCommand==MSG_CMD_CloseGpib)                                         //wei 20150408 Add Close GPIB Command
    {
        ShowCommData("[Handler ==> RS232]", "Program Closed");

        if(GHandler2Gpib->bCloseGpib)                                           //Steven 20180819 : 換位置
        {
            RequestClose("OnMyCopyMsg");                                       //AI(W906-GB-P4) 20260926: golden Close() (rule 4)
            return;
        }
    }
#endif

    if((HMountWnd!=NULL &&
        GHandler2Gpib->HandlerHwnd!=HMountWnd) ||
       GHandler2Gpib->GpibHwnd!=reinterpret_cast<HWND>(this))                   //Frank 20150925 銅鑼異常修改
    //AI(W906-GB-P4) 20260926: golden this->Handle (rule 4); the Handler must fill GpibHwnd with the token
    //   Rs232Engine::BridgeWndToken() returns (= this).
    {
        RequestClose("OnMyCopyMsg");                                           //AI(W906-GB-P4) 20260926: golden Close() (rule 4)
        return;                                                                 //Jou 2015-02-02
    }

    if(iRecvCommand==MSG_CMD_SwitchArmOK)                                       //Steven 20141014 : 神盾測試模式
    {
        ShowCommDataHToR(iRecvCommand);
        iTwoArmTestStep=2;
        SendData.push_back(_ACK_);
        SendCommandToTester(SendData);
    }
    else if(iRecvCommand==MSG_CMD_2ArmTestMode)                                 //Steven 20141014 : 神盾測試模式
    {
        ShowCommDataHToR(iRecvCommand);
        bTwoArmTestMode=true;
    }
    else if(iRecvCommand==MSG_CMD_1ArmTestMode)                                 //Steven 20141014 : 神盾測試模式
    {
        ShowCommDataHToR(iRecvCommand);
        bTwoArmTestMode=false;
    }
    else if(iRecvCommand==MSG_CMD_AbortTest)                                    //Steven 20141014 : 神盾測試模式
    {
        iHasCE=iNoneTest;                                                       //Steven 20231205 : 判斷RS232流程是否異常
        ShowCommDataHToR(iRecvCommand);
        SendStartData.push_back(_STX_);
        SendStartData.push_back('A');
        SendStartData.push_back('B');
        SendStartData.push_back(_ETX_);
        //AI(W906-GB-P4) 20260926: golden quirk kept: the STX A B ETX frame is appended to SendStartData (not cleared
        //   first), but no ENQ is queued and DoRevCommand's static iSendStart is not armed, and SendData -- normally
        //   empty -- is what gets sent (SendCommandToTester returns at iSize<=0).  Compare "CZ id?" (MainForm.cpp:
        //   3513-3526): frame into SendStartData, iSendStart=1, ENQ via SendData, frame sent on the tester's ACK.  So
        //   the abort frame never reaches the tester; later senders clear SendStartData first.
        SendCommandToTester(SendData);
        InitialBarcodeList();
    }
    else if(iRecvCommand==MSG_CMD_TimeOutSkip)                                  //Steven 20150304 : Add GPIB LOG
    {
        iHasCE=iNoneTest;                                                       //Steven 20231205 : 判斷RS232流程是否異常
        ShowCommDataHToR(iRecvCommand);
        InitialBarcodeList();
    }
    else if(iRecvCommand==MSG_CMD_TimeOutRetryWait ||
            iRecvCommand==MSG_CMD_TimeOutRetrySend ||
            iRecvCommand==MSG_CMD_Arm1Down ||
            iRecvCommand==MSG_CMD_Arm2Down ||
            iRecvCommand==MSG_CMD_ContactTestArm1 ||
            iRecvCommand==MSG_CMD_ContactTestArm2 ||
            iRecvCommand==MSG_CMD_CloseGpib ||
            iRecvCommand==MSG_CMD_LotStatus)
    {
        ShowCommDataHToR(iRecvCommand);
    }
    else if(iRecvCommand==MSG_CMD_HandlerHomeStart ||
            iRecvCommand==MSG_CMD_HandlerHomeFinish ||
            iRecvCommand==MSG_CMD_ContactTestAbort)
    {
        iHasCE=iNoneTest;                                                       //Steven 20231205 : 判斷RS232流程是否異常
        ShowCommDataHToR(iRecvCommand);
    }
    else if(iRecvCommand==MSG_CMD_2DIDFormat)
    {
        //AI(W906-GB-P4) 20260926: golden reads HHandler2Gpib (the file-static never-written copy at the top of this
        //   file, always 0 -> "Standard 2DID"), not GHandler2Gpib; kept.
        if(HHandler2Gpib.iStatus[0]==eAMD)
            s1="AMD 2DID";
        else if(HHandler2Gpib.iStatus[0]==eIntel)
            s1="Intel 2DID";
        else
            s1="Standard 2DID";
        ShowCommDataHToR(iRecvCommand, s1);
    }
    else if(iRecvCommand==MSG_CMD_ChangeGpib)                                   //wei 20150409 Add Close GPIB Command
    {
        ShowCommDataHToR(iRecvCommand, "Setting Start");

        bSimulate       =GHandler2Gpib->bSimulate;
        if(bSimulate)
            ShowCommDataHToR(iRecvCommand, "Simulate RS232");

        bSupport32Bin   =GHandler2Gpib->bSupport32Bin;
        if(bSupport32Bin)
        {
            iMaxBinCount=GHandler2Gpib->GpibAddress;                            //Steven 20240912 : 最大Bin值
            s1.sprintf("%d Bin mode", iMaxBinCount);
            ShowCommDataHToR(iRecvCommand, s1);
        }
        else
        {
            iMaxBinCount=16;
            ShowCommDataHToR(iRecvCommand, "16 Bin mode");
        }

        bGpibMode=GHandler2Gpib->bGpibMode;
        if(bGpibMode==false)
            ShowCommDataHToR(iRecvCommand, "RS232 Mode");
        else
            ShowCommDataHToR(iRecvCommand, "GPIB Mode");

        ShowCommDataHToR(iRecvCommand, "Setting End");
    }
    else if(iRecvCommand==MSG_CMD_EnableBarCode)                                //Jimmychiu 20211004 add barcode
    {
        bHasBarCode=true;
        ShowCommDataHToR(iRecvCommand);
    }
    else if(iRecvCommand==MSG_CMD_DisableBarCode)                               //Jimmychiu 20211004 add barcode
    {
        bHasBarCode=false;
        ShowCommDataHToR(iRecvCommand);
    }
    else if(iRecvCommand==MSG_CMD_Command_TTL)                                  //Isaac 20200903 :TTL RS232通訊
    {
        char cCRC[2];

        crc_chk(reinterpret_cast<unsigned char*>(GHandler2Gpib->Message), GHandler2Gpib->iLotStatus, cCRC[0], cCRC[1]);   //AI(W906-GB-P4) 20260926: char* -> unsigned char* (see banner)
        SendStartData.clear();
        for(int i=0; i<GHandler2Gpib->iLotStatus; i++)
        {
            SendStartData.push_back(GHandler2Gpib->Message[i]);
        }
        SendStartData.push_back(cCRC[0]);
        SendStartData.push_back(cCRC[1]);
        SendStartData.push_back('#');

        //AI(W906-GB-P4) 20260926: the prefix tests below never index past the end: the frame always ends with '#', which
        //   matches none of the compared letters, so every && chain stops at or before the last element.
        if(SendStartData[0]=='@' && SendStartData[1]=='0' && SendStartData[2]=='0' &&   //Frank 20220408 Add TTL ASE_JP Mode //Isaac 20210309 :TTL RS232兩塊板子
           SendStartData[3]=='W' && SendStartData[4]=='D' && SendStartData[5]=='U' &&
           SendStartData[6]=='T' && SendStartData[7]=='S')
        {
            SendCommandToTester_TTL(SendStartData, 0);
        }
        else if(SendStartData[0]=='@' && SendStartData[1]=='W' && SendStartData[2]=='C' && SendStartData[3]=='S' &&
           SendStartData[4]=='O' && SendStartData[5]=='T')
        {
            SendCommandToTester_TTL(SendStartData, 0);
        }
        else if(iTTLBoardNum>=1 &&
                SendStartData[0]=='@' && SendStartData[1]=='0' && SendStartData[2]=='0' &&   //Isaac 20210309 :TTL RS232兩塊板子
                SendStartData[3]=='W' && SendStartData[4]=='C' && SendStartData[5]=='S' &&
                SendStartData[6]=='O' && SendStartData[7]=='T')
        {
            SendCommandToTester_TTL(SendStartData, 0);
        }
        else if(iTTLBoardNum>1 &&
                SendStartData[0]=='@' && SendStartData[1]=='0' && SendStartData[2]=='1' &&   //Isaac 20210309 :TTL RS232兩塊板子
                SendStartData[3]=='W' && SendStartData[4]=='C' && SendStartData[5]=='S' &&
                SendStartData[6]=='O' && SendStartData[7]=='T')
        {
            SendCommandToTester_TTL(SendStartData, 1);
        }
    }
    else if(iRecvCommand==MSG_CMD_NONE)                                         //jou 2015-03-23 RS232 double test issue
    {
        AnsiString asTester="";                                                 //Frank 20180723 :TTL RS232通訊
        AnsiString asSendtoBoard1="";                                           //Isaac 20210309 :TTL RS232兩塊板子
        AnsiString asSendtoBoard2="";                                           //Isaac 20210309 :TTL RS232兩塊板子
        bool bNeedTest[2]={true,true};                                          //Isaac 20210309 :TTL RS232兩塊板子
        bManualTest =false;
        iHasCE      =iNoneTest;                                                 //Steven 20231205 : 判斷RS232流程是否異常

        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {
            iStart[i]=GHandler2Gpib->Site[i];                                   //只開site1,site2
            asTester+=(GHandler2Gpib->Site[i]==1) ? AnsiString("1") : AnsiString("0");//Isaac 20200903 :TTL RS232通訊   //11000000_00000000_00000000_00000000
        }                                                                                                               //1      8 9     16 17    24 25    32

        bSimulate       =GHandler2Gpib->bSimulate;
        iTwoArmTestStep =0;
        bSupport32Bin   =GHandler2Gpib->bSupport32Bin;
        if(bSupport32Bin)
        {
            iMaxBinCount=GHandler2Gpib->GpibAddress;
        }
        else
        {
            iMaxBinCount=16;
        }

        // Show Result Data
        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {
            MY_DUT_PAL[i]->cbSiteOn->Checked=iStart[i];
            MY_DUT_PAL[i]->plSite->Caption=(MY_DUT_PAL[i]->cbSiteOn->Checked==true)?"T":"";
        }

        //Jimmychiu 20211004 add barcode
        //<=
        BackIsRun=GHandler2Gpib->IsTest;                                        //jou 2011-12-19 當Handler送出IsTest=false,Task=1;
        if(BackIsRun==false)
        {
            ShowCommDataHToR(iRecvCommand, "HALT TEST");
            InitialBarcodeList();
        }
        else //if(BackIsRun)
        {
            ShowCommDataHToR(iRecvCommand, "START TEST");
            AnsiString buffer="";
            if(bHasBarCode)
            {
                buffer=AnsiString(GHandler2Gpib->Message);
                sBarCode->CommaText=buffer;
                ShowCommDataHToR(iRecvCommand, "BARCODE : "+sBarCode->CommaText);
                if(sBarCode->CommaText!="" && sBarCode->Count==USE_SITE_COUNT)  //Steven 20151217 : Fixed for 2D code
                {
                    for(int i=0; i<USE_SITE_COUNT; i++)
                    {
                        MY_DUT_PAL[i]->labOcr->Caption=sBarCode->Strings[(USE_SITE_COUNT-1)-i];
                    }
                }
                else
                {
                    ShowCommDataHToR(iRecvCommand, "Barcode count error when get from handler!!");
                    for(int i=0; i<USE_SITE_COUNT; i++)
                    {
                        MY_DUT_PAL[i]->labOcr->Caption="ERROR";
                    }
                }
            }
        }
        //<=
        //Jimmychiu 20211004 add barcode

        if(iUseRS232Mode>=InterfaceType_TTL && bSimulate==false)                //Frank 20220408 Add TTL ASE_JP Mode //Isaac 20200903 :TTL RS232通訊
        {
            AnsiString asTest="";
            AnsiString asTest2="";
            char cStart[128];
            char cBuf[128];
            char cCRC[2];

            if(iTTLBoardNum>=2 && bTTLBoardAddr)                                //Isaac 20210309 :TTL RS232兩塊板子
            {                                                                   //asTester:1111 0101
                asSendtoBoard1=asTester.SubString(1,4);                                  //1111+0000
                if(asSendtoBoard1=="0000")
                {
                    bNeedTest[0]=false;
                }
                else
                {
                    asTest.sprintf("@00WSOTS%s0000", asSendtoBoard1);           //@WSOTS11000000
                }

                asSendtoBoard2=asTester.SubString(5,4);                         //0101+0000
                if(asSendtoBoard2=="0000")
                {
                    bNeedTest[1]=false;
                }
                else
                {
                    asTest2.sprintf("@01WSOTS%s0000", asSendtoBoard2);          //@02WSOTS1100000
                }
            }
            else                                                                //iTTLBoardNum>=1 //一塊板子
            {
                asTester=asTester.SubString(1,8);                               //11000000
                if(asTester=="00000000")
                {
                    bNeedTest[0]=false;
                }
                else
                {
                    if(bTTLBoardAddr)
                    {
                        asTest.sprintf("@00WSOTS%s", asTester);                 //@01WSOTS11000000
                    }
                    else
                    {
                        asTest.sprintf("@WSOTS%s", asTester);                   //@WSOTS11000000
                    }
                }
                bNeedTest[1]=false;
            }

            const int iLength=asTest.Length();
            strcpy(cBuf, asTest.c_str());
            crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength, cCRC[0], cCRC[1]);             //AI(W906-GB-P4) 20260926: char* -> unsigned char* (see banner)
            strcpy(cStart, asTest.c_str());                                     //@WSOTS11000000

            SendStartData.clear();
            for(int i=0; i<asTest.Length(); i++)
            {
                SendStartData.push_back(cStart[i]);                             //@WSOTS11000000
            }

            SendStartData.push_back(cCRC[0]);
            SendStartData.push_back(cCRC[1]);                                   //@WSOTS11000000'CRC'
            SendStartData.push_back('#');                                       //@WSOTS11000000'CRC'#

            if(bNeedTest[0]==true)
            {
                SendCommandToTester_TTL(SendStartData, 0);                      //@WSOTS11000000'CRC'#  送測試訊號
                bStartSOT[0]=true;
            }
            else
            {
                bStartSOT[0]=false;
            }

            if(iTTLBoardNum>=2)                                                 //Isaac 20210309 :TTL RS232兩塊板子
            {
                const int iLength2=asTest2.Length();
                strcpy(cBuf, asTest2.c_str());
                crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength2, cCRC[0], cCRC[1]);        //AI(W906-GB-P4) 20260926: char* -> unsigned char* (see banner)
                strcpy(cStart, asTest2.c_str());                                //@WSOTS11000000

                SendStartData1.clear();
                for(int i=0; i<asTest2.Length(); i++)
                {
                    SendStartData1.push_back(cStart[i]);                        //@WSOTS11000000
                }

                SendStartData1.push_back(cCRC[0]);
                SendStartData1.push_back(cCRC[1]);                              //@WSOTS11000000'CRC'
                SendStartData1.push_back('#');                                  //@WSOTS11000000'CRC'#

                if(bNeedTest[1]==true)
                {
                    SendCommandToTester_TTL(SendStartData1, 1);
                    bStartSOT[1]=true;
                }
                else
                {
                    bStartSOT[1]=false;
                }
            }
        }
    }
    else if(iRecvCommand==MSG_CMD_MachineState)                                 //JerryYang 20151109 add for力成 回傳機台狀態
    {
        for(int i=16; i>=0; i--)                                                //JerryYang 20151109 0~16,共17個bit
        {
            s1+=IntToStr(GHandler2Gpib->iStatus[i]);
        }
        int j=1;
        iMachineState=0;
        for(int i=0; i<17; i++)                                                 //二進制轉十進制
        {
            if(i>0)
            {
                j=j*2;
            }
            cTemp1[i]=GHandler2Gpib->iStatus[i];
            cTemp1[i]=cTemp1[i]*j;
            iMachineState=iMachineState+cTemp1[i];
        }
        sMachineStateDecade=IntToStr(iMachineState);
        iMacStateStrLength=sMachineStateDecade.Length();
        ShowCommDataHToR(iRecvCommand, s1);
    }
    else if(iRecvCommand==MSG_CMD_TesterBin)                                    //JerryYang 20160308 add for Maxim_Philippine回傳各Bin數量
    {
        sTestBinCount.sprintf(" %s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sTestBinCount);
    }
    else if(iRecvCommand==MSG_CMD_SoakTime)                                     //JerryYang 20160308 add for Maxim_Philippine回傳加熱時間
    {
        sSoakTime.sprintf(" %s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sSoakTime);
    }
    else if(iRecvCommand==MSG_CMD_JamCode)                                      //JerryYang 20160308 add for Maxim_Philippine回傳Jam code
    {
        sJamCode.sprintf(" %s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sJamCode);
    }
    else if(iRecvCommand==MSG_CMD_SiteMap)                                      //JerryYang 20160308 add for Maxim_Philippine回傳Jam code
    {
        sSiteMap.sprintf(" %s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sSiteMap);
    }
    else if(iRecvCommand==MSG_CMD_AllMassTemp)                                  //JerryYang 20160330 add for Maxim_Philippine回傳All Mass Temp
    {
        sAllMassTemp.sprintf(" %s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sAllMassTemp);
    }
    else if(iRecvCommand==MSG_CMD_TesterMode)                                   //Isaac 20200903 :TTL RS232通訊
    {
        iUseRS232Mode=GHandler2Gpib->GPIBBin;
        iTTLBoardNum=GHandler2Gpib->bSupport32Bin+1;                            //0:1塊板子 ; 1:2塊板子
        bTTLBoardAddr=GHandler2Gpib->bGpibMode;                                 //Reply board address
        CloseTesterComm();
        CloseTesterComm_TTL(0);
        CloseTesterComm_TTL(1);

        if(iUseRS232Mode>=InterfaceType_TTL)
        {
            LoadSetupData_TTL();                                                //Isaac 20200903 :TTL RS232單板通訊
            OpenTesterComm_TTL(0);
            SaveSetupData_TTL();

            if(iTTLBoardNum>=2)
            {
                OpenTesterComm_TTL(1);
                ShowCommDataHToR(iRecvCommand, " Change tester mode TTL(2Board).");
            }
            else
            {
                ShowCommDataHToR(iRecvCommand, " Change tester mode TTL(1Board).");
            }

            if(iUseRS232Mode>=InterfaceType_TTL)
                SendMSG_CMD(MSG_CMD_State_TTL);
        }
        else if(iUseRS232Mode==InterfaceType_Standard)
        {
            LoadSetupData(CommTester);                                          //RS232模式
            ShowCommDataHToR(iRecvCommand, " Change tester mode RS232 Standard.");
            OpenTesterComm();
            SaveSetupData(CommTester);
        }
        else if(iUseRS232Mode==InterfaceType_SLT)
        {
            LoadSetupData(CommTester);                                          //RS232模式
            ShowCommDataHToR(iRecvCommand, " Change tester mode RS232 SLT.");
            OpenTesterComm();
            SaveSetupData(CommTester);
        }
    }
    else if(iRecvCommand==MSG_CMD_State_TTL)                                    //Frank 20180723 :TTL RS232通訊
    {
        char cBuffer[1024];
        char cINIT[1024];
        char cCRC[2];
        AnsiString sString,sStr[2];
        AnsiString TSVolet, Mode;
        AnsiString SOTLogic, DataLogic, EOTLogic, DutLogic, SOTLength, DutLength, BinTimeOut;

        iTTLBoardNum=GHandler2Gpib->bSupport32Bin+1;                            //GHandler2Gpib->bSupport32Bin 0:一塊TTL板 ; 1 :二塊TTL板
        bTTLBoardAddr=GHandler2Gpib->bGpibMode;
        sString.sprintf("TTL Board must use %d board(MSG_CMD_State_TTL)", iTTLBoardNum);//iTTLBoardNum 1:一塊TTL板 ; 2 :二塊TTL板
        ShowCommDataHToR(iRecvCommand, sString);

        sTTLState.sprintf("%s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sTTLState);

        //Data[0] TS+5V
        TSVolet    = sTTLState.SubString(1, 1);

        //Data[1] Bin Mode
        Mode       = sTTLState.SubString(2, 1);
        if(Mode=="0")
        {
            ShowCommDataHToR(iRecvCommand, "Bin Mode is 5 BitBit");
        }
        else if(Mode=="1")
        {
            ShowCommDataHToR(iRecvCommand, "Bin Mode is 10 BitBit");
        }
        else if(Mode=="2")
        {
            ShowCommDataHToR(iRecvCommand, "Bin Mode is 5 BitBinary");
        }
        else
        {
            ShowCommDataHToR(iRecvCommand, "Bin Mode is 10 BitBinary");
        }

        SOTLogic=sTTLState.SubString(3, 8);                                     //Data[2-9] SOT Active Logic
        if(SOTLogic.c_str()[0]=='1')                                            //Positive
            ShowCommDataHToR(iRecvCommand, "SOT Active Logic is Positive");
        else                                                                    //Negative
            ShowCommDataHToR(iRecvCommand, "SOT Active Logic is Negative");

        DataLogic=sTTLState.SubString(11, 8);                                   //Data[10-17] Data Active Logic
        if(DataLogic.c_str()[0]=='1')                                           //Positive
            ShowCommDataHToR(iRecvCommand, "BIN Active Logic is Positive");
        else                                                                    //Negative
            ShowCommDataHToR(iRecvCommand, "BIN Active Logic is Negative");

        EOTLogic=sTTLState.SubString(19, 8);                                    //Data[18-25] EOT Active Logic
        if(EOTLogic.c_str()[0]=='1')                                            //Positive
            ShowCommDataHToR(iRecvCommand, "EOT Active Logic is Positive");
        else                                                                    //Negative
            ShowCommDataHToR(iRecvCommand, "EOT Active Logic is Negative");

        DutLogic=sTTLState.SubString(27, 8);                                    //Data[26-33] DUT Active Logic
        if(DutLogic.c_str()[0]=='1')                                            //Positive
            ShowCommDataHToR(iRecvCommand, "DUT Active Logic is Positive");
        else                                                                    //Negative
            ShowCommDataHToR(iRecvCommand, "DUT Active Logic is Negative");

        SOTLength=sTTLState.SubString(35, 4);                                   //Data[34-37] SOT Width (MS, 最大1000)
        sString.sprintf("SOT Width is %dms", atoi(SOTLength.c_str()));
        ShowCommDataHToR(iRecvCommand, sString);

        DutLength=sTTLState.SubString(39, 4);                                   //Data[38-41] Dut Width (MS, 最大1000)
        sString.sprintf("DUT Width is %dms", atoi(DutLength.c_str()));
        ShowCommDataHToR(iRecvCommand, sString);

        BinTimeOut=sTTLState.SubString(43, 6);                                  //Data[42-47] BIN Time Out (MS, 最大500000)
        sString.sprintf("BIN Time Out is %dms", atoi(BinTimeOut.c_str()));
        ShowCommDataHToR(iRecvCommand, sString);

        if(iTTLBoardNum>=2 || bTTLBoardAddr)
        {
            sString.sprintf("@00WINIT%s%s%s%s%s%s%s%s%s", TSVolet, Mode, SOTLogic, DataLogic, EOTLogic, DutLogic, SOTLength, DutLength, BinTimeOut);
        }
        else
        {
            sString.sprintf("@WINIT%s%s%s%s%s%s%s%s%s", TSVolet, Mode, SOTLogic, DataLogic, EOTLogic, DutLogic, SOTLength, DutLength, BinTimeOut);
        }
        const int iLength=sString.Length();
        ZeroMemory(cBuffer, sizeof(cBuffer));                                   //先清空記憶體再讀取資料, 避免舊資料出現
        ZeroMemory(cINIT, sizeof(cINIT));                                       //先清空記憶體再讀取資料, 避免舊資料出現
        ZeroMemory(cCRC, sizeof(cCRC));                                         //先清空記憶體再讀取資料, 避免舊資料出現
        SendStartData.clear();

        strcpy(cBuffer, sString.c_str());
        crc_chk(reinterpret_cast<unsigned char*>(cBuffer), iLength, cCRC[0], cCRC[1]);              //AI(W906-GB-P4) 20260926: char* -> unsigned char* (see banner)
        strcpy(cINIT, sString.c_str());

        for(int i=0; i<sString.Length(); i++)
        {
            SendStartData.push_back(cINIT[i]);
        }
        SendStartData.push_back(cCRC[0]);
        SendStartData.push_back(cCRC[1]);
        SendStartData.push_back('#');

        if(bCommConnect[1]==true)
        {
            SendCommandToTester_TTL(SendStartData, 0);
        }

        if(iTTLBoardNum>=2)                                                     //Isaac 20210309 :TTL RS232兩塊板子
        {
            sTTLState.sprintf("%s", GHandler2Gpib->Message);
            char cBuffer[1024];
            char cINIT[1024];
            char cCRC[2];
            AnsiString sString,sStr[2];
            AnsiString TSVolet, Mode;
            AnsiString SOTLogic, DataLogic, EOTLogic, DutLogic, SOTLength, DutLength, BinTimeOut;

            //Data[0] TS+5V
            TSVolet    = sTTLState.SubString(1, 1);

            //Data[1] Bin Mode
            Mode       = sTTLState.SubString(2, 1);

            //Data[2-9] SOT Active Logic
            SOTLogic=sTTLState.SubString(3, 8);

            //Data[10-17] Data Active Logic
            DataLogic=sTTLState.SubString(11, 8);

            //Data[18-25] EOT Active Logic
            EOTLogic=sTTLState.SubString(19,8);

            //Data[26-33] DUT Active Logic
            DutLogic=sTTLState.SubString(27, 8);

            //Data[34-37] SOT Width (MS, 最大1000)
            SOTLength=sTTLState.SubString(35, 4);

            //Data[38-41] Dut Width (MS, 最大1000)
            DutLength=sTTLState.SubString(39, 4);

            //Data[42-47] BIN Time Out (MS, 最大500000)
            BinTimeOut=sTTLState.SubString(43, 6);

            sString.sprintf("@01WINIT%s%s%s%s%s%s%s%s%s", TSVolet, Mode, SOTLogic, DataLogic, EOTLogic, DutLogic, SOTLength, DutLength, BinTimeOut);

            const int iLength=sString.Length();
            ZeroMemory(cBuffer, sizeof(cBuffer));                               //先清空記憶體再讀取資料, 避免舊資料出現
            ZeroMemory(cINIT, sizeof(cINIT));                                   //先清空記憶體再讀取資料, 避免舊資料出現
            ZeroMemory(cCRC, sizeof(cCRC));                                     //先清空記憶體再讀取資料, 避免舊資料出現
            SendStartData.clear();

            strcpy(cBuffer, sString.c_str());
            crc_chk(reinterpret_cast<unsigned char*>(cBuffer), iLength, cCRC[0], cCRC[1]);          //AI(W906-GB-P4) 20260926: char* -> unsigned char* (see banner)
            strcpy(cINIT, sString.c_str());

            for(int i=0; i<sString.Length(); i++)
            {
                SendStartData.push_back(cINIT[i]);
            }
            SendStartData.push_back(cCRC[0]);
            SendStartData.push_back(cCRC[1]);
            SendStartData.push_back('#');

            if(bCommConnect[2]==true)
            {
                SendCommandToTester_TTL(SendStartData, 1);
            }
        }
    }
    else if(iRecvCommand==MSG_CMD_DoubleContactCount)                           //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
    {
        sDoubleContactCount.sprintf("%s", GHandler2Gpib->Message);
        ShowCommDataHToR(iRecvCommand, sDoubleContactCount);
    }
    else                                                                        //Isaac 20200903 :TTL RS232通訊
    {
        ShowCommDataHToR(iRecvCommand, "Command is not supported!");
    }
}
//---------------------------------------------------------------------------

}  // namespace rs232std
