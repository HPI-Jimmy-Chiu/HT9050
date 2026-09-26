// ===========================================================================
//  TesterComm/Gpib/GpibCore.cpp -- TSerialPoll core: BINON checks, Handler connect, the ProcessMessage task loop,
//  ProcessAddress, MyGPIBWrite, InitialStartValue and the SendMSG_CMD* family (bridge -> Handler packets).
//
//  AI(W906-GB-P1) 20260926: faithful translation of golden
//  D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525\Main.cpp (Big5/cp950 -> UTF-8), per
//  TesterComm/Gpib/TRANSLATION_RULES.md.  Golden ranges, in file order:
//      760-955    TSerialPoll::CheckGSBINONString
//      956-986    Check2Dsum (free function)
//      987-1454   CheckBINONString (free function)
//      3082-3137  TSerialPoll::ProcessHMountConnect
//      3919-3951  TSerialPoll::SendCaptureFinish
//      3953-4253  TSerialPoll::ProcessMessage
//      4255-4287  TSerialPoll::ProcessAddress
//      4639-4778  TSerialPoll::MyGPIBWrite
//      4780-4792  TSerialPoll::InitialStartValue
//      4812-4894  mnet/l112/l122 function pointers + TSerialPoll::SVON_CLOSE (golden: one /* */ block, gated here)
//      4896-4993  TSerialPoll::SendMSG_CMD(int), SendMSG_CMD(int, AnsiString), SendMSG_CMD_ESC, SendMSG_CMD_INPUTQTY
//  Bodies are golden text line by line (function-local statics, `static int &Task=iMainTask`, the golden quirks
//  listed below).  Deviations, each marked //AI(W906-GB-P1) in place:
//    * `__fastcall` dropped (rule 3).
//    * SendMessage(HMountWnd, WM_COPYDATA, ...) -> PostToHandler(pcp); Close() -> RequestClose(...) (rule 4).
//    * FindWindow("TfMain", ...) -> W906_FindHandlerWnd(): the Handler is attached while mailbox != NULL and
//      HMountWnd != NULL (GpibEngine::Start sets HMountWnd to its token before FormCreate).
//    * this->Handle -> reinterpret_cast<HWND>(this) (rule 4; == GpibEngine::BridgeWndToken()).
//    * TStringList::Strings[i] is a vclcompat proxy: `.Length()` and passing it to sprintf go through
//      AnsiString(...) (a proxy through C varargs would not compile / would be UB).
//    * AnsiString::AnsiCompare, CLK_TCK: file-local stand-ins with the BCB6 semantics (below).
//    * MyGPIBWrite `Task!=NULL` -> `Task!=AnsiString(0)`: the operator BCB6 actually bound (see the note there).
//    * ProcessHMountConnect: log-only fallback for Str1 (the Handler caption golden's title loop would have found).
//    * 4 UB guards that keep the golden outcome: CheckGSBINONString cBuffer[i], CheckBINONString cBuffer size,
//      MyGPIBWrite buffer size, ProcessMessage `ct` start value (see each note).
//    * Widgets are written without TSerialPoll::uiMutex, like the golden bodies (the P7 snapshot owner decides).
//  Not here (other bridge files): WriteLog, SaveResult, UpdateLed, TestGPIB, WriteGpibString, PostToHandler,
//  RequestClose, and every global declared extern in GpibBridge.h.
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace gpibbridge {

//AI(W906-GB-P1) 20260926: golden FindWindow("TfMain", <Handler window caption>) (Main.cpp:3096 / :3110).  In-process
//   there is no Handler window to search: the Handler side is "found" while it is attached, i.e. while the engine owner
//   has set HMountWnd to its non-NULL token and the mailbox exists (GpibEngine::Start, before FormCreate).  The caption
//   argument is ignored; it is kept so both golden call sites keep their shape.
static HWND W906_FindHandlerWnd(const TSerialPoll* self, const char* /*caption*/)
{
    if(self!=NULL && self->mailbox!=NULL && HMountWnd!=NULL)
        return HMountWnd;
    return NULL;
}

//AI(W906-GB-P1) 20260926: BCB6 AnsiString::AnsiCompare (Include/Vcl/dstring.h:100) is SysUtils AnsiCompareStr =
//   CompareString(LOCALE_USER_DEFAULT, 0, S1, Length(S1), S2, Length(S2)) - 2  (<0 / 0 / >0).  vclcompat AnsiString
//   has no AnsiCompare member (rule 5: file-local helper with the golden semantics).
static int W906_AnsiCompare(const AnsiString& S1, const AnsiString& S2)
{
    return CompareStringA(LOCALE_USER_DEFAULT, 0, S1.c_str(), S1.Length(), S2.c_str(), S2.Length()) - 2;
}

//AI(W906-GB-P1) 20260926: golden ProcessMessage divides a DWORD tick delta by CLK_TCK, which BCB6 Include/time.h:53
//   defines as 1000.0 -- a double, so lblTestTime shows "Test Time : 12.345Sec".  MinGW's CLK_TCK is the integer
//   CLOCKS_PER_SEC (and is hidden under -std=c++17), which would truncate to whole seconds; the BCB6 value is spelled
//   out here instead.
static const double W906_BCB_CLK_TCK=1000.0;

//AI(W906-GB-P1) 20260926: golden SendMSG_CMD(int, AnsiString) strncpy's up to 544 bytes starting at VM::cReturn
//   ("Message最大是 256+32+256=544", Main.cpp:4924): it deliberately runs on through GpibStatus[32] and GpibData[256],
//   which follow cReturn contiguously in VM (char arrays, no padding), and zero-pads the whole 544-byte span.  Kept
//   byte-for-byte; this assert pins the MessageDef.h layout that span relies on.
static_assert(offsetof(VM, GpibData) + sizeof(VM::GpibData) - offsetof(VM, cReturn) == 544,
              "VM cReturn+GpibStatus+GpibData must be the contiguous 544-byte span golden SendMSG_CMD writes");

//------------------------------------------------------------------------------
//PASS : false
//FAIL : true
//------------------------------------------------------------------------------
bool TSerialPoll::CheckGSBINONString(AnsiString str)                            //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
{
    bool bErrorFlag=false;
    int iLength;
    TStringList *sList;
    sList=new TStringList();
    AnsiString sBinon;
    char cBuffer[2];
    char cBuffer1;

    sBinon=str.SubString(7, str.Length());
    sBinon=StringReplace(sBinon, "\r", "", TReplaceFlags()<<rfReplaceAll);
    sBinon=StringReplace(sBinon, "\n", "", TReplaceFlags()<<rfReplaceAll);
    sBinon=StringReplace(sBinon, ";",  "", TReplaceFlags()<<rfReplaceAll);
    sBinon=sBinon.UpperCase();
    sList->CommaText=sBinon;
    if(sList->Count!=TOTAL_SITE)
    {
        bErrorFlag=true;
    }
    else
    {
        if(LastSet.iTesterMode==InterfaceType_16BinGS)
        {
            //----------------------------------------------------------------------
            //BINON:A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,2,2;
            //----------------------------------------------------------------------
            //Bin Mapping
            //Handler:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16
            //Tester:   A,  1,  2,  3,  4,  5,  6,  7,  8,  9,  B,  C,  D,  E,  F,  G,  H
            //----------------------------------------------------------------------
            for(int i=0; i<TOTAL_SITE; i++)
            {
                //AI(W906-GB-P1) 20260926: vclcompat Strings[i] is a proxy without .Length(); read it as AnsiString (also golden :844)
                if(AnsiString(sList->Strings[TOTAL_SITE-i-1]).Length()==1)      //Start from site 0
                {
                    strncpy(cBuffer, sList->Strings[TOTAL_SITE-i-1].c_str(), sizeof(cBuffer));
                    //AI(W906-GB-P1) 20260926: golden `cBuffer1=cBuffer[i];` indexes the 2-byte cBuffer with the SITE
                    //   index i (0..31); cBuffer[0] was almost certainly meant.  Golden effect: i==0 reads the site char,
                    //   i==1 reads the strncpy '\0' pad -> else branch -> bErrorFlag=true, i>=2 read stack garbage
                    //   (UB in C++).  Because i==1 always fails, golden InterfaceType_16BinGS ALWAYS ends with
                    //   bErrorFlag=true and every result[]=9999 (SaveResult only saves lstRecord, so the garbage is never
                    //   observed).  Kept: '\0' stands in for the out-of-range reads, same outcome.  Changing it to
                    //   cBuffer[0] would make 16BinGS BINON work -- a behaviour change for the user to decide.
                    cBuffer1=(i<(int)sizeof(cBuffer)) ? cBuffer[i] : '\0';
                    if(cBuffer1>='0' && cBuffer1<='9')
                        result[i]=atoi(cBuffer);
                    else if(cBuffer1=='A')
                        result[i]=0;
                    else if(cBuffer1=='B')
                        result[i]=10;
                    else if(cBuffer1=='C')
                        result[i]=11;
                    else if(cBuffer1=='D')
                        result[i]=12;
                    else if(cBuffer1=='E')
                        result[i]=13;
                    else if(cBuffer1=='F')
                        result[i]=14;
                    else if(cBuffer1=='G')
                        result[i]=15;
                    else if(cBuffer1=='H')
                        result[i]=16;
                    else
                    {
                        result[i]=9999;
                        bErrorFlag=true;
                    }
                }
                else
                {
                    result[i]=9999;
                    bErrorFlag=true;
                }
            }
        }
        else if(LastSet.iTesterMode==InterfaceType_32BinGS)
        {
            //BINON:A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,2,2;
            //----------------------------------------------------------------------
            //Bin Mapping
            //Handler:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
            //Tester:   A,  1,  2,  3,  4,  5,  6,  7,  8,  9,  B,  C,  D,  E,  F, 10, 11
            //
            //Handler: 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
            //Tester:  12, 13, 14, 15, 16, 17, 18, 19, 1A, 1B, 1C, 1D, 1E, 1F, 20, 21
            //----------------------------------------------------------------------
            for(int i=0; i<TOTAL_SITE; i++)
            {
                iLength=AnsiString(sList->Strings[TOTAL_SITE-i-1]).Length();
                sBinon =sList->Strings[TOTAL_SITE-i-1];
                if(iLength==1)      //Start from site 0
                {
                    if(sBinon=="0" || sBinon=="A")
                        result[i]=0;
                    else if(sBinon=="1")
                        result[i]=1;
                    else if(sBinon=="2")
                        result[i]=2;
                    else if(sBinon=="3")
                        result[i]=3;
                    else if(sBinon=="4")
                        result[i]=4;
                    else if(sBinon=="5")
                        result[i]=5;
                    else if(sBinon=="6")
                        result[i]=6;
                    else if(sBinon=="7")
                        result[i]=7;
                    else if(sBinon=="8")
                        result[i]=8;
                    else if(sBinon=="9")
                        result[i]=9;
                    else if(sBinon=="B")
                        result[i]=10;
                    else if(sBinon=="C")
                        result[i]=11;
                    else if(sBinon=="D")
                        result[i]=12;
                    else if(sBinon=="E")
                        result[i]=13;
                    else if(sBinon=="F")
                        result[i]=14;
                    else
                    {
                        result[i]=9999;
                        bErrorFlag=true;
                    }
                }
                else if(iLength==2)
                {
                    if(sBinon=="10")
                        result[i]=15;
                    else if(sBinon=="11")
                        result[i]=16;
                    else if(sBinon=="12")
                        result[i]=17;
                    else if(sBinon=="13")
                        result[i]=18;
                    else if(sBinon=="14")
                        result[i]=19;
                    else if(sBinon=="15")
                        result[i]=20;
                    else if(sBinon=="16")
                        result[i]=21;
                    else if(sBinon=="17")
                        result[i]=22;
                    else if(sBinon=="18")
                        result[i]=23;
                    else if(sBinon=="19")
                        result[i]=24;
                    else if(sBinon=="1A")
                        result[i]=25;
                    else if(sBinon=="1B")
                        result[i]=26;
                    else if(sBinon=="1C")
                        result[i]=27;
                    else if(sBinon=="1D")
                        result[i]=28;
                    else if(sBinon=="1E")
                        result[i]=29;
                    else if(sBinon=="1F")
                        result[i]=30;
                    else if(sBinon=="20")
                        result[i]=31;
                    else if(sBinon=="21")
                        result[i]=32;
                    else
                    {
                        result[i]=9999;
                        bErrorFlag=true;
                    }
                }
                else
                {
                    result[i]=9999;
                    bErrorFlag=true;
                }
            }
        }
        else
        {
            bErrorFlag=true;
        }
    }

    if(bErrorFlag==true)
    {
        SerialPoll->SaveResult();
        for(int i=0; i<TOTAL_SITE; i++)
        {
            result[i]=9999;
        }

        WriteLog("Tester ==> CHECK BINON STRING LENGTH ERROR");                 //Steven 20141212 : Add GPIB Log
    }
    sList->Clear();                                                             //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
    delete sList;
    return bErrorFlag;
}
//------------------------------------------------------------------------------
AnsiString Check2Dsum(AnsiString asChk)                                         //Ifor 20190107 : add AMD Barcode Cheak
{
    int i, j;
    int k=0;
    AnsiString asR;
    int icheck=asChk.Pos("#");
    asChk=asChk.SubString(0, icheck-1);

    const char *chk=asChk.c_str();                                              //AI(W906-GB-P1) 20260926: vclcompat c_str() is const char*
    j=asChk.Length();
    for(i=0; i<j; i++)
    {
        k+=int(chk[i]);
    }
    k=k%128;

//    if(asChk.Pos("DECODEFAIL")==1)
//    {
//        asR.sprintf("040%s%03d\r\n",asChk,k);
//    }
//    else if(asChk.Pos("BARCODEDISABLED")==1)
//    {
//        asR.sprintf("015%s%03d\r\n",asChk,k);
//    }
//    else
//    {
//        asR.sprintf("%03d%s%03d\r\n",j,asChk,k);
//    }
    asR.sprintf("%03d%s%03d", j, asChk, k);                                     //Ifor 20190923 Fix GPIB 回覆命令無 \r\n
    return asR;
}
//------------------------------------------------------------------------------
//jou 判斷BINON字串是否正確 2010-04-13
//PASS : true
//FAIL : false
bool CheckBINONString(char *str)
{
    //BINON:00000000,00000000,00888888,80888888;\n
    //01234567890123456789012345678901234567890
    //          1   1     2  22     3 33

    //AI(W906-GB-P1) 20260926: golden cBuffer[StrLength].  strncpy(.., sizeof(cBuffer)) leaves it unterminated when str
    //   has >= StrLength chars (TestGPIB passes up to iBufferCount == StrLength) and `asBuffer=cBuffer` then reads past
    //   the end.  One extra byte + an explicit terminator keep the golden intent (at most StrLength chars checked).
    char cBuffer[StrLength+1];                                                  //kevin 20140306 cBuffer[100];
    int iLen=0,i;
    bool bFlag=true;
    AnsiString asBuffer, CompBuffer="";
    char asBuffer1;

    strncpy(cBuffer, str, sizeof(cBuffer));
    cBuffer[StrLength]='\0';                                                     //AI(W906-GB-P1) 20260926: see cBuffer above
    asBuffer=cBuffer;
    iLen=asBuffer.Length();
    int K=1,H=0;                                                                // 0-255 bin 一次取3 個字元當一個site   kevin 20140305
    if(iLen<41)
        bFlag=false;

    for(i=0; i<iLen; i++)
    {
        if(LastSet.iTesterMode==InterfaceType_15BinT6577)                       //Steven 20181214 : ASE-CL add T6577
        {
            //BINON:AAAAAAAA,AAAAAAAA,AAAAAAAA,AAAAAA22;
            //----------------------------------------------------------------------
            //Bin Mapping
            //Handler:  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
            //Tester:   A,  1,  2,  3,  4,  5,  6,  7,  8,  9,  B,  C,  D,  E,  F, G
            //----------------------------------------------------------------------
            asBuffer1=cBuffer[i];
            switch(i)
            {
                case 0:
                    if(asBuffer1!='B')
                        bFlag=false;
                    break;
                case 1:
                    if(asBuffer1!='I')
                        bFlag=false;
                    break;
                case 2:
                case 4:
                    if(asBuffer1!='N')
                        bFlag=false;
                    break;
                case 3:
                    if(asBuffer1!='O')
                        bFlag=false;
                    break;
                case 5:
                    if(asBuffer1!=':')
                        bFlag=false;
                    break;
                case 14:
                case 23:
                case 32:
                    if(asBuffer1!=',')
                        bFlag=false;
                    break;
                case 41:
                    if(asBuffer1==';' || asBuffer1=='\r' || asBuffer1=='\0' || asBuffer1=='\n') //Steven 20110406 : 正確的結尾
                        ;
                    else
                        bFlag=false;
                    break;
                case 6:   //site 32
                case 7:   //site 31
                case 8:   //site 30
                case 9:   //site 29
                case 10:  //site 28
                case 11:  //site 27
                case 12:  //site 26
                case 13:  //site 25
                case 15:  //site 24
                case 16:  //site 23
                case 17:  //site 22
                case 18:  //site 21
                case 19:  //site 20
                case 20:  //site 19
                case 21:  //site 18
                case 22:  //site 17
                case 24:  //site 16
                case 25:  //site 15
                case 26:  //site 14
                case 27:  //site 13
                case 28:  //site 12
                case 29:  //site 11
                case 30:  //site 10
                case 31:  //site 09
                case 33:  //site 08
                case 34:  //site 07
                case 35:  //site 06
                case 36:  //site 05
                case 37:  //site 04
                case 38:  //site 03
                case 39:  //site 02
                case 40:  //site 01
                    if(!((asBuffer1>='0' && asBuffer1<='9') ||
                         (asBuffer1>='A' && asBuffer1<='G') ||
                         (asBuffer1>='a' && asBuffer1<='g')))                   //kevin 20170801 add
                    {
                        bFlag=false;
                    }
                    break;
            }
        }
        else if(SerialPoll->iBinSelect<=16)                                     //0-15  bin
        {
            asBuffer1=cBuffer[i];
            switch(i)
            {
                case 0:
                    if(asBuffer1!='B')
                        bFlag=false;
                    break;
                case 1:
                    if(asBuffer1!='I')
                        bFlag=false;
                    break;
                case 2:
                case 4:
                    if(asBuffer1!='N')
                        bFlag=false;
                    break;
                case 3:
                    if(asBuffer1!='O')
                        bFlag=false;
                    break;
                case 5:
                    if(asBuffer1!=':')
                        bFlag=false;
                    break;
                case 14:
                case 23:
                case 32:
                    if(asBuffer1!=',')
                        bFlag=false;
                    break;
                case 41:
                    if(asBuffer1==';' || asBuffer1=='\r' || asBuffer1=='\0' || asBuffer1=='\n') //Steven 20110406 : 正確的結尾
                        ;
                    else
                        bFlag=false;
                    break;
                case 6:   //site 32
                case 7:   //site 31
                case 8:   //site 30
                case 9:   //site 29
                case 10:  //site 28
                case 11:  //site 27
                case 12:  //site 26
                case 13:  //site 25
                case 15:  //site 24
                case 16:  //site 23
                case 17:  //site 22
                case 18:  //site 21
                case 19:  //site 20
                case 20:  //site 19
                case 21:  //site 18
                case 22:  //site 17
                case 24:  //site 16
                case 25:  //site 15
                case 26:  //site 14
                case 27:  //site 13
                case 28:  //site 12
                case 29:  //site 11
                case 30:  //site 10
                case 31:  //site 09
                case 33:  //site 08
                case 34:  //site 07
                case 35:  //site 06
                case 36:  //site 05
                case 37:  //site 04
                case 38:  //site 03
                case 39:  //site 02
                case 40:  //site 01
                    if(!((asBuffer1>='0' && asBuffer1<='9') ||
                         (asBuffer1>='A' && asBuffer1<='F') ||
                         (asBuffer1>='a' && asBuffer1<='f')))
//                         (asBuffer1>='A' && asBuffer1<='G') ||
//                         (asBuffer1>='a' && asBuffer1<='g')))                 //kevin 20170801 add
                    {
                        bFlag=false;
                    }
                    break;
            }
        }
        else if(SerialPoll->iBinSelect==17)                                     //16  bin     //Steven 20140805
        {
            asBuffer1=cBuffer[i];
            switch(i)
            {
                case 0:
                    if(asBuffer1!='B')
                        bFlag=false;
                    break;
                case 1:
                    if(asBuffer1!='I')
                        bFlag=false;
                    break;
                case 2:
                case 4:
                    if(asBuffer1!='N')
                        bFlag=false;
                    break;
                case 3:
                    if(asBuffer1!='O')
                        bFlag=false;
                    break;
                case 5:
                    if(asBuffer1!=':')
                        bFlag=false;
                    break;
                case 14:
                case 23:
                case 32:
                    if(asBuffer1!=',')
                        bFlag=false;
                    break;
                case 41:
                    if(asBuffer1==';' || asBuffer1=='\r' || asBuffer1=='\0' || asBuffer1=='\n') //Steven 20110406 : 正確的結尾
                        ;
                    else
                        bFlag=false;
                    break;
                case 6:   //site 32
                case 7:   //site 31
                case 8:   //site 30
                case 9:   //site 29
                case 10:  //site 28
                case 11:  //site 27
                case 12:  //site 26
                case 13:  //site 25
                case 15:  //site 24
                case 16:  //site 23
                case 17:  //site 22
                case 18:  //site 21
                case 19:  //site 20
                case 20:  //site 19
                case 21:  //site 18
                case 22:  //site 17
                case 24:  //site 16
                case 25:  //site 15
                case 26:  //site 14
                case 27:  //site 13
                case 28:  //site 12
                case 29:  //site 11
                case 30:  //site 10
                case 31:  //site 09
                case 33:  //site 08
                case 34:  //site 07
                case 35:  //site 06
                case 36:  //site 05
                case 37:  //site 04
                case 38:  //site 03
                case 39:  //site 02
                case 40:  //site 01
                    if(!((asBuffer1>='0' && asBuffer1<='9') ||
                         (asBuffer1>='A' && asBuffer1<='G') ||
                         (asBuffer1>='a' && asBuffer1<='g')))
                    {
                        bFlag=false;
                    }
                    break;
            }
        }
        else if(SerialPoll->iBinSelect==33)                                     //32  bin     //Steven 20140805
        {
            asBuffer1=cBuffer[i];
            switch(i)
            {
                case 0:
                    if(asBuffer1!='B')
                        bFlag=false;
                    break;
                case 1:
                    if(asBuffer1!='I')
                        bFlag=false;
                    break;
                case 2:
                case 4:
                    if(asBuffer1!='N')
                        bFlag=false;
                    break;
                case 3:
                    if(asBuffer1!='O')
                        bFlag=false;
                    break;
                case 5:
                    if(asBuffer1!=':')
                        bFlag=false;
                    break;
                case 14:
                case 23:
                case 32:
                    if(asBuffer1!=',')
                        bFlag=false;
                    break;
                case 41:
                    if(asBuffer1==';' || asBuffer1=='\r' || asBuffer1=='\0' || asBuffer1=='\n') //Steven 20110406 : 正確的結尾
                        ;
                    else
                        bFlag=false;
                    break;
                case 6:   //site 32
                case 7:   //site 31
                case 8:   //site 30
                case 9:   //site 29
                case 10:  //site 28
                case 11:  //site 27
                case 12:  //site 26
                case 13:  //site 25
                case 15:  //site 24
                case 16:  //site 23
                case 17:  //site 22
                case 18:  //site 21
                case 19:  //site 20
                case 20:  //site 19
                case 21:  //site 18
                case 22:  //site 17
                case 24:  //site 16
                case 25:  //site 15
                case 26:  //site 14
                case 27:  //site 13
                case 28:  //site 12
                case 29:  //site 11
                case 30:  //site 10
                case 31:  //site 09
                case 33:  //site 08
                case 34:  //site 07
                case 35:  //site 06
                case 36:  //site 05
                case 37:  //site 04
                case 38:  //site 03
                case 39:  //site 02
                case 40:  //site 01
                    if(!((asBuffer1>='0' && asBuffer1<='9') ||
                         (asBuffer1>='A' && asBuffer1<='W') ||
                         (asBuffer1>='a' && asBuffer1<='w')))
                    {
                        bFlag=false;
                    }
                    break;
            }
        }
        else                                                                    //000-255 bin
        {
            asBuffer1=cBuffer[i];
            CompBuffer="";
            for(int j=0; j<K; j++)
                CompBuffer+=cBuffer[i];
            switch(i)
            {
                case 0:
                    if(CompBuffer.Pos("B")==0)
                        bFlag=false;
                    break;
                case 1:
                    if(CompBuffer.Pos("I")==0)
                        bFlag=false;
                    break;
                case 2:
                case 4:
                    if(CompBuffer.Pos("N")==0)
                        bFlag=false;
                    break;
                case 3:
                    if(CompBuffer.Pos("O")==0)
                        bFlag=false;
                    break;
                case 5:
                    if(CompBuffer.Pos(":")==0)
                        bFlag=false;
                    else
                    {
                          K=3;                                                  //一 次取3個字元
                    }
                    break;
                case 30:
                case 55:
                case 80:
                    K=3;                                                        //一 次取3個字元
                    if(CompBuffer.Pos(",")==0)
                        bFlag=false;
                    break;
                case 105:
                    if(CompBuffer.Pos(";")==1 || CompBuffer.Pos("\r")==1 || CompBuffer.Pos("\n")==1)
                        ;
                    else
                        bFlag=false;
                    break;
                    //BINON: 032 031 030 029 028 027 026 025 , 024 023 022 021 020 019 018 017,
                    //       016 015 014 013 012 011 010 009 , 008 007 006 005 004 003 002 001
                case 6:   //site 32     6  7  8
                case 9:   //site 31     9 10 11
                case 12:  //site 30    12 13 14
                case 15:  //site 29    15 16 17
                case 18:  //site 28    18 19 20
                case 21:  //site 27    21 22 23
                case 24:  //site 26    24 25 26
                case 27:  //site 25    27 28 29 ----
                case 31:  //site 24    31 32 33
                case 34:  //site 23    34 35 36
                case 37:  //site 22    37 38 39
                case 40:  //site 21    40 41 42
                case 43:  //site 20    43 44 45
                case 46:  //site 19    46 47 48
                case 49:  //site 18    49 50 51
                case 52:  //site 17    52 53 54  ---
                case 56:  //site 16    56 57 58
                case 59:  //site 15    59 60 61
                case 62:  //site 14    62 63 64
                case 65:  //site 13    65 66 67
                case 68:  //site 12    68 69 70
                case 71:  //site 11    71 72 73
                case 74:  //site 10    74 75 76
                case 77:  //site 09    77 78 79  ----
                case 81:  //site 08    81 82 83
                case 84:  //site 07    84 85 86
                case 87:  //site 06    87 88 89
                case 90:  //site 05    90 91 92
                case 93:  //site 04    93 94 95
                case 96:  //site 03    96 97 98
                case 99:  //site 02    99 100 101
                case 102: //site 01   101 103 104
                    if(K==3)
                    {
                        try
                        {
                            H=StrToInt(CompBuffer);
                            if(H<0 && H>255)
                                bFlag=false;
                            else
                            {
                                if(i== 27 ||i==52||i== 77||i==101)
                                    K=1;
                                i+=2;
                            }
                        }
                        catch(...)
                        {
                            bFlag=false;
                        }
                    }
                    else
                    {
                        if(!((asBuffer1>='0' && asBuffer1<='9') ||
                             (asBuffer1>='A' && asBuffer1<='F') ||
                             (asBuffer1>='a' && asBuffer1<='f') ) )
                        {
                            bFlag=false;
                        }
                    }
                    break;
            }
        }
    }

    if(bFlag==false)
    {
        SerialPoll->SaveResult();
    }
    return bFlag;
}
//------------------------------------------------------------------------------
void TSerialPoll::ProcessHMountConnect()
{
    AnsiString str;
    static AnsiString Str1="";
    static bool bFirst=true;                                                    //wei 20150617 Add version control
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        Str1="";
        bFirst=true;
    }

    if(HMountWnd==NULL)
    {
        TStringList *List=new TStringList();
        List->CommaText="HTXXXX,HT-9046,HT-9045,HT-9046LS,HT-9055,HT-502,HT-505,HT-1032AT,HT-1032,HT-7080,HT-9045_12Site,HT-9046LA,HT-9046CN,HT-9046AU,HT-9046CR,HT-9132LS,HT-9132,HT-9050";    //Steven 20260926 : For HT9050
        for(int i=0; i<List->Count; i++)                                        //Steven 20241001 : 改用For迴圈找機台標頭
        {
            if(HMountWnd==NULL)
            {
                //AI(W906-GB-P1) 20260926: golden FindWindow("TfMain", List->Strings[i].c_str()) (see W906_FindHandlerWnd)
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
        //AI(W906-GB-P1) 20260926: golden FindWindow("TfMain", Str1.c_str()) (see W906_FindHandlerWnd)
        HMountWnd=W906_FindHandlerWnd(this, Str1.c_str());
    }

    if(HMountWnd!=NULL)
    {
        bFind=true;
        //AI(W906-GB-P1) 20260926: golden passes the two HWNDs to %d (32-bit BCB6); this->Handle -> reinterpret_cast<HWND>(this)
        str.sprintf("Handler : %d ; GPIB : %d", (int)(INT_PTR)HMountWnd, (int)(INT_PTR)reinterpret_cast<HWND>(this));
        lblGPIBWnd->Caption=str;
        if(bFirst)
        {
            SleepEx(1000, false);                                               //JerryYang 20230414 : GPIB小程式剛開啟時, 要等一下再送指令, 不然handler軟體FindWindow還找不到GPIB
            SendMSG_CMD(MSG_CMD_Version, GPIBVersion);                          //wei 20150617 Add version control
            SendMSG_CMD(MSG_CMD_TesterMode);
            bFirst=false;
            //AI(W906-GB-P1) 20260926: golden Str1 is the Handler caption FindWindow matched in the title loop above.  The
            //   engine attaches HMountWnd before the first call, so that loop never runs in-process and Str1 would stay
            //   "".  Log text only: fall back to the Handler model golden already read into szMachineType (FormShow,
            //   Gerneral.ini [Version] Model, Main.cpp:535).
            if(Str1=="")
                Str1=szMachineType.Trim();
            str.sprintf("GPIB run with Handler : %s", Str1);
            WriteLog(str);
        }
    }
    else
    {
        #ifndef DEBUG
            WriteLog("GPIB : No handler window, close GPIB,  _ProcessHMountConnect_");
            RequestClose("ProcessHMountConnect");                              //AI(W906-GB-P1) 20260926: golden Close() (rule 4)
        #endif
       bFind=false;
       bFirst=false;                                                            //wei 20150617 Add version control
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::SendCaptureFinish()
{
    AnsiString S;
    if(bFind)
    {
        if(LastSet.bRunHANA_ART==true &&                                        //Steven 20250414 : HANA ART Function
           bHanaDummyTest==true)
        {
            SendMSG_CMD(MSG_CMD_HANA_ART, "DUMMY_TEST_0x42_OK");
            bHanaDummyTest=false;
        }
        else
        {
            AnsiString Str="GPIB to Handler <== ";
            GGpib2Handler.iCommand=MSG_CMD_NONE;                                //Steven 20141014 : Add GPIB Command
            COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
            pcp->dwData=0;
            pcp->cbData=sizeof(GGpib2Handler);
            pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;               //2012-10-11    Dell Fix
            //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
            PostToHandler(pcp);

            for(int i=TOTAL_SITE-1; i>=0; i--)                                  //Steven 20170215 (wei): Add log
            {
                S.sprintf("%d,", GGpib2Handler.Result[i]);                      //Jerry 20170218 (Steven) : Result[i]如果等於-1,會變一個很大的值
                Str+=S;
            }
            WriteLog(Str);
            WriteLog(" ");
            delete pcp;
        }
    }
    IsTestDelay.SetSecAndOn(8);                                                 //20220616 wei 次數計數改為時間延遲
}
//------------------------------------------------------------------------------
void TSerialPoll::ProcessMessage()
{
    static int &Task=iMainTask,iWaitCT=0;
    int data;
    AnsiString Str="", asTemp;
    //AI(W906-GB-P1) 20260926: golden `ct` is uninitialized: a site whose cbBin->ItemIndex matches no branch below keeps
    //   the previous site's ct (kept), and for site 0 golden would read stack garbage (UB in C++) -> starts at 0.
    int ret, sresult[TOTAL_SITE], ct=0;
    char tstr[TOTAL_SITE][16];
    static DWORD StartTick,EndTick;
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h). Task is bound to the global iMainTask (ResetBridgeGlobals).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        iWaitCT=0;
        StartTick=0;
        EndTick=0;
    }
    if(bNeedInital)
    {
        if(Task<200)
        {
            bNeedInital=false;
            Task=1;
        }
    }

    switch(Task)
    {
        case 1:
            data=0;

            for(int i=0; i<TOTAL_SITE; i++)
            {
                data<<=1;
                data|=iStart[(TOTAL_SITE-1)-i];
            }

            if(IsTest==false)                                                   //Steven 20110914
            {
                data=0;
            }

            if(LastSet.bUpperCase)                                              //Eliot 2010_1129 start
            {
                asTemp.sprintf("%08x", data);
                asTemp=asTemp.UpperCase();
                GpibString.sprintf("Fullsites %s\r\n", asTemp);
            }
            else
            {
                GpibString.sprintf("Fullsites %08x\r\n", data);
            }

            if(data==0 && IsTest==true)
            {
                IsTest=false;
                return;
            }

            if(W906_AnsiCompare(LastSet.sGpibString, GpibString)!=0)           //AI(W906-GB-P1) 20260926: golden LastSet.sGpibString.AnsiCompare(GpibString)
            {
                LastSet.sGpibString=GpibString;
                WriteGpibString();
            }

            if(IsTest && bSimulate)                                             //jou 2011-12-22 模擬時需確認IsTest==true,不然會一直送信號給Handler
            {
                Str="SOFTBIN:";
                for(int i=TOTAL_SITE-1; i>=0; i--)
                {
                    if(iStart[i]==1)
                    {
                        if(i==0)
                            asTemp.sprintf("%04x;", i+1);
                        else
                            asTemp.sprintf("%04x,", i+1);
                    }
                    else
                    {
                        if(i==0)
                            asTemp="0000;";
                        else
                            asTemp="0000,";
                    }
                    Str+=asTemp;
                }
                GGpib2Handler.GPIBBin=Str.Length();
                strncpy(GGpib2Handler.cReturn, Str.c_str(), GGpib2Handler.GPIBBin);
                WriteLog(Str);
                if(GGpib2Handler.GPIBBin<544)
                {
                    SendMSG_CMD(MSG_CMD_SBIN);
                    asTemp.sprintf("ECHO%s", Str);
                    WriteLog(asTemp);
                }

                for(int i=0; i<TOTAL_SITE; i++)
                {
                    if(     MY_DUT_PAL[i]->cbBin->ItemIndex==0)                 //2012-10-11    Dell Fix
                        ct=0;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==1)
                        ct=1;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==2)
                        ct=2;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==3)
                        ct=3;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==4)
                        ct=4;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==5)
                        ct=5;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==6)                 //Steven 20220616 : 新增6~15bin
                        ct=6;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==7)
                        ct=7;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==8)
                        ct=8;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==9)
                        ct=9;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==10)
                        ct=10;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==11)
                        ct=11;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==12)
                        ct=12;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==13)
                        ct=13;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==14)
                        ct=14;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==15)
                        ct=33;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==16)
                        ct=random(6)+1;
                    else if(MY_DUT_PAL[i]->cbBin->ItemIndex==17)
                        ct=random(15)+1;
                    else if(iBinSelect==17 && MY_DUT_PAL[i]->cbBin->ItemIndex==18)
                        ct=random(16)+1;                                        //Steven 20140805
                    else if(iBinSelect==33 && MY_DUT_PAL[i]->cbBin->ItemIndex==18)
                        ct=random(254)+1;                                       //Steven 20140805
                    else if(iBinSelect >32 && MY_DUT_PAL[i]->cbBin->ItemIndex==18)
                        ct=random(254)+1;                                       //kevin 20140305  256Bin

                    if(iStart[i]==1)
                    {
                        sresult[i]=ct;
                        sprintf(tstr[i], "%4d", ct);
                    }
                     else
                    {
                        sresult[i]=0;
                        strncpy(tstr[i], "----", sizeof(tstr[i]));
                    }
                }

                Str="";
                for(int i=0; i<TOTAL_SITE; i++)
                {
                    GGpib2Handler.Result[i]=sresult[i];
                    MY_DUT_PAL[i]->plSite->Caption=AnsiString(tstr[i]);
                    asTemp.sprintf("%s ", tstr[i]);
                    Str+=asTemp;
                }

                sprintf(GGpib2Handler.cReturn, "%s", Str.c_str());
                SendCaptureFinish();                                            //bSimulate

                IsTest=false;
                break;
            }

            if(IsTest)                                                          //Steven 20110914 : 換位置,因為有人沒收到0x41就送fullsites?
            {
                Task=100;
            }
            else
            {
                TestGPIB();
            }
            break;
        case 100:
            ret=TestGPIB();
            if(ret==1)
            {
                for(int i=0; i<TOTAL_SITE; i++)
                {
                    if(LastSet.iTesterMode==InterfaceType_Delta_Castle)         //RogerYang 20260430 : 放開註解 //JerryYang 20230414 : Castle也要有保護
                    {
                        GGpib2Handler.Result[i]=result[i];
                    }
                    else
                    {
                        if(bCloseSiteHaveBinErr)                                //Steven 20170214 (wei)加上保護
                        {
                            GGpib2Handler.Result[i]=-1;
                        }
                        else if(LastSet.bSQR41==true && LastSet.bFULLSITES)     //kevin 20130516 避免之前沒收到資料按SKIP 後下一次測試一開始收到BIN ON
                        {
                            GGpib2Handler.Result[i]=result[i];
                        }
                        else
                        {
                            GGpib2Handler.Result[i]=-1;
                        }
                    }
                }
                if(LastSet.bSQR41==true && LastSet.bFULLSITES==false)
                {
                    WriteLog("Tester ==> BINON WITHOUT FULLSITES ERROR");       //Steven 20170214 : Add GPIB Log
                    SendMSG_CMD(MSG_CMD_BinonWithoutFullsite);                  //Steven 20231017 : GPIB flow error need alarm
                }

                if(LastSet.iTesterMode==InterfaceType_15BinQorvo && bNeedRetest==true)     //Steven 20201022 : Add Qorvo protocol
                    GGpib2Handler.GPIBBin=1;
                else
                    GGpib2Handler.GPIBBin=0;
                GGpib2Handler.bError=false;
                GGpib2Handler.bEchoStop=false;                                  //ChungHung 20130326 add

                IsTest=false;

                if(LastSet.bBinonEcho==false                        ||
                   LastSet.bSQR41==true                             ||
                   LastSet.iTesterMode==InterfaceType_Delta_Castle  )           //Steven 20150408 : for bBinonEcho 功能異常
                {
                    SendCaptureFinish();
                }

                LastSet.bSQR41=false;                                           //Steven 20110909
                LastSet.bFULLSITES=false;                                       //kevin 20130516
                LastSet.bSRQC0=false;                                           //jou 2015-09-21 Auto Retest function
                LastSet.bFlagRCMD=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagSVID=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagECID=false;                                        //jou 2015-09-21 Auto Retest function

                dwEnd=GetTickCount();
                Str="Test Time : " + AnsiString((dwEnd-dwStart)/W906_BCB_CLK_TCK) + "Sec";   //AI(W906-GB-P1) 20260926: golden /CLK_TCK
                lblTestTime->Caption=Str;

                Task=1;
                break;
            }
            else if(ret==2)
            {
                for(int i=0; i<TOTAL_SITE; i++)
                    GGpib2Handler.Result[i]=-1;
                GGpib2Handler.bError=true;
                GGpib2Handler.bEchoStop=false;                                  //ChungHung 20130326 add
                SendCaptureFinish();                                            //bError
                IsTest=false;
                LastSet.bSQR41=false;                                           //Steven 20110909
                LastSet.bFULLSITES=false;                                       //kevin 20130516
                LastSet.bSRQC0=false;                                           //jou 2015-09-21 Auto Retest function
                LastSet.bFlagRCMD=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagSVID=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagECID=false;                                        //jou 2015-09-21 Auto Retest function

                dwEnd=GetTickCount();
                Str="Test Time : " + AnsiString((dwEnd-dwStart)/W906_BCB_CLK_TCK) + "Sec";   //AI(W906-GB-P1) 20260926: golden /CLK_TCK
                lblTestTime->Caption=Str;

                Task=1;
                break;
            }
            else if(ret==3)                                                     //ChungHung 20141031 FullSite的Test Time Out 回傳直改為2 回傳值3 bEchoStop   //Steven 20141016 : FullSite的Test Time Out
            {
                for(int i=0; i<TOTAL_SITE; i++)
                    GGpib2Handler.Result[i]=result[i];                          //ChungHung 20141217 for ASE_KR ECHOSTOP
                GGpib2Handler.bError=false;
                GGpib2Handler.bEchoStop=true;                                   //ChungHung 20141031 add for bEchoStop //ChungHung 20130326 add
                SendCaptureFinish();                                            //FullSite的Test Time Out
                IsTest=false;
                LastSet.bSQR41=false;                                           //Steven 20110909
                LastSet.bFULLSITES=false;                                       //kevin 20130516
                LastSet.bSRQC0=false;                                           //jou 2015-09-21 Auto Retest function
                LastSet.bFlagRCMD=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagSVID=false;                                        //jou 2015-09-21 Auto Retest function
                LastSet.bFlagECID=false;                                        //jou 2015-09-21 Auto Retest function

                dwEnd=GetTickCount();
                Str="Test Time : " + AnsiString((dwEnd-dwStart)/W906_BCB_CLK_TCK) + "Sec";   //AI(W906-GB-P1) 20260926: golden /CLK_TCK
                lblTestTime->Caption=Str;

                Task=1;
                break;
            }
            else if(ret==4)                                                     //ChungHung 20141103 add for FullSite Time Out can select
            {
            //    Task=1;
            }
            else if(ret==5)                                                     //Steven 20201022 : For RFMD
            {
                //AI(W906-GB-P1) 20260926: golden quirk kept: `IsTest==false;` is a comparison (no effect), not an assignment
                IsTest==false;
                Task=1;
            }
            break;
        case 200:
            StartTick=GetTickCount();
            iWaitCT=0;
            Task=300;
            break;
        case 300:
            iWaitCT++;
            if(iWaitCT>3 || IsTest==true)                                       //40
            {
                EndTick=GetTickCount();
                iWaitCT=0;
                Task=1;
            }
            break;
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::ProcessAddress()
{
    static int iErrorCT=0;
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        iErrorCT=0;
    }
    if(bGpibMode)
    {
        if(oldGpibAddress!=GpibAddress)
        {
            noncontroller=ibfind ("gpib0");                                     // Open a session to the GPIB board
            if(noncontroller<0)
            {
                if(bSimulate==false)
                    WriteLog("Error Open GPIB0");
                iErrorCT++;
                if(iErrorCT>10)                                                 //jou 2011-11-24 檢查GPIB是否有安裝
                {
                    oldGpibAddress=GpibAddress;                                 //jou 2011-11-23 make code，修正當error open時，不會呆在那
                }
                return;
            }
            iErrorCT=0;
            WriteLog("");
            WriteLog("Find GPIB");
            WriteLog("");
            ibrsc(noncontroller, 0);                                            // Release system control
            ibpad(noncontroller, GpibAddress);                                  // Change primary address from 0 to GpibAddress
            oldGpibAddress=GpibAddress;
            bNeedInital=true;
            LastSet.GpibAddress=GpibAddress;
            StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
            ibtmo(noncontroller, LastSet.iTimeOut);
        }
    }
}
//------------------------------------------------------------------------------
bool TSerialPoll::MyGPIBWrite(AnsiString str, AnsiString Task)
{
    //AI(W906-GB-P1) 20260926: golden buffer[StrLength], tempbuffer[StrLength].  ibrd() below reads up to iBufferCount
    //   (2560 == StrLength) bytes and does not terminate; a full read then makes `S.sprintf("%s", buffer)` run past
    //   the end.  One extra, never-written byte keeps both arrays terminated.  (Golden quirk kept: a shorter read leaves
    //   the tail of an earlier, longer one after it, and "%s" prints that stale tail.)
    static char buffer[StrLength+1], tempbuffer[StrLength+1];
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        memset(buffer, 0, sizeof(buffer));
        memset(tempbuffer, 0, sizeof(tempbuffer));
    }
    bool bOK=false;
    AnsiString Str1, S;
    int iCT=10;
    if(LastSet.iTesterMode==InterfaceType_Delta_Castle)
        iCT=5;

    //Steven 20260428 : 用 ini 參數覆蓋預設值
    int iRetryMax = (LastSet.iMyGpibWriteRetry     > 0) ? LastSet.iMyGpibWriteRetry     : 20;
    int iWaitMS   = (LastSet.iMyGpibWriteWaitMS    > 0) ? LastSet.iMyGpibWriteWaitMS    : 100;
    if(LastSet.iMyGpibWriteThreshold > 0)
        iCT = LastSet.iMyGpibWriteThreshold;
    if(iCT >= iRetryMax)
        iCT = iRetryMax - 1;

    if(LastSet.bGPIBWriteWithout_r_n==false)                                    //Jason 20221005 : 增加 GPIB Write結尾不要加/r/n的選項//
    {
        if(str.AnsiPos("\r")==0)
            str=str+"\r";                                                       //Steven 20210723 : 幫GPIB字串尾巴加上\r\n
        if(str.AnsiPos("\n")==0)
            str=str+"\n";
    }

    int iWaitCount=0;                                                           //Steven 20260428 : 統計 Wait TACS 次數
    for(int i=0; i<iRetryMax; i++)
    {
        UpdateLed();                                                            //jou 2016-03-30 修正GPIB ibwrt 異常
        if((ibsta&TACS) && (!(ibsta&ATN)))                                      // If addressed to talk, send the response "I am a talker"
        {
            ibwrt(noncontroller, str.c_str(), str.Length());                    // Send data across the bus.
            //AI(W906-GB-P1) 20260926: golden `Task!=NULL`: BCB6 AnsiString has only operator!=(const AnsiString&), and NULL is
            //   the int 0 (Include/_null.h), so it compiled to Task!=AnsiString(0), i.e. Task!="0" (almost always true;
            //   Task=="" logs " TALK:..." with a leading space).  With vclcompat, `Task!=NULL` would pick
            //   operator!=(AnsiString, const char*) and mean Task!="" -- a different branch.  Spelled out as BCB6 bound it.
            if(Task!=AnsiString(0))
                S.sprintf("%s TALK:%s", Task, str);
            else
                S.sprintf("TALK:%s", str);
            WriteLog(S);
            //Steven 20260428 : 若曾 Wait, 紀錄一筆 turnaround 時間
            if(iWaitCount>0)
            {
                Str1.sprintf("%s MyGPIBWrite OK after %d wait(s) (~%dms)",
                             Task, iWaitCount, iWaitCount*iWaitMS);
                WriteLog(Str1);
            }
            bOK=true;
            break;
        }
        else
        {
            ibwait(noncontroller, 0);                                           // Wait until non-controller is listener and ATN line is dropped.
            UpdateLed();                                                        //jou 2016-03-30 修正GPIB ibwrt 異常
            if(i>iCT)
            {
                if((ibsta&LACS) && !(ibsta&ATN))
                {
                    //Steven 20260428 : 加上 ibsta/iberr 細節
                    Str1.sprintf("ibwrt fail %s, change to LACS [retry=%d/%d ibsta=0x%04X iberr=%d]",
                                 str, i+1, iRetryMax, ibsta, iberr);
                    WriteLog(Str1);

                    ibrd(noncontroller, buffer, iBufferCount);
                    strncpy(tempbuffer, buffer, sizeof(tempbuffer));            //Steven 20250926 : 變更GPIB write流程

                    if(ibsta&ERR)
                    {

                    }
                    else
                    {
                        S.sprintf("Byte=%d     str=%s", ibcnt, buffer);
                        mmoBINON->Lines->Add(S);
                        S.sprintf("%s", buffer);
                        WriteLog(S);
                    }
                    break;
                }

                //Steven 20260428 : 加上 ibsta/iberr 細節
                Str1.sprintf("ibwrt fail %s [retry=%d/%d ibsta=0x%04X (TACS=%d LACS=%d ATN=%d CIC=%d ERR=%d) iberr=%d]",
                             str, i+1, iRetryMax, ibsta,
                             (ibsta&TACS)?1:0, (ibsta&LACS)?1:0, (ibsta&ATN)?1:0,
                             (ibsta&CIC) ?1:0, (ibsta&ERR) ?1:0, iberr);
                WriteLog(Str1);
                if(bNEXTSTEP_CMD==true)                                         //Ifor 20190606 : add NEXTSTEP CMD不可以Reset GPIB
                {
                    bNEXTSTEP_CMD=false;
                    return false;
                }

                SleepEx(100, false);
                if(ibsta&ERR)
                {
                    WriteLog("MyGPIBWrite ==> Reset GPIB Card (ibfind/ibrsc/ibpad/ibtmo)");//Steven 20260428
                    noncontroller=ibfind("gpib0");                              // Open a session to the GPIB board
                    if(noncontroller<0)
                    {
                        if(bSimulate==false)
                            WriteLog("MyGPIBWrite ==> Reset GPIB FAIL (ibfind returns <0)");//Steven 20260428
                        bNEXTSTEP_CMD=false;
                        return false;
                        //return 2;
                    }
                    oldGpibAddress=GpibAddress;
                    ibrsc(noncontroller, 0);                                    // Release system control  //輸入指令 ibrsc 0解除系統控制權(i.e., 設定此卡片為non-controller).//Request or release system control.
                    ibpad(noncontroller, GpibAddress);                          // Change primary address from 0 to GpibAddress
                    ibtmo(noncontroller, LastSet.iTimeOut);                     //Change or disable the I/O timeout period
                    WriteLog("MyGPIBWrite ==> Reset GPIB Done");                //Steven 20260428
                }
            }
            else
            {
                iWaitCount++;
                //Steven 20260428 : verbose 時才顯示 ibsta 細節，一般只寫簡單 Wait TACS（避免 log 量大增）
                if(LastSet.bMyGpibWriteVerboseLog)
                {
                    S.sprintf("%s Wait TACS[%d/%d]:%s ibsta=0x%04X (TACS=%d LACS=%d ATN=%d CIC=%d) iberr=%d",
                              Task, i+1, iRetryMax, str, ibsta,
                              (ibsta&TACS)?1:0, (ibsta&LACS)?1:0, (ibsta&ATN)?1:0,
                              (ibsta&CIC)?1:0, iberr);
                }
                else
                {
                    S.sprintf("%s Wait TACS:%s", Task, str);
                }
                WriteLog(S);
                SleepEx(iWaitMS, false);                                        //Ifor 20220319 : 10 --> 100   //Steven 20260428 : 改用 ini 參數
            }
        }
    }
    //Steven 20260428 : 只有真的跑完全部 retry 內圈 (i 到達 iRetryMax) 才是「GIVE UP」，
    //                  若是 LACS 分支 break 離開 (接收者模式), 上面已記錄 "change to LACS" 不需重複
    if(!bOK && iWaitCount>=iRetryMax)
    {
        Str1.sprintf("%s MyGPIBWrite GIVE UP after %d retries: ibsta=0x%04X iberr=%d str=%s",
                     Task, iRetryMax, ibsta, iberr, str);
        WriteLog(Str1);
    }
    bNEXTSTEP_CMD=false;
    return bOK;
}
//------------------------------------------------------------------------------
void TSerialPoll::InitialStartValue()
{
    if(LastSet.sGpibString.AnsiPos("00000000")!=0)
    {
        if(LastSet.MachineType==2)                                              //9047
            LastSet.sGpibString=LastSet.sGpibString.SubString(1, LastSet.sGpibString.Length()-8)+"FFFFFFFF";
        else if(LastSet.MachineType==1)                                         //9046
            LastSet.sGpibString=LastSet.sGpibString.SubString(1, LastSet.sGpibString.Length()-4)+"FFFF";
        else
            LastSet.sGpibString=LastSet.sGpibString.SubString(1, LastSet.sGpibString.Length()-2)+"FF";
    }
    GpibString.sprintf("%s", LastSet.sGpibString);                              //Steven 20110914
}
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden Main.cpp:4812-4894 is ONE /* ... */ block: the mnet/l112/l122 function-pointer
//   typedefs + globals (:4815-4828) and TSerialPoll::SVON_CLOSE (:4830-4893).  So golden has no live definition (and
//   golden Main.h does not declare it); GpibBridge.h declares it "kept for completeness".  No caller anywhere.  The
//   golden "/*" (:4812) and "*/" (:4894) lines are not reproduced inside the #if 0 (a "/*" there would swallow the
//   #endif); everything else is golden text.
#if 0 // TODO(W906-GB-P1): dead code, loads PCI_L112C/L122C/CMnet DLLs of the motion card
//先達軸卡 SVON_CLOSE()
//------------------------------------------------------------------------------
typedef I16 (PASCAL *_mnet_set_ring_config)         (U16 RingNO, U16 BaudRate);
typedef I16 (PASCAL *_mnet_reset_ring)              (U16 RingNo);
typedef I16 (PASCAL *_mnet_start_ring)              (U16 RingNo);
typedef I16 (PASCAL *_mnet_get_ring_active_table)   (U16 RingNo, U32 *DevTable);
typedef I16 (PASCAL *_mnet_set_dll_close_ring)      (U16 RingNo, U16 On_Off);
typedef I16 (PASCAL *_l122_open)                    (I16* existcards);
typedef I16 (PASCAL *_l112_open)                    (I16* existcard);
_mnet_set_ring_config       pfn_mnet_set_ring_config;
_mnet_reset_ring            pfn_mnet_reset_ring;
_mnet_start_ring            pfn_mnet_start_ring;
_mnet_get_ring_active_table pfn_mnet_get_ring_active_table;
_mnet_set_dll_close_ring    pfn_mnet_set_dll_close_ring;
_l122_open pfn_l122_open;
_l112_open pfn_l112_open;
#endif
//=====
bool TSerialPoll::SVON_CLOSE(void)
{
#if 0 // TODO(W906-GB-P1): dead code, loads PCI_L112C/L122C/CMnet DLLs of the motion card
    if(LastSet.MachName=="9045GPIB")
       HMountWnd=FindWindow("TfMain", "HT9045");
    else if(LastSet.MachName=="9046GPIB")
       HMountWnd=FindWindow("TfMain", "HT9046");
    else if(LastSet.MachName=="2601GPIB")  //2012-03-02    Dell for HT-2601
       HMountWnd=FindWindow("TfMain","HT2601");
    else if(LastSet.MachName=="9045GPIB_12Site")
        HMountWnd=FindWindow("TfMain","HT9045_12Site");
    else if(LastSet.MachName=="9046_32GPIB")
        HMountWnd=FindWindow("TfMain","HT9046_32");

    if(HMountWnd!=NULL || LastSet.MachName=="2601GPIB")
    {
        return true;
    }

    HINSTANCE hPCI_L122CDll, hCMnetdll, hPCI_L112CDll; //DLL句柄

    if(!FileExists("D:\\HT9045\\EXE\\PCI_L112C.dll") ||
       !FileExists("D:\\HT9045\\EXE\\PCI_L122C.dll") ||
       !FileExists("D:\\HT9045\\EXE\\CMnet.dll"))           //GPIB不用另帶DLL
        return true;

    Sleep(500);    //jou 2012-10-15 必須等待HT9045真的關閉,不然會出現error

    hCMnetdll       = LoadLibrary("D:\\HT9045\\EXE\\CMnet.dll");
    hPCI_L122CDll   = LoadLibrary("D:\\HT9045\\EXE\\PCI_L122C.dll");
    hPCI_L112CDll   = LoadLibrary("D:\\HT9045\\EXE\\PCI_L112C.dll");

    short exist122, exist112;
    short i;
    U32 lDevTable[2];

    if(hPCI_L112CDll!=NULL && hPCI_L122CDll!=NULL && hCMnetdll!= NULL)
    {
        pfn_mnet_set_ring_config       = (_mnet_set_ring_config)       GetProcAddress(hCMnetdll,       ("_mnet_set_ring_config"));
        pfn_mnet_reset_ring            = (_mnet_reset_ring)            GetProcAddress(hCMnetdll,       ("_mnet_reset_ring"));
        pfn_mnet_start_ring            = (_mnet_start_ring)            GetProcAddress(hCMnetdll,       ("_mnet_start_ring"));
        pfn_mnet_get_ring_active_table = (_mnet_get_ring_active_table) GetProcAddress(hCMnetdll,       ("_mnet_get_ring_active_table"));
        pfn_mnet_set_dll_close_ring    = (_mnet_set_dll_close_ring)    GetProcAddress(hCMnetdll,       ("_mnet_set_dll_close_ring"));
        pfn_l122_open                  = (_l122_open)                  GetProcAddress(hPCI_L122CDll,   ("_l122_open"));
        pfn_l112_open                  = (_l112_open)                  GetProcAddress(hPCI_L112CDll,   ("_l112_open"));
        //=========L112/L122 open
        (*pfn_l122_open)(&exist122);
        (*pfn_l112_open)(&exist112);

        if(exist122>0 || exist112>0)
        {
            for(i=0; i<2; i++)
            {
                (*pfn_mnet_set_ring_config      )(i, 3);//20M
         //       (*pfn_mnet_reset_ring           )(i);
                (*pfn_mnet_get_ring_active_table)(i, &lDevTable[0]);
                (*pfn_mnet_start_ring           )(i);
                if(pfn_mnet_set_dll_close_ring!=NULL)       //Steven 20140422 : 有可能用到舊的DLL造成死機
                    (*pfn_mnet_set_dll_close_ring   )(i, 1);
            }
        }
        //========
    }
    return true;
#endif
    return true;
}
//------------------------------------------------------------------------------
void TSerialPoll::SendMSG_CMD(int CMD)                               //Steven 20141016 : FullSite的Test Time Out
{
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

    AnsiString Str;
    if((int)GGpib2Handler.iCommand>=slCmdList->Count)                           //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
        Str.sprintf("GPIB to Handler <== CMD %d", GGpib2Handler.iCommand);
    else
        //AI(W906-GB-P1) 20260926: Strings[i] is a vclcompat proxy; AnsiString(...) before it goes through sprintf's varargs
        Str.sprintf("GPIB to Handler <== %s", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]));
    WriteLog(Str);

    //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
