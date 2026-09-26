// ===========================================================================
//  TesterComm/Gpib/GpibCastleAmdHana.cpp -- three TSerialPoll blocks of the GPIB bridge (H9046_32GPIB.exe):
//
//    (1) AMD / ATC temperature controller aux line over CommAMD.  This RS232 line is NOT the tester RS232; it stays
//        inside the GPIB engine (user ruling).
//          golden Main.cpp:6591-7060   TimerTModeTimer, QueryRDY, btnSendTempClick, SendMode, CommAMDReceiveData
//    (2) Delta Castle tester flow
//          golden Main.cpp:7099-8094   ProcessStatusString_Delta_Castle, TestGPIBForCastle, GetResultForCastle
//    (3) HANA ART manual command panel
//          golden Main.cpp:8122-8292   btnHANA_SendCBClick, btnSend_EDClick, InitHANA_ART, DoSendCommand(int),
//                                      DoSendCommand(AnsiString), InitializeSRQCodeMap,
//                                      btnHANA_SendCBTotesterClick, btnSend_EDToTesterClick
//
//  AI(W906-GB-P1) 20260926.  Golden: D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp, decoded cp950 ->
//  UTF-8 (zero U+FFFD).  Rules: TesterComm/Gpib/TRANSLATION_RULES.md.  The bodies below were copied from the golden
//  file line by line (golden comments kept, `__fastcall` dropped with the comment column kept, `Pointer` -> `void*`),
//  and then only the edits listed here were applied; every one is also marked //AI(W906-GB-P1) at its line.
//
//  THREADING.  CommAMDReceiveData is called on the TesterComm thread by TSerialPoll::DrainRx(), not on the TComm
//  reader thread, so its body is golden text.  It keeps golden's reliance on Buffer being NUL-terminated
//  (`strlen(data)`; BufferLength is unused, as in golden) -- DrainRx must hand over a NUL-terminated buffer.
//  Nothing here takes uiMutex itself; the widgets are written the way golden writes them, on the TesterComm thread.
//  BRIDGE LIFE.  TimerTModeTimer and TestGPIBForCastle have golden function-local statics; each re-arms them to
//  their golden initialisers when g_bridgeLife changes (GpibBridge.h "V906 bridge life").
//
//  EDITS AGAINST GOLDEN
//   E1  AnsiString through C varargs is UB in C++ (non-trivially-copyable class through `...`) -> `.c_str()`:
//         sprintf(cmd,"%c%s%c%c",0x02,asCMD,...)      golden :6726 :6753 :6797
//         sprintf(cmd,"H2A:  %s",(AnsiString)cmd)     golden :6673 (inside #ifdef DEBUG, off) :7518 :7602
//         sBarCodeMsg.sprintf(..., sList->Strings[i]) golden :7375 -> AnsiString(...) -- AnsiString::sprintf converts
//                                                     AnsiString args, not the StringsProxy vclcompat Strings[i] returns.
//   E2  strupr (golden :7719 :7852) is hidden by MinGW's <string.h> under -std=c++17 / CMAKE_CXX_EXTENSIONS OFF
//       (same finding as EJ1N/TextProcess.cpp:74-79); a TU-local static with the golden ASCII semantics is below.
//   E3  golden `SerialPoll->Height=960/840;` (:8139 :8196) gated: TSerialPoll has no TForm base (rule 6).
//   H   In-process hardening.  Golden reads or writes past a fixed buffer only for malformed input or an unset local;
//       as its own process that corrupted the bridge, in-process it would corrupt the Handler.  Each guard is a no-op
//       for the input golden handles correctly:
//       H1  CommAMDReceiveData @1040: strcpy(str[128], asData)                          golden :6867
//       H2  @1010: dTC/dTJ[i] for i<iDataCount; the clamp is 64 but the arrays hold 32  golden :6972-6976
//       H3  @1010: site-mapping indices into dTC/dTJ and dBySiteTC/TJ unchecked         golden :6980-6987
//       H4  @1014: bCheckDiodeThermal[<ATC field>] unchecked                             golden :7032
//       H5  Delta_Castle str/str2/cmd zero-initialised (golden :7104): NEXTSTEP 1/3 with rgController->ItemIndex==1
//           write an UNSET cmd to CommAMD (strlen on stack garbage), now 0 bytes; RUNPROFILE's str2 had no NUL.
//       H6  Delta_Castle strcpy(str2/str[100], sRecipe) -- sRecipe is tester text       golden :7265 :7528
//       H7  READDIODE writes ATCNowTJTemp[4] for i<iATC_Use_Heat_Count/2 (up to 16)     golden :7108 :7296
//       H8  TestGPIBForCastle: buffer[ibcnt] with ibcnt==StrLength (buffer/tempbuffer sized StrLength+1, same
//           remedy as GpibTestGpib.cpp; terminator lines unchanged); sprintf(str[512], ..., buffer) -> snprintf
//                                                                                        golden :7645 :7848
//       H9  GetResultForCastle reads tstr (:7986) before writing it (:8001)             golden :7911
//       H10 btnSendTempClick: cmd unset when rgController->ItemIndex is neither 0 nor 1 golden :6738
//
//  GOLDEN QUIRKS KEPT VERBATIM (they look like bugs; they are behaviour, so they are not fixed)
//   Q1  btnHANA_SendCBTotesterClick compares Text=="DUMMYTEST_START_SRQ", but the combo holds the map keys
//       ("DUMMYTEST_START_SRQ0x42"), so the dummy-test branch never runs from the combo.            golden :8269
//   Q2  DoSendCommand(AnsiString sCmd) ignores sCmd and writes edCmdHANAARTtoTester->Text.          golden :8208-8212
//   Q3  READTEMP / READDAQ formats end in a lone '%' ("1 %5.2f %5.2f%"): an invalid printf conversion (UB in C).
//       Kept because the text goes to the tester.  What BCB6's RTL emitted for it is NOT verified against the
//       vclcompat vsnprintf path -- check on a Castle before trusting READTEMP/READDAQ replies. golden :7612-7632
//   Q4  Delta_Castle ordering: the second READDAQ branch (:7624, Pos!=0) only sees READDAQ not at position 1 (the
//       first, :7330, answers "0.0"); TESTPARTS (:7341) swallows TESTPARTSREADY? before its own branch (:7435);
//       CHUCKID? (:7384) can write twice (str2, then "0") and logs str2 as "SRQBYTE".
//   Q5  READDIODE sends "" for test modes other than Single/Dual/QualSite1X4/QualSite2X2.            golden :7315-7327
//   Q6  TestGPIBForCastle passes the int Task to MyGPIBWrite's AnsiString Task: logged as "1"/"200".
//   Q7  TestGPIBForCastle: strupr returns its argument, so strncpy(buffer,pstr,..) / strcpy(buffer,pstr) are
//       self-copies (formally overlapping, byte-identical on msvcrt/ucrt).                          golden :7722 :7855
//   Q8  CommAMDReceiveData @1048: the first asData1 assignment is dead.                             golden :7046
//   Q9  QueryRDY / SendMode: both iArm branches write CommAMD; CommAMD2 is never used here.         golden :6728 :6798
//   Q10 LRC frame: when byLRC_cal is 0 the strlen() of the frame stops before 0x03.                 golden :6726 ...
//   Q11 GetResultForCastle: the close-site 9999 loop, the first caption loop and the tester-error loop cover 8
//       sites, the final caption loop 32; bCloseSiteHaveBinErr is always false where bCleanResult tests it.
//   Q12 TestGPIBForCastle case 100 falls through into case 200 (golden `//break;`).                golden :7828
//   Q13 TestGPIBForCastle's `static AnsiString sRecipe` shadows the global sRecipe and is unused.   golden :7664
//   Q14 CommAMDReceiveData @1010: the value loop (iATC_Result[i], i<iDataCount) and the TC/TJ pair loop
//       (dTC[i]=iATC_Result[2i], i<iDataCount) share one bound, so one of them is off by 2x (value count vs
//       controller count); with a value count the upper half of dTC/dTJ reads the 9999 fill (999.9).  golden :6962-6976
//
//  VCLCOMPAT SEMANTIC GAPS that change these bodies' behaviour (not fixed here -- vclcompat is off-limits)
//   V1  TStringList::Strings[i] out of range returns "" instead of raising EStringListError, so golden's try/catch(...)
//       in CommAMDReceiveData no longer abandons a short @1010/@1014 frame: the missing fields parse as 0.
//       MATERIALID? (golden :7375) likewise sends "" fields instead of throwing when sBarCode has < 32 entries.
//   V2  TComboBox: `ItemIndex=0` does not update `Text` (VCL does).  After InitHANA_ART / InitializeSRQCodeMap the two
//       combos' Text stays "" until the web page writes it; btnHANA_SendCB*Click read ->Text.
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace gpibbridge {

//AI(W906-GB-P1) 20260926: E2 -- golden calls the Borland RTL strupr (in-place ASCII upper-case, returns its argument).
//  MinGW's <string.h> does not declare it under -std=c++17; this TU-local copy has the same semantics and, being
//  found first by unqualified lookup inside namespace gpibbridge, also wins where a CRT does declare ::strupr.
static char* strupr(char* s)
{
    for(char* p=s; *p; ++p)
        *p=(char)toupper((unsigned char)*p);
    return s;
}

//AI(W906-GB-P1) 20260926: H1/H6 -- golden strcpy into a fixed char[] from external text.  strncpy semantics (the
//  tail is zero-filled) with the last byte forced to NUL; identical to strcpy whenever the text fits.
static void BoundedStrCpy(char* dst, size_t dstSize, const char* src)
{
    if(dstSize==0)
        return;
    strncpy(dst, src, dstSize-1);
    dst[dstSize-1]='\0';
}

//AI(W906-GB-P1) 20260926: H3 -- index check for the ATC_MAX_SITE-sized ATC temperature arrays.
static bool AtcIdxOk(int idx)
{
    return idx>=0 && idx<ATC_MAX_SITE;
}

//------------------------------------------------------------------------------
//  golden Main.cpp:6591-7060 -- AMD / ATC temperature controller (CommAMD aux line)
//------------------------------------------------------------------------------
void TSerialPoll::TimerTModeTimer(TObject *Sender)
{
    AnsiString S;
    char cmd[128];
    static int iGetTempDelay=0;
//    static int iGetCount=0;
    static int iCheckStatus=0;                                                  //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
    static int iSendCount=0;                                                    //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
    static int iAutoSwitchTC=0;                                                 //Ifor 20220615 : add BINON後TJ模式下需切回TC

    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life.  Golden: every Handler
    //   RunTestProgram / AMD 2DIDFormat change relaunches the exe, so these start at their initialisers each
    //   life (see g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        iGetTempDelay=0;
        iCheckStatus=0;
        iSendCount=0;
        iAutoSwitchTC=0;
    }

    if(LastSet.i2DIDFormat!=eAMD)                                               //JerryYang 20200422 2DID format
    {
        return;
    }

    if(bTempHasReady==true &&
       IsTest==true && LastSet.bSQR41==true)                                    //JerryYang 20241205 : fix 還沒開始測試流程的時候收到REM會傳0x41造成異常
    {
        ibwait(noncontroller, 0);                                               // Update Status variable
        if(LastSet.iTesterMode!=InterfaceType_Delta_Castle)                     //Ifor 20201020 add:修正Delta_Castle 測試流程收到測試機REM訊號回覆0x41問題導致空測
        {
            if(ibsta&REM)
            {
                WriteLog("Timer SRQ:0x41");
                ibrsv(noncontroller, 0x41);
                #ifdef DEBUG
                    if(chkCMDLog->Checked==true)
                    {
                        S.sprintf("ibrsp Report: %s", "0x41");
                        WriteLog(S.c_str());
                    }
                #endif
            }
        }
    }

    iCheckStatus++;
    if(iCheckStatus>=1000)                                                      //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
    {
        iCheckStatus=0;
        sprintf(cmd,"@1048,1,1#");
        CommAMD->WriteCommData(cmd,strlen(cmd));
        mmoBINON->Lines->Add(cmd);
        if(bHasSendCheckCmd==true)
        {
            iSendCount++;
            if(iSendCount>=3)
            {
                labDebugMode->Visible=true;
                labDebugMode->Caption="ATC Connect Err";                        //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
                SendMSG_CMD(MSG_CMD_AMDRS232Connect , "1");                     //Ifor 20200220 : add AMD Rs232 Connect Error Alarm
            }
        }
        else
        {
            iSendCount=0;
            bHasSendCheckCmd=true;
            labDebugMode->Visible=false;
            labDebugMode->Caption="!!DEBUG MODE!!";                             //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
            SendMSG_CMD(MSG_CMD_AMDRS232Connect , "0");                         //Ifor 20200220 : add AMD Rs232 Connect Error Alarm
        }
        return;
    }

    if(LastSet.i2DIDFormat==eAMD)
    {
        iGetTempDelay++;
        if(iGetTempDelay>=10)
        {
            if(bHasSiteMapping==true)
            {
                if(iSwitchTJDelay<=0)                                           //Ifor 20210303 add:收到TC/TJ Switch 訊號延遲5次不讀取溫度
                {
                    if(iATC_Use_Heat_Count>4)                                   //Ifor 20201023 add: ATC 控制器數量
                        sprintf(cmd, "@1010,1,8,#");
                    else
                        sprintf(cmd, "@1010,1,4,#");
                    CommAMD->WriteCommData(cmd, strlen(cmd));
                    iGetTempDelay=0;
                    #ifdef DEBUG
                        if(chkCMDLog->Checked==true)
                        {
                            //AI(W906-GB-P1) 20260926: E1 golden passed (AnsiString)cmd through C sprintf varargs (UB in C++); .c_str() of the same temporary
                            sprintf(cmd, "H2A:  %s", ((AnsiString)cmd).c_str());
                            S.sprintf("%s", cmd);
                            WriteLog(S.c_str());
                        }
                    #endif
                }
                else                                                            //Ifor 20210303 add:收到TC/TJ Switch 訊號延遲5次不讀取溫度
                {
                    iSwitchTJDelay--;
                }
            }
            else
            {
                SendMSG_CMD(MSG_CMD_SiteMap);
                iGetTempDelay=0;
            }
        }
    }

    if(iHasNEXTSTEP2==2)
    {
        iAutoSwitchTC++;
        if(iAutoSwitchTC>=10)
        {
            if(iATC_Use_Heat_Count>4)
                sprintf(cmd, "1038,8,0,0,0,0,0,0,0,0");
            else
                sprintf(cmd, "1038,4,0,0,0,0");

            CommAMD->WriteCommData(cmd,strlen(cmd));
            S.sprintf("Auto Switch TC: %s", cmd);
            WriteLog(S);
            iHasNEXTSTEP2=0;
        }
    }
    else
    {
        iAutoSwitchTC=0;
    }
}
//---------------------------------------------------------------------------
void TSerialPoll::QueryRDY(int iArm)
{
    char cmd[128];
    AnsiString asCMD;
    char cLRC[8];
    sprintf(cmd, "RDY0000");
    asCMD.sprintf("RDY0000");
    int iLRC_sum=0;
    char byLRC_cal=0;
    for(int i=0; i<7; i++)
        iLRC_sum+=cmd[i];
    byLRC_cal=~(iLRC_sum&0xff)+1;
    //AI(W906-GB-P1) 20260926: E1 golden passed AnsiString asCMD through C sprintf varargs (UB in C++) -> .c_str()
    sprintf(cmd, "%c%s%c%c", 0x02, asCMD.c_str(), byLRC_cal, 0x03);

    if(iArm==0)
        CommAMD->WriteCommData(cmd, strlen(cmd));
    else if(iArm==1)
        CommAMD->WriteCommData(cmd, strlen(cmd));
}
//---------------------------------------------------------------------------
void TSerialPoll::btnSendTempClick(TObject *Sender)
{
    dtComm1=Now();

    //AI(W906-GB-P1) 20260926: H10 golden `char cmd[128];` -- cmd stays unset when rgController->ItemIndex is neither 0 nor 1 and
    //  is then strlen()ed and written to CommAMD; zero-initialised so that case writes 0 bytes instead of stack garbage.
    char cmd[128]={0};
    AnsiString asCMD;
    char cLRC[8];
    AnsiString str, S;

    if(rgController->ItemIndex==1)
    {
        sprintf(cmd, "TSW000%d", cbMode->ItemIndex);
        asCMD.sprintf("TSW000%d", cbMode->ItemIndex);
        int iLRC_sum=0;
        char byLRC_cal=0;
        for(int i=0; i<7; i++)
            iLRC_sum+=cmd[i];
        byLRC_cal = ~(iLRC_sum&0xff)+1;

        //AI(W906-GB-P1) 20260926: E1 golden passed AnsiString asCMD through C sprintf varargs (UB in C++) -> .c_str()
        sprintf(cmd, "%c%s%c%c", 0x02, asCMD.c_str(), byLRC_cal, 0x03);
    }
    else if(rgController->ItemIndex==0)
    {
        if(iCurrentArm==2)
        {
            sprintf(cmd, "1038,4,-1,-1,%d,%d", cbMode->ItemIndex, cbMode->ItemIndex);
        }
        else
        {
            sprintf(cmd, "1038,4,%d,%d,-1,-1", cbMode->ItemIndex, cbMode->ItemIndex);
        }
        if(cbMode->ItemIndex==0)
        {
            bCheckTC[0]=true;
        }
        else if(cbMode->ItemIndex==1)
        {
            bCheckTJ[0]=true;
        }
    }

    str.sprintf("Button2Click");
    lstRecord->Items->Add(str);
    lstRecord->ItemIndex=lstRecord->Items->Count-1;
    CommAMD->WriteCommData(cmd, strlen(cmd));
    S.sprintf("H2A: %s", cmd);
    WriteLog(S);
}
//---------------------------------------------------------------------------
void TSerialPoll::SendMode(int iArm, int iMode)
{
    char cmd[128];
    AnsiString asCMD;
    char cLRC[8];

    sprintf(cmd, "TSW000%d", iMode);
    asCMD.sprintf("TSW000%d", iMode);
    int iLRC_sum=0;
    char byLRC_cal=0;
    for(int i=0; i<7; i++)
        iLRC_sum+=cmd[i];
    byLRC_cal=~(iLRC_sum&0xff)+1;

    //AI(W906-GB-P1) 20260926: E1 golden passed AnsiString asCMD through C sprintf varargs (UB in C++) -> .c_str()
    sprintf(cmd, "%c%s%c%c", 0x02, asCMD.c_str(), byLRC_cal, 0x03);
    if(iArm==0)
        CommAMD->WriteCommData(cmd, strlen(cmd));
    else if(iArm==1)
        CommAMD->WriteCommData(cmd, strlen(cmd));

    iTestStart=GetTickCount();
}
//---------------------------------------------------------------------------
void TSerialPoll::CommAMDReceiveData(TObject *Sender,
      void* Buffer, WORD BufferLength)
{
    char *data;
    data=(char*)Buffer;
    AnsiString asData,asData1;
    AnsiString asDataTemp;
    AnsiString S;
    char str[128];
    bool bFlag=false;
    TStringList *sDataList;
    int iDataCount=0;

    int iStartPos=0, iEndPos=0, iPos1;

    mmoBINON->Lines->Add(data);
    iTestEnd=GetTickCount();
    Label24->Caption= iTestEnd-iTestStart;
    iTestEnd=0;
    iTestStart=0;

    asData="";
    for(int i=0; i<(int)strlen(data); i++)
    {
        asData+=data[i];
    }

    if(rgController->ItemIndex==1)
    {
        if(asData.Pos("YES_RDY")>=1)                                            //new control
        {
            if(bCheckTC[0])
            {
                bCheckTC[0]=false;
            }
            else if(bCheckTJ[0])
            {
                bCheckTJ[0]=false;
            }
            else if(bCheckTC[1])
            {
                bCheckTC[1]=false;
            }
            else if(bCheckTJ[1])
            {
                bCheckTJ[1]=false;
            }
        }
    }
    else if(rgController->ItemIndex==0)
    {
        ReWorkString:
        asData=asData.Trim();

        if(asData.Pos("@1040")>=1 && asData.Pos("#")>1 &&
           IsTest==true && LastSet.bSQR41==true)                                //JerryYang 20241205 : fix 還沒開始測試流程的時候收到REM會傳0x41造成異常
        {
            asData1=asData;                                                     //@123+@456+
            asData=asData1.SubString(1, asData.Pos("#"));                       //S1=@123+

            asData1=asData1.SubString(strlen(asData.c_str())+1, strlen(asData1.c_str())); //S=@456+
            //AI(W906-GB-P1) 20260926: H1 golden strcpy(str, asData.c_str()) -- str is char[128] and asData is ATC text; bounded copy
            //  (strncpy also zero-fills the tail, so the str[8]/str[10]/str[14] reads below never see stack garbage).
            BoundedStrCpy(str, sizeof(str), asData.c_str());
            bTempHasReady=false;
            if(iCurrentArm==1)                                                  //Handler ==> Arm 1 Down
            {
                if(str[8]=='0' && bCheckTC[0])
                {
                    if(str[10]=='1')
                    {
                        bCheckTC[0]=false;
                        bTempHasReady=true;
                    }
                }
                else if(str[8]=='1' && bCheckTJ[0])
                {
                    if(str[10]=='1')
                    {
                        bCheckTJ[0]=false;
                        bTempHasReady=true;
                    }
                }
            }
            if(iCurrentArm==2)                                                  //Handler ==> Arm 2 Down
            {
                if(str[8]=='0' && bCheckTC[1])
                {
                    if(str[14]=='1')
                    {
                        bCheckTC[1]=false;
                        bTempHasReady=true;
                    }
                }
                else if(str[8]=='1' && bCheckTJ[1])
                {
                    if(str[14]=='1')
                    {
                        bCheckTJ[1]=false;
                        bTempHasReady=true;
                    }
                }
            }
            asData=asData1;
            if(asData.Pos("@")>=1)
                goto ReWorkString;
        }
        else if(asData.Pos("@1010")>=1 && asData.Pos("#")>1)
        {                                                                       //Ifor 20190709 : add 紀錄接收ATC溫度Log
            try
            {
                S="";
                S.sprintf("A2H: %s", asData);
                mmoBINON->Lines->Add(S);
                if(asData.Length()!=0)
                {
                    iStartPos = asData.Pos("@");
                    iEndPos   = asData.Pos("#");

                    if(iEndPos>iStartPos)
                    {
                        if(iStartPos==0 && iEndPos!=0)
                        {
                            asData = asData.SubString(iEndPos+1, asData.Length());
                            asDataTemp = "";
                        }
                        else if(iStartPos==0 && iEndPos==0)
                        {
                            asData = "";
                        }
                        else if(iStartPos!=0 && iEndPos==0)
                        {
                            asDataTemp = asData.Trim();
                            asData = "";
                        }
                        else if(iStartPos!=0 && iEndPos!=0)
                        {
                            asDataTemp  = asData.SubString(iStartPos+1, iEndPos-iStartPos-2); //Eliot 2016_0720
                            asData      = asData.SubString(iEndPos+1, asData.Length());
                            bFlag       = true;
                        }
                    }
                }

                if(bFlag==true)
                {
                    sDataList=new TStringList();
                    sDataList->CommaText=asDataTemp;
                    iDataCount=atoi(sDataList->Strings[1].c_str());
                    if(asATC_SiteMapping=="XXX")
                    {

                    }
                    else
                    {
                        if(iDataCount>ATC_MAX_SITE*2)
                            iDataCount=ATC_MAX_SITE*2;

                        for(int i=0; i<ATC_MAX_SITE*2; i++)                     //Data Result
                        {
                            iATC_Result[i]=9999;
                            if(i<iDataCount)
                            {
                                iATC_Result[i]=atoi(sDataList->Strings[i+2].c_str());
                            }
                        }

                        S="";                                                   //清空資料避免資料異常
                        for(int i=0; i<(iDataCount); i++)
                        {
                            //AI(W906-GB-P1) 20260926: H2 golden runs i up to iDataCount (clamped to ATC_MAX_SITE*2=64) but dTC/dTJ
                            //  hold ATC_MAX_SITE=32 and iATC_Result[1+i*2] ends at 63: stop at 32 (no-op for a well-formed frame).
                            if(i>=ATC_MAX_SITE)
                                break;
                            dTC[i] = iATC_Result[0+(i*2)]/10.0;
                            dTJ[i] = iATC_Result[1+(i*2)]/10.0;
                        }

                        sDataList->Clear();
                        sDataList->CommaText=asATC_SiteMapping;
                        for(int j=0; j<sDataList->Count; j++)
                        {
                            //AI(W906-GB-P1) 20260926: H3 golden indexes dTC/dTJ with the raw site-mapping value (a "0" or a
                            //  non-number gives -1) and dBySiteTC/TJ with Heat_Count/2+j, all unchecked; each golden line now runs only
                            //  when both of its indices are inside [0, ATC_MAX_SITE).  Golden lines and their order unchanged.
                            if(AtcIdxOk(0+j) && AtcIdxOk(atoi(sDataList->Strings[j].c_str())-1))
                                dBySiteTC[0+j]=dTC[atoi(sDataList->Strings[j].c_str())-1];
                            if(AtcIdxOk((iATC_Use_Heat_Count/2)+j) && AtcIdxOk(atoi(sDataList->Strings[j].c_str())+(iATC_Use_Heat_Count/2)-1))
                                dBySiteTC[(iATC_Use_Heat_Count/2)+j]=dTC[atoi(sDataList->Strings[j].c_str())+(iATC_Use_Heat_Count/2)-1];

                            if(AtcIdxOk(0+j) && AtcIdxOk(atoi(sDataList->Strings[j].c_str())-1))
                                dBySiteTJ[0+j]=dTJ[atoi(sDataList->Strings[j].c_str())-1];
                            if(AtcIdxOk((iATC_Use_Heat_Count/2)+j) && AtcIdxOk(atoi(sDataList->Strings[j].c_str())+(iATC_Use_Heat_Count/2)-1))
                                dBySiteTJ[(iATC_Use_Heat_Count/2)+j]=dTJ[atoi(sDataList->Strings[j].c_str())+(iATC_Use_Heat_Count/2)-1];
                        }
                    }
                    sDataList->Clear();
                    delete sDataList;
                }
                bSendGetTemp=false;
            }
            catch(...)
            {

            }
        }
        else if(asData.Pos("@1014")>=1 && asData.Pos("#")>1)                    //Ifor 20190510 : add TJ Check Error @1014,2,001,0(1014命令，2資料筆數，001錯誤碼，1 Error)
        {
            try
            {
                if(asData.Length()!=0)
                {
                    iStartPos=asData.Pos("@");
                    iEndPos  =asData.Pos("#");
                    if(iStartPos==0 && iEndPos!=0)
                    {
                        asData=asData.SubString(iEndPos+1, asData.Length());
                        asDataTemp="";
                    }
                    else if(iStartPos==0 && iEndPos==0)
                    {
                        asData="";
                    }
                    else if(iStartPos!=0 && iEndPos==0)
                    {
                        asDataTemp=asData.Trim();
                        asData="";
                    }
                    else if(iStartPos!=0 && iEndPos!=0)
                    {
                        asDataTemp=asData.SubString(iStartPos+1, iEndPos-(iStartPos+1));
                        bFlag=true;
                    }
                }

                if(bFlag==true)
                {
                    sDataList=new TStringList();
                    sDataList->CommaText=asDataTemp;
                    //AI(W906-GB-P1) 20260926: H4 golden indexes bCheckDiodeThermal[TOTAL_SITE] with the raw ATC field; guarded.
                    if(atoi(sDataList->Strings[3].c_str())>=0 && atoi(sDataList->Strings[3].c_str())<TOTAL_SITE)
                        bCheckDiodeThermal[atoi(sDataList->Strings[3].c_str())]=true;
                    sDataList->Clear();
                    delete sDataList;
                }
            }
            catch(...)
            {

            }
        }
        else if(asData.Pos("@1048")>=1&& asData.Pos("#")>1)                     //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
        {
            if(asData.Length()!=0)
            {
                asData1=asData.SubString(1, asData.Pos("#"));
                asData1=asData.SubString(asData.Pos("#")-1, 1);
                if(asData1=="1")
                {
                    sprintf(str,"@1048,1,2#");
                    CommAMD->WriteCommData(str, strlen(str));
                }
                else if(asData1=="2")
                {
                    bHasSendCheckCmd=false;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
//  golden Main.cpp:7062-7097 (ShowSimulateItem, btnRunModeClick) is not in this file.
//------------------------------------------------------------------------------
//  golden Main.cpp:7099-8094 -- Delta Castle tester flow
//------------------------------------------------------------------------------
bool TSerialPoll::ProcessStatusString_Delta_Castle(AnsiString Str, AnsiString BS, AnsiString Task)     //Ifor 20200603 add: Delta Castle Command
{
    bool bResult=false;
    AnsiString S, S2, cCommData, sData;
    AnsiString sPcName, sVersion, sBarCodeMsg, sSiteMapData="";
    //AI(W906-GB-P1) 20260926: H5 golden `char str[100], str2[100], cmd[128];` uninitialised.  NEXTSTEP 1/3 with
    //  rgController->ItemIndex==1 (or !=0 && !=1) never sets cmd and then strlen()s it and writes it to CommAMD;
    //  RUNPROFILE builds str2 without a terminator.  Zero-initialised: those paths now write 0 bytes / a terminated
    //  string; every other path overwrites the buffers before use exactly as golden.
    char str[100]={0}, str2[100]={0}, cmd[128]={0};
    TStringList *SL;
    int iSendUseSite[TOTAL_SITE];
    byte data=0;
    //AI(W906-GB-P1) 20260926: H7 golden `double ATCNowTJTemp[4];` -- READDIODE prints slots it never filled when the test mode
    //  and iATC_Use_Heat_Count disagree; zero-initialised (see also the loop guard in READDIODE).
    double ATCNowTJTemp[4]={0};

    S=Str;

    if(S.Pos("IDENTIFY?")==1)
    {
        strcpy(str2, "Summit 4.2.2");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("*IDN")==1)                                                   //2016-06-06    Dell    for AMD Test
    {
        strcpy(str2,"1 Delta_Design,SUMMIT,0,Summit 4.2.2");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("WHICH?")==1)
    {
        strcpy(str2,"Hontech01");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("NUMTESTSITES?")==1)
    {
        if(asATC_SiteMapping!="XXX")                                            //Ifor 20230306 add
        {
            TStringList *sDataList;
            sDataList=new TStringList();
            sDataList->Clear();
            sDataList->CommaText=asATC_SiteMapping;

            int iCount=sDataList->Count;

            if(iCount>8)                                                        //JerryYang 20230414 : add
                iCount=8;

            sData = AnsiString(iCount);
            strcpy(str2, sData.c_str());

            sDataList->Clear();
            delete sDataList;
        }
        else
        {
            if(Task=="0001")
            {
                strcpy(str2,"1");
            }
            else
            {
                strcpy(str2,"4");
            }
        }

        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("SET SITEDISABLE?")==1)
    {
        strcpy(str2,"0");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("VERIFYTEST?")==1)
    {
        strcpy(str2,"0");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("CMDREPLY 0")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("SYSTEMMODE 1")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("TESTERMODE 1")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("EMULATIONMODE 2")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("ERRORCLEAR")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("SRQMASK 63")==1)
    {
        strcpy(str2," ");
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("START")==1)
    {
        bResult=true;
    }
    else if(S.Pos("STATUS?")==1)
    {
        if(Task=="0001")
        {
//            CheckHandlerStatus();
//            if(bSafedoor)
//                iSafedoorStatus=25167872;
        }
        sprintf(str2,"%d",iSafedoorStatus);
        MyGPIBWrite(str2, Task);
        bResult=true;
    }
    else if(S.Pos("SETPOINT?")!=0 && S.Pos("ZONE")!=0)
    {
        SendMSG_CMD(MSG_CMD_HandlerTemperature);
        bResult=true;
    }
    else if(S.Pos("MASSTEMP?")!=0 && S.Pos("ZONE")!=0)
    {
        SendMSG_CMD(MSG_CMD_ActualTemp);
        bResult=true;
    }
    else if(S.Pos("TESTERMODE?")==1)
    {
        if(Task=="0001")
        {
            strcpy(str2,"Normal");
            MyGPIBWrite(str2, Task);
            bResult=true;
        }
    }
    else if(S.Pos("TESTERMODE NORMAL")==1)
    {
        bResult=true;
    }
    else if(S.Pos("STOP")==1)
    {
        bResult=true;
    }
    else if(S.Pos("RECIPENAME?")==1)
    {
        if(Task=="0001")
        {
            s1=sRecipe.SubString(16, 1) ;
            s2=sRecipe.SubString(6, 3);
            if(s1!="X" || (s2!="M1P" && s2!="R2P" && s2!=LastSet.sPackageType))
                strcpy(str2,"Error");
            else
                //AI(W906-GB-P1) 20260926: H6 golden strcpy(str2, sRecipe.c_str()) -- str2 is char[100], sRecipe is tester text
                BoundedStrCpy(str2, sizeof(str2), sRecipe.c_str());
            MyGPIBWrite(str2, Task);
            bResult=true;
        }
    }
    else if(S.Pos("RECIPE ACTIVATEGRP")==1)
    {
        sRecipe=S;
        sRecipe=sRecipe.SubString(20, sRecipe.Length());
        bResult=true;
    }
    else if(S.Pos("LOADPROFILE")==1)
    {
        SleepEx(1, false);
        MyGPIBWrite("1", Task);
        bResult=true;
    }
    else if(S.Pos("READDIODE")==1)
    {
        SleepEx(1, false);

        int iWitchArm=0;
        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            iWitchArm=0;
        }
        else if(iCurrentArm==2)                                                 //Handler ==> Arm 2 Down
        {
             iWitchArm=iATC_Use_Heat_Count/2;
        }

        for(int i=0; i<iATC_Use_Heat_Count/2; i++)
        {
            //AI(W906-GB-P1) 20260926: H7 golden writes ATCNowTJTemp[i] for i<iATC_Use_Heat_Count/2 (up to 16) into a
            //  4-slot array; stop at 4 (no-op for the 4/8-controller Castle configurations golden supports).
            if(i>=4)
                break;
            if(iATCUseChannel[i+iWitchArm]==1)
            {
                if(dBySiteTJ[i+iWitchArm]<25 || dBySiteTJ[i+iWitchArm]>200)
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
    else if(S.Pos("READDAQ")==1)
    {
        SleepEx(1, false);
        MyGPIBWrite("0.0", Task);
        bResult=true;
    }
    else if(S.Pos("SETTEMP?")==1)
    {
        SendMSG_CMD(MSG_CMD_HandlerTemperature);
        bResult=true;
    }
    else if(S.Pos("TESTPARTS")==1)                                              //2016-06-06    Dell    for AMD Test
    {
        if(IsTest==true)                                                        //Ifor 20251124 add:Delta_Castle新增FullSite確認流程
        {
            if(bIsTestStart==false)
            {
                bIsTestStart=true;

                data =  iStart[0]*1  + iStart[1]*2  + iStart[2]*4  + iStart[3]*8  +
                        iStart[4]*16 + iStart[5]*32 + iStart[6]*64 + iStart[7]*128;
                LastSet.bFULLSITES=true;
                sData = AnsiString(data);
                strcpy(str2, sData.c_str());
                MyGPIBWrite(str2, Task);
            }
            else
            {
                S.sprintf("%s ERROR : TESTPARTS process error", Task);
                WriteLog(S.c_str());
            }
        }
        else
        {
            strcpy(str2,"0");
            MyGPIBWrite(str2, Task);
        }
        bResult=true;
    }
    else if (S.Pos("MATERIALID?")==1)                                           //Ifor 20201110 add:Hygon 2D Code 回覆格式
    {
        TStringList *sList;
        sList=new TStringList();
        sList->CommaText=sBarCode->CommaText;

        //AI(W906-GB-P1) 20260926: E1 golden passed Strings[i] (a property -> AnsiString in BCB6).  vclcompat returns a
        //  StringsProxy, which AnsiString::sprintf does NOT convert and would push through C varargs; wrap each in AnsiString().
        sBarCodeMsg.sprintf("%s:%s:%s:%s\r\n", AnsiString(sList->Strings[31]), AnsiString(sList->Strings[30]), AnsiString(sList->Strings[29]), AnsiString(sList->Strings[28]));
        ibwrt(noncontroller, sBarCodeMsg.c_str(), sBarCodeMsg.Length());
        S.sprintf("%s TALK:%s", Task, sBarCodeMsg);
        WriteLog(S);
        LastSet.bHasBarCode=true;
        bResult=true;
        sList->Clear();                                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete sList;
    }
    else if(S.Pos("CHUCKID?")==1)
    {
        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            strcpy(str2,"1");
        }
        else
        {
            strcpy(str2,"2");
        }
        MyGPIBWrite(str2, Task);

        if(Task=="0001")
        {
            if(IsTest==true)
            {
                if(bIsTestStart==false)
                {
                    bIsTestStart=true;

                    data =  iStart[0]*1  + iStart[1]*2  + iStart[2]*4  + iStart[3]*8  +
                            iStart[4]*16 + iStart[5]*32 + iStart[6]*64 + iStart[7]*128;

                    S.sprintf("%s SRQBYTE:%s", Task, str2);
                    WriteLog(S.c_str());
                }
                else
                {
                    S="0001 ERROR : srqbyte? process error";
                    WriteLog(S.c_str());
                }
            }
            else
            {
                strcpy(str2, "0");
                MyGPIBWrite(str2, Task);
                bResult=true;
            }
        }
        else
        {
            if(IsTest==true)
            {
                if(bIsTestStart==false)
                {
                    bIsTestStart=true;
                }
            }
        }
        bResult=true;
    }
    else if(S.Pos("SRQBYTE?")==1 || S.Pos("TESTPARTSREADY?")==1)
    {
        if(IsTest==true)
        {
            if(bIsTestStart==false)
            {
                bIsTestStart=true;

                data =  iStart[0]*1  + iStart[1]*2  + iStart[2]*4  + iStart[3]*8  +
                        iStart[4]*16 + iStart[5]*32 + iStart[6]*64 + iStart[7]*128;
                LastSet.bFULLSITES=true;                                        //JerryYang 20230414 : Castle也要有保護
                sData = AnsiString(data);
                strcpy(str2, sData.c_str());
                MyGPIBWrite(str2, Task);
            }
            else
            {
                S.sprintf("%s ERROR : srqbyte? process error", Task);
                WriteLog(S.c_str());
            }
        }
        else
        {
            strcpy(str2,"0");
            MyGPIBWrite(str2, Task);
        }
        bResult=true;
    }
    else if(S.Pos("NEXTSTEP 1")==1)                                             // 2015.07.01 , Joye , Add GET2DID? Command
    {
        SleepEx(1,false);
        ibrsv(noncontroller,0x00);
        MyGPIBWrite("1", Task);

        for(int i=0; i<TOTAL_SITE; i++)
        {
            if(i<iATC_Use_Heat_Count)
                iStart[i]=1;

            if(iStart[i]==1)
            {
                iSendUseSite[i]=0;
            }
            else
            {
                iSendUseSite[i]=-1;
            }
        }

        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            bCheckTC[0]=true;
            if(rgController->ItemIndex==1)
            {
                SendMode(0, 0);
            }
            else if(rgController->ItemIndex==0)
            {
                if(iATC_Use_Heat_Count>4)                                        //Ifor 20201023 add: ATC 控制器數量
                    sprintf(cmd,"1038,8,%d,%d,%d,%d,-1,-1,-1,-1", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
                else
                    sprintf(cmd,"1038,4,%d,%d,-1,-1", iSendUseSite[0], iSendUseSite[1]);
            }
        }
        else if(iCurrentArm==2)                                                 //Handler ==> Arm 1 Down
        {
            bCheckTC[1]=true;
            if(rgController->ItemIndex==1)
            {
                SendMode(1, 0);
            }
            else if(rgController->ItemIndex==0)
            {
                if(iATC_Use_Heat_Count>4)                                       //Ifor 20201023 add: ATC 控制器數量
                    sprintf(cmd,"1038,8,-1,-1,-1,-1,%d,%d,%d,%d", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
                else
                    sprintf(cmd,"1038,4,-1,-1,%d,%d", iSendUseSite[0], iSendUseSite[1]);
            }
        }

        if(iCurrentArm==1 || iCurrentArm==2)
        {
            CommAMD->WriteCommData(cmd, strlen(cmd));
            //AI(W906-GB-P1) 20260926: E1 golden passed (AnsiString)cmd through C sprintf varargs (UB in C++); .c_str() of the same temporary
            sprintf(cmd,"H2A:  %s", ((AnsiString)cmd).c_str());
            S.sprintf("%s :%s", Task, cmd);
            WriteLog(S.c_str());
        }
        bResult=true;
    }
    else if(S.Pos("RUNPROFILE")==1)                                             // 2016.07.01 , Joye , AMD Run ProFile
    {
        SleepEx(1, false);

        //AI(W906-GB-P1) 20260926: H6 golden strcpy(str, sRecipe.c_str()) -- str is char[100], sRecipe is tester text
        BoundedStrCpy(str, sizeof(str), sRecipe.c_str());
        for(int i=0; i<20; i++)
        {
            str2[i]=str[18+i];
            if(str[9+i]==0)
                break;
        }
        S="Set Temp.="+AnsiString(str2);
        WriteLog(S.c_str());
        bResult=true;
        MyGPIBWrite("1", Task);

    }
    else if(S.Pos("SENDERROR")==1)                                              // 2016.07.04 , Joye , AMD Send Error
    {
        SleepEx(1, false);
        MyGPIBWrite("1", Task);
        bResult=true;
    }
    else if(S.Pos("NEXTSTEP 3")==1)                                             //Ifor 20200603 add: NEXTSTEP 3 切換至TJ模式
    {
        SleepEx(1, false);
        ibrsv(noncontroller, 0x00);
        MyGPIBWrite("1", Task);

        for(int i=0; i<TOTAL_SITE; i++)
        {
            if(i<iATC_Use_Heat_Count)
                iStart[i]=1;

            if(iStart[i]==1)
            {
                iSendUseSite[i]=1;
            }
            else
            {
                iSendUseSite[i]=-1;
            }
        }

        if(iCurrentArm==1)                                                      //Handler ==> Arm 1 Down
        {
            bCheckTJ[0]=true;
            if(rgController->ItemIndex==1)
            {
                SendMode(0, 1);
            }
            else if(rgController->ItemIndex==0)
            {
                if(iATC_Use_Heat_Count>4)                                       //Ifor 20201023 add: ATC 控制器數量
                    sprintf(cmd, "1038,8,%d,%d,%d,%d,-1,-1,-1,-1", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
                else
                    sprintf(cmd, "1038,4,%d,%d,-1,-1", iSendUseSite[0], iSendUseSite[1]);
            }
        }
        else if(iCurrentArm==2)                                                 //Handler ==> Arm 2 Down
        {
            bCheckTJ[1]=true;
            if(rgController->ItemIndex==1)
            {
                SendMode(1, 1);
            }
            else if(rgController->ItemIndex==0)
            {
                if(iATC_Use_Heat_Count>4)                                       //Ifor 20201023 add: ATC 控制器數量
                    sprintf(cmd, "1038,8,-1,-1,-1,-1,%d,%d,%d,%d", iSendUseSite[0], iSendUseSite[1], iSendUseSite[2], iSendUseSite[3]);
                else
                    sprintf(cmd, "1038,4,-1,-1,%d,%d", iSendUseSite[0], iSendUseSite[1]);
            }
        }

        if(iCurrentArm==1 || iCurrentArm==2)
        {
            CommAMD->WriteCommData(cmd,strlen(cmd));
            //AI(W906-GB-P1) 20260926: E1 golden passed (AnsiString)cmd through C sprintf varargs (UB in C++); .c_str() of the same temporary
            sprintf(cmd,"H2A:  %s", ((AnsiString)cmd).c_str());
            S.sprintf("%s :%s", Task, cmd);
            WriteLog(S.c_str());
        }
        bResult=true;
    }
    else if(Str.Pos("READTEMP")!=0)                                             //Ifor 20201030 add:讀取TC溫度
    {
        if(iCurrentArm==1)
        {
            S2.sprintf("1 %5.2f %5.2f%", dBySiteTC[0], dBySiteTC[1]);
        }
        else if(iCurrentArm==2)
        {
            S2.sprintf("1 %5.2f %5.2f%", dBySiteTC[2], dBySiteTC[3]);
        }

        ibwrt(noncontroller, S2.c_str(), S2.Length());
        S.sprintf("%s TALK:%s", Task, S2);
        WriteLog(S.c_str());
        bResult=true;
    }
    else if(Str.Pos("READDAQ")!=0)                                              //Ifor 20201030 add:讀取TJ溫度
    {
        if(iCurrentArm==1)
        {
            S2.sprintf("1 %5.2f %5.2f%", dBySiteTJ[0], dBySiteTJ[1]);
        }
        else if(iCurrentArm==2)
        {
            S2.sprintf("1 %5.2f %5.2f%", dBySiteTJ[2], dBySiteTJ[3]);
        }
        ibwrt(noncontroller, S2.c_str(), S2.Length());
        S.sprintf("%s TALK:%s", Task, S2);
        WriteLog(S.c_str());
        bResult=true;
    }

    return bResult;
}
//------------------------------------------------------------------------------
int TSerialPoll::TestGPIBForCastle()                                            // 2015.08.29 , Joye , Castle
{
    //AI(W906-GB-P1) 20260926: H8 golden `static char buffer[StrLength], tempbuffer[StrLength];` -- ibrd reads up to
    //   iBufferCount (== StrLength) bytes and each read is followed by `buffer[ibcnt]=0` / `='\0'`, so a full
    //   2560-byte read writes index 2560, one past the end (UB in C++; in BCB6 it hit the next static).  The +1
    //   keeps the golden intent (NUL-terminate the text just read) without the overrun; the golden terminator
    //   lines and the StrLength / iBufferCount uses below are unchanged (same remedy as GpibTestGpib.cpp).
    static char buffer[StrLength+1], tempbuffer[StrLength+1];
    static int &Task = iGbibTask;
    static bool bEchoNG=false;
    static char str[512],str2[512];
    static AnsiString S, BS;
    AnsiString sData;
    int iPos;

    char *pstr, C;
    char tstr[TOTAL_SITE][16];

    byte data=0;
    int len=0, iCount=0;

    bool bIsTesterError;
    bool bIsFormatError=false;                                                  //JerryYang 20230414 : Castle也要有保護

    AnsiString sBinData="";
    AnsiString sBin="";
    static AnsiString sRecipe="";
    int iPos1, iPos2;
    AnsiString asBuffer="";
    int iSendUseSite[TOTAL_SITE];
    char cmd[128];

    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life.  Golden: every Handler
    //   RunTestProgram relaunches the exe, so these start at their initialisers each life (see g_bridgeLife in
    //   GpibBridge.h).  Task is bound to the global iGbibTask (ResetBridgeGlobals resets it).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        memset(buffer, 0, sizeof(buffer));
        memset(tempbuffer, 0, sizeof(tempbuffer));
        bEchoNG=false;
        memset(str, 0, sizeof(str));
        memset(str2, 0, sizeof(str2));
        S=""; BS="";
        sRecipe="";
    }

    switch(Task)
    {
        case 1:
            bEchoNG=false;
            if(oldGpibAddress!=GpibAddress)
            {
                noncontroller = ibfind ("gpib0");                               // Release system control
                if(noncontroller<0)
                {
                    if(bSimulate==false)
                        WriteLog("0001 Error Open GPIB0");
                    return 2;
                }
                oldGpibAddress=GpibAddress;
                LastSet.GpibAddress=GpibAddress;
                StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
                ibrsc(noncontroller, 0);                                        //輸入指令 ibrsc 0解除系統控制權(i.e., 設定此卡片為non-controller).
                ibpad(noncontroller, GpibAddress);
                ibtmo(noncontroller, LastSet.iTimeOut);
                if(IsTest)
                {
                    Task=100;
                    break;
                }
            }

            if(IsTest==false)
            {
                if(IsTestDelay.Off()==false)                                    //20220616 wei 次數計數改為時間延遲
                {
                    break;
                }

                ibwait(noncontroller, 0);                                       // Wait until non-controller is listener and ATN line is dropped.
                UpdateLed();
//                ERROR_PROCESS = false;
                if(ibsta&LACS)                                                  //if LTX ,sned first all system will very slow
                {
                    ibrd(noncontroller, buffer, iBufferCount);                  // Read data bytes
                    strncpy(tempbuffer, buffer, sizeof(tempbuffer));            //Steven 20110608
                    if(ibcnt==0)
                    {
                        buffer[ibcnt]='\0';
                        break;
                    }
                    asBuffer.sprintf("Byte=%d     str=%s", ibcnt, buffer);
                    mmoBINON->Lines->Add(asBuffer);
                    buffer[ibcnt]=0;                                            //Ifor 20201019 add:避免資料長度未清除
                    BS=buffer;
                    pstr=strupr(buffer);
                    S="0001 LISTEN:"+AnsiString(pstr);
                    WriteLog(S);
                    strncpy(buffer, pstr, sizeof(buffer));
                    S=buffer;

                    ibwait(noncontroller, 0);

                    if(S.Pos("IDENTIFY?")==1)
                    {
                        strcpy(str2, "Summit 4.2.2");
                        MyGPIBWrite(str2, Task);
                        return 0;
                    }
                    else if(S.Pos("TESTRESULTS")==1)
                    {
                        if(IsTest==true)
                        {
                            // testresults 1,1,6,
                            if(bIsTestStart==true)
                            {
                                bIsTestStart=false;
                                S=pstr;
                                return GetResultForCastle(S);
                            }
                            else
                            {
                                S.sprintf("%s ERROR : testresults process error", "0001");
                                WriteLog(S.c_str());
                            }
                        }
                        else
                        {
                            S.sprintf("%s ERROR : Test results command error", "0001");
                            WriteLog(S.c_str());
                            return 0;
                        }
                    }
                    else if(ProcessStatusString_Delta_Castle(S, BS, "0001"))    //Ifor 20200603 add: Delta Castle Command
                    {
                        break;
                    }
                    else
                    {
                        WriteLog("0001 ERROR : Not Defined this Command");
                        break;
                    }
                    //----------------------------------------------------------
                }
                break;
            }
            else
            {
                Task = 100;
            }
            break;

        case 100:
            ibstop(noncontroller);

            //Ifor 20230313 add for Castle
            //<==
            int iStartSite[8];
            ZeroMemory(iStartSite, sizeof(iStartSite));

            if(iTestMode==SingleSite)
            {
                iStartSite[0]=1;
            }
            else if(iTestMode==DualSite || iTestMode==DualSite2x1)
            {
                iStartSite[0]=1;
                iStartSite[1]=1;
            }
            else if(iTestMode==TriSite1X3)
            {
                iStartSite[0]=1;
                iStartSite[1]=1;
                iStartSite[2]=1;
            }
            else if(iTestMode==QualSite1X4 || iTestMode==QualSite2X2 || iTestMode==QualSite2X2N)
            {
                iStartSite[0]=1;
                iStartSite[1]=1;
                iStartSite[2]=1;
                iStartSite[3]=1;
            }
            else
            {
                iStartSite[0]=1;
                iStartSite[1]=1;
                iStartSite[2]=1;
                iStartSite[3]=1;
                iStartSite[4]=1;
                iStartSite[5]=1;
            }
            data =  iStartSite[0]*1  + iStartSite[1]*2  + iStartSite[2]*4  + iStartSite[3]*8  +
                    iStartSite[4]*16 + iStartSite[5]*32 + 1*64 + 1*128;
            //<==
            //Ifor 20230313 add for Castle

            //data = 0x41;
            LastSet.bSQR41=true;
            ibrsv(noncontroller, data);
            S="0100 SRQ:"+AnsiString(data);
            WriteLog(S.c_str());

            Task=200;

            //break;

        case 200:
            if(IsTest==true)
            {
                ibwait(noncontroller, 0);                                       // Wait until non-controller is listener and ATN line is dropped.

                UpdateLed();
//                ERROR_PROCESS = false;

                if(ibsta&LACS || ibsta&ATN)
                {
                    ibrd(noncontroller, buffer, iBufferCount);                  // Read data bytes
                    if(ibcnt==0)
                    {
                        buffer[ibcnt]='\0';
                        break;
                    }
                    buffer[ibcnt]='\0';
                    memset (str, '\0', sizeof (str));
                    //AI(W906-GB-P1) 20260926: H8 golden sprintf(str, ...) -- str is char[512], buffer holds up to 2560 tester chars
                    std::snprintf(str, sizeof(str), "Byte=%d , str=%s", ibcnt, buffer);

                    mmoBINON->Lines->Add(str);
                    //BS   = buffer;
                    pstr =strupr(buffer);
                    S    ="0200 LISTEN:"+AnsiString(pstr);
                    WriteLog(S.c_str());
                    strcpy(buffer, pstr);
                    S = buffer;

                    if(S.Pos("TRS")==1 || S.Pos("trs")==1 || (S.Pos("TESTRESULTS")==1))
                    {
                        if(IsTest==true)
                        {
                            // testresults 1,1,6,
                            if( bIsTestStart == true )
                            {
                                bIsTestStart = false;

                                S=pstr;

                                return GetResultForCastle(S);
                            }
                            else
                            {
                                S.sprintf("%s ERROR : testresults process error", "0200");
                                WriteLog(S.c_str());
                            }
                        }
                        else
                        {
                            S.sprintf("%s ERROR : Test results command without IsTest=true  error", "0200");
                            WriteLog(S.c_str());
                            return 0;
                        }
                    }
                    else if(ProcessStatusString_Delta_Castle(S, BS, "0200"))    //Ifor 20200603 add: Delta Castle Command
                    {
                        break;
                    }
                    else
                    {
                        S.sprintf("%s ERROR : Not Defined this Command", "0200");
                        WriteLog(S.c_str());
                        break;
                    }
                    //----------------------------------------------------------
                }
                break;
            }
            else
            {
                Task = 1;
            }
            break;
    }
    return 0;
}
//------------------------------------------------------------------------------
int TSerialPoll::GetResultForCastle(AnsiString S)                               //JerryYang 20230411 : 包成function
{
    AnsiString sBinData;
    bool bIsFormatError=false, bIsTesterError=false;
    //AI(W906-GB-P1) 20260926: H9 golden `char tstr[TOTAL_SITE][16];` -- the first MY_DUT_PAL caption loop (8 sites) reads tstr
    //  before anything but the bCloseSiteHaveBinErr "9999" slots is written (AnsiString(tstr[i]) on unterminated
    //  stack bytes); zero-initialised, so those captions read "" where golden showed garbage.
    char tstr[TOTAL_SITE][16]={{0}};
    byte data=0;

    //Ifor 20230313 add for Castle
    //<==
    if(S.Pos("TESTRESULTS")==1)
    {
        sBinData=S.SubString(13, S.Length());
    }
    else if(S.Pos("TRS")==1 || S.Pos("trs")==1)
    {
        sBinData=S.SubString(5, S.Length()-4);
    }

    ZeroMemory(result, sizeof(result));

    TStringList *sDataList;
    sDataList=new TStringList();
    sDataList->Clear();

    sDataList->CommaText=sBinData;

    int iCount=sDataList->Count;

    if((S.Pos("TESTRESULTS")==1 && sDataList->Count>8) ||                       //大於4個site用TESTRESULTS, 最多8site
       ((S.Pos("TRS")==1 || S.Pos("trs")==1) && sDataList->Count>4)||           //4個site以下用TRS
        sBinData.Pos(" ")>0)
    {
        bIsFormatError=true;
        WriteLog("Tester ==> TESTRESULTS format is error");                     //Steven 20141212 : Add GPIB Log
    }
    else
    {
        for(int i=0; i<iCount; i++)
        {
            result[i]=atoi(sDataList->Strings[i].c_str());
        }
    }
    sDataList->Clear();
    delete sDataList;
    //<==
    //Ifor 20230313 add for Castle

    bCloseSiteHaveBinErr=false;                                                 //Steven 20141016 : 關Site不能有Bin
    for(int i=0;i<TOTAL_SITE;i++)
    {
        if(iStart[i]==0)
        {
            if(result[i]==0 || result[i]==10 || result[i]==9999)                //Kevin 20141212 : 測式機沒有ic回 BIN 10
            {
                result[i]=0;
            }
            else
            {
                bCloseSiteHaveBinErr=true;
            }
        }
    }

    if(bCloseSiteHaveBinErr==true)
    {
        for(int i=0; i<8; i++)
        {
            if(iStart[i]==1)
            {
                strncpy(tstr[i], "9999", sizeof(tstr[i]));
                result[i]=9999;                                                 //Steven 20170214 (wei): Add
            }
        }
    }

    try
    {
        for(int i=0; i<8; i++)
        {
            MY_DUT_PAL[i]->plSite->Caption=AnsiString(tstr[i]);
        }
    }
    catch(...)
    {
        WriteLog("MY_DUT_PAL->plSite->Caption ERROR");
    }


    if(bCloseSiteHaveBinErr)
    {
        WriteLog("Tester ==> CLOSE SITE HAS BIN ERROR");                        //Steven 20141212 : Add GPIB Log
        return 2;
    }

    for(int i=0; i<TOTAL_SITE; i++)
    {
        ZeroMemory(tstr[i], sizeof(tstr[i]));                                   //Steven 20160215 : 先清空記憶體再讀取資料, 避免舊資料出現
        if(iStart[i]==1)
        {
            sprintf(tstr[i], "%4d", result[i]);
        }
        else
        {
            strncpy(tstr[i], "----", sizeof(tstr[i]));
        }
    }

    bIsTesterError=false;                                                       //Steven 20150109 : 反向
    for(int i=0;i<8;i++)
    {
        if(iStart[i]==1)
        {
            if(result[i]<0 || result[i]>16)                                     //JerryYang 20230709 : fix castle bin 0 error
            {
                bIsTesterError=true;
                WriteLog("Tester ==> BIN IS <0 OR >MAX_BIN ERROR");             //Steven 20141212 : Add GPIB Log
                break;
            }
        }
    }

    bool bCleanResult=false;                                                    //Ifor 20251124 add:設定Result資料 9999
    if(bIsTesterError || bCloseSiteHaveBinErr || bIsFormatError)
    {
        WriteLog("Tester ==> BIN IS <0 OR >MAX_BIN ERROR");                     //Steven 20141212 : Add GPIB Log
        bCleanResult=true;
    }

    if(LastSet.bSQR41==false)                                                   //Ifor 20251124 add:未送0x41收到Bin資料報警
    {
        WriteLog("Tester ==> BINON WITHOUT 0x41 ERROR");
        SendMSG_CMD(MSG_CMD_BinonWithout0x41);
        bCleanResult=true;
    }
    else if(LastSet.bSQR41==true && LastSet.bFULLSITES==false)                  //Ifor 20251124 add:送出0x41&未收到FULLSITES命令，收到Bin資料報警
    {
        WriteLog("Tester ==> BINON WITHOUT FULLSITES ERROR");
        SendMSG_CMD(MSG_CMD_BinonWithoutFullsite);
        bCleanResult=true;
    }

    if(bCleanResult==true)
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            if(iStart[i]==1)
            {
                result[i]=9999;
            }
        }
    }

    IsTest=false;
    LastSet.bSQR41=false;
    LastSet.bFULLSITES=false;

    try
    {
        for(int i=0; i<TOTAL_SITE; i++)
        {
            MY_DUT_PAL[i]->plSite->Caption=AnsiString(tstr[i]);
        }
    }
    catch(...)
    {
        WriteLog("MY_DUT_PAL->plSite->Caption ERROR");
    }

    noncontroller=ibfind("gpib0");                                              // Release system control

    if(noncontroller<0)
    {
        WriteLog("0200 Error Open GPIB0");
        return 2;
    }
    data=0;
    ibrsv(noncontroller,data);
    S.sprintf("%s SRQ:%s", "0200", "0");
    WriteLog(S.c_str());

    ibrsc (noncontroller, 0);
    ibpad (noncontroller, GpibAddress);
    ibtmo(noncontroller, LastSet.iTimeOut);

    ibwait (noncontroller , 0);

    return 1;
}
//------------------------------------------------------------------------------
//  golden Main.cpp:8096-8120 (cbGPIBWriteWithout_r_nClick, btnUpdateClick, btnUpdateMouseDown/Up) is not in
//  this file.
//------------------------------------------------------------------------------
//  golden Main.cpp:8122-8292 -- HANA ART manual command panel
//------------------------------------------------------------------------------
void TSerialPoll::btnHANA_SendCBClick(TObject *Sender)
{
    AnsiString Str=cbCmdHANAART->Text;
    SendMSG_CMD(MSG_CMD_HANA_ART, Str);
}
//------------------------------------------------------------------------------
void TSerialPoll::btnSend_EDClick(TObject *Sender)
{
    AnsiString Str=edCmdHANAART->Text;
    SendMSG_CMD(MSG_CMD_HANA_ART, Str);
}
//------------------------------------------------------------------------------
void TSerialPoll::InitHANA_ART()                                                //JimmyChiu 20250214 : For Hana ART
{
    if(LastSet.bRunHANA_ART==true)                                              //Steven 20250414 : HANA ART Function
    {
        SerialPoll->pnlHanaART->Visible=true;
#if 0 // TODO(W906-GB-P1): TSerialPoll has no TForm base, so no Height (GpibBridge.h, rule 1); form height has no web meaning. golden Main.cpp:8139
        SerialPoll->Height=960;
#endif
        edCmdHANAART->Text="";
        cbCmdHANAART->Clear();
        cbCmdHANAART->Items->Add("DEVON:8,25,0,CT01");
        cbCmdHANAART->Items->Add("SETUPOK");
        cbCmdHANAART->Items->Add("SETUPSTOP");
        cbCmdHANAART->Items->Add("HDMODE?");
        cbCmdHANAART->Items->Add("STEPOK?");
        cbCmdHANAART->Items->Add("LOADER?");
        cbCmdHANAART->Items->Add("TESTOK");
        cbCmdHANAART->Items->Add("TESTSTOP");
        cbCmdHANAART->Items->Add("LOTEND:COMP");
        cbCmdHANAART->Items->Add("RMODEOK");
        cbCmdHANAART->Items->Add("PRIMETESTSTARTOK");
        cbCmdHANAART->Items->Add("NORMALSTART");
        cbCmdHANAART->Items->Add("LOTRT:ENGLOT,3000,XE");
        cbCmdHANAART->Items->Add("DUMMYTESTSTART");
        cbCmdHANAART->Items->Add("ERRORSTART");
        cbCmdHANAART->Items->Add("ERROREND");
        cbCmdHANAART->Items->Add("ERRORCLEAR");
        cbCmdHANAART->Items->Add("AUTOSTART");
        cbCmdHANAART->Items->Add("INITSTART");
        cbCmdHANAART->Items->Add("LOADERREQ");
        cbCmdHANAART->Items->Add("LOTINFORM");
        cbCmdHANAART->Items->Add("FQATESTSTART");
        cbCmdHANAART->Items->Add("FQATESTEND");
        cbCmdHANAART->Items->Add("WPSOCKETREQ");
        cbCmdHANAART->Items->Add("FBINMANUALREQ");
        cbCmdHANAART->Items->Add("REALPARASEQ");
        cbCmdHANAART->Items->Add("LOTINFOMETHOD");
        cbCmdHANAART->Items->Add("RFID");
        cbCmdHANAART->Items->Add("SBL");
        cbCmdHANAART->Items->Add("AUTODUMPING");
        cbCmdHANAART->Items->Add("EQPMODEL");
        cbCmdHANAART->Items->Add("HDMODE");
        cbCmdHANAART->Items->Add("INLINEUPDATE");
        cbCmdHANAART->Items->Add("TDATA?");
        cbCmdHANAART->Items->Add("CDATA?");
        cbCmdHANAART->Items->Add("SDATA?");
        cbCmdHANAART->Items->Add("JDATA?");
        cbCmdHANAART->Items->Add("ADATA?");
        cbCmdHANAART->Items->Add("DATACLEAR");
        cbCmdHANAART->Items->Add("PRIMETESTEND");
        cbCmdHANAART->Items->Add("RETESTEND");
        cbCmdHANAART->Items->Add("ID?");
        cbCmdHANAART->Items->Add("MAP?");
        cbCmdHANAART->Items->Add("CT?");
        cbCmdHANAART->Items->Add("TEMPSET?");
        cbCmdHANAART->Items->Add("SOAK?");
        cbCmdHANAART->Items->Add("PMODEOK");
        cbCmdHANAART->ItemIndex=0;
        //
        InitializeSRQCodeMap();
    }
    else
    {
        SerialPoll->pnlHanaART->Visible=false;
#if 0 // TODO(W906-GB-P1): TSerialPoll has no TForm base, so no Height (GpibBridge.h, rule 1); form height has no web meaning. golden Main.cpp:8196
        SerialPoll->Height=840;
#endif
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::DoSendCommand(int iCmd)                                       //JimmyChiu 20250401 : Add for Hana ART
{
    ibstop(noncontroller);                                                      //jou 20230907 : 修正0x59測試機收不到的問題
    AnsiString smsg=AnsiString().sprintf("\r\nHandler ==> Tester Hana cmd, SRQ:0x%s", IntToHex(iCmd, 2));
    WriteLog(smsg);
    ibrsv(noncontroller, iCmd);
}
//------------------------------------------------------------------------------
void TSerialPoll::DoSendCommand(AnsiString sCmd)                                //JimmyChiu 20250401 : Add for Hana ART
{
    AnsiString buffer=edCmdHANAARTtoTester->Text;
    MyGPIBWrite(buffer);
}
//------------------------------------------------------------------------------
void TSerialPoll::InitializeSRQCodeMap()                                        //JimmyChiu 20250401 : Add for Hana ART
{
    srqCodeMap.clear();
    srqCodeMap["SETUP_INFORM_REQUEST_SRQ0x55"]      = HANA_ART_SMILL::SETUP_INFORM_REQUEST_SRQ0x55;
    srqCodeMap["SETUP_INFORM_RECEIVE_OK_SRQ0x56"]   = HANA_ART_SMILL::SETUP_INFORM_RECEIVE_OK_SRQ0x56;
    srqCodeMap["SETUP_INFORM_RECEIVE_FAIL_SRQ0x57"] = HANA_ART_SMILL::SETUP_INFORM_RECEIVE_FAIL_SRQ0x57;
    srqCodeMap["LOT_START_SRQ0x63"]                 = HANA_ART_SMILL::LOT_START_SRQ0x63;
    srqCodeMap["LOTON_READ_SUCCESS_SRQ0x51"]        = HANA_ART_SMILL::LOTON_READ_SUCCESS_SRQ0x51;
    srqCodeMap["LOTON_READ_FAIL_SRQ0x52"]           = HANA_ART_SMILL::LOTON_READ_FAIL_SRQ0x52;
    srqCodeMap["LOT_END_SRQ0x64"]                   = HANA_ART_SMILL::LOT_END_SRQ0x64;
    srqCodeMap["STANDBY_TESTMODE_SRQ0x53"]          = HANA_ART_SMILL::STANDBY_TESTMODE_SRQ0x53;
    srqCodeMap["PRIME_START_SRQ0x50"]               = HANA_ART_SMILL::PRIME_START_SRQ0x50;
    srqCodeMap["PRIME_END_SRQ0x54"]                 = HANA_ART_SMILL::PRIME_END_SRQ0x54;
    srqCodeMap["NORMAL_START_SRQ0x41"]              = HANA_ART_SMILL::NORMAL_START_SRQ0x41;
    srqCodeMap["RETEST_START_SRQ0x65"]              = HANA_ART_SMILL::RETEST_START_SRQ0x65;
    srqCodeMap["RETEST_END_SRQ0x66"]                = HANA_ART_SMILL::RETEST_END_SRQ0x66;
    srqCodeMap["DUMMYTEST_START_SRQ0x42"]           = HANA_ART_SMILL::DUMMYTEST_START_SRQ0x42;
    srqCodeMap["ERROR_START_SRQ0x67"]               = HANA_ART_SMILL::ERROR_START_SRQ0x67;
    srqCodeMap["ERROR_END_SRQ0x68"]                 = HANA_ART_SMILL::ERROR_END_SRQ0x68;
    srqCodeMap["ERROR_CLEAR_SRQ0x69"]               = HANA_ART_SMILL::ERROR_CLEAR_SRQ0x69;
    srqCodeMap["AUTO_START_SRQ0x40"]                = HANA_ART_SMILL::AUTO_START_SRQ0x40;
    srqCodeMap["INIT_START_SRQ0x62"]                = HANA_ART_SMILL::INIT_START_SRQ0x62;
    srqCodeMap["LOADER_REQ_SRQ0x61"]                = HANA_ART_SMILL::LOADER_REQ_SRQ0x61;
    srqCodeMap["LOT_INFORM_SRQ0x58"]                = HANA_ART_SMILL::LOT_INFORM_SRQ0x58;
    srqCodeMap["FQATEST_START_SRQ0x71"]             = HANA_ART_SMILL::FQATEST_START_SRQ0x71;
    srqCodeMap["FQATEST_END_SRQ0x72"]               = HANA_ART_SMILL::FQATEST_END_SRQ0x72;
    srqCodeMap["WPSOCKET_REQ_SRQ0x73"]              = HANA_ART_SMILL::WPSOCKET_REQ_SRQ0x73;
    srqCodeMap["FBIN_MANUAL_REQ_SRQ0x74"]           = HANA_ART_SMILL::FBIN_MANUAL_REQ_SRQ0x74;
    srqCodeMap["REAL_PARA_SEQ_SRQ0x75"]             = HANA_ART_SMILL::REAL_PARA_SEQ_SRQ0x75;
    srqCodeMap["LOT_INFO_METHOD_SRQ0x76"]           = HANA_ART_SMILL::LOT_INFO_METHOD_SRQ0x76;
    srqCodeMap["RF_ID_SRQ0x77"]                     = HANA_ART_SMILL::RF_ID_SRQ0x77;
    srqCodeMap["SBL_SRQ0x78"]                       = HANA_ART_SMILL::SBL_SRQ0x78;
    srqCodeMap["AUTODUMPING_SRQ0x79"]               = HANA_ART_SMILL::AUTODUMPING_SRQ0x79;
    srqCodeMap["EQP_MODEL_SRQ0x43"]                 = HANA_ART_SMILL::EQP_MODEL_SRQ0x43;
    srqCodeMap["HD_MODE_SRQ0x59"]                   = HANA_ART_SMILL::HD_MODE_SRQ0x59;
    srqCodeMap["INLINEUPDATE_SRQ0x60"]              = HANA_ART_SMILL::INLINEUPDATE_SRQ0x60;
    srqCodeMap["CLEAR_START_SRQ0x00"]               = HANA_ART_SMILL::CLEAR_START_SRQ0x00;
    //
    cbCmdHANAARTtoTester->Text="";
    cbCmdHANAARTtoTester->Clear();
    for(std::map<AnsiString, HANA_ART_SMILL::SRQCode>::iterator it=srqCodeMap.begin(); it!=srqCodeMap.end(); it++)
    {
        cbCmdHANAARTtoTester->Items->Add(it->first.c_str());
    }
    cbCmdHANAARTtoTester->ItemIndex=0;
}
//------------------------------------------------------------------------------
void TSerialPoll::btnHANA_SendCBTotesterClick(TObject *Sender)
{
    AnsiString str=cbCmdHANAARTtoTester->Text;
    std::map<AnsiString, HANA_ART_SMILL::SRQCode>::iterator it=srqCodeMap.find(str);
    if(it!=srqCodeMap.end())
    {
        DoSendCommand((int)it->second);

        if(cbCmdHANAARTtoTester->Text=="DUMMYTEST_START_SRQ")                   //Steven 20250414 : HANA ART Function
        {
            for(int i=0; i<16; i++)
            {
                MY_DUT_PAL[i]->cbSiteOn->Checked = true;
                iStart[i]=1;
            }
            WriteLog("Dummy Start Test");
            IsTest=true;
            bSimulate=false;
            bNeedInital=false;
            bHanaDummyTest=true;
        }
    }
    else
    {
        DoSendCommand(0xFF);                                                    // 返回無效值
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::btnSend_EDToTesterClick(TObject *Sender)
{
    DoSendCommand(edCmdHANAARTtoTester->Text);
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
