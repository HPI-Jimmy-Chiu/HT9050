//---------------------------------------------------------------------------
//  Interface/InterfaceSYS.cpp -- translated whole-file from golden reference
//    HT9011UC_Code_V3.33.906.0_20260618/Interface/InterfaceSYS.cpp (531 lines)
//
//  AI(W5-InterfaceSYS-Translate) 20260710: see InterfaceSYS.h for the module's
//  role (WM_COPYDATA IPC bridge to the GPIB/Tester, ESD, Auto-Update and
//  Event-Log-Analyzer helper programs).  This TU realizes:
//    - GPIBInterface_Data class bodies (pure Win32 FindWindow/SendMessage IPC,
//      no golden gaps other than the dtor `delete iter` note below).
//    - The outbound command builders _CloseGpibProgram / _RunTestProgram /
//      _RunTestProgram_BarMess / _SendADDRToInterfaceProgram /
//      _SendInformationToInterfaceProgram -- these are exactly what
//      atester.cpp's GATED GetTesterResult (golden :849-2563; calls
//      _RunTestProgram_BarMess/_RunTestProgram at golden :1521/:1525, reads
//      bEcho/bGPIBError/bExist/bUnderTest) will call once un-gated -- see the
//      translate report for the precise confirmation (and the
//      PlaceOSTestResultToTray correction: that symbol lives in
//      Interface/TesterTCP.cpp, NOT here -- out of this unit's scope).
//    - The inbound TEST-echo decoder _GPIBGetCommand_TEST (the AnsiString
//      "BARCODEERR:" parse is 100% ACTIVE/faithful and 1-based-AnsiString
//      exercised; the 3 statements that write into fBarCode's members are
//      ACTIVE too, as of the W5-comms INTEGRATE pass below).
//    - The WM_COPYDATA receive dispatcher _OnMyCopyMsg_Interface (signature
//      adapted -- see InterfaceSYS.h note -- and one golden non-compiling
//      line reinterpreted; see inline note at the call site).
//    - The ESD / EventLog / AutoUpdate senders (originally gated where they
//      touched FormsFacade TfMain members that did not exist yet -- HESDWnd /
//      HEventLogWnd / HAutoUpdateWnd / oldGpibAddress).
//
//  AI(W5-comms-Integrate) 20260710: the serial integrate pass added the 4
//  HWND/int members to FormsFacade's TfMain (FormsFacade.h/.cpp) and the 3
//  bGPIBTestBarCodeError/iBinReturnMess[32]/bGPIBTestBarCodeFormatError
//  members to aHotPlateSubstrate.h's TfBarCode, then lifted every `#if 0`
//  gate this unit's translate report flagged as blocked ONLY on those fields
//  (verbatim bodies, per the report's own "once added, simply lift this gate"
//  note) -- see the inline AI(W5-comms-Integrate) comments at each former gate.
//---------------------------------------------------------------------------
#include "InterfaceSYS.h"

#include "cpublic.h"
#include "cprod.h"
#include "cmydef.h"

#include "aHotPlateSubstrate.h"     // TestSocket (TMyKitSuck) / fBarCode (TfBarCode, BarCode/BarCode.h) / MyDBIProcess
#include "canary_support.h"         // LastSet (LAST_GENERAL_SET minimal shim -- .iTester)
#include "CosFunction.h"            // CosFunction (.bUseMDB)
#include "FormsFacade.h"            // fMain (TfMain facade)

#include <cstdlib>                  // atoi (SendCommand_EventLog/AutoUpdate -- currently gated, see below)

//---------------------------------------------------------------------------
//編寫紀錄
//bGPIBError , bEcho , bExist 錯誤程序  支援
//bDoubleContact  支援
//

using namespace std;
#define iGPIBMAXItem 32

GPIBInterface_Data *GPIBSend=new GPIBInterface_Data("TSerialPoll", "Interface", WM_Interface_GPIB);
M_V *MV_Event=new M_V;
M_V *MV_ESD=new M_V;
M_V *MV_AutoUpdate=new M_V;                                                     //Steven 20150728 : Auto Update