void TSerialPoll::SendMSG_CMD(int CMD, AnsiString Message)           //wei 20150617 Add version control  //wei 20150914 修改AnsiString Message-->char *Message
{
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    memset(GGpib2Handler.cReturn, '\0', sizeof(GGpib2Handler.cReturn));
    //AI(W906-GB-P1) 20260926: 544-byte span kept exactly (see the static_assert at the top of this file)
    strncpy(GGpib2Handler.cReturn, Message.c_str(), 544);                       //Message最大是 256+32+256=544    //Steven 20191009 : GpibStatus --> cReturn

    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

    AnsiString Str;
    if(CMD!=92)                                                                 //MSG_CMD_AMDRS232Connect 不記錄
    {
        if((int)GGpib2Handler.iCommand>=slCmdList->Count)                       //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
            Str.sprintf("GPIB to Handler <== CMD %d : %s", GGpib2Handler.iCommand, GGpib2Handler.cReturn);
        else
            //AI(W906-GB-P1) 20260926: Strings[i] is a vclcompat proxy; AnsiString(...) before it goes through sprintf's varargs
            Str.sprintf("GPIB to Handler <== %s : %s", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]), GGpib2Handler.cReturn);
        WriteLog(Str);
    }

    //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
void TSerialPoll::SendMSG_CMD_ESC(int CMD, int iEcho)                //Steven 20201022 : For RFMD
{
    //iEcho  0:ECHONG, 1:CHECKEMPTY, 2:ECHOOK
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    GGpib2Handler.GPIBBin=iEcho;
    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

    AnsiString Str, str1;
    if(iEcho==0)
        str1="ECHONG";
    else if(iEcho==1)
        str1="CHECKEMPTY";
    else
        str1="ECHOOK";

    if((int)GGpib2Handler.iCommand>=slCmdList->Count)                           //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
        Str.sprintf("GPIB to Handler <== CMD %d (%s)", GGpib2Handler.iCommand, str1);
    else
        //AI(W906-GB-P1) 20260926: Strings[i] is a vclcompat proxy; AnsiString(...) before it goes through sprintf's varargs
        Str.sprintf("GPIB to Handler <== %s (%s)", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]), str1);
    WriteLog(Str);

    //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
