// ===========================================================================
//  TesterComm/Rs232/Rs232Parse.cpp -- TfRS232Main tester-frame parsing: GetAnalysisString, Del1stVec, the tester
//  command dispatcher DoRevCommand, and ReceiveData_TCPIP (data from the SOFT_SIMULTE uServer socket).
//
//  AI(W906-GB-P4) 20260926: faithful translation of golden
//  D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\MainForm.cpp (Big5/cp950 -> UTF-8), per
//  TesterComm/Rs232/TRANSLATION_RULES.md.  Golden ranges, in file order:
//      3159-3218  TfRS232Main::GetAnalysisString
//      3219-3226  TfRS232Main::Del1stVec
//      3227-3726  TfRS232Main::DoRevCommand   (ENQ/ACK, ST, CF, CE, BARCODE?, GET2DID?, BA, CZ id?/which?/status?/
//                                              testerbin?/all masstemp? + CB/sitemap?/jam? + jamnumber?/soaktime?/
//                                              doublecontact?, CD, CN, _CLEAR_, everything else = Rev cycle count)
//      3727-3745  TfRS232Main::ReceiveData_TCPIP
//  Protocol reference (read-only): D:\RS232Standard\.github\skills\rs232-standard-interface
//  (HT9xxx RS232_Interface_V12.11.843).  Golden answers CA / CH / CI / CK with no branch of their own: they fall into
//  the final `else` (Rev cycle count) like any unknown command -- kept.
//
//  Bodies are golden text line by line (function-local statics, the golden #ifdef DEBUG / #ifndef DEBUG blocks at
//  :3399-3411 / :3467-3480 -- the switch below makes them the release build: iResult stored, "BA WITHOUT CE" check
//  active).  Deviations, each marked //AI(W906-GB-P4) in place:
//    * `vector<Byte>` -> `std::vector<Byte>`; `itoa` -> W906_itoa (hidden under -std=c++17, rule 7).
//    * DoRevCommand statics re-armed per program life (rule 13).
//    * CommaTextProxy has no .Length() / operator[]: read through AnsiString(...) (rule 11).
//    * BARCODE? / GET2DID?: FIXED by user ruling 20260926 (answer, in the GPIB bridge's reply format) -- see the two
//      blocks.  Original note: golden indexes CommaText from 0; BCB6 AnsiString is 1-based and range-checked, so golden
//      threw ERangeError at i==0 (golden never answered).
//    * 9 strcpy's into fixed arrays -> W906_StrCpyBounded + W906_GoldenByte (rule 15; same bytes to the tester).
//    * ReceiveData_TCPIP: exit at the point where golden spins forever (see the note there).
//  Golden quirks kept as golden (they look like bugs; the user decides):
//    * `delete List` is skipped by the GET2DID? early return, by the final `else` return and by the ERangeError
//      path: one TStringList leaked each time.
//    * The "Rev cycle clear" (final else, Jimmychiu 20240122) clears nothing: reaching iRevCycleClear only falls
//      through to iNowRevCycle=0 and `delete List`.
//    * CD / CN always answer [STX][ETX]: golden never stores sHandlerID / sSiteOnOff (OnMyCopyMsg has no
//      MSG_CMD_HanderIDRS232 / MSG_CMD_GetSiteOnOff branch; the Handler's answer lands in "Command is not supported!"
//      MainForm.cpp:1382-1385).
//    * CF answers "8" whatever the site count; CZ id? / CZ which? read the Handler's Gerneral.ini (asHGeneralPath)
//      and, when the key is missing or empty, WRITE the default "HT-9046" into it (golden cmydef.cpp:317-329).
//  Not here (other RS232 files): SendCommandToTester, SendMSG_CMD, SendResultFinish, AddBinData, ShowCommData,
//  MyDeCodeASCII, CheckAndReadIniData, CommTesterReceiveData (golden :1388-1405, same dispatch loop as
//  ReceiveData_TCPIP) and every global declared extern in Rs232Bridge.h.
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#if RS232STD_GOLDEN_DEBUG
#define DEBUG
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif

