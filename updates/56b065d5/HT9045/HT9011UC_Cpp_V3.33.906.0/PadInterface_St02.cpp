// =============================================================================
//  PadInterface_St02.cpp  --  RS-232 operator pad: protocol, serial port owner, serve-loop job
//
//  AI(W906-ST02-P1) 20261005 (St02-E): golden 0618 uPadInterface.cpp (protocol half) + the pad part of
//  Motor/TrayStepMotor.cpp (RS232Init :56-87, comTrayStepMotorReceiveData :402-513) + rs232.cpp :221-234 (boot port
//  check).  The inline half (tables, the four gate methods) is PadInterface_St02.h -- read its header first.
//
//  Port-tree adaptations (interface, not behaviour; every one also in the MR text):
//   (1) dmTrayMotor / TrayStepMotor is not ported (only dfm2rc metadata).  This file owns golden's
//       `dmTrayMotor->comTrayStepMotor` (one vclcompat Spcomm::TComm, golden TrayStepMotor.dfm:15-42 values).
//       t07 (tray step motor) / t08 (vibration motor) frames are counted and dropped: those drivers are not ported.
//       DoTrayStepMotor() (golden uPadInterface.cpp:897) is not called for the same reason.
//   (2) TPadRS232Thread (golden :31-49, Synchronize(Main232) + SleepEx(1)) = a 1 ms kDelay job on the serve loop's
//       FastClock (W906_PadFastClockAdd in PadInterfaceClock_St02.cpp, registered only when ControlPanelMode==1 at start, as golden creates the
//       thread only then, main.cpp:22479 / :10142).  FastClock is serviced inside the blocking boxes too, which is
//       golden's mymessbox.cpp:557 / note.cpp:3379 Main232 pump (「解決 Alarm 無法按按鈕」).  golden's Suspend at
//       program close (main.cpp:11638) has no counterpart: the job ends with the loop.
//   (3) vclcompat TComm calls OnReceiveData on its reader thread (SPComm posts to the main thread) and does not
//       NUL-terminate (survey T2/T3).  The reader thread only queues each received chunk whole; W906_PadThreadTick
//       (main thread) hands every chunk to golden's receive handler, ONE CALL PER CHUNK as golden: no buffer across
//       chunks (AI(W906-ST02-P1b) 20261006, Ifor01's !221 review: the old cross-chunk buffer had no bound and held a
//       CR-less tail until the next CR).  Golden semantics kept: the unit is the chunk's first 3 characters
//       (TrayStepMotor.cpp:410), a t05 chunk goes to CommReceiveList whole, and Main232's do...while
//       (uPadInterface.cpp:716-745) parses every CR-terminated frame in it; a chunk with no CR is parsed once, a
//       CR-less tail after the last CR is dropped unparsed.  The pad and the reader both use ReadIntervalTimeout=1 ms,
//       the same split exposure SPComm had (a frame splits only if the pad pauses > 1 ms inside it).
//   (4) vclcompat StartComm silently falls back to a simulated port when the real one cannot be opened (survey T1);
//       SPComm raises.  A SIM fallback is therefore treated as an open failure (bRs232Ok stays false, Main232 case
//       20 retries every ~11 s as golden).  Tests force SIM through W906_PadPortForTest.
//   (5) StrToInt("0x"+key) throws on a bad frame; golden's exception would leave Synchronize.  Here it is caught
//       per frame, logged as [Recv Error], and the next frame is processed.
//   (6) ControlPanelMode==0 (every machine today) = byte-for-byte unchanged: the boot open, the boot port check and
//       the job all return / are skipped at mode 0 (golden also opens the shared port for a step-motor-only or
//       vibration-only machine; those drivers are not ported, so nothing would use it).
// =============================================================================
#include "PadInterface_St02.h"
#include "MachineType.h"            // SOFT_SIMULTE
#include "myTimer.h"                // TQPF_Timer
#include "cmydef.h"                 // InitialOK / iControlPanelMode / LoaderUnload_StepMotor / SystemYear…
#include "common.h"                 // asPadCommLogPath / MyForceDirectories
#include "cpublic.h"                // GetTimeInfo
#include "cprod.h"                  // CheckFileExist
#include "database.h"               // HSys.TrayStepMotor_ComPort
#include "EJ1N/TextProcess.h"       // GetCOMPortStatus
#include "canary_support.h"         // ShowMyMessage
#include "vclcompat/Comm.h"
#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