void TSerialPoll::SendMSG_CMD_INPUTQTY(int CMD, AnsiString LotID, int Count, AnsiString ProcessCode)      //Steven 20161025 : SCK ART function
{
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(GGpib2Handler);

    GGpib2Handler.iCommand=CMD;
    GGpib2Handler.GPIBBin=Count;
    strncpy(GGpib2Handler.GpibData, LotID.c_str(), sizeof(GGpib2Handler.GpibData));
    strncpy(GGpib2Handler.cReturn, ProcessCode.c_str(), sizeof(GGpib2Handler.cReturn)); //Steven 20190625 : Add process code for ART

    pcp->lpData=(unsigned char *)&GGpib2Handler.iCommand;

    AnsiString Str;
    if((int)GGpib2Handler.iCommand>=slCmdList->Count)                           //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
        Str.sprintf("GPIB to Handler <== CMD %d : %s %d %s", GGpib2Handler.iCommand, LotID, Count, ProcessCode);
    else
        //AI(W906-GB-P1) 20260926: Strings[i] is a vclcompat proxy; AnsiString(...) before it goes through sprintf's varargs
        Str.sprintf("GPIB to Handler <== %s : %s %d %s", AnsiString(slCmdList->Strings[GGpib2Handler.iCommand]), LotID, Count, ProcessCode);
    WriteLog(Str);

    //AI(W906-GB-P1) 20260926: golden SendMessage(HMountWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp); (rule 4)
    PostToHandler(pcp);
    delete pcp;
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
