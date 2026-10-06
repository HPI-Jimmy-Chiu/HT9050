#include "MachineDefine.h"
#pragma hdrstop

#include "MyBinDisp.h"


#include "mymessbox.h"
#include "cmydef.h"
#include "cprod.h"
#include "TextProcess.h"                                                        //Sam 20240604 : 新增 BinDisplay TFT
#include "database.h"            //Eastsun 20260513 KYEC: HSys.BinDisCtrl

#define Bin_MAX_RETRY 2
#define Bin_MAX_WAIT  3
#define Bin_STX       2
#define Bin_ETX       3
#define Bin_CR        13
#define Bin_LF        10
//------------------------------------------------------------------------------
#pragma link "SPComm"
#pragma resource "*.dfm"
TDataModule3 *DataModule3;
//------------------------------------------------------------------------------
TQPF_Timer tMagTimer;
bool bFisrtSend=false;
bool bFisrtBinSend=false;
char cSendCommandBuf[320];
char cSendBinCommandBuf[320];
AnsiString anSendCommandBuf;
AnsiString anSendBinCommandBuf;
int iSendCount=0;
int iSendBinCount=0;

bool  bFlash[MAX_BIN_UNIT];                                                     //Eastsun 20260513 : 閃爍功能
int   iFlashColorA[MAX_BIN_UNIT];                                               //Eastsun 20260513 : 閃爍功能
int   iFlashColorB[MAX_BIN_UNIT];                                               //Eastsun 20260513 : 閃爍功能
int   iFlashPeriodMs[MAX_BIN_UNIT];                                             //Eastsun 20260513 : 閃爍功能
bool  bFlashShowA[MAX_BIN_UNIT];                                                //Eastsun 20260513 : 閃爍功能
bool  bFlashCom;
int iOldTimerInterval;                                                          //Eastsun 20260513 : 閃爍功能