extern void MyDBIProcess(AnsiString S1, AnsiString S2);                         // golden cMyDB.h (same forward declaration as rs232.cpp)
extern Word SystemHour, SystemMin, SystemSec, SystemMSec;

//------------------------------------------------------------------------------ golden uPadInterface.cpp:25-29
static TQPF_Timer tSendDataDelay;
static int iReceiceTask=1;

//------------------------------------------------------------------------------ the shared port (golden dmTrayMotor->comTrayStepMotor)
namespace {
struct PadPortRx {
    CRITICAL_SECTION cs;
    std::vector<std::string> chunks;                                            // one entry per OnReceiveData call (golden: one ReceiveData call each) (3)
    PadPortRx() { ::InitializeCriticalSection(&cs); }
};
PadPortRx& Rx() { static PadPortRx r; return r; }

Spcomm::TComm* g_com          = 0;
bool           g_testSim      = false;                                          // W906_PadPortForTest: SIM is intended, not a failure
unsigned long  g_droppedT07T08 = 0;                                             // (1)

void QueueRx(void* Buffer, unsigned short BufferLength)                         // reader thread: only queue (3)
{
    PadPortRx& r=Rx();
    ::EnterCriticalSection(&r.cs);
    r.chunks.push_back(std::string((const char*)Buffer, BufferLength));
    ::LeaveCriticalSection(&r.cs);
}

Spcomm::TComm* Com()                                                            // golden TrayStepMotor.dfm:15-42
{
    if(g_com==0)
    {
        g_com=new Spcomm::TComm(0);
        g_com->CommName="COM2";
        g_com->BaudRate=115200;
        g_com->ParityCheck=false;
        g_com->Outx_XonXoffFlow=false;
        g_com->Inx_XonXoffFlow=false;
        g_com->ByteSize=Spcomm::_8;
        g_com->Parity=Spcomm::None;
        g_com->StopBits=Spcomm::_1;
        g_com->ReadIntervalTimeout=100;                                         // RS232Init sets 1 (golden :71)
        g_com->OnReceiveData=[](vclcompat::TObject* /*Sender*/, void* Buffer, Spcomm::Word BufferLength) { QueueRx(Buffer, BufferLength); };
    }
    return g_com;
}

bool OpenFailedToSim()                                                          // (4)
{
    return Com()->IsSimMode() && g_testSim==false;
}

// golden TdmTrayMotor::comTrayStepMotorReceiveData (Motor/TrayStepMotor.cpp:402-513), the t05 arm, per received chunk (3)
void DispatchChunk(const std::string& raw)
{
    AnsiString sReciveData = raw.c_str();                                       // golden `(char *)Buffer`: stops at a NUL
    AnsiString sUnit;

    sUnit=sReciveData.SubString(1, 3);
    if(sUnit=="t05")                                                            //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    {
        fPadInterface->CommReceiveLength.push_back((int)raw.size());
        fPadInterface->CommReceiveList.push_back(sReciveData);                 //Sam 20220617 asCommand >> sReciveData

        fPadInterface->RecordCommunication("[Recv]", sReciveData);             //Sam 20220617 asCommand >> sReciveData
    }
    else if(sUnit=="t07" || sUnit=="t08")                                       // tray step motor / vibration motor: not ported (1)
    {
        g_droppedT07T08++;
    }
}

void DrainRx()                                                                  // main thread (3)
{
    std::vector<std::string> got;
    PadPortRx& r=Rx();
    ::EnterCriticalSection(&r.cs);
    got.swap(r.chunks);                                                         // everything queued is handed over now: nothing waits for a CR
    ::LeaveCriticalSection(&r.cs);

    for(std::vector<std::string>::size_type i=0; i<got.size(); i++)
        DispatchChunk(got[i]);
}
}  // namespace