namespace rs232std {

//AI(W906-GB-P4) 20260926: Borland <stdlib.h> itoa(value, string, radix) (golden DoRevCommand :3306).  MinGW hides
//   itoa under -std=c++17 (rule 7).  Borland semantics: radix 2..36; a leading '-' only for a negative value in
//   radix 10 (any other radix formats the unsigned bit pattern); returns its buffer argument.  Golden only calls it
//   with radix 10 and value 1..32 (USE_SITE_COUNT).
static char* W906_itoa(int value, char* str, int radix)
{
    char tmp[sizeof(unsigned int)*8+1];
    char* out=str;
    if(radix<2 || radix>36)
    {
        *out='\0';
        return str;
    }
    unsigned int u;
    if(radix==10 && value<0)
    {
        *out++='-';
        u=0u-static_cast<unsigned int>(value);
    }
    else
    {
        u=static_cast<unsigned int>(value);
    }
    int n=0;
    do
    {
        const unsigned int d=u%static_cast<unsigned int>(radix);
        tmp[n++]=static_cast<char>(d<10u ? '0'+d : 'a'+(d-10u));
        u/=static_cast<unsigned int>(radix);
    } while(u!=0u);
    while(n>0)
        *out++=tmp[--n];
    *out='\0';
    return str;
}

//AI(W906-GB-P4) 20260926: rule 15.  Golden `strcpy(cX, sX.c_str())` copies a Handler answer (built from MV::Message,
//   char[2048], MessageDef.h:327) into a fixed array: cMachineStateDec[6], cTestBinCount[1024] (local), cAllMassTemp[100],
//   cSiteMap[256], cJamCode[5], cSoakTime[5], cDoubleContactCount[5], cHandlerID[100], cSiteOnOff[256].  Ordinary
//   answers overrun some of them: OnMyCopyMsg stores " %s" (leading blank), so every jam (Handler GetCZJamCode sends
//   the 4-digit code, " 0110" = 6 bytes with the '\0') overruns cJamCode[5], ambient soak time (" NONE") overruns
//   cSoakTime[5], and a machine state >= 100000 overruns cMachineStateDec[6].  In golden the spill landed in padding /
//   the next global; in-process it would corrupt whatever the linker put after the array.  This copies at most
//   dstSize-1 chars + '\0' and, like strcpy, leaves the bytes after the '\0' untouched: byte-identical to strcpy
//   whenever the string fits.
static void W906_StrCpyBounded(char* dst, size_t dstSize, const char* src)
{
    if(dst==NULL || dstSize==0)
        return;
    size_t n=0;
    if(src!=NULL)
    {
        while(n+1<dstSize && src[n]!='\0')
        {
            dst[n]=src[n];
            ++n;
        }
    }
    dst[n]='\0';
}

//AI(W906-GB-P4) 20260926: companion of W906_StrCpyBounded.  Golden then sends cX[i] for i < the source length
//   (iMacStateStrLength is set together with sMachineStateDecade, MainForm.cpp:1136-1137), i.e. it reads back exactly
//   the bytes its strcpy wrote -- including the ones that spilled past cX[] -- so the tester got the whole string.
//   Here: cX[i] inside the bounded copy, the source byte beyond it, '\0' past both (not reached by golden's loops).
static char W906_GoldenByte(const char* buf, size_t bufSize, const AnsiString& src, int i)
{
    if(i>=0 && static_cast<size_t>(i)+1<bufSize)
        return buf[i];
    if(i>=0 && i<src.Length())
        return src.c_str()[i];
    return '\0';
}

//AI(W906-GB-P4) 20260926: the two helpers that reproduced golden's ERangeError on BARCODE? / GET2DID?
//   (W906_BcbIndexOk, W906_GoldenRangeError) were removed when the user ruled 20260926 that both commands must
//   answer in the GPIB format -- the fixed loops index 1..Length and cannot go out of range.

//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden `vector<Byte>` (using namespace std).
AnsiString TfRS232Main::GetAnalysisString(std::vector<Byte> &ReceiveData)
{
    AnsiString sBack="_None_";
    if(ReceiveData.empty())
    {
    }
    else if(ReceiveData[0]==static_cast<Byte>(_STX_))
    {
        int stxPosition=0;
        int etxPosition=0;
        int iSize=ReceiveData.size();
        for(int i=0; i<iSize; i++)
        {
            if(ReceiveData[i]==static_cast<Byte>(_ETX_))
            {
                etxPosition=i;
                break;
            }
        }

        if(etxPosition>0)
        {
            sBack="";
            for(int i=stxPosition+1; i<etxPosition; i++)
            {
                sBack+=MyDeCodeASCII((int)ReceiveData[i]);
            }

            while(ReceiveData.size()>0)//remove data
            {
                iSize=ReceiveData.size();
                if(ReceiveData[0]==static_cast<Byte>(_ETX_))
                {
                    Del1stVec(ReceiveData);
                    break;
                }
                else
                {
                    Del1stVec(ReceiveData);
                }
            }
        }
    }
    else if(ReceiveData[0]==static_cast<Byte>(_ACK_))
    {
        sBack="_ACK_";
        Del1stVec(ReceiveData);
    }
    else if(ReceiveData[0]==static_cast<Byte>(_ENQ_))
    {
        sBack="_ENQ_";
        Del1stVec(ReceiveData);
    }
    else if(ReceiveData[0]!=_ENQ_ && ReceiveData[0]!=_ACK_ && ReceiveData[0]!=_STX_)    //Steven 20210805 : 清空雜訊
    {
        sBack="_CLEAR_";
    }
    return sBack;
}
//---------------------------------------------------------------------------
void TfRS232Main::Del1stVec(std::vector<Byte> &ReceiveData)
{
    if(!ReceiveData.empty())
    {
        ReceiveData.erase(ReceiveData.begin());
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::DoRevCommand(AnsiString sCommand)
{
    static int iNowRevCycle=0;
    static int iSendStart=0;
    char cSit[MAX_SITE_COUNT]={0};
    char cTemp[5] = {0};
    bool bFirst = true;// , bError = false;
    int iStrLength=0;
    int SiteNo = 3;
    int iSite, iBinCode,i;
    static AnsiString asData="", asDataTemp="";
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).  asData / asDataTemp are never used by golden; re-armed anyway.
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        iNowRevCycle=0;
        iSendStart=0;
        asData="";
        asDataTemp="";
    }
    ZeroMemory(cSit, sizeof(cSit));
    ZeroMemory(cTemp, sizeof(cTemp));
    TStringList *List=new TStringList;

    if(sCommand.Pos("_ENQ_")>0)                                                 // receive "05" ,then send "06"
    {
        iSendStart=0;                                                           //Jou 2015-02-02
        SendData.clear();
        SendData.push_back(_ACK_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("_ACK_")>0)
    {
        if(iSendStart!=0)                                                       //Test send "06"
        {
            SendCommandToTester(SendStartData);

            if(iSendStart==2)                                                   //測試中
            {
                if(iHasCE==iWaitReplyCE)                                        //Steven 20231205 : 判斷RS232流程是否異常
                {
                    iHasCE=iReplyCE;
                }

                for(int i=0; i<USE_SITE_COUNT; i++)                             //Steven 20231101 : 正確送出資料後, 才清除開site的flag
                {
                    iStart[i]=false;                                            //wei 20150603 送出Site就清除
                }
            }
            iSendStart=0;
        }
    }
    else if(sCommand.Pos("ST")>0)                                               //Steven 20141014 : 神盾測試模式
    {
        SendMSG_CMD(MSG_CMD_SwitchArm);
    }
    else if(sCommand.Pos("CF")>0)                                               // Tester send "02CF03 " 0x43,0x46
    {
        SendStartData.clear();                                                  //Jou 2015-02-02
        SendStartData.push_back(_STX_);
        SendStartData.push_back('8');
        SendStartData.push_back(_ETX_);

        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CE")>0)                                               // Tester send "02CE03 " 0x43,0x45
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);

        if(bSimulate==false)                                                    //Steven 20210409 : 離線模式下,不要回覆測試site
        {
            bFirst=true;
            iSendStart=1;
            for(int i=0; i<USE_SITE_COUNT; i++)                                 //jou 2015-03-23 use site
            {
                if(iStart[i]==true)                                             // IndexArm ready to test
                {
                    iSendStart=2;                                               //Steven 20231114 : 確認有要測試才等於true, 避免秒差造成不測試
                    if(bFirst==false)
                    {
                        SendStartData.push_back(',');
                    }

                    //AI(W906-GB-P4) 20260926: golden itoa(i+1, cSit, 10) (rule 7, see W906_itoa).
                    W906_itoa(i+1, cSit, 10);

                    if(i<9)
                    {
                        SendStartData.push_back(cSit[0]);
                    }
                    else                                                        //jou 2012-11-07 修正10位數以上的 site，無法測試。
                    {
                        SendStartData.push_back(cSit[0]);
                        SendStartData.push_back(cSit[1]);
                    }
                    iHasCE=iWaitReplyCE;                                        //Steven 20231205 : 判斷RS232流程是否異常
                    bFirst=false;
                }
            }
        }
        else
        {
            iSendStart=1;
        }

        SendStartData.push_back(_ETX_);

        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    //JimmyChiu 20211004 : add Barcode?指令
    //<==
    else if(sCommand.Pos("BARCODE?")>0)                                         //add Barcode?指令
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        SendStartData.push_back('B');                                           //Steven 20211101 : 修改RS232要比照GPIB回復Barcode命令
        SendStartData.push_back('A');
        SendStartData.push_back('R');
        SendStartData.push_back('C');
        SendStartData.push_back('O');
        SendStartData.push_back('D');
        SendStartData.push_back('E');
        SendStartData.push_back(':');
        //AI(W906-GB-P4) 20260926: golden `sBarCode->CommaText.Length()` (CommaTextProxy has no .Length(), rule 11).
        const int iStrLength=AnsiString(sBarCode->CommaText).Length();
        //AI(W906-GB-P4) 20260926: USER RULING 20260926 (decision list item 2 = B): "修成會回答，比照 GPIB 的通訊格式".
        //   golden pushed `sBarCode->CommaText[i]` with i from 0; BCB6 AnsiString is 1-based and range-checked, so golden
        //   threw ERangeError at i==0 and BARCODE? never answered (same in Rev12.12.854 / .873 / .874).  Fixed: take
        //   characters 1..Length.  The payload is now exactly the GPIB bridge's reply "BARCODE:<CommaText>;"
        //   (GpibCommands.cpp BARCODE? / golden GPIB Main.cpp ProcessStatusString), inside RS232's own [STX]...[ETX]
        //   frame and ENQ/ACK handshake (unchanged).
        const AnsiString asBarCodeList=AnsiString(sBarCode->CommaText);
        for(int i=1; i<=iStrLength; i++)
        {
            SendStartData.push_back(asBarCodeList[i]);
        }
        SendStartData.push_back(';');
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("GET2DID?")>0)                                         //add GET2DID?指令
    {
        if( sBarCode->CommaText=="" || sBarCode->Count!=USE_SITE_COUNT)
        {
            ReceiveData.clear();
            SendData.clear();
            //AI(W906-GB-P4) 20260926: golden returns here without `delete List` (one TStringList leaked) and without
            //   the iNowRevCycle=0 below; kept.
            return;
        }
        SendStartData.clear();
        SendStartData.push_back(_STX_);

        sBarCode_ASE_CL->Clear();
        for(int i=0; i<USE_SITE_COUNT; i++)
        {
            sBarCode_ASE_CL->Add(sBarCode->Strings[USE_SITE_COUNT-1-i]);
        }
        //AI(W906-GB-P4) 20260926: golden `sBarCode_ASE_CL->CommaText.Length()` (rule 11).
        const int iStrLength=AnsiString(sBarCode_ASE_CL->CommaText).Length();
        //AI(W906-GB-P4) 20260926: USER RULING 20260926 (decision list item 2 = B): "修成會回答，比照 GPIB 的通訊格式".
        //   golden had the same 0-based indexing as BARCODE? above, and here it ALWAYS threw (the guard demands 32
        //   entries, so CommaText is never empty): GET2DID? never answered.  Fixed: characters 1..Length, then the
        //   trailing ',' the GPIB bridge sends -- its reply is `sBarCodeMsg.sprintf("%s,", sBarCode_ASE_CL->CommaText)`
        //   (GpibCommands.cpp GET2DID?, KaiChen 20191126 中壢日月光 format).  Frame [STX]...[ETX] and ENQ/ACK unchanged.
        const AnsiString asBarCodeListCL=AnsiString(sBarCode_ASE_CL->CommaText);
        for(int i=1; i<=iStrLength; i++)
        {
            SendStartData.push_back(asBarCodeListCL[i]);
        }
        SendStartData.push_back(',');
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    //<==
    //JimmyChiu 20211004 : add Barcode?指令
    else if(sCommand.Pos("BA")>0)                                               //wei 20150604  只收到B  // Tester send result ex."02 BA 1,1,1;2,1,1 03" 0x42,0x41
    {
        AnsiString iTestResult="";

        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {
            iResult[i]=0;
        }

        SiteNo=3;
        iStrLength=sCommand.Length();
        iTestResult=sCommand.SubString(SiteNo, iStrLength);
        iStrLength=iTestResult.Length();

        #ifdef DEBUG
        ShowCommData("Tester Result", sCommand);
        #endif

        AddBinData(iTestResult);                                                //Steven 20171013 (wei) : 畫面顯示從測試機收到的Bin別

        #ifndef DEBUG
        if(iHasCE!=iReplyCE)                                                    //Steven 20231205 : 判斷RS232流程是否異常
        {
            ShowCommData("[Tester]", "BA WITHOUT CE ERROR!!");
            iHasCE=iRS232Error;
        }
        #endif

        while(iTestResult!="")
        {
            List->Clear();
            if(iTestResult.AnsiPos(";")!=0)                                     //Steven 20250405 : 變更分bin判斷方式
            {
                List->CommaText=iTestResult.SubString(1, iTestResult.AnsiPos(";")-1);
                iTestResult    =iTestResult.SubString(iTestResult.AnsiPos(";")+1, iTestResult.Length());
            }
            else
            {
                List->CommaText=iTestResult;
                iTestResult="";
            }

            if(List->Count==3)
            {
                iSite       =atoi(List->Strings[0].c_str())-1;
                if(List->Strings[2]=="A" || List->Strings[2]=="a")
                {
                    iBinCode=10;
                }
                else if(List->Strings[2]=="B" || List->Strings[2]=="b")
                {
                    iBinCode=11;
                }
                else if(List->Strings[2]=="C" || List->Strings[2]=="c")
                {
                    iBinCode=12;
                }
                else if(List->Strings[2]=="D" || List->Strings[2]=="d")
                {
                    iBinCode=13;
                }
                else if(List->Strings[2]=="E" || List->Strings[2]=="e")
                {
                    iBinCode=14;
                }
                else if(List->Strings[2]=="F" || List->Strings[2]=="f")
                {
                    iBinCode=15;
                }
                else
                {
                    iBinCode=atoi(List->Strings[2].c_str());
                }

                if(iBinCode>=iMaxBinCount)
                    iBinCode=999;

                if(iSite<0 || iSite>=USE_SITE_COUNT || iBinCode<0)              //jou 2015-03-23 use site
                {
                    continue;
                }

                #ifndef DEBUG
                if(iHasCE==iRS232Error)                                         //Steven 20231205 : 判斷RS232流程是否異常
                {
                    iResult[iSite]=999;
                }
                else
                {
                    iResult[iSite]=iBinCode;
                }
                #endif

                #ifdef DEBUG
                ShowCommData("Tester ", List->CommaText, AnsiString(iBinCode));
                #endif
            }

//            if(iTestResult.AnsiPos(";")!=0)
//                iTestResult=iTestResult.SubString(iTestResult.AnsiPos(";")+1, iTestResult.Length());   //砍掉分號
//            else
//                iTestResult="";
        }

        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {
            GGpib2Handler.Result[i]=iResult[i];
            iStart[i]=false;
        }

        SendResultFinish();
        iHasCE=iNoneTest;

        //Steven 20210120 : 處理ENQ黏在BA命令後面
        //==>
//        iStrLength=ReceiveData.size();
//        if(ReceiveData[iStrLength-2]==_ETX_ && ReceiveData[iStrLength-1]==_ENQ_)
//        {
            iSendStart=0;  //Jou 2015-02-02
            SendData.clear();
            SendData.push_back(_ACK_);
            SendCommandToTester(SendData);
//        }
        //<==
        //Steven 20210120 : 處理ENQ黏在BA命令後面
    }
    else if(sCommand.Pos("CZ id?")>0)                                           //Steven 20200218 : for力成,測試機Send "CZ id?"
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        //AI(W906-GB-P4) 20260926: reads the Handler's Gerneral.ini (asHGeneralPath) and WRITES "HT-9046" into it when
        //   [Version] Model is missing or empty (golden cmydef.cpp:317-329) -- the production-shared file unless the
        //   engine redirects asHGeneralPath.
        AnsiString sID=CheckAndReadIniData(asHGeneralPath, "Version", "Model", AnsiString("HT-9046"));
        const int iStrLength=sID.Length();
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sID.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ which?")>0)                                        //Steven 20200218 : for力成,測試機Send "CZ which?"
    {
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        //AI(W906-GB-P4) 20260926: same Gerneral.ini read / write-back as CZ id? ([Version] Machine ID).
        AnsiString sID=CheckAndReadIniData(asHGeneralPath, "Version", "Machine ID", AnsiString("HT-9046"));
        const int iStrLength=sID.Length();
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sID.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ status?")>0)                                       //JerryYang 20151109 for力成,測試機Send "CZ status?"
    {
        //AI(W906-GB-P4) 20260926: this and the CZ / CD / CN branches below rely on golden SendMessage semantics: the
        //   Handler's answer (OnMyCopyMsg -> sMachineStateDecade etc.) is processed while SendMSG_CMD is still
        //   waiting, so the lines after it already see the fresh value (PostToHandler / mailbox contract).
        SendMSG_CMD(MSG_CMD_MachineState);                                      //向handler要機台狀態

        SendStartData.clear();
        SendStartData.push_back(_STX_);

        //AI(W906-GB-P4) 20260926: golden strcpy(cMachineStateDec, sMachineStateDecade.c_str()) and cMachineStateDec[i]
        //   (rule 15, see W906_StrCpyBounded / W906_GoldenByte).
        W906_StrCpyBounded(cMachineStateDec, sizeof(cMachineStateDec), sMachineStateDecade.c_str());
        for(i=0; i<iMacStateStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cMachineStateDec, sizeof(cMachineStateDec), sMachineStateDecade, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ testerbin?")>0)                                    //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ testerbin?"
    {
        SendMSG_CMD(MSG_CMD_TesterBin);                                         //向handler要各Bin數量

        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iTestBinLength=sTestBinCount.Length();
        char cTestBinCount[1024];
        ZeroMemory(cTestBinCount, sizeof(cTestBinCount));
        //AI(W906-GB-P4) 20260926: golden strcpy(cTestBinCount, sTestBinCount.c_str()) into this LOCAL 1024-byte array
        //   (it shadows the global) and cTestBinCount[i] (rule 15).
        W906_StrCpyBounded(cTestBinCount, sizeof(cTestBinCount), sTestBinCount.c_str());
        for(i=0; i<iTestBinLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cTestBinCount, sizeof(cTestBinCount), sTestBinCount, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ all masstemp?")>0 ||                               //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ all masstemp?"
            sCommand.Pos("CB")>0)
    {
        SendMSG_CMD(MSG_CMD_AllMassTemp);                                       //向handler要溫度

        SendStartData.clear();
        SendStartData.push_back(_STX_);
        //AI(W906-GB-P4) 20260926: golden strcpy(cAllMassTemp, sAllMassTemp.c_str()) and cAllMassTemp[i] (rule 15).
        W906_StrCpyBounded(cAllMassTemp, sizeof(cAllMassTemp), sAllMassTemp.c_str());
        const int iStrLength=sAllMassTemp.Length();
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cAllMassTemp, sizeof(cAllMassTemp), sAllMassTemp, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------

        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ sitemap?")>0)                                      //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ sitemap?"
    {
        SendMSG_CMD(MSG_CMD_SiteMap);                                           //向handler要SiteMap
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        //AI(W906-GB-P4) 20260926: golden strcpy(cSiteMap, sSiteMap.c_str()) and cSiteMap[i] (rule 15).
        W906_StrCpyBounded(cSiteMap, sizeof(cSiteMap), sSiteMap.c_str());
        const int iStrLength=sSiteMap.Length();
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cSiteMap, sizeof(cSiteMap), sSiteMap, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ jam?")>0 ||                                        //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ jam?"
            sCommand.Pos("CZ jamnumber?")>0)                                    //Isaac 20170825(jou) for Amkor_philippine,測試機Send "CZ jamnumber?"
    {
        SendMSG_CMD(MSG_CMD_JamCode);                                           //向handler要機台Jam Code
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iJamCodeLength=sJamCode.Length();
        //AI(W906-GB-P4) 20260926: golden strcpy(cJamCode, sJamCode.c_str()) and cJamCode[i] (rule 15): cJamCode is
        //   char[5] and a jam answer is " 0110" (5 chars + '\0'), so golden overran it on every jam.
        W906_StrCpyBounded(cJamCode, sizeof(cJamCode), sJamCode.c_str());
        for(i=0; i<iJamCodeLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cJamCode, sizeof(cJamCode), sJamCode, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ soaktime?")>0)                                     //JerryYang 20160308 for Maxim_Philippine,測試機Send "CZ soaktime?"
    {
        SendMSG_CMD(MSG_CMD_SoakTime);                                          //向handler要Soak time
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sSoakTime.Length();
        //AI(W906-GB-P4) 20260926: golden strcpy(cSoakTime, sSoakTime.c_str()) and cSoakTime[i] (rule 15; " NONE"
        //   overran char[5]).
        W906_StrCpyBounded(cSoakTime, sizeof(cSoakTime), sSoakTime.c_str());
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cSoakTime, sizeof(cSoakTime), sSoakTime, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CZ doublecontact?")>0)                                //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
    {
        SendMSG_CMD(MSG_CMD_DoubleContactCount);                                //向handler要doublecontactcount
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        //AI(W906-GB-P4) 20260926: golden strcpy(cDoubleContactCount, sDoubleContactCount.c_str()) (rule 15).  This
        //   branch already sends from the AnsiString, as golden.
        W906_StrCpyBounded(cDoubleContactCount, sizeof(cDoubleContactCount), sDoubleContactCount.c_str());
        const int iStrLength=sDoubleContactCount.Length();
        for(int i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(sDoubleContactCount.c_str()[i]);
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CD")>0)
    {
        //AI(W906-GB-P4) 20260926: golden never assigns sHandlerID (OnMyCopyMsg has no MSG_CMD_HanderIDRS232 branch), so
        //   this answers [STX][ETX]; kept.
        SendMSG_CMD(MSG_CMD_HanderIDRS232);                                     //向handler要handler ID
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sHandlerID.Length();
        //AI(W906-GB-P4) 20260926: golden strcpy(cHandlerID, sHandlerID.c_str()) and cHandlerID[i] (rule 15).
        W906_StrCpyBounded(cHandlerID, sizeof(cHandlerID), sHandlerID.c_str());
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cHandlerID, sizeof(cHandlerID), sHandlerID, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("CN")>0)
    {
        //AI(W906-GB-P4) 20260926: golden never assigns sSiteOnOff (no MSG_CMD_GetSiteOnOff branch in OnMyCopyMsg), so
        //   this answers [STX][ETX]; kept.
        SendMSG_CMD(MSG_CMD_GetSiteOnOff);
        SendStartData.clear();
        SendStartData.push_back(_STX_);
        const int iStrLength=sSiteOnOff.Length();
        //AI(W906-GB-P4) 20260926: golden strcpy(cSiteOnOff, sSiteOnOff.c_str()) and cSiteOnOff[i] (rule 15).
        W906_StrCpyBounded(cSiteOnOff, sizeof(cSiteOnOff), sSiteOnOff.c_str());
        for(i=0; i<iStrLength; i++)
        {
            SendStartData.push_back(W906_GoldenByte(cSiteOnOff, sizeof(cSiteOnOff), sSiteOnOff, i));
        }
        SendStartData.push_back(_ETX_);
        iSendStart=1;
        // ---------------------------------------------------------------------
        SendData.push_back(_ENQ_);
        SendCommandToTester(SendData);
    }
    else if(sCommand.Pos("_CLEAR_")>0)                                          //Steven 20210805 : 清空雜訊
    {
        ReceiveData.clear();
        SendData.clear();
    }
    else                                                                        //Jimmychiu 20240122 : add Rev cycle clear
    {
        iNowRevCycle++;
        if(iNowRevCycle<iRevCycleClear)
        {
            //AI(W906-GB-P4) 20260926: golden returns without `delete List` (one TStringList leaked per unrecognised
            //   command, iRevCycleClear-1 times out of iRevCycleClear); kept.
            return;
        }
    }
    iNowRevCycle=0;

    List->Clear();
    delete List;
}
//---------------------------------------------------------------------------
void TfRS232Main::ReceiveData_TCPIP(char* cGet, int iRevLen)
{
    ReceiveData.clear();
    for(int i=0; i<iRevLen; i++)
    {
        ReceiveData.push_back(*((Byte*)cGet+i));
    }

    ShowCommData("[T->H]", ReceiveData);
    AnsiString sCommand=GetAnalysisString(ReceiveData);                         //Jimmychiu 20240125 : Add AnalysisString function
    int iMaxDeal=10,iNowDeal=0;
    while(((sCommand.Pos("_None_")>0)==false) || iNowDeal>iMaxDeal)             //clear when get none or over deal
    {
        //AI(W906-GB-P4) 20260926: hang guard (rule 3, UB).  The `|| iNowDeal>iMaxDeal` term (the comment's intent --
        //   stop "when get none or over deal" -- needed `&&`) keeps the loop alive once one receive has held 12+
        //   commands, and "_None_" is then a fixed point: DoRevCommand("_None_") matches no branch (it only counts
        //   iNowRevCycle) and leaves ReceiveData alone, so GetAnalysisString keeps returning "_None_".  Golden
        //   RS232Standard.exe spins there forever, leaking a TStringList on iRevCycleClear-1 of every iRevCycleClear
        //   calls, and iNowDeal++ overflows int (UB).  In-process that is the TesterComm thread lost and the Handler's
        //   heap growing.  Leaving at exactly that point changes nothing golden finishes: all 12+ commands are still
        //   dispatched, and the loop ends in the state it ends in for 0..11.  CommTesterReceiveData (:1399) has the
        //   same loop.
        if(sCommand.Pos("_None_")>0)
            break;
        DoRevCommand(sCommand);
        sCommand=GetAnalysisString(ReceiveData);
        iNowDeal++;
    }
}
//---------------------------------------------------------------------------

}  // namespace rs232std