__fastcall TDataModule3::TDataModule3(TComponent* AOwner)
    : TDataModule(AOwner)
{
};
//------------------------------------------------------------------------------
// 建構子
//------------------------------------------------------------------------------
TMyBinDispCtrl::TMyBinDispCtrl()
{
    for(int i=0; i<MAX_BIN_UNIT; i++)
    {
        bHasUnitArray[i]=false;
        bSetBin[i]=false;
        for(int j=0; j<TEST_MAX_BIN; j++)                                       //kevin 20140326     MAX_BIN_UNIT //for(j=0; j<MAX_BIN_UNIT; j++)
            iSetBin[i][j]=-1;
        iSetColor[i]=1;
        Alias[i]="";
        bSliding[i]=true;
        bSliding_Cnt[i]=true;
        iVersion[i]=0;
        iBinNow[i]=0;                                                           // 顯示器目前的Bin
        iColorNow[i]=1;                                                         // 顯示器目前的的Color
        bHasError[i]=false;                                                     // 顯示器是否有出錯
        iSetCount[i]=0;                                                         //Sam 20240604 : 新增 BinDisplay TFT

        bFlash[i]         = false;                                              //==> Eastsun 20260513 KYEC: AutoChanging flash array init (v1.0)
        iFlashColorA[i]   = 1;
        iFlashColorB[i]   = 3;
        iFlashPeriodMs[i] = 600;
        bFlashShowA[i]    = true;
    }

    slBinDispLog=new TMyStringList("D:\\HT9045_Log\\BinDisplayLog",             //Steven 20210910 : Bin顯示器的log
                                   "BinDisplayLog",
                                   "Date, Time, Action, Message");
    iSetBin[0][0]=111;                                                          //Loader
    iSetBin[1][0]=104;                                                          //Empty Tray
    iSetBin[2][0]=102;                                                          //Color Tray
    iSetColor[0]=3;                                                             //橘色
    iSetColor[1]=3;
    iSetColor[2]=3;

    bStopProcess=false;
    bStartSetColor=false;
    bStartSetBin=false;
    bStartOnce=false;                                                           //Sam 20240604 : 新增 BinDisplay TFT
    bStartCycle=false;
    bHasUnit=false;
    BinDispRecv=false;
    ComPort=4;
    ComParity=None;

    dDelaySec=5.0;                                                              //Sam 20240604 : 顯示器輪巡時間改為 double
    InitialOK=false;

    Timer1=new TTimer(NULL);
    Timer1->OnTimer=Timer1Timer;
    Timer1->Interval=200;                                                       //kevin 20140327  30 --> 200
    iOldTimerInterval=Timer1->Interval;                                       //Eastsun 20260514
    Timer1->Enabled=true;
    iTotalInstalledUnit=-1;
    iBinDispCtrlTask=1;
    bFirstInit=true;                                                            //Steven 20110407
    iRusStatus=eBDP_Initial;

    sUnitName[eBinDispLoader]="Loader";                                         //Sam 20240604 : 新增 BinDisplay TFT
    sUnitName[eBinDispEmpty]="Empty";
    sUnitName[eBinDispColor]="Color";
    sUnitName[eBinDispAuto1]="Auto1";
    sUnitName[eBinDispAuto2]="Auto2";
    sUnitName[eBinDispAuto3]="Auto3";
    sUnitName[eBinDispFix1]="Fix1";
    sUnitName[eBinDispFix2]="Fix2";
    sUnitName[eBinDispFix3]="Fix3";
    sUnitName[eBinDispFix4]="Fix4";
    sUnitName[eBinDispFix5]="Fix5";
    sUnitName[eBinDispFix6]="Fix6";
    sUnitName[eBinDispBulkBox]="Box";
    sUnitName[eBinDispMag1]="Magazine01";
    sUnitName[eBinDispMag2]="Magazine02";
    sUnitName[eBinDispMag3]="Magazine03";
    sUnitName[eBinDispMag4]="Magazine04";
    sUnitName[eBinDispMag5]="Magazine05";
    sUnitName[eBinDispMag6]="Magazine06";
    sUnitName[eBinDispMag7]="Magazine07";
    sUnitName[eBinDispMag8]="Magazine08";
    sUnitName[eBinDispMag9]="Magazine09";
    sUnitName[eBinDispMag10]="Magazine10";
    sUnitName[eBinDispMag11]="Magazine11";
    sUnitName[eBinDispMag12]="Magazine12";
    sUnitName[eBinDispMag13]="Magazine13";
    sUnitName[eBinDispMag14]="Magazine14";
};
//------------------------------------------------------------------------------
// 解構子
//------------------------------------------------------------------------------
TMyBinDispCtrl::~TMyBinDispCtrl()
{
    try
    {
        Timer1->Enabled=false;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TMyBinDispCtrl");
    }
}
//------------------------------------------------------------------------------
void  TMyBinDispCtrl::SetComParity(TParity Parity)  {ComParity=Parity;}                 // 設定顯示器群組是利用那一個Parity
bool  TMyBinDispCtrl::UnitHasInstall(int Index)     {return bHasUnitArray[Index];}      // 確認該顯示器使是否有啟用
void  TMyBinDispCtrl::CloseUnit(int Index)          {bHasUnitArray[Index]=false;}       // 關閉該顯示器
void  TMyBinDispCtrl::OpenUnit(int Index)           {bHasUnitArray[Index]=true;}        // 開啟該顯示器
void  TMyBinDispCtrl::SetDelayTime(double Sec)      {dDelaySec=Sec;}                    // 設定多久輪替一次 //Sam 20240604 : 顯示器輪巡時間改為 double
double   TMyBinDispCtrl::GetDelayTime()             {return dDelaySec;}                 // 設定多久輪替一次
int   TMyBinDispCtrl::GetTotalInstalledUnit()       {return iTotalInstalledUnit+1;}     // 取得總共安裝了幾個顯示器
int   TMyBinDispCtrl::GetColorNow(int Index)        {return iColorNow[Index];};         // 顯示器目前的顏色
int   TMyBinDispCtrl::GetBinNow(int Index)          {return iBinNow[Index];};           // 顯示器目前的字元
bool  TMyBinDispCtrl::GerErrNow(int Index)          {return bHasError[Index];};         // 顯示器是否有出錯
//---------------------------------------------------------------------------
AnsiString TMyBinDispCtrl::GetRunStatus()                                       // 回傳目前在執行的動作
{
    AnsiString Message="";
    switch(iRusStatus)
    {
        case eBDP_Initial:      Message="Initialing...";    break;
        case eBDP_GetStatus:    Message="Get status...";    break;
        case eBDP_ColorSet:     Message="Color Setting.";   break;
        case eBDP_BinSet:       Message="Bin Setting.";     break;
        case eBDP_BinRun:       Message="Bin Running.";     break;
        case eBDP_DispErr:      Message="Display Error!!";  break;
    }

    return Message;
}
//---------------------------------------------------------------------------
void  TMyBinDispCtrl::SetComPort(AnsiString port)                               // 設定顯示器群組是利用那一個COM PORT
{
    ComPort=port;
}
//---------------------------------------------------------------------------
void  TMyBinDispCtrl::SetComPort2(AnsiString port)                              // 設定顯示器群組是利用那一個COM PORT
{
    ComPort2=port;
}
//---------------------------------------------------------------------------
void TMyBinDispCtrl::ProcessStopStart(bool Value)
{
    bStopProcess=Value;
    if(bFirstInit==true)
    {
        InitialTask();                                                          //Sam 20240604 : 新增 BinDisplay TFT
        bFirstInit=false;
    }
    else
    {
        if(bStopProcess==true)                                                  //Steven 20110216
            iBinDispCtrlTask=50;
    }
}
//------------------------------------------------------------------------------
// COM PORT 接收資料function
//------------------------------------------------------------------------------
void __fastcall TMyBinDispCtrl::CommBinReceiveData(TObject *Sender,
      Pointer Buffer, WORD BufferLength)
{
    if(BufferLength>=1024)                                                      //Steven 20141110 : 避免記憶體破壞
        return;
    AnsiString asLog22="";

    ZeroMemory(BinDispCom2Buffer, sizeof(BinDispCom2Buffer));
    if(NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {
        sReadBuffer="";
        memcpy(BinDispCom2Buffer, Buffer, BufferLength);
        BinDispCom2Buffer[BufferLength]='\x0';
        for(int i=0; i<BufferLength; i++)
        {
            asLog22.sprintf("%02X", BinDispCom2Buffer[i]);
            sReadBuffer+= asLog22;
        }

        if(MAGAZINE_BIN_DISP_TYPE==eTFT)
        {
            sReadBufferMag="";
            for(int i=0; i<BufferLength; i++)
            {
                asLog22.sprintf("%02X", BinDispCom2Buffer[i]);
                sReadBufferMag+= asLog22;
            }
        }
    }
    else
    {
        strncpy(BinDispCom2Buffer, (char*)Buffer, BufferLength);
        BinDispCom2Buffer[BufferLength]='\x0';

        sReadBuffer.sprintf("%s", AnsiString(BinDispCom2Buffer));
        if(MAGAZINE_BIN_DISP_TYPE==eTFT)
        {
            sReadBufferMag="";
            for(int i=0; i<BufferLength; i++)
            {
                asLog22.sprintf("%02X", BinDispCom2Buffer[i]);
                sReadBufferMag+= asLog22;
            }
        }
    }
    if(sReadBuffer=="")                                                         //kevin 20170710 (wei) add bin display
        return;

    ShowCommLog(BinDispCom2Buffer, -1, "");                                     //Sam 20240604 : 新增 BinDisplay TFT
    BinDispRecv=true;
};
//------------------------------------------------------------------------------
void __fastcall TMyBinDispCtrl::CommBinReceiveData2(TObject *Sender,
      Pointer Buffer, WORD BufferLength)
{
    if(BufferLength>=1024)                                                      //Steven 20141110 : 避免記憶體破壞
        return;

    ZeroMemory(BinDispCom2Buffer2, sizeof(BinDispCom2Buffer2));
    strncpy(BinDispCom2Buffer2, (char*)Buffer, BufferLength);
    BinDispCom2Buffer2[BufferLength]='\x0';

    sReadBuffer2.sprintf("%s", AnsiString(BinDispCom2Buffer2));
    if(sReadBuffer2=="")                                                        //kevin 20170710 (wei) add bin display
        return;

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(AnsiString("Recv, ")+sReadBuffer2);
    BinDispRecv2=true;
};
//------------------------------------------------------------------------------
// 設定address=Index 是有存在的
//------------------------------------------------------------------------------
void TMyBinDispCtrl::InstalledUnit(int Index)
{
    bHasUnitArray[Index]=true;
    bHasUnit=true;
    bSetBin[Index]=true;
    if(iTotalInstalledUnit<Index)
        iTotalInstalledUnit=Index;
};
//------------------------------------------------------------------------------
// 顯示器控制中心
//------------------------------------------------------------------------------
// IsAnyFlashing: any BinDisp currently flashing?
//---------------------------------------------------------------------------
bool IsAnyFlashing(void)
{
    for(int k=0; k<MAX_BIN_UNIT; k++)
        if(bFlash[k])
            return true;
    return false;
}
//---------------------------------------------------------------------------
void __fastcall TMyBinDispCtrl::Timer1Timer(TObject *Sender)
{
    static bool bRun=false;
    {
        AnsiString Str="";

        if(InitialOK==false)                                                    //jou 2010-05-19 start : 未initital完成,不能執行
            return;

        if(NUMBER_PANEL_TYPE==3 ||
           NUMBER_PANEL_TYPE==4)                                                //Sam 20240604 : 新增 BinDisplay TFT
        {

        }
        else
        {
            return;
        }

        if(bStopProcess==false)
            return;

        if(bRun)
            return;

        bRun=true;

        int &Task=iBinDispCtrlTask;

        AnsiString CN;

        if(IsAnyFlashing() && IniConfig.bP53AutoChangingFlashWarn)              //Eastsun 20260514
        {
            Timer1->Interval=50;
        }
        else
        {
            Timer1->Interval=iOldTimerInterval;
        }

        switch(Task)
        {
            case 1:
                iRusStatus=eBDP_Initial;                                        //Sam 20240604 : 新增 BinDisplay TFT
                if(ComPort.Pos("COM")==0)
                {
                    bRun=false;
                    return;
                }
                CommBin=DataModule3->BinDisp;
                CommBin->OnReceiveData=CommBinReceiveData;
                CommBin->Parity=ComParity;

                CommBin2=DataModule3->BinDisp2;
                CommBin2->OnReceiveData=CommBinReceiveData2;
                CommBin2->Parity=ComParity;

                bStartSetBin=true;                                              //重設Bin
                bStartSetColor=true;                                            //重設顏色
                bStartOnce=true;                                                //開始進行設定TFT顯示器設定    //Sam 20240604 : 新增 BinDisplay TFT
                bStartCycle=true;                                               //開始進行 TFT 顯示器
                if(NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
                {
                    Timer1->Interval=30;
                    CommBin->ReadIntervalTimeout=50;
                    CommBin->Inx_XonXoffFlow=false;
                    CommBin->Outx_XonXoffFlow=false;
                }
                for(int i=0; i<MAX_BIN_UNIT; i++)
                {
                    bHasError[i]=false;
                }
                #ifndef SOFT_SIMULTE

                if(GetCOMPortStatus(ComPort))
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort;
                        CommBin->CommName=CN;
                        CommBin->StartComm();
                        Str.sprintf("BinDisp, Start Comm OK., %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error open com port");
                        Str.sprintf("BinDisp, Start Comm NG!, %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    Task=50;
                    iStartGetStatusTask=1;
                }
                else
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort;
                        CommBin->CommName=CN;
                        CommBin->StopComm();
                        Str.sprintf("BinDisp, Stop Comm OK., %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error close com port");
                        Str.sprintf("BinDisp, Stop Comm NG!, %s", CommBin->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                }

                if(GetCOMPortStatus(ComPort2))
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort2;
                        CommBin2->CommName=CN;
                        CommBin2->StartComm();
                        Str.sprintf("BinDisp, Start Comm OK., %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error open com port");
                        Str.sprintf("BinDisp, Start Comm NG!, %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    Task=50;
                    iStartGetStatusTask=1;
                }
                else
                {
                    try
                    {
                        CN="\\\\.\\"+ComPort2;
                        CommBin2->CommName=CN;
                        CommBin2->StopComm();
                        Str.sprintf("BinDisp, Stop Comm OK., %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                    catch(...)
                    {
                        MyDBIProcess("Exception", "TMyBinDispCtrl::Timer1Timer");
                        ShowMyMessage("Error close com port");
                        Str.sprintf("BinDisp, Stop Comm NG!, %s", CommBin2->CommName);
                        slBinDispLog->AddTextWithDateTime(Str);
                    }
                }

                #else
                    bStartSetColor=false;
                    Task=50;
                    iStartGetStatusTask=1;
                #endif
                break;
            case 50:
                if(DoStartGetStatus())
                {
                    Task=100;
                }
                break;
            case 100:
                if(bHasUnit==false)
                    break;
                #ifndef SOFT_SIMULTE
                if(GetCOMPortStatus(ComPort))                                   //假設RS232斷線
                {
                    iRusStatus=eBDP_Initial;                                    //Sam 20240604 : 新增 BinDisplay TFT
                    CommBin->StopComm();
                    CommBin2->StopComm();
                    for(int i=0; i<MAX_BIN_UNIT; i++)
                    {
                        iBinNow[i]=0;                                           // 顯示器目前的Bin
                        iColorNow[i]=1;                                         // 顯示器目前的的Color
                    }

                    InitialTask();                                              //Sam 20240604 : 新增 BinDisplay TFT
                    break;
                }

                if(GetCOMPortStatus(ComPort2))                                  //假設RS232斷線
                {
                    iRusStatus=0;
                    CommBin2->StopComm();

                    for(int i=0; i<MAX_BIN_UNIT; i++)
                    {
                        iBinNow[i]=0;                                           // 顯示器目前的Bin
                        iColorNow[i]=1;                                         // 顯示器目前的的Color
                    }

                    bStartSetColor=true;                                        //重設顏色
                    bStartSetBin=true;                                          //重設Bin
                    Task=1;
                    break;
                }

                if(IsAnyFlashing())                                             //Eastsun 20260513 : 閃爍功能
                {
                    bStartSetColor = true;
                }
                #endif

                if(NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
                {
                    if(bStartOnce)
                    {
                        iOnceTask=1;
                        Task=400;
                    }
                    else if(bStartCycle)
                    {
                        iCycleTask=1;
                        Task=500;
                    }
                    else if(iCycleTask==50 && BinDisCycleDelay.Off())           //Eastsun 20260929 : rotation time up, continue DoCycle to show next bin
                    {
                        Task=500;
                    }
                }
                else
                {
                     if(bStartSetColor)
                     {
                         iStartSetColorTask=1;
                         Task=200;
                     }
                     else if(bStartSetBin)
                     {
                         iStartSetBinTask=1;
                         Task=300;
                     }
                     else if(bStartSetBin==false)
                     {
                         iStartSetBinTask=100;
                         Task=300;
                     }
                }
                break;
            case 200:
                if(DoStartSetColor())
                {
                    bFlashCom=true;
                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                    {
                        iStartSetBinTask=1;
                        Task=300;
                    }
                    else
                    {
                        bStartSetColor=false;
                        Task=100;
                    }
                }
                break;
            case 300:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                }
                if(DoStartSetBin())
                {
                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                        Task=600;
                    else
                    Task=100;
                }
                break;
            case 400:                                                           //NUMBER_PANEL_TYPE==4
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                }
                if(DoOnce())
                {
                    bStartOnce=false;

                    if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT)
                    {
                        Task=200;
                        iStartSetColorTask=1;
                    }
                    else
                    {
                    Task=100;
                }
                }
                break;
            case 500:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                }
                if(DoCycle())
                {
                    bStartCycle=false;
                    Task=100;
                }
                break;
            case 600:
                if(IsAnyFlashing() && !bStartSetBin)                            //Eastsun 20260513 : 閃爍功能
                {
                    Task=100;
                }
                if(DoCycle())
                {
                    bStartCycle=false;
                    Addr=0;
                    iStartSetBinTask=100;
                    Task=300;
                }
                break;

        }
    }
    bRun=false;
//    #endif
}
//------------------------------------------------------------------------------
void TMyBinDispCtrl::WriteTargetBin(int Index, int *bin, int color)
{
    if(Index>MAX_BIN_UNIT)                                                      //jou 2011-08-15 start : Addr破壞記憶體
        return;
    if(Index<0)
        return;

    //bin = -1      : 不顯示，所以填入123
    //bin = 0~99    : 顯示數字0~99
    //bin = 100~125 : 顯示字母A~Z

    // bin輸入值對照表
    //  A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z
    //  100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125

    iSetColor[Index]=color;
    for(int i=0; i<TEST_MAX_BIN; i++)                                           //kevin 20140326
        iSetBin[Index][i]=bin[i];

    if(CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2 && Index==eBinDispAuto1)
        iSetBin[Index][0]=104;

    if(CUSTOMER_CODE==CC_MAXIM_THAILAND &&
       TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                           //Ifor 20260225 add:客戶要求功能有開啟才顯示"R"
        iSetBin[1][0]=117;

    iSetBin[0][1]=-1;                                                           //Loader
    bStartSetBin=true;
    bStartSetColor=true;
    bStartOnce=true;                                                            //Sam 20240604 : 新增 BinDisplay TFT
    bStartCycle=true;
    bSetBin[Index]=true;

    if(NUMBER_PANEL_TYPE==4)                                                    //AI 20260707 (RogerYang) : TFT(type4)無legacy回讀,主動同步現值供ChangeBinDispStatus,否則面板恆紅+0
    {
        iColorNow[Index]=iSetColor[Index];                                      //對應 GetColorNow → 面板顏色
        iBinNow[Index]  =iSetBin[Index][0];                                     //對應 GetBinNow  → 面板數字/L/E/C/X
    }
}
//---------------------------------------------------------------------------
void TMyBinDispCtrl::WriteTargetCount(int Index, int iCount)                    //Sam 20240604 : 新增 BinDisplay TFT
{
    if(Index>=0 && Index<MAX_BIN_UNIT)
        iSetCount[Index]=iCount;
    else
        ShowMyMessage("WriteTargetCount error");
}
//---------------------------------------------------------------------------
//Addr: 0~F
//Comm:
//0 Read Status     : 2,X,0,3,0,0,8,0,0,0,0,1   XX,XX   \R\N
//1 Write Num       : 2,X,0,6,0,0,8,0,0,0,X,X   XX,XX   \R\N    0~99
//2 Write Eng       : 2,X,0,6,0,0,8,1,0,0,X,X   XX,XX   \R\N    A~Z = 0~26
//3 R LIGHT         : 2,X,0,6,0,0,8,2,0,0,0,1   XX,XX   \R\N
//4 G LIGHT         : 2,X,0,6,0,0,8,2,0,0,0,2   XX,XX   \R\N
//5 R+G LIGHT       : 2,X,0,6,0,0,8,2,0,0,0,3   XX,XX   \R\N
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteBin(int Addr, int Command, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X06008%d00%02d00%c%c", Addr+38, Command, Value, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X06008%d00%02d00%c%c", Addr+32, Command, Value, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X06008%d00%02d00%s%s, WriteBin", Addr+38, Command, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X06008%d00%02d00%s%s, WriteBin", Addr+32, Command, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
};
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteBin2(int Addr, int Command, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X06008%d00%02d00%c%c", Addr+38, Command, Value, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X06008%d00%02d00%c%c", Addr+32, Command, Value, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin2->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X06008%d00%02d00%s%s, WriteBin", Addr+38, Command, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X06008%d00%02d00%s%s, WriteBin", Addr+32, Command, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
};
//------------------------------------------------------------------------------
void TMyBinDispCtrl::ShowCommLog(unsigned char ucLog[20], int Address, AnsiString sFun) //Sam 20240604 : 新增 BinDisplay TFT
{
    AnsiString sLog="",asLog="", asLog22="";

    for(int i=0; i<20; i++)
    {
        asLog22.sprintf("[%02X]", ucLog[i]);                                    //記錄16進制
        asLog=asLog+" "+asLog22;
    }

    if(Address==-1)                                                             //Recv
        sLog.sprintf("Recv, %s", asLog);
    else if(Address>=0 && Address<eBinDispTotal)
        sLog.sprintf("Send, %s, %s %s", asLog, sUnitName[Address], sFun);
    else
        sLog.sprintf("Send, %s, %s %s", asLog, "Undefined", sFun);

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(sLog);
}
//------------------------------------------------------------------------------
int iAddArrayTFT[]={0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                    0x28, 0x29, 0x2a, 0x2b, 0x00, 0x01, 0x02, 0x03,
                    0x04, 0x05 ,0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                    0x0c, 0x0d, 0x0e};
 //------------------------------------------------------------------------------
void TMyBinDispCtrl::command_TFT_Input(char *cStr, int index, int iDisplabel, AnsiString sValue)    //Sam 20240604 : 新增 BinDisplay TFT
{
    int iHeader         =0x3a,
        iByteCount   [2]={0x00, 0x00},
        iFunctionCode[2]={0x00, 0x00},
        iDataItem    [2]={0x00, 0x01},
        iNumberOfData[9]={0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
        cLRC1           =0x00;

    iFunctionCode[1]=0x03;                                                      //文字輸入
    iByteCount[1]   =0x0d;                                                      //傳送的字元長度，資料顯示最多可以 30 Byte 資料，目前先卡上限 9 byte
    iDataItem[1]=iDisplabel;

    int iLen=0;
    if(sValue!="")
    {
        iLen=sValue.Length();
        if(iLen>9)
            iLen=9;

        for(int i=0; i<iLen ;i++)
            iNumberOfData[i]=MyASCIIToDec(sValue[i+1]);
    }
    else
    {
        iNumberOfData[0]=0x00;
    }

    sprintf(cStr,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader,
            iAddArrayTFT[index],
            iByteCount[0],
            iByteCount[1],
            iFunctionCode[0],
            iFunctionCode[1],
            iDataItem[0],
            iDataItem[1],
            iNumberOfData[0],
            iNumberOfData[1],
            iNumberOfData[2],
            iNumberOfData[3],
            iNumberOfData[4],
            iNumberOfData[5],
            iNumberOfData[6],
            iNumberOfData[7],
            iNumberOfData[8]);
    cLRC1=A_Create_LRC(&cStr[1],16);                                            //LRC 計算範圍 iAddArrayTFT ~ iNumberOfData[8] (不包含iHeader)
    cStr[17]=cLRC1;
    cStr[18]=Bin_CR;
    cStr[19]=Bin_LF;
}
//------------------------------------------------------------------------------
void TMyBinDispCtrl::command_TFT_Font(char *cStr, int index, int iDisplabel, Byte iXPos, Byte iYPos, Byte iWidth, Byte iHeigh, Byte iFontSize, Byte iFill, int iColor)  //Sam 20240604 : 新增 BinDisplay TFT
{
    int iHeader=0x3a,
        iFunctionCode[2]={0x00, 0x00},
        iByteCount   [2]={0x00, 0x00},
        iDataItem    [2]={0x00, 0x01},
        iNumberOfData[9]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00},
        cLRC1           =0x00;

    iFunctionCode[1]=0x02;                                                      //文字框設定
    iByteCount[1]   =0x0d;                                                      //傳送的字元長度(輸入的字型是設定的)
    iDataItem[1]    =iDisplabel;                                                //

    iNumberOfData[0]=iXPos;                                                     //位置 X
    iNumberOfData[1]=iYPos;                                                     //位置 Y
    iNumberOfData[2]=iWidth;                                                    //寬度
    iNumberOfData[3]=iHeigh;                                                    //高度

    if(iColor==1)                                                               //紅色
    {
        iNumberOfData[4]=0xff;                                                  //字型顏色 R
        iNumberOfData[5]=0x00;                                                  //字型顏色 G
        iNumberOfData[6]=0x00;                                                  //字型顏色 B
    }
    else if(iColor==2)                                                          //綠色
    {
        iNumberOfData[4]=0x22;
        iNumberOfData[5]=0x8b;
        iNumberOfData[6]=0x22;
    }
    else if(iColor==3)                                                          //橘色
    {
        iNumberOfData[4]=0xff;
        iNumberOfData[5]=0x80;
        iNumberOfData[6]=0x00;
    }
    else if(iColor==4)                                                          //背景是灰色的透明的 "EA" & Count
    {
        iNumberOfData[4]=0xff;
        iNumberOfData[5]=0xff;
        iNumberOfData[6]=0xf0;
    }
    else                                                                        //黑色
    {
        iNumberOfData[4]=0x00;
        iNumberOfData[5]=0x00;
        iNumberOfData[6]=0x00;
    }

    iNumberOfData[7]=iFontSize;                                                 //字型大小
    iNumberOfData[8]=iFill;                                                     //透明度

    sprintf(cStr, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader,
            iAddArrayTFT[index],
            iByteCount[0],
            iByteCount[1],
            iFunctionCode[0],
            iFunctionCode[1],
            iDataItem[0],
            iDataItem[1],
            iNumberOfData[0],
            iNumberOfData[1],
            iNumberOfData[2],
            iNumberOfData[3],
            iNumberOfData[4],
            iNumberOfData[5],
            iNumberOfData[6],
            iNumberOfData[7],
            iNumberOfData[8]);
    cLRC1=A_Create_LRC(&cStr[1], 16);                                           //LRC 計算範圍 iAddArrayTFT ~ iNumberOfData[8] (不包含iHeader)
    cStr[17]=cLRC1;
    cStr[18]=Bin_CR;
    cStr[19]=Bin_LF;
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::InitialTask()                                            //Sam 20240604 : 新增 BinDisplay TFT
{
    iOnceTask=1;
    iCycleTask=1;
    iBinDispCtrlTask=1;
    bStartOnce=true;
    bStartCycle=true;
    bStartSetColor=true;
    bStartSetBin=true;                                                          //重設Bin
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteBin_TFT(int index, int ivalue)                      //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    AnsiString sWrite="";
    if(ivalue==999 || ivalue==104)
    {
        if(index==1)
            sWrite="Empty";
        else
            sWrite="  E";
    }
    else if(ivalue==111)
    {
        sWrite="Loader";
    }
    else if(ivalue==102)
    {
        sWrite="Color";
    }
    else if(ivalue==-1)
    {
        sWrite="---";
    }
    else
    {
        sWrite.sprintf("%03d", ivalue);                                         //顯示 Bin 別卡3位數。
    }

    command_TFT_Input(cSendCommand, index, 0x01, sWrite);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Write Bin : "+sWrite);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteBinWord_TFT(int index,int ivalue)                   //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    AnsiString sWrite="";
    if(ivalue==999 || ivalue==104)
        sWrite="";
    else if(ivalue==111)
        sWrite="";
    else if(ivalue==102)
        sWrite="";
    else
        sWrite="Bin";

    command_TFT_Input(cSendCommand, index, 0x02, sWrite);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Write Bin Word : "+sWrite);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteEA_TFT(int index,int ivalue)                        //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    AnsiString sWrite="";
    if(ivalue==999 || ivalue==104)
        sWrite="";
    else if(ivalue==111)
        sWrite="";
    else if(ivalue==102)
        sWrite="";
    else
        sWrite="EA";

    command_TFT_Input(cSendCommand, index, 0x03, sWrite);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Write EA : "+sWrite);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteCount_TFT(int index,int ivalue, int iCount)         //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    AnsiString sWrite="";
    if(ivalue==999 || ivalue==104)
        sWrite="";
    else if(ivalue==111)
        sWrite="";
    else if(ivalue==102)
        sWrite="";
    else if(ivalue==-1)
        sWrite="";
    else
        sWrite.sprintf("%d", iCount);

    command_TFT_Input(cSendCommand, index, 0x04, sWrite);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Write Count : "+sWrite);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetFontBin_TFT(int index, int iColor, int iValue)        //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    unsigned char iXPos=0x00, iYPos=0x00, iWidth=0x00, iHeigh=0x00, iFontSize=0x00, iFill=0x00;
    if(iValue==999 || iValue==104)                                              //Empty
    {
        iXPos=0x10;
        iYPos=0x20;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x14;
        iFill=0xff;
    }
    else if(iValue==111)                                                        //Loader
    {
        iXPos=0x09;
        iYPos=0x20;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x14;
        iFill=0xff;
    }
    else if(iValue==102)                                                        //Color
    {
        iXPos=0x09;
        iYPos=0x20;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x14;
        iFill=0xff;
    }
    else
    {
        iXPos=0x15;
        iYPos=0x15;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x03;
        iFill=0xff;
    }
    command_TFT_Font(cSendCommand, index, 0x01, iXPos, iYPos, iWidth, iHeigh, iFontSize, iFill, iColor);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Set Bin Font");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetFontBinWord_TFT(int index, int iColor, int iValue)    //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    unsigned char iXPos=0x00, iYPos=0x00, iWidth=0x00, iHeigh=0x00, iFontSize=0x00, iFill=0x00;

    if(iValue==999 || iValue==104)                                              //Empty
    {
        //都不要填預設就不顯示
    }
    else if(iValue==111)                                                        //Loader
    {
        //都不要填預設就不顯示
    }
    else if(iValue==102)                                                        //Color
    {
        //都不要填預設就不顯示
    }
    else                                                                        //BIN
    {
        iXPos=0x09;
        iYPos=0x01;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x01;
        iFill=0xff;
        if(iColor==3)                                                           //若沒有使用到的 "Bin" 與 Bin 別顯示不要同步。 "Bin" 顯示黑色 "---" 顯示橘色
            iColor=-1;
    }
    command_TFT_Font(cSendCommand, index, 0x02, iXPos, iYPos, iWidth, iHeigh, iFontSize, iFill, iColor);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Set Bin Word Font");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetFontEA_TFT(int index, int iColor, int iValue)         //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    unsigned char iXPos=0x00, iYPos=0x00, iWidth=0x00, iHeigh=0x00, iFontSize=0x00, iFill=0x00;
    iColor=4;
    if(iValue==999 || iValue==104)                                              //Empty
    {
        //都不要填預設就不顯示
    }
    else if(iValue==111)                                                        //Loader
    {
        //都不要填預設就不顯示
    }
    else if(iValue==102)                                                        //Color
    {
         //都不要填預設就不顯示
    }
    else
    {
        iXPos=0x60;
        iYPos=0x58;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x01;
        iFill=0xff;
    }
    command_TFT_Font(cSendCommand, index, 0x03, iXPos, iYPos, iWidth, iHeigh, iFontSize, iFill, iColor);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Set EA Font");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetFontCount_TFT(int index, int iColor, int iValue)      //Sam 20240604 : 新增 BinDisplay TFT
{
    char cSendCommand[20]="";
    unsigned char iXPos=0x00, iYPos=0x00, iWidth=0x00, iHeigh=0x00, iFontSize=0x00, iFill=0x00;
    iColor=4;

    if(iValue==999 || iValue==104)                                              //Empty
    {
        //都不要填預設就不顯示
    }
    else if(iValue==111)                                                        //Loader
    {
        //都不要填預設就不顯示
    }
    else if(iValue==102)                                                        //Color
    {
        //都不要填預設就不顯示
    }
    else
    {
        iXPos=0x01;
        iYPos=0x58;
        iWidth=0x00;
        iHeigh=0x00;
        iFontSize=0x01;
        iFill=0xff;
    }
    command_TFT_Font(cSendCommand, index, 0x04, iXPos, iYPos, iWidth, iHeigh, iFontSize, iFill, iColor);
    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "Set Count Font");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetBackGround_TFT(int index)                             //Sam 20240604 : 新增 BinDisplay TFT
{
    int iHeader         =0x3a,
        iByteCount[2]   ={0x00, 0x0d},
        iFunctionCode[2]={0x00, 0x01},
        iDataItem[2]    ={0x00, 0x02},
        iNumberOfData[9]={0x30, 0x30, 0x00, 0x59, 0xaa, 0x30, 0x69, 0X69, 0x69},
        iDelimiter[2]   ={0x0d, 0x0a};
    AnsiString Str="", sSendCommand="";
    char cSendCommand[20]="";
    unsigned char Btmp1;

    if(index<3)
    {
        iDataItem[1]=0x00;
    }

    Btmp1=iAddArrayTFT[index]+iByteCount[0]+iByteCount[1]+iFunctionCode[0]+iFunctionCode[1]+iDataItem[0]+iDataItem[1]+
          iNumberOfData[0]+iNumberOfData[1]+iNumberOfData[2]+iNumberOfData[3]+iNumberOfData[4]+
          iNumberOfData[5]+iNumberOfData[6]+iNumberOfData[7]+iNumberOfData[8];
    Btmp1=~Btmp1;
    Btmp1+=1;

    sprintf(cSendCommand, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader, iAddArrayTFT[index], iByteCount[0], iByteCount[1], iFunctionCode[0],
            iFunctionCode[1], iDataItem[0], iDataItem[1], iNumberOfData[0], iNumberOfData[1],
            iNumberOfData[2], iNumberOfData[3], iNumberOfData[4], iNumberOfData[5], iNumberOfData[6],
            iNumberOfData[7], iNumberOfData[8], Btmp1, iDelimiter[0], iDelimiter[1]);

    CommBin->WriteCommData(cSendCommand, 20);

    ShowCommLog(cSendCommand, index, "Set BackGround");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::SetNoBackGround_TFT(int index)                           //Sam 20240604 : 新增 BinDisplay TFT
{
    int iHeader         =0x3a,
        iFunctionCode[2]={0x00, 0x04},
        iByteCount[2]   ={0x00, 0x0d},
        iDataItem[2]    ={0x00, 0x01},
        iNumberOfData[9]={0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x07, 0x00},
        iDelimiter[2]   ={0x0d, 0x0a};
    AnsiString Str;
    char cSendCommand[20]="";
    AnsiString sSendCommand="";
    unsigned char Btmp1;

    Btmp1=iAddArrayTFT[index]+iByteCount[0]+iByteCount[1]+iFunctionCode[0]+iFunctionCode[1]+iDataItem[0]+iDataItem[1]+
          iNumberOfData[0]+iNumberOfData[1]+iNumberOfData[2]+iNumberOfData[3]+iNumberOfData[4]+
          iNumberOfData[5]+iNumberOfData[6]+iNumberOfData[7]+iNumberOfData[8];
    Btmp1=~Btmp1;
    Btmp1+=1;

    sprintf(cSendCommand, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader, iAddArrayTFT[index], iByteCount[0], iByteCount[1], iFunctionCode[0],
            iFunctionCode[1], iDataItem[0], iDataItem[1], iNumberOfData[0], iNumberOfData[1],
            iNumberOfData[2], iNumberOfData[3], iNumberOfData[4], iNumberOfData[5], iNumberOfData[6],
            iNumberOfData[7], iNumberOfData[8], Btmp1, iDelimiter[0], iDelimiter[1]);
    CommBin->WriteCommData(cSendCommand, 20);

    ShowCommLog(cSendCommand, index, "Set NoBackGround");
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::MagazineWriteBin_HTA18(int index, int ivalue)            //JerryYang 20230515 : 二位數BIN DISP, HTA18
{
    int iHeader         =0x3a,
        iAddress[2]     ={0, 0},
        iFunctionCode[2]={0x30, 0x32},
        iDataItem[4]    ={0x30, 0x30, 0x38, 0x31},
        iNumberOfData[4]={0x30, 0x30, 0x30, 0x30},
        iErrorCheck     =0,
        iCRC_High       =0,
        iCRC_Low        =0,
        iCRC_Temp[6],
        iDelimiter[2]   ={0x0d, 0x0a};
    AnsiString Str;
    char casAddress[4]="", cCheckSumTemp[12]="", cSendCommand[24]="", cSetLimit[8]="";

    if(index>=100)
    {
        index-=100;
        iHeader=0x3b;
    }

    if(ivalue>=100)
    {
        ivalue=0;

        iFunctionCode[0]=0x30;
        iFunctionCode[1]=0x30;

        iDataItem[0]=0x30;
        iDataItem[1]=0x30;
        iDataItem[2]=0x38;
        iDataItem[3]=0x32;
    }

    sprintf(casAddress,"%02d",index);
    iAddress[0]=0x30+(casAddress[0]-0x30);
    iAddress[1]=0x30+(casAddress[1]-0x30);
    sprintf(cSetLimit,"%04d",ivalue*10);
    iNumberOfData[0]=cSetLimit[0];
    iNumberOfData[1]=cSetLimit[1];
    iNumberOfData[2]=cSetLimit[2];
    iNumberOfData[3]=cSetLimit[3];
    iCRC_Temp[0]=(iAddress[0]-0x30)*16+iAddress[1]-0x30;
    iCRC_Temp[1]=(iFunctionCode[0]-0x30)*16+iFunctionCode[1]-0x30;
    iCRC_Temp[2]=(iDataItem[0]-0x30)*16+iDataItem[1]-0x30;
    iCRC_Temp[3]=(iDataItem[2]-0x30)*16+iDataItem[3]-0x30;
    iCRC_Temp[4]=(iNumberOfData[0]-0x30)*16+iNumberOfData[1]-0x30;
    iCRC_Temp[5]=(iNumberOfData[2]-0x30)*16+iNumberOfData[3]-0x30;
    iErrorCheck=-iCRC_Temp[0]-iCRC_Temp[1]-iCRC_Temp[2]-iCRC_Temp[3]-iCRC_Temp[4]-iCRC_Temp[5];
    sprintf(cCheckSumTemp,"%X",iErrorCheck);
    iCRC_High   =cCheckSumTemp[6];
    iCRC_Low    =cCheckSumTemp[7];
    sprintf(cSendCommand, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader, iAddress[0], iAddress[1], iFunctionCode[0], iFunctionCode[1], iDataItem[0], iDataItem[1], iDataItem[2], iDataItem[3],
            iNumberOfData[0], iNumberOfData[1], iNumberOfData[2], iNumberOfData[3],
            iCRC_High, iCRC_Low,
            iDelimiter[0], iDelimiter[1]);
    CommBin->WriteCommData(cSendCommand, strlen(cSendCommand));

    Str.sprintf("Send, %s, WriteBin", SendBuffer);

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::MagazineWriteBin_BT008(int index, int ivalue)            //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
{
    int iHeader         =0x3a,
        iAddress[2]     ={0, 0},
        iFunctionCode[2]={0x30, 0x32},
        iDataItem[4]    ={0x30, 0x30, 0x38, 0x31},
        iNumberOfData[4]={0x30, 0x30, 0x30, 0x30},
        iErrorCheck     =0,
        iCRC_High       =0,
        iCRC_Low        =0,
        iCRC_Temp[6],
        iDelimiter[2]   ={0x0d, 0x0a};
    AnsiString Str;
    char casAddress[4]="", cCheckSumTemp[12]="", cSendCommand[24]="", cSetLimit[8]="";

    int iAddArray[15]={14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

    if(index>=100)
    {
        index-=100;
        iHeader=0x3b;
    }

    sprintf(casAddress,"%02d",iAddArray[index]);
    iAddress[0]=0x30+(casAddress[0]-0x30);
    iAddress[1]=0x30+(casAddress[1]-0x30);
    if(ivalue==999)
    {
        iNumberOfData[0]=0x30;                                                  //0
        iNumberOfData[1]=0x30;
        iNumberOfData[2]=0x45;                                                  //E
        iNumberOfData[3]=0x30;
    }
    else if(ivalue==-1)
    {
        iDataItem[3]=0x32;

        iFunctionCode[0]=0x30;
        iFunctionCode[1]=0x30;

        iNumberOfData[0]=0x30;                                                  //0
        iNumberOfData[1]=0x30;
        iNumberOfData[2]=0x30;
        iNumberOfData[3]=0x30;
    }
    else
    {
        sprintf(cSetLimit, "%04d", ivalue*10);
        iNumberOfData[0]=cSetLimit[0];
        iNumberOfData[1]=cSetLimit[1];
        iNumberOfData[2]=cSetLimit[2];
        iNumberOfData[3]=cSetLimit[3];
    }

    iCRC_Temp[0]=(iAddress[0]-0x30)*16+iAddress[1]-0x30;
    iCRC_Temp[1]=(iFunctionCode[0]-0x30)*16+iFunctionCode[1]-0x30;
    iCRC_Temp[2]=(iDataItem[0]-0x30)*16+iDataItem[1]-0x30;
    iCRC_Temp[3]=(iDataItem[2]-0x30)*16+iDataItem[3]-0x30;


    if(iNumberOfData[0]>=0x40 && iNumberOfData[1]>=0x40)
        iCRC_Temp[4]=(iNumberOfData[0]-0x37)*0x10+(iNumberOfData[1]-0x37);
    else if(iNumberOfData[0]>=0x40 && iNumberOfData[1]<0x40)
         iCRC_Temp[4]=(iNumberOfData[0]-0x37)*0x10+(iNumberOfData[1]-0x30);
    else if(iNumberOfData[0]<0x40 && iNumberOfData[1]>=0x40)
          iCRC_Temp[4]=(iNumberOfData[0]-0x30)*0x10+(iNumberOfData[1]-0x37);
    else if(iNumberOfData[0]<0x40 && iNumberOfData[1]<0x40)
         iCRC_Temp[4]=(iNumberOfData[0]-0x30)*0x10+(iNumberOfData[1]-0x30);

    if(iNumberOfData[2]>=0x40)
        iCRC_Temp[5]=(iNumberOfData[2]-0x37)*0x10+(iNumberOfData[3]-0x30);
    else
        iCRC_Temp[5]=(iNumberOfData[2]-0x30)*0x10+(iNumberOfData[3]-0x30);

    iErrorCheck=-iCRC_Temp[0]-iCRC_Temp[1]-iCRC_Temp[2]-iCRC_Temp[3]-iCRC_Temp[4]-iCRC_Temp[5];
    sprintf(cCheckSumTemp, "%X", iErrorCheck);
    iCRC_High   =cCheckSumTemp[6];
    iCRC_Low    =cCheckSumTemp[7];
    sprintf(cSendCommand, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader, iAddress[0], iAddress[1], iFunctionCode[0], iFunctionCode[1], iDataItem[0], iDataItem[1], iDataItem[2], iDataItem[3],
            iNumberOfData[0], iNumberOfData[1], iNumberOfData[2], iNumberOfData[3],
            iCRC_High,iCRC_Low,
            iDelimiter[0],iDelimiter[1]);
    CommBin->WriteCommData(cSendCommand, strlen(cSendCommand));

    Str.sprintf("Send, %s, WriteBin", SendBuffer);

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::MagazineWriteBin_TFT(int index,int ivalue, bool bmulti) //JerryYang 20231106 : TFT
{
    int iHeader=0x3a,
        iFunctionCode[2]={0x00, 0x03},
        iByteCount[2]={0x00, 0x0d},
        iDataItem[2]={0x00,0x01},
        iNumberOfData[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        iDelimiter[2]={0x0d,0x0a};
    AnsiString Str;
    char cSendCommand[21]="", cSetLimit[8]="";

//    int iAddArray[15];
//    int iAddArray_1[15]={0x0f,0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};    //Ifor 20240124 顯示相反
//    int iAddArray_2[15]={0x01,0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};    //Ifor 20240124 KYEC要求Magazine顯示相反

    unsigned char Btmp1;    

//    for(int i=0; i<15; i++)
//    {
//        if(TestIF_File.iMagDisplayOrder==0)
//        {
//            iAddArray[i]=iAddArray_1[i];
//        }
//        else
//        {
//            iAddArray[i]=iAddArray_2[i];
//        }
//    }


//    sprintf(casAddress,"%02d",iAddArray[index]);
//    sprintf(casAddress, "%02d", index);

    if(ivalue==999)
    {
        iNumberOfData[6]=0x45;    //E
    }
    else if(ivalue==-1)
    {
        iNumberOfData[6]=0x2d;  //---
        iNumberOfData[7]=0x2d;
        iNumberOfData[8]=0x2d;

    }
    else
    {
        sprintf(cSetLimit,"%03d",ivalue);
        iNumberOfData[6]=cSetLimit[0];
        iNumberOfData[7]=cSetLimit[1];
        iNumberOfData[8]=cSetLimit[2];
//        iNumberOfData[9]=0x33;
    }

//    Btmp1=iAddArray[index]+iByteCount[0]+iByteCount[1]+iFunctionCode[0]+iFunctionCode[1]+iDataItem[0]+iDataItem[1]+
//          iNumberOfData[0]+iNumberOfData[1]+iNumberOfData[2]+iNumberOfData[3]+iNumberOfData[4]+
//          iNumberOfData[5]+iNumberOfData[6]+iNumberOfData[7]+iNumberOfData[8];
    Btmp1=index+iByteCount[0]+iByteCount[1]+iFunctionCode[0]+iFunctionCode[1]+iDataItem[0]+iDataItem[1]+
          iNumberOfData[0]+iNumberOfData[1]+iNumberOfData[2]+iNumberOfData[3]+iNumberOfData[4]+
          iNumberOfData[5]+iNumberOfData[6]+iNumberOfData[7]+iNumberOfData[8];
    Btmp1=~Btmp1;
    Btmp1+=1;

    sprintf(cSendCommand,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader,index,iByteCount[0], iByteCount[1],iFunctionCode[0],
            iFunctionCode[1],iDataItem[0],iDataItem[1], iNumberOfData[0], iNumberOfData[1],
            iNumberOfData[2], iNumberOfData[3], iNumberOfData[4], iNumberOfData[5], iNumberOfData[6],
            iNumberOfData[7], iNumberOfData[8], Btmp1, iDelimiter[0], iDelimiter[1]);

    if(bmulti==false)
    {
        CommBin->WriteCommData(cSendCommand, 20);
        Str.sprintf("Send, %s, WriteBin", cSendCommand);                            //Ifor 20240906 add:SendBuffer =>cSendCommand

        if(IniConfig.bC14SaveBinDisplayLog)     //Steven 20220309 : BinDisplay Log
            slBinDispLog->AddTextWithDateTime(Str);
    }
    else
    {
        if(bFisrtBinSend==true)
        {
            bFisrtBinSend=false;
            iSendBinCount=0;
            memset(cSendBinCommandBuf, '\0', sizeof(cSendBinCommandBuf));
        }
        for(int i=0; i<20; i++)
        {
            cSendBinCommandBuf[i+iSendBinCount*20]=cSendCommand[i];
        }
        iSendBinCount++;
    }
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::MagazineWriteBinFont_TFT(int index, int iColor, bool binitial, bool bmulti, int iBinNoOverride) //JerryYang 20231106 : TFT   //Eastsun 20260825 : Mag Bin RGB
{
    int iHeader=0x3a,
        iFunctionCode[2]={0x00, 0x02},
        iByteCount[2]={0x00, 0x0d},
        iDataItem[2]={0x02,0x01},
        iNumberOfData[9]={0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        iDelimiter[2]={0x0d,0x0a};
    AnsiString Str;
    char casAddress[4]="", cSendCommand[21]="";

//    int iAddArray[15];
    int iWhichlayer=index;
//    int iAddArray_1[15]={0x0f,0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};    //Ifor 20240124 顯示相反
//    int iAddArray_2[15]={0x01,0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};    //Ifor 20240124 KYEC要求Magazine顯示相反

    unsigned char Btmp1;


//    for(int i=0; i<15; i++)
//    {
//        if(TestIF_File.iMagDisplayOrder==0)
//        {
//            iAddArray[i]=iAddArray_1[i];
//        }
//        else
//        {
//            iAddArray[i]=iAddArray_2[i];
//        }
//    }
//    if(binitial==false)
//    {
//        if(TestIF_File.iMagDisplayOrder==0)
//            iWhichlayer=index+1;
//        else
//            iWhichlayer=index;
//    }
//    else
//    {
//        iWhichlayer=index;
//    }

    sprintf(casAddress,"%02d",iWhichlayer);

    if(iColor==4)
    {
        iFunctionCode[0]=0x00;
        iFunctionCode[1]=0x04;
        iDataItem[0]=0x00;
        iDataItem[1]=0x01;
        iNumberOfData[0]=0x30;
        iNumberOfData[1]=0x30;
        iNumberOfData[2]=0x30;
        iNumberOfData[3]=0x30;
        iNumberOfData[4]=0x30;
        iNumberOfData[5]=0x00;
        iNumberOfData[6]=0x00;
        iNumberOfData[7]=0x05;

        if(iWhichlayer==0x0f)
            iNumberOfData[8]=0x00;
        else
            iNumberOfData[8]=0x30;
    }
    else if(iColor==6)    //JerryYang 20240201 : add
    {
        iFunctionCode[0]=0x00;
        iFunctionCode[1]=0x01;
        iDataItem[0]=0x00;
        iDataItem[1]=0x00;
        iNumberOfData[0]=0x30;
        iNumberOfData[1]=0x30;
        iNumberOfData[2]=0x00;
        iNumberOfData[3]=0x00;
        iNumberOfData[4]=0xff;
        iNumberOfData[5]=0xff;
        iNumberOfData[6]=0xff;
        iNumberOfData[7]=0xff;
        iNumberOfData[8]=0xff;
    }
    else if(iColor==5)
    {
        iFunctionCode[0]=0x00;
        iFunctionCode[1]=0x01;
        iDataItem[0]=0x00;
        iDataItem[1]=0x00;
        iNumberOfData[0]=0x30;
        iNumberOfData[1]=0x30;
        iNumberOfData[2]=0x00;
        iNumberOfData[3]=0x00;
        iNumberOfData[4]=0xA0;
        iNumberOfData[5]=0x50;
        iNumberOfData[6]=0x00;
        iNumberOfData[7]=0x00;
        iNumberOfData[8]=0x00;
    }
    else
    {
        int iBinNoRGB = (iBinNoOverride>=0) ? iBinNoOverride                             //Eastsun 20260825 : Mag Bin RGB
                      : ((index>=0 && index<MAX_BIN_UNIT) ? iSetBin[index][0] : -1);
        int iBinR, iBinG, iBinB;                                                      //Eastsun 20260911 : 預設色改由GetBinRGB統一提供
        GetBinRGB(iBinNoRGB, iColor, iBinR, iBinG, iBinB);                            //Eastsun 20260911
        iNumberOfData[4] = (unsigned char)iBinR;                                      //Eastsun 20260911
        iNumberOfData[5] = (unsigned char)iBinG;                                      //Eastsun 20260911
        iNumberOfData[6] = (unsigned char)iBinB;                                      //Eastsun 20260911

        iNumberOfData[7]=0x0c;  //字體12
        iNumberOfData[8]=0xff;  //透明度
    }



    Btmp1=iWhichlayer+iByteCount[0]+iByteCount[1]+iFunctionCode[0]+iFunctionCode[1]+iDataItem[0]+iDataItem[1]+
          iNumberOfData[0]+iNumberOfData[1]+iNumberOfData[2]+iNumberOfData[3]+iNumberOfData[4]+
          iNumberOfData[5]+iNumberOfData[6]+iNumberOfData[7]+iNumberOfData[8];
    Btmp1=~Btmp1;
    Btmp1+=1;

    sprintf(cSendCommand,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader,iWhichlayer,iByteCount[0], iByteCount[1],iFunctionCode[0],
            iFunctionCode[1],iDataItem[0],iDataItem[1], iNumberOfData[0], iNumberOfData[1],
            iNumberOfData[2], iNumberOfData[3], iNumberOfData[4], iNumberOfData[5], iNumberOfData[6],
            iNumberOfData[7], iNumberOfData[8], Btmp1, iDelimiter[0], iDelimiter[1]);


    if(bmulti==false)
    {
        CommBin->WriteCommData(cSendCommand, 20);
        Str.sprintf("Send, %s, WriteBin", cSendCommand);                        //Ifor 20240906 add:SendBuffer =>cSendCommand

        if(IniConfig.bC14SaveBinDisplayLog)     //Steven 20220309 : BinDisplay Log
            slBinDispLog->AddTextWithDateTime(Str);
    }
    else
    {
        if(bFisrtSend==true)
        {
            bFisrtSend=false;
            iSendCount=0;
            memset(cSendCommandBuf, '\0', sizeof(cSendCommandBuf));
        }
        for(int i=0; i<20; i++)
        {
            cSendCommandBuf[i+iSendCount*20]=cSendCommand[i];
        }
        iSendCount++;
    }
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteColor(int Addr, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+38, Value, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+32, Value, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X06008200%02d%s%s, WriteColor", Addr+38, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X06008200%02d%s%s, WriteColor", Addr+32, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
};
//------------------------------------------------------------------------------
void TMyBinDispHT9046::WriteColor2(int Addr, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+38, Value, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+32, Value, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin2->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X06008200%02d%s%s, WriteColor", Addr+38, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X06008200%02d%s%s, WriteColor", Addr+32, Value, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
};
//------------------------------------------------------------------------------
void TMyBinDispHT9046::ReadVersion(int Addr)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X030080000100%c%c", Addr+38, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X030080000100%c%c", Addr+32, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X0300800001%s%s, ReadVersion", Addr+38, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X0300800001%s%s, ReadVersion", Addr+32, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::ReadVersion2(int Addr)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X030080000100%c%c", Addr+38, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X030080000100%c%c", Addr+32, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR(&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);                                   //Set return number by byte
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin2->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf("Send, :%02X0300800001%s%s, ReadVersion", Addr+38, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));
    else
        Str.sprintf("Send, :%02X0300800001%s%s, ReadVersion", Addr+32, MyDeCodeASCII(SendBuffer[13]), MyDeCodeASCII(SendBuffer[14]));

    if(IniConfig.bC14SaveBinDisplayLog)                                         //Steven 20220309 : BinDisplay Log
        slBinDispLog->AddTextWithDateTime(Str);
}
//------------------------------------------------------------------------------
void TMyBinDispHT9046::ReadVersion_TFT(int index)                               //Sam 20240604 : 新增 BinDisplay TFT
{

    int iHeader         =0x3a,
        iByteCount   [2]={0x00, 0x0d},
        iFunctionCode[2]={0x08, 0x00},
        iDataItem    [2]={0x30, 0x30},
        iNumberOfData[9]={0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31},
        cLRC1           =0x00;
    char cSendCommand[20]="";
    sprintf(cSendCommand, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            iHeader,
            iAddArrayTFT[index],
            iByteCount[0],
            iByteCount[1],
            iFunctionCode[0],
            iFunctionCode[1],
            iDataItem[0],
            iDataItem[1],
            iNumberOfData[0],
            iNumberOfData[1],
            iNumberOfData[2],
            iNumberOfData[3],
            iNumberOfData[4],
            iNumberOfData[5],
            iNumberOfData[6],
            iNumberOfData[7],
            iNumberOfData[8]);
    cLRC1=A_Create_LRC(&cSendCommand[1], 16);                                   //LRC 計算範圍 iAddArrayTFT ~ iNumberOfData[8] (不包含iHeader)
    cSendCommand[17]=cLRC1;
    cSendCommand[18]=Bin_CR;
    cSendCommand[19]=Bin_LF;

    CommBin->WriteCommData(cSendCommand, 20);
    ShowCommLog(cSendCommand, index, "ReadVersion");
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartSetBin()
{
    static bool bSet=false;
    static bool bMagBlink[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, bMagBlinkStatus[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static int iErrCount[MAX_BIN_UNIT]={0};
    static int iCount[MAX_BIN_UNIT]={0};

    int &Task=iStartSetBinTask;
    int iAddArray[15]={14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};       //JerryYang 20230515 : Magazine BIN DISP
    int iRealIndex=999;
    int iAddress[2]={0, 0};
    char casAddress[4]="";
    AnsiString sCheckWord="", Str="";
    static int iSendCMD=0;
    int iSnedLen=0;    


    if(bStartSetBin==true)
    {
        iRusStatus=eBDP_BinSet;
    }
    else
    {
        iRusStatus=eBDP_BinRun;
    }
    //if(MyMessageBox->fShow && iMagazineStatus!=0)
    if(iMagazineStatus!=0)                                                      //Ifor 20240308 Fix:結束取Tray流程後恢復顯示器背光
    {
        iSendCMD=0;
        bMagBlink[iAuto3MagazineIndex]=true;
        if(BinSelect[iTestRunMode].bMagazineLink[iAuto3MagazineIndex]==true)
        {
            for(int i=0; i<14; i++)
            {
                if(BinSelect[iTestRunMode].bMagazineLink[iAuto3MagazineIndex-i]==false)
                {
                    iRealIndex=iAuto3MagazineIndex-i;
                    break;
                }
            }

            for(int i=0; i<=iAuto3MagazineIndex; i++)
            {
                if(iAuto3MagazineIndex-i>=iRealIndex)
                {
                    if(iAuto3MagazineIndex-i!=iRealIndex)
                    {
                        bMagBlink[iRealIndex+i]=true;
                    }
                }
            }
        }
        else
        {
            bMagBlink[iAuto3MagazineIndex]=false;
        }

        if(bSet==false)
        {
            tMagTimer.SetSecAndOn(0.8);
            bSet=true;
        }
        else
        {
            if(tMagTimer.Off())
            {
                bFisrtSend=true;
                for(int i=0; i<=iAuto3MagazineIndex; i++)
                {
                    if(bMagBlink[i]==true)
                    {
                        if(bMagBlinkStatus[i]==true)
                        {
                            MagazineWriteBinFont_TFT(i+1, 5, false, true,iSetBin[Addr][iCount[Addr]]);        //JerryYang 20250918 : addr i -> i+1, 站號從1開始         //Ifor 20240227 add: Address 改至Function確認 i+1 =>i
                        }
                        else
                        {
                            MagazineWriteBinFont_TFT(i+1, 6, false, true, iSetBin[Addr][iCount[Addr]]);                 //Ifor 20240227 add: Address 改至Function確認i+1 =>i
                        }
                        bMagBlinkStatus[i]=!bMagBlinkStatus[i];
                    }
                }
                iSnedLen=iSendCount*20;
                CommBin->WriteCommData(cSendCommandBuf, iSnedLen);
                for(int i=0; i<iSnedLen; i++)
                {
                    anSendCommandBuf=anSendCommandBuf+(IntToHex((int)cSendCommandBuf[i],2));
                }
                Str.sprintf("Send, %s, WriteBin", anSendCommandBuf);                        //Ifor 20240906 add:SendBuffer =>cSendCommand

                if(IniConfig.bC14SaveBinDisplayLog)     //Steven 20220309 : BinDisplay Log
                    slBinDispLog->AddTextWithDateTime(Str);
                tMagTimer.SetSecAndOn(1);                    
            }
        }
        return false;
    }
    else
    {
        for(int i=0; i<=iAuto3MagazineIndex; i++)
        {
            bMagBlink[i]=false;
        }

        if(iSendCMD<=3) //Ifor 20240206 add:Magazine 取Tray完成後恢復閃爍狀態   //Ifor 20240906 add:Bin資料串接減少次數 8 => 3
        {
            if(tMagTimer.Off())
            {
                bFisrtSend=true;
                for(int i=1; i<=MAX_MGZ_TRAY; i++)
                {
                    MagazineWriteBinFont_TFT(i, 6,false, true, iSetBin[Addr][iCount[Addr]]);                             //Ifor 20240227 add: Address 改至Function確認 i+1 =>i
                }
                iSnedLen=iSendCount*20;
                CommBin->WriteCommData(cSendCommandBuf, iSnedLen);
                for(int i=0; i<iSnedLen; i++)
                {
                    anSendCommandBuf=anSendCommandBuf+(IntToHex((int)cSendCommandBuf[i],2));
                }
                Str.sprintf("Send, %s, WriteBin", anSendCommandBuf);                        //Ifor 20240906 add:SendBuffer =>cSendCommand

                if(IniConfig.bC14SaveBinDisplayLog)     //Steven 20220309 : BinDisplay Log
                    slBinDispLog->AddTextWithDateTime(Str);

                tMagTimer.SetSecAndOn(1.8);
                iSendCMD++;
            }
        }
    }
    switch(Task)
    {
        case 1:                                                                 //初始化，只要重設定Bin，就要初始化一次
            Addr=0;
            Task=100;
            for(int i=0; i<MAX_BIN_UNIT; i++)
            {
                if(AUTO3_IS_MAGAZINE==0 &&                                      //JerryYang 20230515 : 沒裝Magazine就不要顯示
                   i>=eBinDispMag1 &&
                   i<=eBinDispMag14)
                {
                    bSliding[i]=false;
                    bSetBin[i]=false;
                }
                else if(AUTO3_IS_MAGAZINE==1 && i==eBinDispAuto3)
                {
                    bSliding[i]=false;
                    bSetBin[i]=false;
                }
                else if(NUMBER_PANEL_TYPE==4 && i>=eBinDispLoader && i<=eBinDispBulkBox)
                {
                    bSliding[i]=false;
                    bSetBin[i]=false;
                }
                else if(i==eBinDispBulkBox)                                     //Jimmychiu 20231030 : bin Box not display
                {
                    bSliding[i]=false;
                    bSetBin[i]=false;
                }
                else
                {
                    bSliding[i]=true;
                    bSliding_Cnt[i]=true;
                    bSetBin[i]=true;
                    iCount[i]=0;
                }
            }
            break;
        case 100:                                                               //開始設定
            while(1)
            {
                if(Addr>=MAX_BIN_UNIT)                                          //第一次設定完成，準備開始輪播
                {
                    Addr=0;
                    if(bStartSetBin)
                    {
                        Task=1000;
                        bStartSetBin=false;
                        return false;
                    }
                    return true;

                    }
                else if(NUMBER_PANEL_TYPE==4 && Addr>=eBinDispLoader && Addr<=eBinDispBulkBox)
                {
                    Addr++;
                    break;
                }
                else if(Addr==eBinDispBulkBox)                                     //Jimmychiu 20231124 : Fixed for Bin disaplay not change.
                {
                    Addr++;
                    break;
                }



                if(bSliding[Addr]==true &&
                   bSetBin[Addr]==true &&
                   bHasUnitArray[Addr])
                {
                    if(iSetBin[Addr][iCount[Addr]]==-1)                         //Steven 20140404 Start: Fixed 256Bin
                    {
                        iCount[Addr]=0;
                    }
                    else if(iTestBinCount>=16 && Addr>=3 &&
                            iSetBin[Addr][iCount[Addr]]>=100 &&
                            iSetBin[Addr][iCount[Addr]]!=104)                   //256 bin 超過顯示bin
                    {
                        if(MAGAZINE_BIN_DISP_TYPE==eHTBT008 || MAGAZINE_BIN_DISP_TYPE==eTFT)                    //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
                        {

                        }
                        else
                        {
                            if(Addr==BinSelect[iTestRunMode].IfError+3)
                                iCount[Addr]++;
                            else
                                iCount[Addr]=0;
                        }
                    }

                    if(Addr>=eBinDispMag1 && Addr<=eBinDispMag14)               //JerryYang 20230515 : Magazine BIN DISP
                    {
                        if(MAGAZINE_BIN_DISP_TYPE==eHTA18)                      //JerryYang 20230515 : 二位數BIN DISP, HTA18
                        {
                            if(iSetBin[Addr][0]==-1)                            //沒有設定，所以寫X
                            {
                                MagazineWriteBin_HTA18(AddBinDisp[Addr], 0);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]<100)            //數字
                            {
                                MagazineWriteBin_HTA18(AddBinDisp[Addr], iSetBin[Addr][iCount[Addr]]);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]==104 ||
                                    iSetBin[Addr][iCount[Addr]]>=100)
                            {
                                MagazineWriteBin_HTA18(AddBinDisp[Addr], 100);
                            }
                            else
                            {
                                Addr++;
                                break;
                            }
                        }
                        else if(MAGAZINE_BIN_DISP_TYPE==eTFT)
                        {
                            MagazineWriteBin_TFT(AddBinDisp[Addr],  iSetBin[Addr][iCount[Addr]]);
                        }
                        else                                                    //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
                        {
                            if(iSetBin[Addr][0]==-1)                            //沒有設定，顯示"--"
                            {
                                MagazineWriteBin_BT008(AddBinDisp[Addr], -1);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]>=0 &&
                                    iSetBin[Addr][iCount[Addr]]<=999)           //E   //數字
                            {
                                MagazineWriteBin_BT008(AddBinDisp[Addr], iSetBin[Addr][iCount[Addr]]);
                            }
                            else
                            {
                                Addr++;
                                break;
                            }
                        }
                    }
                    else if(Addr==eBinDispBulkBox)                              //Jimmychiu 20231124 : Fixed for Bin disaplay not change.
                    {
                        Addr++;
                        break;
                    }
                    else
                    {
                        if(AUTO_EMPTY_COLOR<3)
                        {
                            if(Addr==eBinDispAuto4 || Addr==eBinDispAuto5 || Addr==eBinDispAuto6 ||
                               (Addr>=eBinDispFix7 && Addr<=eBinDispFix12))
                            {
                                Addr++;
                                break;
                            }
                            else if(iSetBin[Addr][0]==-1)                       //沒有設定，所以寫X
                            {
                                WriteBin(Addr, 1, 123-100);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]<100)            //數字
                            {
                                WriteBin(Addr, 0, iSetBin[Addr][iCount[Addr]]);
                            }                                                   //ChungHung 20140419 修正顯示問題
                            else if(Addr<=3 || iSetBin[Addr][iCount[Addr]]==104)
                            {
                                WriteBin(Addr, 1, iSetBin[Addr][iCount[Addr]]-100);
                            }
                            else
                            {
                                Addr++;
                                break;
                            }
                        }
                        else
                        {
                            if(iSetBin[Addr][0]==-1)                            //沒有設定，所以寫X
                            {
                                if((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                                   (Addr>=eBinDispFix7 && Addr<=eBinDispFix12))
                                {
                                    WriteBin2(AddBinDisp[Addr], 1, 123-100);
                                    BinDispRecv2=false;
                                }
                                else
                                {
                                    WriteBin(AddBinDisp[Addr], 1, 123-100);
                                }
                            }
                            else if(iSetBin[Addr][iCount[Addr]]<100)            //數字
                            {
                                if((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                                   (Addr>=eBinDispFix7 && Addr<=eBinDispFix12))
                                {
                                    WriteBin2(AddBinDisp[Addr], 0, iSetBin[Addr][iCount[Addr]]);
                                }
                                else
                                {
                                    WriteBin(AddBinDisp[Addr], 0, iSetBin[Addr][iCount[Addr]]);
                                }
                            }                                                   //ChungHung 20140419 修正顯示問題
                            else if(Addr<=3 || iSetBin[Addr][iCount[Addr]]==104)
                            {
                                if((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                                   (Addr>=eBinDispFix7 && Addr<=eBinDispFix12))
                                {
                                    WriteBin2(AddBinDisp[Addr], 1, iSetBin[Addr][iCount[Addr]]-100);
                                }
                                else
                                {
                                    WriteBin(AddBinDisp[Addr], 1, iSetBin[Addr][iCount[Addr]]-100);
                                }
                            }
                            else
                            {
                                Addr++;
                                break;
                            }
                        }
                    }
                    Task=200;
                    BinDispRecv=false;
                    BinDispRecv2=false;
                    BinDisDelay.SetSecAndOn(2);
                    break;
                }
                Addr++;
            }
            break;
        case 200:                                                               //接收回傳資料
            if(BinDispRecv || BinDispRecv2)
            {
                if(Addr>=MAX_BIN_UNIT)
                {
                    Task=100;
                    break;
                }

                if(MAGAZINE_BIN_DISP_TYPE==eHTBT008 && Addr>=eBinDispMag1)      //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
                {
                    sprintf(casAddress,"%02d",iAddArray[MagAddr]);
                    iAddress[0]=0x30+(casAddress[0]-0x30);
                    iAddress[1]=0x30+(casAddress[1]-0x30);

                    if(iSetBin[Addr][iCount[Addr]]==-1)                         //沒有設定BIN
                    {
                        sCheckWord.sprintf(":%c%c0022", iAddress[0], iAddress[1]);
                    }
                    else
                    {
                        sCheckWord.sprintf(":%c%c0212", iAddress[0], iAddress[1]);
                    }
                }
                else
                {
                    int iAddrFix=Addr;
                    if(AUTO_EMPTY_COLOR>=3 && ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) || (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                    {
                         if(Addr>=eBinDispFix1 && Addr<=eBinDispFix6)
                            iAddrFix=Addr-eBinDispFix1;
                         else
                            iAddrFix=Addr-eBinDispFix7+6;
                    }
                    else if(AUTO_EMPTY_COLOR>=3 && Addr>=eBinDispAuto4 && Addr<=eBinDispAuto6)
                    {
                        iAddrFix=Addr-eBinDispAuto4+6;
                    }

                    if(iVersion[Addr]==1)                                       //舊模組
                    {                                                           //Steven 20110326 : 比對回傳值
                        if(iAddrFix>=10)
                            sCheckWord.sprintf(":%02X06020010", iAddrFix+38);
                        else
                            sCheckWord.sprintf(":%02X06020010", iAddrFix+32);
                    }
                    else if(iVersion[Addr]==2)                                  //新模組
                    {
                        if(iAddrFix>=10)
                        {
                            if(iSetBin[Addr][0]==-1)                            //沒有設定，所以寫X
                            {
                                sCheckWord.sprintf(":%02X060201%02d", iAddrFix+38, 123-100);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]<100)            //數字
                            {
                                sCheckWord.sprintf(":%02X060200%02d", iAddrFix+38, iSetBin[Addr][iCount[Addr]]);
                            }
                            else                                                //字母
                            {
                                sCheckWord.sprintf(":%02X060201%02d", iAddrFix+38, iSetBin[Addr][iCount[Addr]]-100);
                            }
                        }
                        else
                        {
                            if(iSetBin[Addr][0]==-1)                            //沒有設定，所以寫X
                            {
                                sCheckWord.sprintf(":%02X060201%02d", iAddrFix+32, 123-100);
                            }
                            else if(iSetBin[Addr][iCount[Addr]]<100)            //數字
                            {
                                sCheckWord.sprintf(":%02X060200%02d", iAddrFix+32, iSetBin[Addr][iCount[Addr]]);
                            }
                            else                                                //字母
                            {
                                sCheckWord.sprintf(":%02X060201%02d", iAddrFix+32, iSetBin[Addr][iCount[Addr]]-100);
                            }
                        }
                    }
                }

                if(AUTO_EMPTY_COLOR>=3 &&
                   ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                    (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                {
                    if(sReadBuffer2.Pos(sCheckWord)==1 ||
                       (MAGAZINE_BIN_DISP_TYPE==eHTA18 && Addr>=eBinDispMag1))  //JerryYang 20230515 : 二位數BIN DISP, HTA18
                    {
                        iBinNow[Addr]=iSetBin[Addr][iCount[Addr]];
                        bHasError[Addr]=false;                                  // 顯示器是否有出錯
                        iErrCount[Addr]=0;

                        iCount[Addr]++;
                        if(Addr<3)
                            bSetBin[Addr]=false;
                        if(iSetBin[Addr][iCount[Addr]]==-1 ||                   //如果是0 表示已經到最後一個Bin，所以從頭開始Show
                           iCount[Addr]>=iTestBinCount)                         //Steven 20140404 : MAX_BIN_UNIT --> iTestBinCount
                        {
                            iCount[Addr]=0;
                        }
                        if(iSetBin[Addr][0]==-1 || iSetBin[Addr][1]==-1)        //如果是X 表示該位置沒有設Bin，不需要輪撥
                        {
                            bSliding[Addr]=false;
                        }

                        if(Addr<3)                                              //L / E /C 不需要輪撥
                            bSliding[Addr]=false;

                        Addr++;                                                 //Steven 20211230 : 修正Bin顯示器異常恢復後不會輪播的問題
                        if(Addr>=MAX_BIN_UNIT)
                        {
                            Task=100;
                            break;
                        }
                    }
                    else
                    {
                        iErrCount[Addr]++;
                    }
                }
                else
                {
                    if(sReadBuffer.Pos(sCheckWord)==1 ||
                       ((MAGAZINE_BIN_DISP_TYPE==eTFT || MAGAZINE_BIN_DISP_TYPE==eHTA18)  && Addr>=eBinDispMag1))  //JerryYang 20230515 : 二位數BIN DISP, HTA18
                    {
                        iBinNow[Addr]=iSetBin[Addr][iCount[Addr]];
                        bHasError[Addr]=false;                                  // 顯示器是否有出錯
                        iErrCount[Addr]=0;

                        iCount[Addr]++;
                        if(Addr<3)
                            bSetBin[Addr]=false;

                        if(iSetBin[Addr][iCount[Addr]]==-1 ||                   //如果是0 表示已經到最後一個Bin，所以從頭開始Show
                           iCount[Addr]>=iTestBinCount)                         //Steven 20140404 : MAX_BIN_UNIT --> iTestBinCount
                        {
                            iCount[Addr]=0;
                        }

                        if(iSetBin[Addr][0]==-1 || iSetBin[Addr][1]==-1)        //如果是X 表示該位置沒有設Bin，不需要輪撥
                        {
                            bSliding[Addr]=false;
                        }

                        if(Addr<3)                                              //L / E /C 不需要輪撥
                            bSliding[Addr]=false;

                        Addr++;                                                 //Steven 20211230 : 修正Bin顯示器異常恢復後不會輪播的問題
                        if(Addr>=MAX_BIN_UNIT)
                        {
                            Task=100;
                            break;
                        }
                    }
                    else
                    {
                        iErrCount[Addr]++;
                    }
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(Addr>=MAX_BIN_UNIT)                                              //JerryYang 20230515 : 避免超出陣列
            {
                Addr=0;
            }
            if(iErrCount[Addr]>5)
            {
                iRusStatus=eBDP_DispErr;                                        //Sam 20240604 : 新增 BinDisplay TFT
                Task=100;
                bHasError[Addr]=true;                                           // 顯示器是否有出錯
                if(IniConfig.bG16BinDispNeedAlarm==false)                       //Steven 20211220 : 修正G16, Bin顯示器異常要alarm
                {
                    CommBin->StopComm();
                    CommBin2->StopComm();
                    iBinDispCtrlTask=1;
                    bStartSetColor=true;                                        //重設顏色
                    bStartSetBin=true;                                          //重設Bin
                }
            }
            break;
        case 1000:
            BinDisDelay.SetSecAndOn(dDelaySec);                                 //Sam 20240604 : 顯示器輪巡時間改為 double
            Task=1100;
        case 1100:
            if(BinDisDelay.Off())
            {
                for(int i=0; i<MAX_BIN_UNIT ; i++)
                      if(bHasUnitArray[i])
                        bSetBin[i]=true;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartSetColor()
{
    AnsiString sCheckWord="";
    AnsiString Str;
    int &Task=iStartSetColorTask;
    static int iErrCount[MAX_BIN_UNIT]={0};
    static bool bFirst=false;
    static int iCount[MAX_BIN_UNIT]={0};
    iRusStatus=2;

    switch(Task)
    {
        case 1:
            Addr=0;
            Task=100;
            if(MAGAZINE_BIN_DISP_TYPE==eTFT && bFirst==false)
                Task=10;

            for(int i=0; i<MAX_BIN_UNIT; i++)
            {
//                if(i>=eBinDispMag1 && i<=eBinDispMag14)                   //JerryYang 20230515 : Magazine BIN DISP
//                {
//                    bSetColor[i]=false;
//                }
//                else
                if(i==eBinDispBulkBox)                                  //Jimmychiu 20231030 : bin Box not display
                {
                    bSetColor[i]=false;
                }
                else if(AUTO3_IS_MAGAZINE==1 && i==eBinDispAuto3)
                {
                    bSetColor[i]=false;
                }
                else if(AUTO_EMPTY_COLOR==3 && i==eBinDispAuto6)                //Jimmychiu 20231124 : Fixed for Bin disaplay not change.
                {
                    bSetColor[i]=false;
                }
                else if(AUTO_EMPTY_COLOR<3 &&
                        (i==eBinDispAuto4 || i==eBinDispAuto5 || i==eBinDispAuto6 ||
                        (i>=eBinDispFix7 && i<=eBinDispFix12)))
                {
                    bSetColor[i]=false;
                }
                else if(NUMBER_PANEL_TYPE==4 && i>=eBinDispLoader && i<=eBinDispBulkBox)
                {
                    bSetColor[i]=false;
                }
                else
                {
                    bSetColor[i]=true;
                }
            }
            ProcessFlash();                                                     //Eastsun 20260513 : 顯示器閃爍功能  裡面有更動到bSetColor

            break;
        case 10:
            while(1)
            {
                if(MAGAZINE_BIN_DISP_TYPE==eTFT)
                {
                    if(Addr>eBinDispMag14)   //JerryYang 20230515 : Magazine BIN DISP
                    {
                        MagazineWriteBinFont_TFT(15, 4, false, false, iSetBin[Addr][iCount[Addr]]);
                        Task=70;
                        BinDisDelay.SetSecAndOn(2);
                        Addr=0;
                        break;
                    }
                }
                else
                {
                    if(Addr>=eBinDispMag1 && Addr<=eBinDispMag14)   //JerryYang 20230515 : Magazine BIN DISP
                        return true;
                }

                if(bSetColor[Addr]==true && bHasUnitArray[Addr] && Addr>=eBinDispMag1 && Addr<=eBinDispMag14)
                {
                    MagazineWriteBinFont_TFT(AddBinDisp[Addr], 4, false, false, iSetBin[Addr][iCount[Addr]]);
                    BinDispRecv=false;
                    BinDisDelay.SetSecAndOn(2);
                    Task=50;
                    break;
                }
                Addr++;
            }
            BinDispRecv=false;
            BinDisDelay.SetSecAndOn(1);
            break;
        case 50:
            if(BinDispRecv)
            {
                sCheckWord.sprintf("3A%02X0D", iAddArrayTFT[Addr]);

                if(sReadBufferMag.Pos(sCheckWord)==1)
                {
                    bHasError[Addr]=false;                                      //模組沒有出錯
                    iErrCount[Addr]=0;
                    Addr++;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=10;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
            }

            if(iErrCount[Addr]>=1)
            {
                iRusStatus=4;
                bHasError[Addr]=true;
                iErrCount[Addr]=0;
                CommBin->StopComm();
                CommBin2->StopComm();
                iBinDispCtrlTask=1;
                bStartSetColor=true;                                            //重設顏色
                bStartSetBin=true;                                              //重設Bin
            }
            break;
        case 70:
            if(BinDispRecv)
            {
                sCheckWord.sprintf("3A%02X0D", 15);

//                if(sReadBufferMag.Pos(sCheckWord)==1)  //test 暫時MARK掉
                {
                    Task=100;
                }
//                else
                {

                }

            }
            else if(BinDisDelay.Off())
            {
                iRusStatus=4;
                CommBin->StopComm();
                CommBin2->StopComm();
                iBinDispCtrlTask=1;
                bStartSetColor=true;                                            //重設顏色
                bStartSetBin=true;                                              //重設Bin
            }

            break;
        case 100:
            while(1)
            {
                if(MAGAZINE_BIN_DISP_TYPE==eTFT)
                {
                    if(Addr>eBinDispMag14)   //JerryYang 20230515 : Magazine BIN DISP
                        return true;
                }
                else
                {
                    if(Addr>=eBinDispMag1 && Addr<=eBinDispMag14)   //JerryYang 20230515 : Magazine BIN DISP
                        return true;
                }

                if(bSetColor[Addr]==true && bHasUnitArray[Addr])
                {
                    if(MAGAZINE_BIN_DISP_TYPE==eTFT && Addr>=eBinDispMag1 && Addr<=eBinDispMag14)
                    {
                        MagazineWriteBinFont_TFT(AddBinDisp[Addr], iSetColor[Addr],false,false, iSetBin[Addr][iCount[Addr]]);
                    }
                    else if(AUTO_EMPTY_COLOR>=3 &&
                       ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                        (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                    {
                        WriteColor2(AddBinDisp[Addr], iSetColor[Addr]);         //設定顏色
                    }
                    else
                    {
                        WriteColor(AddBinDisp[Addr], iSetColor[Addr]);          //設定顏色
                    }

                    Task=200;
                    BinDispRecv2=false;
                    BinDispRecv=false;
                    BinDisDelay.SetSecAndOn(2);
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv || BinDispRecv2)
            {
                if(Addr>=MAX_BIN_UNIT)
                {
                    Task=100;
                    break;
                }

                if(iVersion[Addr]==1)                                           //舊模組
                {
                    if(AddBinDisp[Addr]>=10)
                        sCheckWord.sprintf(":%02X06020010", AddBinDisp[Addr]+38);
                    else
                        sCheckWord.sprintf(":%02X06020010", AddBinDisp[Addr]+32);
                }
                else if(iVersion[AddBinDisp[Addr]]==2)                          //新模組
                {
                    if(AddBinDisp[Addr]>=10)
                        sCheckWord.sprintf(":%02X060202%02d", AddBinDisp[Addr]+38, iSetColor[Addr]);
                    else
                        sCheckWord.sprintf(":%02X060202%02d", AddBinDisp[Addr]+32, iSetColor[Addr]);
                }

                if(AUTO_EMPTY_COLOR>=3 && ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) || (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                {
                    if(sReadBuffer2.Pos(sCheckWord)==1)
                    {
                        iColorNow[Addr]=iSetColor[Addr];                        //紀錄現在的顏色
                        bHasError[Addr]=false;                                  //模組沒有出錯
                        iErrCount[Addr]=0;
                        bSetColor[Addr]=false;
                    }
                    else
                    {
                        iErrCount[Addr]++;
                    }
                }
                else
                {
                    if(sReadBuffer.Pos(sCheckWord)==1 || (Addr>=eBinDispMag1 && Addr<=eBinDispMag14))
                    {
                        iColorNow[Addr]=iSetColor[Addr];                        //紀錄現在的顏色
                        bHasError[Addr]=false;                                  //模組沒有出錯
                        iErrCount[Addr]=0;
                        bSetColor[Addr]=false;
                    }
                    else
                    {
                        iErrCount[Addr]++;
                    }
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>5)
            {
                iRusStatus=4;
                bHasError[Addr]=true;
                iErrCount[Addr]=0;
                CommBin->StopComm();
                CommBin2->StopComm();
                iBinDispCtrlTask=1;
                bStartSetColor=true;                                            //重設顏色
                bStartSetBin=true;                                              //重設Bin
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartGetStatus()
{
    AnsiString sCheckWord="";
    int &Task=iStartGetStatusTask;
    static int iErrCount[MAX_BIN_UNIT]={0};

    iRusStatus=1;

    switch(Task)
    {
        case 1:
            Addr=0;
            Task=100;
            for(int i=0; i<MAX_BIN_UNIT; i++)
            {
                if(AUTO3_IS_MAGAZINE==0 &&
                   i>=eBinDispMag1 && i<=eBinDispMag14)                         //JerryYang 20230515 : 沒裝Magazine就不要顯示
                {
                    bGetStatus[i]=false;
                    bHasUnitArray[i]=false;
                }
                else if(AUTO3_IS_MAGAZINE==1 && i==eBinDispAuto3)
                {
                    bGetStatus[i]=false;
                    bHasUnitArray[i]=false;
                }
                else if(i==eBinDispBulkBox)                                     //Jimmychiu 20231124 : Fixed for Bin disaplay not change.
                {
                    bGetStatus[i]=false;
                    bHasUnitArray[i]=false;
                }
                else if(AUTO_EMPTY_COLOR==3 && i==eBinDispAuto6)
                {
                    bGetStatus[i]=false;
                    bHasUnitArray[i]=false;
                }
                else if((AUTO_EMPTY_COLOR<3 &&
                        (i==eBinDispAuto4 || i==eBinDispAuto5 || i==eBinDispAuto6 ||
                        (i>=eBinDispFix7 && i<=eBinDispFix12))))               //Sam 20241204 : Addr --> i for Bin display
                {
                    bGetStatus[i]=false;
                    bHasUnitArray[i]=false;
                }
                else
                {
                    bHasUnitArray[i]=true;
                    bGetStatus[i]=true;
//                    bHasUnitArray[i]=false;  //暫時不問版本號
//                    bGetStatus[i]=false;    //暫時不問版本號
                }
            }
            break;
        case 100:
            while(1)
            {
                if(Addr>=MAX_BIN_UNIT)                                          //JerryYang 20230515 : Magazine BIN DISP
                    return true;

                if(Addr>=eBinDispMag1 && Addr<=eBinDispMag14)                   //JerryYang 20230515 : Magazine BIN DISP
                    return true;

                if(AUTO_EMPTY_COLOR>=3 &&
                   ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                    (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                {
                    if(bGetStatus[Addr]==true)
                    {
                        if(Addr>=eBinDispFix1 && Addr<=eBinDispFix6)
                            ReadVersion2(Addr-eBinDispFix1);                    //設定取得版本      //JerryYang 20230925 : AUTO4的Address從0開始
                        else
                            ReadVersion2(Addr-eBinDispFix7+6);                  //設定取得版本      //JerryYang 20230925 : AUTO4的Address從0開始
                        Task=200;
                        BinDispRecv=false;
                        BinDispRecv2=false;
                        BinDisDelay.SetSecAndOn(2);
                        break;
                    }
                }
                else
                {
                    if(bGetStatus[Addr]==true)
                    {
                        if(NUMBER_PANEL_TYPE==4)                                //Sam 20250428 : 修正 BinDisp TFT
                        {
                            ReadVersion_TFT(Addr);
                        }
                        else if(AUTO_EMPTY_COLOR>=3 &&
                           Addr>=eBinDispAuto4 &&
                           Addr<=eBinDispAuto6)                                 //L E C A1 A2 A3 A4 A5 依照順序
                        {
                            ReadVersion(6+(Addr-eBinDispAuto4));                //設定取得版本
                        }
                        else
                        {
                            ReadVersion(Addr);                                  //設定取得版本
                        }
                        Task=200;
                        BinDispRecv=false;
                        BinDispRecv2=false;
                        BinDisDelay.SetSecAndOn(2);
                        break;
                    }
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv || BinDispRecv2)
            {
                if(Addr>=MAX_BIN_UNIT)
                {
                    Task=100;
                    break;
                }

                int iAddrFix=Addr;
                if(AUTO_EMPTY_COLOR>=3 &&
                   ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                    (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                {
                     if(Addr>=eBinDispFix1 && Addr<=eBinDispFix6)
                        iAddrFix=Addr-eBinDispFix1;
                     else
                        iAddrFix=Addr-eBinDispFix7+6;
                }
                else if(AUTO_EMPTY_COLOR>=3 &&
                        Addr>=eBinDispAuto4 &&
                        Addr<=eBinDispAuto6)
                {
                    iAddrFix=Addr-eBinDispAuto4+6;
                }

                if(NUMBER_PANEL_TYPE==4)                                        //Sam 20240604 : 新增 BinDisplay TFT
                {
                    sCheckWord.sprintf("3A%02X0D080030313030303030303030", iAddArrayTFT[iAddrFix]);
                    if(sReadBuffer.Pos(sCheckWord)==1)
                    {
                        AnsiString sVer=sReadBuffer.SubString(31,2);
                        iVersion[Addr]=atoi(sVer.c_str())-30;
                    }
                    else
                        iVersion[Addr]=0;
                }
                else
                {
                    if(iAddrFix>=10)
                        sCheckWord.sprintf(":%02X03020001", iAddrFix+38);
                    else
                        sCheckWord.sprintf(":%02X03020001", iAddrFix+32);

                    if(AUTO_EMPTY_COLOR>=3 &&
                       ((Addr>=eBinDispFix1 && Addr<=eBinDispFix6) ||
                        (Addr>=eBinDispFix7 && Addr<=eBinDispFix12)))
                    {
                        if(sReadBuffer2.Pos(sCheckWord)==1)                     //舊模組
                        {
                            iVersion[Addr]=1;
                        }
                        else
                        {
                            if(iAddrFix>=10)
                                sCheckWord.sprintf(":%02X03020002", iAddrFix+38);
                            else
                                sCheckWord.sprintf(":%02X03020002", iAddrFix+32);
                            if(sReadBuffer2.Pos(sCheckWord)==1)                 //新模組
                            {
                                iVersion[Addr]=2;
                            }
                            else
                            {
                                iVersion[Addr]=0;
                            }
                        }
                    }
                    else
                    {
                        if(sReadBuffer.Pos(sCheckWord)==1)                      //舊模組
                        {
                            iVersion[Addr]=1;
                        }
                        else
                        {
                            if(iAddrFix>=10)
                                sCheckWord.sprintf(":%02X03020002", iAddrFix+38);
                            else
                                sCheckWord.sprintf(":%02X03020002", iAddrFix+32);

                            if(sReadBuffer.Pos(sCheckWord)==1)                  //新模組
                            {
                                iVersion[Addr]=2;
                            }
                            else
                            {
                                iVersion[Addr]=0;
                            }
                        }
                    }
                }

                if(iVersion[Addr]!=0)
                {
                    bHasUnitArray[Addr]=true;
                    bGetStatus[Addr]=false;
                    iErrCount[Addr]=0;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>5)
            {
                iRusStatus=eBDP_DispErr;                                        //Sam 20240604 : 新增 BinDisplay TFT
                iErrCount[Addr]=0;
                if(IniConfig.bG16BinDispNeedAlarm==true)                        //Steven 20211220 : 修正G16, Bin顯示器異常要alarm
                {
                    if(Addr>=3)
                    {
                        bHasError[Addr]=true;                                   // 顯示器是否有出錯
                    }
                    else
                    {
                        bHasUnitArray[Addr]=false;
                        bGetStatus[Addr]=false;
                    }
                }
                else
                {
                    bHasUnitArray[Addr]=false;
                    bGetStatus[Addr]=false;
                }
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoOnce()                                                 //Sam 20240604 : 新增 BinDisplay TFT
{
    int &Task=iOnceTask;
    static int iSetType=0;
    iRusStatus=eBDP_BinSet;
    switch(Task)
    {
        case 1:
            iSetType=0;
            Task=100;
            DoOnceTFT(true, iSetType);
        case 100:
            if(DoOnceTFT(false, iSetType))
            {
                if(iSetType<7)
                {
                    iSetType++;
                    DoOnceTFT(true, iSetType);
                }
                else
                {
                    return true;
                }
            }
            break;
    }
    return false;

}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoCycle()                                                //Sam 20240604 : 新增 BinDisplay TFT
{
    int &Task=iCycleTask;
    static int iWriteType=0;
    static int iCntCycle=0;
    iRusStatus=eBDP_BinRun;
    switch(Task)
    {
        case 1:
            iWriteType=0;
            Task=100;
            DoCycleTFT(true, iWriteType);
            for(int i=0; i<MAX_BIN_UNIT; i++)
            {
                iCountTFT[i]=0;
                if(bHasUnitArray[i])
                {
                    bSliding[i]=true;
                    bSliding_Cnt[i]=true;
                }
            }
            break;
        case 50:
            if(BinDisCycleDelay.Off())
            {
                iWriteType=0;
                iCntCycle=0;
                Task=100;
                DoCycleTFT(true, iWriteType);
            }
            break;
        case 100:
            if(DoCycleTFT(false, iWriteType))
            {
                if(iWriteType==1 && iCntCycle<2)                                //Count 更新顯示3次後，才輪巡一次 Bin
                {
                    iCntCycle++;
                    DoCycleTFT(true, iWriteType);
                }
                else if(iWriteType<1)
                {
                    iWriteType++;
                    DoCycleTFT(true, iWriteType);
                }
                else
                {
                    Task=50;
                    BinDisCycleDelay.SetSecAndOn(dDelaySec/10.0);
                    return true;  
                }
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoOnceTFT(bool bReset, int iSetType)                     //Sam 20240604 : 新增 BinDisplay TFT
{
    int &Task=iOnceTFTTask;
    AnsiString sCheckWord="";
    static int iErrCount[MAX_BIN_UNIT]={0};
    static bool bSet[MAX_BIN_UNIT]={false};

    if(bReset)
    {
        iOnceTFTTask=1;
        return false;
    }

    switch(Task)
    {
        case 1:
            Addr=0;
            ZeroMemory(iErrCount, sizeof(iErrCount));
            ZeroMemory(bSet, sizeof(bSet));
        case 100:
            while(1)
            {
                if(Addr>=MAX_BIN_UNIT)
                    return true;
                else if((Addr>=eBinDispMag1 && Addr<=eBinDispMag14))
                    bSet[Addr]=false;
                else if(bHasUnitArray[Addr])
                    bSet[Addr]=true;

                if(bSet[Addr]==true && bHasUnitArray[Addr])
                {
                    if(iSetType==0)
                        SetNoBackGround_TFT(Addr);
                    else if(iSetType==1)
                        SetBackGround_TFT (Addr);
                    else if(iSetType==2)
                        SetFontBin_TFT    (Addr, iSetColor[Addr], iSetBin[Addr][0]);
                    else if(iSetType==3)
                        SetFontBinWord_TFT(Addr, iSetColor[Addr], iSetBin[Addr][0]);
                    else if(iSetType==4)
                        SetFontEA_TFT     (Addr, iSetColor[Addr], iSetBin[Addr][0]);
                    else if(iSetType==5)
                        SetFontCount_TFT  (Addr, iSetColor[Addr], iSetBin[Addr][0]);
                    else if(iSetType==6)
                        WriteBinWord_TFT  (Addr, iSetBin[Addr][0]);
                    else if(iSetType==7)
                        WriteEA_TFT       (Addr, iSetBin[Addr][0]);
                    BinDispRecv=false;
                    BinDisDelay.SetSecAndOn(2);
                    Task=200;
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv)
            {
                if(iSetType==0)
                    sCheckWord.sprintf("3A%02X0D00040001303030303000000700", iAddArrayTFT[Addr]);
                else if(iSetType==1)
                    sCheckWord.sprintf("3A%02X", iAddArrayTFT[Addr]);
                else if(iSetType==2)
                    sCheckWord.sprintf("3A%02X0D00020001", iAddArrayTFT[Addr]);
                else if(iSetType==3)
                    sCheckWord.sprintf("3A%02X0D00020002", iAddArrayTFT[Addr]);
                else if(iSetType==4)
                    sCheckWord.sprintf("3A%02X0D00020003", iAddArrayTFT[Addr]);
                else if(iSetType==5)
                    sCheckWord.sprintf("3A%02X0D00020004", iAddArrayTFT[Addr]);
                else if(iSetType==6)
                    sCheckWord.sprintf("3A%02X0D00030002", iAddArrayTFT[Addr]);
                else if(iSetType==7)
                    sCheckWord.sprintf("3A%02X0D00030003", iAddArrayTFT[Addr]);

                if(sReadBuffer.Pos(sCheckWord)==1)
                {
                    bHasError[Addr]=false;                                      //模組沒有出錯
                    iErrCount[Addr]=0;
                    bSet[Addr]=false;
                    Addr++;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>5)
            {
                iRusStatus=eBDP_DispErr;
                bHasError[Addr]=true;
                iErrCount[Addr]=0;
                CommBin->StopComm();
                InitialTask();
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyBinDispHT9046::DoCycleTFT(bool bReset, int iWriteType)                  //Sam 20240604 : 新增 BinDisplay TFT
{
    int &Task=iCycleTFTTask;
    AnsiString sCheckWord="";
    static int iErrCount[MAX_BIN_UNIT]={0};
    static bool bSet[MAX_BIN_UNIT]={false};

    if(bReset)
    {
        iCycleTFTTask=1;
        return false;
    }

    switch(Task)
    {
        case 1:
            Addr=0;
            ZeroMemory(iErrCount, sizeof(iErrCount));
            ZeroMemory(bSet, sizeof(bSet));
        case 100:
            while(1)
            {
                if(Addr>=MAX_BIN_UNIT)
                    return true;
                else if((Addr>=eBinDispMag1 && Addr<=eBinDispMag14))
                    bSet[Addr]=false;                    
                else if(bHasUnitArray[Addr])
                    bSet[Addr]=true;

                bool bdispFix1=(Addr==eBinDispFix1);                                            //Eastsun 20260917
                bool bdispFix2=(Addr==eBinDispFix2);
                bool bdispFix3=(Addr==eBinDispFix3);
                bool bdispFix4=(Addr==eBinDispFix4);                                            //Eastsun 20260917
                bool bdispFix5=(Addr==eBinDispFix5);
                bool bdispFix6=(Addr==eBinDispFix6);
                if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)   //Eastsun 20260917
                {
                    bdispFix2=false;
                    bdispFix3=false;
                    bdispFix4=false;
                    bdispFix5=false;
                    bdispFix6=false;
                }

                if(NUMBER_PANEL_TYPE==4 && MAGAZINE_BIN_DISP_TYPE==eTFT && (Addr==eBinDispAuto1 || Addr==eBinDispAuto2 ||
                   bdispFix1 || bdispFix2 || bdispFix3 || bdispFix4 || bdispFix5 || bdispFix6)) //Eastsun 20260917 : 修正FIX123 數值不會變化問題
                {
                    bSet[Addr]=true;
                    bSliding[Addr]=true;
                }

                if((iWriteType==0 && bSet[Addr]==true && bHasUnitArray[Addr] && bSliding[Addr]==true) ||
                   (iWriteType==1 && bSet[Addr]==true && bHasUnitArray[Addr] && bSliding_Cnt[Addr]==true))
                {
                    if(iSetBin[Addr][iCountTFT[Addr]]==-1)
                    {
                        iCountTFT[Addr]=0;
                    }

                    if(iWriteType==0)
                        WriteBin_TFT    (Addr,  iSetBin[Addr][iCountTFT[Addr]]);
                    else if(iWriteType==1)
                        WriteCount_TFT  (Addr,  iSetBin[Addr][iCountTFT[Addr]], iSetCount[Addr]);

                    BinDispRecv=false;
                    BinDisCycleDelay.SetSecAndOn(2);
                    Task=200;
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv)
            {
                if(iWriteType==0)
                    sCheckWord.sprintf("3A%02X0D00030001", iAddArrayTFT[Addr]);
                else if(iWriteType==1)
                    sCheckWord.sprintf("3A%02X", iAddArrayTFT[Addr]);

                if(sReadBuffer.Pos(sCheckWord)==1)
                {
                    if(iWriteType==0)
                    {
                        if(iSetBin[Addr][0]==-1 || iSetBin[Addr][1]==-1)        //如果是X 表示該位置沒有設Bin，不需要輪撥
                            bSliding[Addr]=false;
                        if(Addr<3)                                              //L / E /C 不需要輪撥
                            bSliding[Addr]=false;

                        iCountTFT[Addr]++;
                        if(Addr<3)
                            bSetBin[Addr]=false;

                        if(iSetBin[Addr][iCountTFT[Addr]]==-1 ||                //如果是0 表示已經到最後一個Bin，所以從頭開始Show
                           iCountTFT[Addr]>=iTestBinCount)                      //Steven 20140404 : MAX_BIN_UNIT --> iTestBinCount
                        {
                            iCountTFT[Addr]=0;
                        }
                    }
                    else if(iWriteType==1)
                    {
                        if(iSetBin[Addr][0]==-1)
                            bSliding_Cnt[Addr]=false;
                        if(Addr<3)                                              //L / E /C 不需要輪撥顯示 Count
                            bSliding_Cnt[Addr]=false;
                    }
                    bHasError[Addr]=false;                                      //模組沒有出錯
                    iErrCount[Addr]=0;
                    bSet[Addr]=false;
                    Addr++;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisCycleDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>5)
            {
                iRusStatus=eBDP_DispErr;
                bHasError[Addr]=true;
                iErrCount[Addr]=0;
                CommBin->StopComm();
                InitialTask();
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
void __fastcall TDataModule3::DataModuleDestroy(TObject *Sender)
{
    BinDisp->StopComm();
}
//------------------------------------------------------------------------------



//==> Eastsun 20260513 KYEC: AutoChanging flash centralised implementation (v2.0 minimal)
//---------------------------------------------------------------------------
// StartFlash: pin this BinDisp to flash between colorA / colorB
//---------------------------------------------------------------------------
void StartFlash(int Index, int colorA, int colorB, int periodMs)
{
    if(Index<0 || Index>=MAX_BIN_UNIT)
        return;
    if(periodMs < 50)
        periodMs = 50;
    bFlash[Index]         = true;
    iFlashColorA[Index]   = colorA;
    iFlashColorB[Index]   = colorB;
    iFlashPeriodMs[Index] = periodMs;
    bFlashShowA[Index]    = true;
    bFlashCom=true;
}
//---------------------------------------------------------------------------
// IsAnyFlashing: any BinDisp currently flashing?
//---------------------------------------------------------------------------
//bool IsAnyFlashing(void)
//{
//    for(int k=0; k<MAX_BIN_UNIT; k++)
//        if(bFlash[k])
//            return true;
//    return false;
//}
//---------------------------------------------------------------------------
// ProcessFlash: called by Timer1Timer; toggles color for each flashing BinDisp
//---------------------------------------------------------------------------
void TMyBinDispCtrl::ProcessFlash(void)                                         //Eastsun 20260513 : 閃爍功能
{
    if(!IniConfig.bP53AutoChangingFlashWarn)
        return;
    if(!IsAnyFlashing())
        return;
    for(int k=0; k<MAX_BIN_UNIT; k++)
    {
        if(!bFlash[k])
        {
            bSetColor[k] = false;
            continue;
        }

        //if(FlashTimer[k].Off())
        bSetColor[k] = true;
        if(bFlashCom)
        {
            bFlashShowA[k] = !bFlashShowA[k];
            iFlashColorA[k]++;
            if(iFlashColorA[k]>3)
                iFlashColorA[k]=1;
            //iSetColor[k] = bFlashShowA[k] ? iFlashColorA[k] : iFlashColorB[k];
            iSetColor[k] = iFlashColorA[k];
            bFlashCom=false;
            //FlashTimer[k].SetMSAndOn(iFlashPeriodMs[k]);
        }
    }
}
//------------------------------------------------------------------------------
//==> Eastsun 20260513 KYEC 防工傷 Auto1/Auto2 換盤閃爍警示 helper
void TMyBinDispCtrl::ClearAutoChangingWarn(int AutoTarget)
{
    if(!IniConfig.bP53AutoChangingFlashWarn)
         return;
    //Eastsun 20260513 KYEC: 放寬到 Auto1/Auto2/Auto3         
    if(AutoTarget < 0 || AutoTarget > 2)
         return;
    if(!bAutoChangingWarn[AutoTarget])
        return;

    bAutoChangingWarn[AutoTarget] = false;

    for(int k=0; k<MAX_BIN_UNIT; k++)
    {
        if(bFlash[k])
        {
            iSetColor[k] = iAutoColorBak[AutoTarget];
            bFlash[k]=false;
        }
    }
    iStartSetColorTask=1;
    iBinDispCtrlTask=200;
    //bFlashCom=false;
}
//<== Eastsun 20260513
//--------------------------------------------------------------------------
//==> Eastsun 20260513 KYEC: AutoChange-tray flashing warn entry (v1.0 central scanner)
    //Eastsun 20260513 KYEC: central scanner in BinDispCtrl, 250ms toggle, color-only (no bin re-send)
void TMyBinDispCtrl::FlashPro(int AutoTarget)
{
    bool bFlashThis = (AutoTarget==0 || AutoTarget==1);
    if(AutoTarget==2 && MAGAZINE_BIN_DISP_TYPE==eMagBinUninstall)
        bFlashThis = true;
    if(IniConfig.bP53AutoChangingFlashWarn && bFlashThis && !bAutoChangingWarn[AutoTarget])
    {
        int iBdIdx;
        if(AutoTarget==0)
            iBdIdx = eBinDispAuto1;
        else if(AutoTarget==1)
            iBdIdx = eBinDispAuto2;
        else
            iBdIdx = eBinDispAuto3;
        iAutoColorBak[AutoTarget] = HSys.BinDisCtrl->GetColorNow(iBdIdx);
        bAutoChangingWarn[AutoTarget] = true;
        StartFlash(iBdIdx, 1 , 4 , 250);
    }
}
//--------------------------------------------------------------------------