//==============================================================================  golden uPadInterface.cpp:281-315
bool TfPadInterface::OpenCommPort()
{
    if(bRs232Ok)
        return true;

    try
    {
        if(g_testSim==false)
        {
            HANDLE handle = CreateFile(Com()->CommName.c_str(),                 //KenHsieh 20211222 : Pad與步進馬達為同一Comport
                                       GENERIC_READ | GENERIC_WRITE,
                                       0,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                       0);

            if(handle==INVALID_HANDLE_VALUE)
            {
                RecordCommunication("[Connect]", " INVALID_HANDLE_VALUE");
                return false;
            }

            CloseHandle(handle);
        }
        Com()->StartComm();                                                     //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        if(OpenFailedToSim())                                                   // (4) SPComm would have raised here
        {
            Com()->StopComm();
            throw 0;
        }
        RecordCommunication("[Connect]", " OK");
        bRs232Ok=true;
    }
    catch(...)
    {
        RecordCommunication("[Connect]", " FAIL");
        bRs232Ok=false;
        return false;
    }

    return true;
}
//==============================================================================  golden :319-334
bool TfPadInterface::CloseCommPort()
{
    try
    {
        Com()->StopComm();                                                      //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        RecordCommunication("[Disconnect]", " OK");
        bRs232Ok=false;
    }
    catch(...)
    {
        RecordCommunication("[Disconnect]", " FAIL");
        return false;
    }

    return true;
}
//==============================================================================  golden :338-352
void TfPadInterface::PadWriteDataToFile(AnsiString cFilePath, AnsiString cData)
{
    if(CheckFileExist(cFilePath)==false)
        return;

    FILE *pFile;
    pFile=fopen(cFilePath.c_str(), "a+");

    if(pFile!=NULL)
    {
        fputs(cData.c_str(), pFile);
        fputs("\n", pFile);
        fclose(pFile);
    }
}
//==============================================================================  golden :356-376
void TfPadInterface::RecordCommunication(AnsiString aTitle, AnsiString Command)
{
    AnsiString sFileName="", sMegTime="", asLog;

    GetTimeInfo();
    sFileName.sprintf("%s\\%04d\\%02d\\%02d", asPadCommLogPath, SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(sFileName);

    sFileName.sprintf("%s\\%04d\\%02d\\%02d\\%02d%02d.txt", asPadCommLogPath, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
    sMegTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d %03d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
    asLog.sprintf("%s,   %s ==> %s", sMegTime, aTitle, Command);

    if((int)MemoLines.size()>=1000)                                             // golden Memo_PadInterface->Lines->SaveToFile (overwrites)
    {
        FILE *pFile=fopen(sFileName.c_str(), "wb");
        if(pFile!=NULL)
        {
            for(size_t i=0; i<MemoLines.size(); i++)
            {
                fputs(MemoLines[i].c_str(), pFile);
                fputs("\r\n", pFile);
            }
            fclose(pFile);
        }
        MemoLines.clear();
    }
    MemoLines.push_back(asLog);
    PadWriteDataToFile(sFileName, asLog);
}
//==============================================================================  golden :378-383
void TfPadInterface::ResetComm()
{
    CloseCommPort();
    Com()->CommName="\\\\.\\"+HSys.TrayStepMotor_ComPort;                       //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    OpenCommPort();
}
//==============================================================================  golden :396-415
void TfPadInterface::RequestPadVersion()
{
    AnsiString sData;
    RequestData.clear();

    RequestData.push_back('t');
    RequestData.push_back('0');
    RequestData.push_back('5');
    RequestData.push_back('1');
    RequestData.push_back('1');
    RequestData.push_back('2');
    RequestData.push_back('0');

    for(int i=0; i<(int)RequestData.size(); i++)
    {
        sData+=(AnsiString)RequestData[i];
    }
    bRequestVer=false;
    SendCommand(sData);
}
//==============================================================================  golden :492-510
void TfPadInterface::SendCommand(AnsiString sData)
{
    if(bRs232Ok==false)
    {
        RecordCommunication("[Send]", sData);
        RecordCommunication("[Connect Error]", " Send Fail");
        return;
    }

    int iSize=sData.Length()+1;
    if(iSize<=0)
    {
        return;
    }

    sData=sData+AnsiString('\r');
    std::string w(sData.c_str());                                               // WriteCommData takes char*
    Com()->WriteCommData(&w[0], (Spcomm::Word)iSize);                           //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    RecordCommunication("[Send]", sData);
}
//==============================================================================  golden :545-665
void TfPadInterface::DoScanPanelLed(int iAddress, int iKey)
{
    int iOldAddress=iAddress;
    if(iAddress>0)                                                              //KenHsieh 20220308 : 新增為三個面板
         iAddress=1;

    if(bShow==false)                                                            //KenHsieh 20220401 : Pad頁面顯示不判斷
    {                                                                           //Sam 20220310 : 修正 SafeLock & PAD_PannelEnable  問題
        if(iOldAddress==1)                                                      //Jimmychiu 20260304 : Fixed for 3rd Pad enable pannel error
        {
            if(iKey==PAD_PannelEnable)                                          //KenHsieh 20220401 : Enable優先判斷
            {
                PadItem[epn_SnRearPadActive].mlEvent->Value=true;
                SendSwitchStatus("SwRearActiveLed", true);
            }
            else if(iKey==0)
            {
                PadItem[epn_SnRearPadActive].mlEvent->Value=false;
                SendSwitchStatus("SwRearActiveLed",false);
            }
        }

        if(PadItem[epn_SnRKSafeLock].mlEvent->Value==false)                     //後面板 SafeLock 狀態
        {
            if(PAD_SafeLock&iKey || iKey==4 || iKey==0)                         //iKey=4 is unlock
            {
                //解除 SafeLock
            }
            else
            {
                return;
            }
        }
        else if(PadItem[epn_SnRearPadActive].mlEvent->Value==false)             //後面板 PannelEnable 狀態
        {
            if(iAddress>0)                                                      //後面板所有按鍵都失效
            {
                if((PAD_PannelEnable&iKey) || (PAD_SafeLock&iKey))              //除了 PAD_PannelEnable & PAD_SafeLock
                {
                }
                else if(iKey&PAD_Pause)                                         //除了 PAD_Psuse    //KenHsieh 20220401 : 新增Pause
                {
                    PadItem[17].mlEvent->Value=true;
                    return;
                }
                else if(iKey&PAD_AlarmReset)                                    //除了 PAD_AlarmReset
                {
                    PadItem[epn_SnRKAlarmReset].mlEvent->Value=true;
                    return;
                }
                else
                {
                    PadItem[epn_SnRKPause].mlEvent->Value=false;                //kevin 20220624 add clean pause
                    PadItem[epn_SnRKAlarmReset].mlEvent->Value=false;           //kevin 20220624 add clean AlarmReset
                    return;
                }
            }
        }
        else
        {
            if(iAddress==0)                                                     //前面板所有按鍵都失效
            {
                if(iKey&PAD_Pause)                                              //除了 PAD_Psuse //KenHsieh 20220401 : 新增Pause
                {
                    PadItem[epn_SnFKPause].mlEvent->Value=true;
                    return;
                }
                else if(iKey&PAD_AlarmReset)                                    //除了 PAD_AlarmReset
                {
                    PadItem[epn_SnFKAlarmReset].mlEvent->Value=true;
                    return;
                }
                else
                {
                    PadItem[epn_SnFKPause].mlEvent->Value=false;                //kevin 20220624 add clean pause
                    PadItem[epn_SnFKAlarmReset].mlEvent->Value=false;           //kevin 20220624 add clean AlarmReset
                    return;
                }
            }
        }
    }

    for(int i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].iData&iKey)
        {
            if(iOldAddress==2 &&
               PadItem[i].iData&PAD_PannelEnable)                               //Jimmychiu 20260304 : Fixed for 3rd Pad enable pannel error
            {
                //pass
            }
            else if(PadItem[i].iData==PAD_SafeLock)
            {
                PadItem[i].mlEvent->Value=false;
                SendSwitchStatus("SwRKSafeLock", true);                         //KenHsieh 20211228 : 區分實體IO與通訊面板
            }
            else if(PadItem[i].mlEvent->Tag==iAddress)
            {
                PadItem[i].mlEvent->Value=true;
            }
            else if(iOldAddress==2 &&
                   PadItem[i].mlEvent->Tag==iOldAddress)
            {
                  PadItem[i].mlEvent->Value=true;
            }
        }
        else
        {
            if((iKey&PAD_SafeLock)==false &&
               PadItem[i].InputName=="SnRKSafeLock")                            //KenHsieh 20211221 : 因SafeLock按鈕從復歸型改為保持型，故修改判斷方式
            {
                PadItem[i].mlEvent->Value=true;
                SendSwitchStatus("SwRKSafeLock", false);                        //KenHsieh 20211228 : 區分實體IO與通訊面板
            }
            else
            {
                PadItem[i].mlEvent->Value=false;
            }
        }
    }
}
//==============================================================================  golden :667-694
void TfPadInterface::DoUpdataPadStatus(int iAddress, int iKey)
{
    int iTag=(iAddress>0)?1:0;
    for(int i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].mlEvent->Tag==iTag)                                       //KenHsieh 20211221 : 新增前後面板判斷，避免前後同顆按鈕操作時顯示錯誤
        {
            if(PadItem[i].iData&iKey)
            {
                if(PadItem[i].PadName=="SwFrontActiveLed" ||
                   PadItem[i].PadName=="SwRKSafeLock"     ||                    //KenHsieh 20211228 : 區分實體IO與通訊面板
                   PadItem[i].PadName=="SwRKManualStep"   ||
                   PadItem[i].PadName=="SwRKManualTStart")
                {
                    bPadStatus[i]=true;
                }
                else if(PadItem[i].mlEvent->Tag==iAddress)
                {
                    bPadStatus[i]=true;
                }
            }
            else
            {
                bPadStatus[i]=false;
            }
        }
    }
}
//==============================================================================  golden :696-749
void TfPadInterface::ProcessReceiceData()
{
    int &iTask=iReceiceTask;
    int iAddress, iPadKey;
    AnsiString S, sAddress, aPadKey;

    if(CommReceiveLength.size()==0)
    {
        CommReceiveList.clear();
        return;
    }

    switch(iTask)
    {
        case 1:
            aReciveData=CommReceiveList[0];
            CommReceiveList.erase(CommReceiveList.begin());
            CommReceiveLength.erase(CommReceiveLength.begin());
            iTask=10;
            // fall through (golden: no break)
        case 10:
            do
            {
                try                                                             // (5)
                {
                    S=aReciveData.SubString(6, 2);  { extern void (*g_W906PadNote)(const char*); if(g_W906PadNote && !S.Pos("90")) g_W906PadNote(aReciveData.c_str()); }   //AI(W906-PADNOTE) 20261007: EastSun「實體按鈕案home還是沒反應」-- every packet from the pad that is not the lamp echo ("90") goes to the op log at once (the pad log only reaches disk every 1000 lines); end of file
                    if(S.Pos("00") && aReciveData.Length()>=14)                 //檢查是否是INPUT訊號
                    {
                        sAddress =aReciveData.SubString(4, 1);
                        iAddress =atoi(sAddress.c_str());                       //檢查判斷前後面板

                        aPadKey  =aReciveData.SubString(8, 6);                  //檢查按鈕
                        iPadKey  =StrToInt("0x"+aPadKey);

                        DoScanPanelLed(iAddress,iPadKey);
                    }
                    else if(S.Pos("20") && aReciveData.Length()>=8)
                    {
                        bRequestVer = true;
                    }
                    else if(S.Pos("90") && aReciveData.Length()>=14)            //檢查是否是INPUT訊號
                    {
                        bSendSwitchStatusing=false;
                        sAddress =aReciveData.SubString(4, 1);
                        iAddress =atoi(sAddress.c_str());                       //檢查判斷前後面板

                        aPadKey  =aReciveData.SubString(8, 6);                  //檢查按鈕
                        iPadKey  =StrToInt("0x"+aPadKey);
                        DoUpdataPadStatus(iAddress,iPadKey);
                    }
                }
                catch(...)
                {
                    RecordCommunication("[Recv Error]", aReciveData);
                }
                aReciveData = aReciveData.Delete(1, aReciveData.Pos("\r"));
                aReciveData.Trim();                                             // golden: result unused (no effect)
            }while(aReciveData.Pos("\r")>0);
            iTask=1;
            break;
    }
}
//==============================================================================  golden :751-847
bool TfPadInterface::ProcessSendDataNew()                                       //丟通訊資料改為固定時間前後面板的 bPadStatus 狀態，若有改變在一次丟整個面板 SW 狀態
{
    static int iTag=0;                                                          //循環切換丟前後面板
    static bool bOldPadStatus[32]={false};
    static bool bRun3rdPannel=false;
    static AnsiString sData1="";
    int iStatue=0x000000;
    bool bNeedSendData=false;                                                   //狀態有改變才需要丟資料
    AnsiString sData="";                                                        //KenHsieh 20220308 : 新增為三個面板

    if(tSendDataDelay.Off())                                                    //JerryYang 20230309 : 不要太密集的送
    {
        if(bRun3rdPannel==true)                                                 //Jimmychiu 20260304 : Fixed for 3rd Pad enable pannel error
        {
            bRun3rdPannel=false;
            SendCommand(sData1);
            sData1="";
            tSendDataDelay.SetMSAndOn(50);
            return true;
        }
        bNeedSendData=false;
        for(int i=0; i<CheckPadItem; i++)
        {
            if(PadItem[i].btnEvent->Tag==iTag)
            {
                if(bOldPadStatus[i]!=bPadStatus[i])                             //檢查狀態是否有改變
                {
                    bOldPadStatus[i]=bPadStatus[i];
                    bNeedSendData=true;
                }

                if(bPadStatus[i])
                {
                    iStatue|=PadItem[i].iData;
                }
            }
        }

        if(bNeedSendData==false)
        {
            if(iTag==0)
                iTag=1;
            else
                iTag=0;
        }
        else
        {
            SendData.clear();

            SendData.push_back('t');
            SendData.push_back('0');
            SendData.push_back('5');
            if(iTag==0)
            {
                SendData.push_back(IntToHex(PAD_FrontControl, 1));
            }
            else
            {
                SendData.push_back(IntToHex(PAD_RearControl, 1));
            }
            SendData.push_back(IntToHex(PAD_ControlDLC, 1));
            SendData.push_back('9');
            if(bPadLedBling)                                                    // golden cb_PadInterface_PadLedBling->Checked
            {
                SendData.push_back(IntToHex(PAD_LedBling, 1));
            }
            else
            {
                SendData.push_back(IntToHex(PAD_LedLight, 1));
            }

            SendData.push_back(IntToHex(iStatue, 6));
            for(int i=0; i<(int)SendData.size(); i++)
            {
                sData+=(AnsiString)SendData[i];
            }
            SendCommand(sData);

            if(bShow)                                                           //KenHsieh 20220402 : 頁面顯示時送出之時間不同
                tSendDataDelay.SetMSAndOn(550);                                 //先試試看 300ms
            else
                tSendDataDelay.SetMSAndOn(50);
            if(sData.Pos("t051")>0)
            {
                sData1=StringReplace(sData, "t051", "t052", TReplaceFlags()<<rfReplaceAll);
                bRun3rdPannel=true;                                             //Jimmychiu 20260304 : Fixed for 3rd Pad enable pannel error
            }

            if(iTag==0)
                iTag=1;
            else
                iTag=0;
            return true;
        }
    }
    return false;
}
//==============================================================================  golden :861-938
void TfPadInterface::Main232()
{
    if(InitialOK==false)
        return;

    if(iControlPanelMode==0)
        return;

    static int Task=1;
    static TQPF_Timer hTimeOut, hDelay;
    static TQPF_Timer StartTime;

    bool bHasSendPad=false;
    int iInterval;

    ProcessReceiceData();

    bHasSendPad=fPadInterface->ProcessSendDataNew();

    if(bHasSendPad)                                                             //JerryYang 20230309 : 不要太密集的送
    {
        return;
    }

    if(bRs232Ok && bScanSwitch)                                                 //KenHsieh 20211224 : 新增按鈕狀態掃描
    {
        SendCommand("t051400000000");
        bScanSwitch=false;
        bHasSendPad=true;
    }

    if(bHasSendPad)                                                             //JerryYang 20230309 : 不要太密集的送
    {
        bHasSendPad=false;
        return;
    }
    // golden :897 dmTrayMotor->DoTrayStepMotor() -- the tray step motor is not ported (file header (1))

    switch(Task)
    {
        case 1:
            StartTime.LatchCycleTimeSec(true);
            Task=10;
//            break;
        case 10:
            iInterval=StartTime.LatchCycleTimeSec();                            //sec

            if(iInterval>=10)
            {
                RequestPadVersion();
                hTimeOut.SetSecAndOn(1);
                Task=20;
            }
            break;
        case 20:
            if(bRs232Ok && bRequestVer)
            {
                Task=1;
            }
            else if(bRs232Ok==false)
            {
                hDelay.SetSecAndOn(0.1);
                ResetComm();
                Task=50;
            }
            else if(hTimeOut.Off())
            {
                Task=1;
            }
            break;
        case 50:
            if(hDelay.Off())
            {
                Task=1;
            }
            break;
    }
}