// AI(W5-InterfaceSYS-Translate) 20260710: golden main.cpp:15151-15156 defines these 5
// globals (bGPIBError/bEcho/bUnderTest/bExist/iBin[4][8]); main.cpp itself is not yet
// translated (W7, huge TfMain VCL form).  Declared `extern` here exactly as golden --
// a LINK-time (not compile-time) dependency: this TU compiles clean either way, and as
// a STATIC library member it only needs these resolved if something actually calls into
// the functions below.  Flagged in the translate report as needing a temporary home
// until main.cpp lands; this unit's own test TU supplies local stand-in definitions
// purely to link+run standalone (see tests/test_interfacesys.cpp).
extern bool bGPIBError;                                                         //OLD : handles Tester-side data-format error reporting.
extern bool bEcho, bUnderTest;                                                  //OLD : handles the Tester echo (Echo=true once Handler received the Tester's data)
extern bool bExist;                                                             //OLD : handles the Tester timeout path
extern bool bDoubleContact;                                                     // already defined in the translated atester.cpp (bDoubleContact=false)
extern unsigned int iBin[4][8];
//==============================================================================
GPIBInterface_Data::GPIBInterface_Data(AnsiString aFormName, AnsiString aCaptionName, WPARAM WPARAM_TO)
{
    bModeType     = 0x00 ;
    bCommandType  = 0x00 ;
    bCommand      = 0x00 ;
    FormName      = aFormName;
    CaptionName   = aCaptionName;
    _WPARAM_TO    = WPARAM_TO;

    vData.clear();
    vMessage.clear();
};
//==============================================================================
GPIBInterface_Data::~GPIBInterface_Data()
{
    try
    {
        // AI(W5-InterfaceSYS-Translate) 20260710: golden loops
        // `for(iterator iter=vData.begin(); ...; ++iter) delete iter;` over both vectors.
        // Under BCB6's old RW-STL, vector<Byte>::iterator was a raw Byte*, so `delete iter`
        // merely compiled there -- a harmless/UB no-op, since these Bytes are plain values
        // living inside the vector's contiguous storage, never individually new'd/owned.
        // Under the standard library (libstdc++/MSVC __normal_iterator wrapper types),
        // `delete` on a non-pointer iterator is a hard compile error, so a literal port is
        // not possible.  Kept the semantically-equivalent effect: drop the buffers.
        vData.clear();
        vMessage.clear();
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~GPIBInterface_Data");
    }
};
//==============================================================================
bool GPIBInterface_Data::bFindProgram()
{
    _HVisionWnd=FindWindow(FormName.c_str(), CaptionName.c_str());
    if(_HVisionWnd)
        return true;
    else
        return false;
};
//==============================================================================
void GPIBInterface_Data::PushData(int iDatain[], int iSize)
{
    vData.clear();
    for(int i=0; i<iSize; i++)
        vData.push_back((Byte)*(iDatain+i));
};
//==============================================================================
void GPIBInterface_Data::PushData(bool bDatain[], int iSize)
{
    vData.clear();
    for(int i=0; i<iSize; i++)
    {
        if(*(bDatain+i))
            vData.push_back('1');
        else
            vData.push_back('0');
    }
};
//==============================================================================
void GPIBInterface_Data::PushMessage(int iMessagein[], int iSize)
{
    vMessage.clear();
    for(int i=0; i<iSize; i++)
        vMessage.push_back((Byte)*(iMessagein+i));
};
//==============================================================================
void GPIBInterface_Data::PushMessage(bool bMessagein[], int iSize)
{
    vMessage.clear();
    for(int i=0; i<iSize; i++)
    {
        if(*(bMessagein+i))
            vMessage.push_back('1');
        else
            vMessage.push_back('0');
    }
};
//==============================================================================
void GPIBInterface_Data::PushMessage(Byte ByteMessagein[], int iSize)
{
    vMessage.clear();
    for(int i=0; i<iSize; i++)
        vMessage.push_back(*(ByteMessagein+i));
};
//==============================================================================
void GPIBInterface_Data::PushMessage(vector <Byte> vMessagein, int iSize)
{
    vMessage.clear();
    for(int i=0; i<iSize; i++)
        vMessage.push_back(vMessagein[i]);
}
//==============================================================================
void GPIBInterface_Data::SendStructMessage_SendMessage()
{
    if(bFindProgram()==false)
        return ;

    pMV                 =new M_V;
    pcp                 =new COPYDATASTRUCT;
    pMV->bModeType      =bModeType;
    pMV->bCommandType   =bCommandType;
    pMV->bCommand       =bCommand;
    pMV->iDataSize      =vData.size();
    pMV->iMessageSize   =vMessage.size();

    for(unsigned int i=0; i<vData.size(); i++)
    {
        pMV->bData[i]=vData[i];
    }
    for(unsigned int i=0; i<vMessage.size(); i++)
    {
        pMV->bMessage[i]=vMessage[i];
    }
    pcp->dwData=_WPARAM_TO;
    pcp->cbData=sizeof(M_V);
    pcp->lpData=(M_V*)pMV;

    SendMessage(_HVisionWnd, WM_COPYDATA, _WPARAM_TO, (LPARAM)pcp);
    delete pMV;
    delete pcp;
};
//==============================================================================
void GPIBInterface_Data::Clear()
{
    bModeType     = 0x00;
    bCommandType  = 0x00;
    bCommand      = 0x00;
    vData.clear();
    vMessage.clear();
};
//==============================================================================
//1.送出Close GPIB Program
void _CloseGpibProgram()
{
    //結構
    GPIBSend->Clear();
    GPIBSend->bModeType    = TYPE_HANDLER_GPIB;
    GPIBSend->bCommandType = CommandType_CLOSE;
    GPIBSend->SendStructMessage_SendMessage();
}
//==============================================================================
//2.Send START RUN GPIB InterFace Program
bool _RunTestProgram_BarMess(int iDataSize, bool *bSiteOpen, int iMessageSize, Byte *bMessage)
{
    //全域變數
    bEcho  = false;
    bExist = false;
    bUnderTest=true;
    //結構
    GPIBSend->Clear();
    GPIBSend->bModeType    = TYPE_HANDLER_GPIB;
    GPIBSend->bCommandType = CommandType_BARCODE;

    if(LastSet.iTester==ON_LINE)
        GPIBSend->bCommand |= CommandType_TEST_REAL;                            //Steven 20150713 : 整理LastSet.iTester

    if(TestISTimeOut)
        GPIBSend->bCommand |= CommandType_TEST_TIMEOUT;

    GPIBSend->PushData(bSiteOpen, iDataSize);
    GPIBSend->PushMessage(bMessage, iMessageSize);
    GPIBSend->SendStructMessage_SendMessage();

    return true;
}
//==============================================================================
bool _RunTestProgram(int iDataSize, bool *bSiteOpen)
{
    //全域變數
    bEcho  = false;
    bExist = false;
    bUnderTest=true;
    //iSendGpibTestHome=true;                                                   //kevin 20150626 送出測試訊號 等收到資料才能歸home
    //結構
    GPIBSend->Clear();
    GPIBSend->bModeType    = TYPE_HANDLER_GPIB;
    GPIBSend->bCommandType = CommandType_TEST;

    if(LastSet.iTester==ON_LINE)
        GPIBSend->bCommand |= CommandType_TEST_REAL;                            //Steven 20150713 : 整理LastSet.iTester
//    if(bDoubleContact&LastSet.bGPIBSupportDoubleContact0x42Command)
//        GPIBSend->bCommand |= CommandType_TEST_DOUBLE;
    if(TestISTimeOut)
        GPIBSend->bCommand |= CommandType_TEST_TIMEOUT;

    GPIBSend->PushData(bSiteOpen, iDataSize);

    GPIBSend->SendStructMessage_SendMessage();

    return true;
}
//==============================================================================
//3.Send GPIB ADDR To InterFace Program
void _SendADDRToInterfaceProgram(int iADDR)
{
    GPIBSend->Clear();
    GPIBSend->bModeType    =TYPE_HANDLER_GPIB;
    GPIBSend->bCommandType =CommandType_INFSEND;
    GPIBSend->bCommand     =CommandType_INFSEND_SENDGPIBADDR;
    GPIBSend->PushData(&iADDR, 1);
    GPIBSend->SendStructMessage_SendMessage();
}
//==============================================================================
//4.Send Information To InterFace Program
void _SendInformationToInterfaceProgram(Byte _BModeType, Byte _bCommandType, Byte _bCommand, int _iData[], int _iDataSize, int _iMessage[], int _iMessageSize)
{
    (void)_iMessage; (void)_iMessageSize;                                       // golden also ignores these two params (never used in the body)
    GPIBSend->bModeType    = _BModeType;
    GPIBSend->bCommandType = _bCommandType;
    GPIBSend->bCommand     = _bCommand ;
    GPIBSend->PushData(_iData,_iDataSize);
    GPIBSend->SendStructMessage_SendMessage();
}
//==============================================================================
//1.Command_TEST 相關//---------------------------------------------------------
//2.Command_ECHOBACK 相關//-----------------------------------------------------
bool _FeedBackHandlerVariable(M_V &pVM)
{
    if(pVM.bCommand==CommandType_ECHOBACK_BACKGPIBADDR)
    {
        // AI(W5-comms-Integrate) 20260710: fMain->oldGpibAddress (golden main.h:1220)
        // is now a real member on the TfMain facade (FormsFacade.h) -- gate lifted.
        fMain->oldGpibAddress=pVM.bData[0];
    }

    return false;
}
//==============================================================================
bool _GPIBGetCommand_TEST(M_V &pVM)
{
    char cBuffer[1024];
    AnsiString asBuffer ="";
    AnsiString asReMess[iGPIBMAXItem] ={""};
    int k=0;

    if(pVM.bCommand & CommandType_TESTECHOOK)
    {
        int iCount=0;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(TestIF.iSiteMap[i][j]<=0)                                    //Hung 20111108 add
                {
                    iBin[i][j]=0;
                }
                else
                {
                    if((TestIF.iSiteMap[i][j]-1)<int(pVM.iDataSize))
                    {
                        iBin[i][j]=pVM.bData[TestIF.iSiteMap[i][j]-1];
                    }
                    else
                    {
                        iBin[i][j]=0;
                    }
                    iCount++;
                }
            }
        }

        memset(cBuffer,'\0', sizeof(cBuffer));
        for(int i=0; i<int(pVM.iMessageSize); i++)
            *(cBuffer+i)=(char) *(pVM.bMessage+i);
        asBuffer=cBuffer;

        if(asBuffer.Pos("BARCODEERR:")==1)
        {
            asBuffer.Delete(1, 11);

            if(asBuffer.Pos(";") !=0) asBuffer=asBuffer.SubString(1, asBuffer.Pos(";")-1);
            if(asBuffer.Pos("\r")!=0) asBuffer=asBuffer.SubString(1, asBuffer.Pos("\r")-1);

            for(int j=0; j<asBuffer.Length(); j++)
            {
                if(asBuffer[j+1]!=',')
                {
                    asReMess[k]=asReMess[k]+asBuffer[j+1];
                }
                else
                {
                    k++;
                }
            }

            if(k==iGPIBMAXItem-1)
            {
                // AI(W5-comms-Integrate) 20260710: fBarCode->iBinReturnMess[32] /
                // bGPIBTestBarCodeError (golden BarCode/BarCode.h:782/783) are now real
                // members on TfBarCode (aHotPlateSubstrate.h) -- gate lifted.
                for(int i=0; i<iGPIBMAXItem; i++)
                {
                    fBarCode->iBinReturnMess[iGPIBMAXItem-i-1]=asReMess[i];
                }
                fBarCode->bGPIBTestBarCodeError=true;
            }
            else
            {
                // AI(W5-comms-Integrate) 20260710: bGPIBTestBarCodeFormatError (golden
                // BarCode/BarCode.h:784) is now a real member on TfBarCode -- gate lifted.
                fBarCode->bGPIBTestBarCodeFormatError=true;
            }
        }
        else
        {
            // AI(W5-comms-Integrate) 20260710: bGPIBTestBarCodeError/
            // bGPIBTestBarCodeFormatError (golden BarCode/BarCode.h:782/784) are now real
            // members on TfBarCode -- gate lifted.
            fBarCode->bGPIBTestBarCodeError      =false;
            fBarCode->bGPIBTestBarCodeFormatError=false;
        }
        bGPIBError=false;
        bTimeOutForNoFullSite=false;
        bEcho     =true;
        bUnderTest=false;
        bExist    =true;
        return true;
    }
    else
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                if(TestIF.iSiteMap[i][j]<=0)                                    //Hung 20111108 add
                {
                    iBin[i][j]=0;
                }
                else
                {
                    if((TestIF.iSiteMap[i][j]-1)<int(pVM.iDataSize))
                        iBin[i][j]=pVM.bData[TestIF.iSiteMap[i][j]-1];
                    else
                        iBin[i][j]=0;
                }
            }
        }

        bTimeOutForNoFullSite=false;
        bGPIBError=true;
        bEcho     =true;
        bUnderTest=false;
        bExist    =true;
        return true;
    }
}
//==============================================================================
//0.Command 接收端
M_V *pVM = 0;
// AI(W5-InterfaceSYS-Translate) 20260710: signature adapted from golden
// `void _OnMyCopyMsg_Interface(TMessage &msg)` -- see InterfaceSYS.h note.
void _OnMyCopyMsg_Interface(LPARAM lParam)
{
    PCOPYDATASTRUCT P   =(PCOPYDATASTRUCT) lParam;
    // AI(W5-InterfaceSYS-Translate) 20260710: golden (InterfaceSYS.cpp:378) literally
    // reads:
    //     &pVM->bModeType     =(Byte *)P->lpData;
    // i.e. assigning INTO the result of `&pVM->bModeType` -- an address-of expression,
    // which is a prvalue and therefore never an assignable lvalue in standard C++ (nor,
    // most likely, under BCB6 either; this reads like a never-exercised source typo).
    // The evidently-intended effect -- confirmed by the tail of this function, which has
    // `//delete pVM;` commented OUT, i.e. pVM is meant to be a borrowed, NON-OWNING view
    // of the incoming COPYDATASTRUCT payload, never a fresh heap allocation -- is a plain
    // reinterpret-assignment of the M_V* itself:
    pVM = (M_V*)P->lpData;

    if(pVM->bModeType==TYPE_HANDLER_GPIB)
    {
        if(pVM->bCommandType      == CommandType_TEST)
        {
        }
        else if(pVM->bCommandType == CommandType_TESTECHO)
        {
            _GPIBGetCommand_TEST(*pVM);
        }
        else if(pVM->bCommandType == CommandType_CLOSE)
        {
            _CloseGpibProgram();
        }
        else if(pVM->bCommandType == CommandType_ECHOBACK)
        {
            _FeedBackHandlerVariable(*pVM);
        }
    }
    else
    {
    }
//    delete pVM;
}
//---------------------------------------------------------------------------
void _SendStructMessage_Send(Byte bType, Byte bSendCommandType, Byte bSendCommand, vector <Byte> vData, vector <Byte> vMessage)
{
    // AI(W5-comms-Integrate) 20260710: fMain->HESDWnd (golden main.h:1216, HWND) is
    // now a real member on the TfMain facade (FormsFacade.h) -- gate lifted.  Golden's
    // OWN logic here is "return early if no ESD program window was ever found", so an
    // offline machine (HESDWnd stays NULL -- nothing ever calls FindWindow to set it
    // in this Sim build) takes that exact early-return path -- behavior-neutral.
    if(fMain->HESDWnd==NULL)
    {
        return;
    }

    M_V            *pMV =new M_V;
    COPYDATASTRUCT *pcp =new COPYDATASTRUCT;
    pMV->bModeType      =bType;
    pMV->bCommandType   =bSendCommandType;
    pMV->bCommand       =bSendCommand;
    pMV->iDataSize      =vData.size();

    for(unsigned int i=0; i<vData.size(); i++)
    {
        pMV->bData[i]=vData[i];
    }
    pMV->iMessageSize=vMessage.size();

    for(unsigned int i=0; i<vMessage.size(); i++)
    {
        pMV->bMessage[i]=vMessage[i];
    }

    pcp->dwData=WM_ESD_Program;
    pcp->cbData=sizeof(M_V);
    pcp->lpData=(M_V*)pMV;

    int Lengh=0;
    if(bEPHandleToESD || bTempHandleToESD)                                      //kevin 20160106
    {
        if(bEPHandleToESD)
        {
           Lengh=bEPMessage.Length();
           bEPHandleToESD=false;
           memcpy(pMV->cSendData, bEPMessage.c_str(), Lengh);
        }

        if(bTempHandleToESD)
        {
          Lengh= bTempMessage.Length();
          bTempHandleToESD=false;
          memcpy(pMV->cSendData, bTempMessage.c_str(), Lengh);
        }
    }
    SendMessage(fMain->HESDWnd, WM_COPYDATA, WM_ESD_Program, (LPARAM)pcp);
    delete pcp;
    delete pMV;
}
//******************************************************************************
//
//  注意!! SendCommand_ESD為Handler與ESD通訊相關, 修改時要小心!!
//
//******************************************************************************
void SendCommand_ESD(ESD_COMMAND CMD)
{
    if(USE_NOVX3360 || USE_KASUGA || USE_KASUGA_Fan ||                          //KaiChen 20191225 ：KASUGA Fan 通訊  //kevin 20180821
       iUseHTIonBarFunction!=0)                                                 //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
    {
        Byte  bModeType;
        Byte  bCommandType;
        Byte  bCommand;
        vector <Byte>  vSendData;
        vSendData.clear();
        vector <Byte>  vSendMessage;
        vSendMessage.clear();
        //============================
        bModeType    = TYPE_HANDLER_ESD;
        bCommandType = (Byte)CommandType_ESD;
        bCommand     = (Byte)CMD;
        _SendStructMessage_Send(bModeType, bCommandType, bCommand, vSendData, vSendMessage);
    }
}
//---------------------------------------------------------------------------
// AI(W906-ELA-P3) 20260927: installed by wb_serve (EventLogAnalysis/ElaService.cpp W906_ElaPost); NULL = no in-process analyzer
void (*W906_ElaPostHook)(int) = 0;
void SendCommand_EventLog(EventLog_COMMAND CMD, AnsiString Data)                //Steven 20231127 : 改用分析器
{
    if(CosFunction.bUseMDB==false)
    {
        if(W906_ElaPostHook)                //AI(W906-ELA-P3) 20260927: in-process ElaHub (golden OnMyCopyMsg only raises bEL_<CMD>, Analyzer.cpp:487-558); the WM_COPYDATA path below stays live for an external EventlogAnalyzer.exe
            W906_ElaPostHook((int)CMD);
        // AI(W5-comms-Integrate) 20260710: fMain->HEventLogWnd (golden main.h:1217,
        // HWND) is now a real member on the TfMain facade -- gate lifted.
        if(fMain->HEventLogWnd==NULL)
            fMain->HEventLogWnd=FindWindow("TfrmELA", "Event Log Analyzer");

        if(fMain->HEventLogWnd==NULL)
            return;

        M_V            *pMV =new M_V;
        COPYDATASTRUCT *pcp =new COPYDATASTRUCT;

        pMV->bModeType      =TYPE_HANDLER_EventLog;
        pMV->bCommandType   =(Byte)CommandType_EventLog;
        pMV->bCommand       =(Byte)CMD;
        pMV->iDataSize      =Data.Length();
        pMV->bData[0]       =atoi(Data.c_str());
        pMV->iMessageSize   =0;
        pcp->dwData=WM_EventAnalysis;
        pcp->cbData=sizeof(M_V);
        pcp->lpData=(M_V*)pMV;
        SendMessage(fMain->HEventLogWnd, WM_COPYDATA, WM_EventAnalysis, (LPARAM)pcp);
        delete pcp;
        delete pMV;
    }
}
//---------------------------------------------------------------------------
void SendCommand_AutoUpdate(AutoUpdate_COMMAND CMD, AnsiString Data)            //Steven 20150728 : Auto Update
{
    // AI(W5-comms-Integrate) 20260710: fMain->HAutoUpdateWnd (golden main.h:1218,
    // HWND) is now a real member on the TfMain facade -- gate lifted.
    fMain->HAutoUpdateWnd=FindWindow("TfAutoUpdate", "fAutoUpdate");
    if(fMain->HAutoUpdateWnd==NULL)
        return;

    M_V            *pMV =new M_V;
    COPYDATASTRUCT *pcp =new COPYDATASTRUCT ;

    pMV->bModeType      =TYPE_HANDLER_AutoUpdate;
    pMV->bCommandType   =(Byte)CommandType_AutoUpdate;
    pMV->bCommand       =(Byte)CMD;
    pMV->iDataSize      =Data.Length();
    pMV->bData[0]       =atoi(Data.c_str());
    pMV->iMessageSize   =0;

    pcp->dwData=WM_AutoUpdate;
    pcp->cbData=sizeof(M_V);
    pcp->lpData=(M_V*)pMV;
    SendMessage(fMain->HAutoUpdateWnd, WM_COPYDATA, WM_AutoUpdate, (LPARAM)pcp);
    delete pcp;
    delete pMV;
}
//---------------------------------------------------------------------------