//==============================================================================  golden Motor/TrayStepMotor.cpp:56-87
void W906_PadPortRS232Init(AnsiString ComPort)
{
    if(iControlPanelMode==0)                                                    // (6)
        return;
    #ifndef SOFT_SIMULTE
    AnsiString Str;

    if(iControlPanelMode && LoaderUnload_StepMotor)                             //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        Str="Tray Step Motor & Control Panel : ";
    else if(LoaderUnload_StepMotor)
        Str="Tray Step Motor : ";
    else if(iControlPanelMode)
        Str="Control Panel : ";

    Com()->BaudRate=115200;
    Com()->CommName="\\\\.\\"+ComPort;
    Com()->ReadIntervalTimeout=1;                                               //Sam 20220617 : 加快控制面板通訊
    try
    {
        Com()->StartComm();                                                     //僅能啟動一次
        if(OpenFailedToSim())                                                   // (4)
        {
            Com()->StopComm();
            throw 0;
        }
        if(iControlPanelMode)                                                   //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        {
            fPadInterface->bRs232Ok=true;
            // golden MyPad232Thread->Resume(): the FastClock job (file header (2))
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TCOM2::RS232Init");
        ShowMyMessage(Str+ComPort+" port error", "");                           //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    }
    #else
    (void)ComPort;
    #endif
}
//==============================================================================  golden rs232.cpp:221-234
void W906_RS232InitPadCheck(bool *flag)
{
    AnsiString Str;

    if(iControlPanelMode==0)                                                    // (6): golden also checks for LoaderUnload_StepMotor alone
        return;
    if(LoaderUnload_StepMotor==true ||                                          //Steven 20200529 : Loader入Tray改步進
       iControlPanelMode)
    {
        if(iControlPanelMode && LoaderUnload_StepMotor)                         //KenHsieh 20211222 : Pad與步進馬達為同一Comport
            Str="Tray Step Motor & Control Panel : ";
        else if(LoaderUnload_StepMotor)
            Str="Tray Step Motor : ";
        else if(iControlPanelMode)
            Str="Control Panel : ";

        flag[9]=GetCOMPortStatus(HSys.TrayStepMotor_ComPort);                   //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        if(flag[9]==false)
            ShowMyMessage(Str+HSys.TrayStepMotor_ComPort+" port error", "");    //KenHsieh 20211222 : Pad與步進馬達為同一Comport
    }
}
//==============================================================================  golden uPadInterface.cpp:36-40 (+ the drain, (3))
void W906_PadThreadTick()
{
    DrainRx();
    if(iControlPanelMode || LoaderUnload_StepMotor)                             //KenHsieh 20211222 : Pad與步進馬達為同一Comport
        fPadInterface->Main232();
}
// the serve-loop job (file header (2)) is W906_PadFastClockAdd in PadInterfaceClock_St02.cpp: its own TU, because cinitial /
// rs232 pull THIS object into every executable that links ht9045_sm, and most of them do not link FastClock.cpp
//==============================================================================  test seams (ctest only)
Spcomm::TComm* W906_PadPortForTest(bool forceSim)                               // SIM port that counts as open (file header (4))
{
    g_testSim=forceSim;
    if(forceSim)
        Com()->SetSimMode(true);
    return Com();
}
unsigned long W906_PadDroppedT07T08()
{
    return g_droppedT07T08;
}
size_t W906_PadRxQueuedBytesForTest()                                           // AI(W906-ST02-P1b) 20261006: bytes still queued between the reader and the main thread (3)
{
    PadPortRx& r=Rx();
    size_t n=0;
    ::EnterCriticalSection(&r.cs);
    for(std::vector<std::string>::size_type i=0; i<r.chunks.size(); i++)
        n+=r.chunks[i].size();
    ::LeaveCriticalSection(&r.cs);
    return n;
}
void W906_PadResetStateForTest()                                                // statics of Main232 / ProcessSendDataNew stay (golden statics)
{
    PadPortRx& r=Rx();
    ::EnterCriticalSection(&r.cs);
    r.chunks.clear();
    ::LeaveCriticalSection(&r.cs);
    iReceiceTask=1;
    g_droppedT07T08=0;
}

//==============================================================================
//AI(W906-PADNOTE) 20261007: the op-log sink for the pad's key / unknown packets (ProcessReceiceData); tools/wb_serve.cpp W906_OpLogInit sets it, 0 = nothing (ctests)
void (*g_W906PadNote)(const char*) = 0;
//==============================================================================
// AI(W906-W155) 20261007 (St02-E): golden 0618 uPadInterface.cpp:417-474 TfPadInterface::SendSwitchStatus(TBtnPanelLane *bpPtr),
//   verbatim (TBtnPanelLane -> PAD_BTNLANE_W906 {Alias, Down}; cb_PadInterface_PadLedBling->Checked -> bPadLedBling).  Golden calls it
//   from the IO page's BtnPanelClick after every MotionNet / 1203 output click (iosetview.cpp:1095 / :1100); the static iStatue is
//   shared by front and rear (golden quirk, kept).  The frame goes out through SendCommand (the only byte writer, :274-292).
void TfPadInterface::SendSwitchStatus(PAD_BTNLANE_W906 *bpPtr)
{
    AnsiString sData;
    static int iStatue=0x000000;

    for(int i=0; i<CheckPadItem; i++)
    {
        if(AnsiString(PadItem[i].PadName)==bpPtr->Alias)
        {
            if(bpPtr->Down)
            {
                iStatue|=PadItem[i].iData;
                bPadStatus[i]=true;
            }
            else
            {
                if((iStatue&PadItem[i].iData)==PadItem[i].iData)
                    iStatue^=PadItem[i].iData;
                bPadStatus[i]=false;
            }
        }
    }

    SendData.clear();
    SendData.push_back('t');
    SendData.push_back('0');
    SendData.push_back('5');
    if(bpPtr->Alias.Pos("RK")>0 || bpPtr->Alias.Pos("SwRearActiveLed")>0)       //Rear Pad  //KenHsieh 20211221 : Front新增Enable燈號
    {
        SendData.push_back(IntToHex(PAD_RearControl, 1));
    }
    else
    {
        SendData.push_back(IntToHex(PAD_FrontControl, 1));
    }
    SendData.push_back(IntToHex(PAD_ControlDLC, 1));
    SendData.push_back('9');
    if(bPadLedBling)                                                            // golden cb_PadInterface_PadLedBling->Checked
    {
        SendData.push_back(IntToHex(PAD_LedBling, 1));
    }
    else
    {
        SendData.push_back(IntToHex(PAD_LedLight, 1));
    }

    SendData.push_back(IntToHex(iStatue, 6));
    for(int i=0; i<(int)SendData.size(); i++)
    {
        sData+=(AnsiString)SendData[i];
    }
    SendCommand(sData);
    if(sData.Pos("t051")>0)
    {
        AnsiString sData1=StringReplace(sData, "t051", "t052", TReplaceFlags()<<rfReplaceAll);
        SendCommand(sData1);                                                    //Jimmychiu 20260304 : Fixed for 3rd Pad enable pannel
    }
}

// AI(W906-W155) 20261007 (St02-E): the IO page's Panel tab (golden iosetview.cpp ScanLed :2778-2782): with iControlPanelMode==1 a pad
//   key's square is ProcessScanKey(alias) = PadItem[i].mlEvent->Value (uPadInterface.cpp:849-857) and a pad button's lamp is its
//   bPadStatus (what SendSwitchStatus last set).  Returns false (nothing to say) in mode 0 or for any other alias; *on is the RAW pad
//   value -- the caller applies the IO table's InType inversion as golden :2795-2796.
bool W906_PadIoPoint(const AnsiString& alias, bool* on)
{
    if(iControlPanelMode!=1 || on==0)
        return false;
    TfPadInterface* p=fPadInterface;
    if(p->IsPadKey(alias))
    {
        *on=p->ProcessScanKey(alias);
        return true;
    }
    for(int i=0; i<p->CheckPadItem; i++)
    {
        if(AnsiString(p->PadItem[i].PadName)==alias)
        {
            *on=p->bPadStatus[i];
            return true;
        }
    }
    return false;
}
