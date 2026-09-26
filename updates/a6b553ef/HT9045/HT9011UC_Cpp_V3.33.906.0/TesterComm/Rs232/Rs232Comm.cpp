// ===========================================================================
//  TesterComm/Rs232/Rs232Comm.cpp -- the three serial ports of the RS232Standard program (tester RS232 + TTL board 1 /
//  board 2): receive handlers, open / close / send, the Standard-page buttons and Timer1, in the V906 process.
//
//  AI(W906-GB-P4) 20260926: faithful translation of golden
//  D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\MainForm.cpp (Big5/cp950 -> UTF-8), per
//  TesterComm/Rs232/TRANSLATION_RULES.md.  Golden ranges, in file order:
//      1388-1406  CommTesterReceiveData                (MainForm.dfm CommTester.OnReceiveData)
//      1407-1646  CommTester_TTLReceiveData            (CommTester_TTL.OnReceiveData)
//      1648-1705  btnUpdateClick, btClearClick, btManualTestClick
//      1707-1847  Timer1Timer                          (Timer1, 300 ms; Rs232Engine::RunOnce fires it)
//      1849-1912  CloseTesterComm, CloseTesterComm_TTL
//      1914-2141  OpenTesterComm, OpenTesterComm_TTL
//      2143-2227  SendCommandToTester, SendCommandToTester_TTL
//      2914-3113  CommTester_TTL_2ReceiveData          (CommTester_TTL_2.OnReceiveData)
//  THREADING.  The three receive handlers are NOT wired to the TComm threads: the TComm callbacks only QueueRx()
//  (Rs232Ui.cpp), and Rs232Engine's DrainRx() calls them here on the TesterComm thread with (Sender = the TComm, a
//  NUL-terminated copy, length), like golden where SPComm posts the data to the form's thread.  Everything in this
//  file runs on that one thread; no lock is taken here (uiMutex belongs to the engine loop).
//  Bodies are golden text line by line.  Deviations, each marked //AI(W906-GB-P4) in place:
//    * `__fastcall` dropped, `Pointer` -> void* (rules 3/4); SPComm enumerators spelled Spcomm::... .
//    * Function-local statics re-armed per program life (rule 13): Timer1Timer, both TTL receive handlers.
//    * `asTester+=iStart[i]` / `asTestResult+=(int)x` -> `+=AnsiString(...)`: BCB6 AnsiString has only
//      operator+=(const AnsiString&), so the int became decimal text; vclcompat would bind operator+=(char).
//    * crc_chk(cBuf, ...) -> crc_chk(reinterpret_cast<unsigned char*>(cBuf), ...).
//    * TTL receive handlers: the fixed-offset header tests ReceiveData1[1..8] read through W906_RxAt (golden byte in
//      range, 0 past the end -- golden reads past the vector's end on any short chunk, UB in C++).
//    * CommTester_TTLReceiveData: the per-byte Result[i-iPushback] / iStart[i-5-iPushback] stores of the "no SOT"
//      branch keep only their in-range targets (golden wrote below Result[0] and iStart[0]).
//    * CommTesterReceiveData: the endless tail of golden's `|| iNowDeal>iMaxDeal` loop is cut (see the note there).
//    * Timer1Timer: SendToBack + SetForegroundWindow(HMountWnd) gated (no window; HMountWnd is a token); the
//      cbBin->ItemIndex=0 writes also set ->Text as VCL does (W906_ComboSyncText).
//    * Open*: ::CreateFile -> ::CreateFileA; ->StartComm() -> W906_GoldenStartComm (restores SPComm's two exceptions,
//      so "Connect:FAIL" happens where golden's happened); OpenTesterComm_TTL's MessageBox -> UiNotice and its
//      cStr[50] copy bounded (rule 15).  SendCommandToTester(_TTL): Buff[1024] copy clamped (rule 15).
//    * `#ifdef SOFT_SIMULTE uServer->...` kept (SOFT_SIMULTE is on, Rs232Bridge.h banner).
//  Golden quirks kept as they are (they look like bugs; the behaviour is golden's):
//    * SendCommandToTester_TTL(Data, 1) writes board 2's frame to CommTester_TTL (board 1's port).
//    * CommTester_TTL_2ReceiveData stores board 2's bins in Result[0..3] / iStart[0..3] (board 1's slots; board 2
//      serves sites 5-8), and its "Double SOT" retry re-sends sites 1-8 (asTester.SubString(1,8)) as "@01WSOTS...".
//    * OpenTesterComm_TTL(1) with an empty TTL2 name clears bCommConnect[1] (TTL1's flag).
//    * SendCommandToTester under SOFT_SIMULTE forces bCommConnect[0]=true, so it always "sends" (a closed TComm
//      drops the bytes) and always mirrors them to uServer; it sends strlen(Buff) bytes (stops at the first 0x00).
//    * Both Open* probe the port twice with the identical name before giving up.
//    * The TTL receive handlers parse one chunk as one frame (no reassembly) and ReceiveData1.clear() each chunk.
//    * The "RBIN data length error" branches only reset sites 1-16; btnUpdateClick's Text.ToInt() throws on a
//      non-numeric baud text (golden: EConvertError shown by the VCL application).
//  Real files / ports these bodies touch:
//    * COM ports named by golden's Setup.ini (LoadSetupData / LoadSetupData_TTL, D:\RS232Standard\System\Setup.ini,
//      Rs232Engine::OverrideIniPaths can move it): "\\.\"+ComNameTester (RS232 tester line, CommTester),
//      "\\.\"+ComNameTTL / ComNameTTL_2 (TTL boards, 115200 8N1, CommTester_TTL / CommTester_TTL_2).  Each Open*
//      first probes the device with CreateFileA (a second identical try if the first fails; the handle is closed at
//      once), then opens it for good through TComm::StartComm (vclcompat: CreateFileA + reader / writer threads).
//      Writes: SendCommandToTester -> CommTester, SendCommandToTester_TTL -> CommTester_TTL (both boards, see above).
//    * SOFT_SIMULTE simulator socket: Timer1Timer calls uServer->DoOpenCommuncation() every tick and
//      SendCommandToTester mirrors each frame to uServer->SendCommand (uSocketServer, Rs232Support.cpp: golden
//      127.0.0.1:59999; that file keeps the vclcompat TServerSocket in its default SIM mode, so no real socket).
//    * Files: none directly.  btnUpdateClick -> SaveSetupData writes Setup.ini; every ShowCommData /
//      ShowCommData_TTL line goes to the D:\RS232Log log files (Rs232Log.cpp); AddBinData may write the bin log.
//    * Handler: SendMSG_CMD (MSG_CMD_Version, MSG_CMD_State_TTL, MSG_CMD_SwitchArm) and SendResultFinish, through
//      PostToHandler (the mailbox); ProcessHandlerConnect once a second from Timer1Timer.
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

// golden MainForm.cpp:16-17 build switches (rule 2).  Rs232Bridge.h now applies them itself at its end, so this copy
// is a no-op kept for the rule; guarded so it can never redefine (MachineType.h also defines SOFT_SIMULTE).
#if RS232STD_GOLDEN_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#ifndef SOFT_SIMULTE
#define SOFT_SIMULTE
#endif
#endif

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace rs232std {

//AI(W906-GB-P4) 20260926: golden MainForm.h has `#include <vector>` + `using namespace std;`; the bodies spell
//   `vector<Byte>`.  This using-declaration (this TU only, just the one name) keeps that text unchanged.
using std::vector;

//AI(W906-GB-P4) 20260926: bounds-safe element read for the TTL receive handlers.  Golden tests fixed offsets of
//   ReceiveData1 (up to [8]) without looking at its size; a chunk shorter than the frame being tested read past the
//   vector's end (BCB6: whatever heap bytes followed; C++: undefined behaviour).  In range the golden byte is returned
//   unchanged; past the end, 0 -- which equals none of the characters golden compares with, so a short chunk takes
//   the "no match" path, as golden's garbage bytes practically always did.
static Byte W906_RxAt(const vector<Byte>& Data, int i)
{
    if(i>=0 && i<(int)Data.size())
        return Data[i];
    return 0;
}

//AI(W906-GB-P4) 20260926: VCL TCustomComboBox::SetItemIndex also puts Items[ItemIndex] into ->Text; vclcompat
//   TComboBox keeps the two apart (vclcompat/Controls.h TComboBox), so an ItemIndex write here is followed by this
//   (same helper as TesterComm/Gpib/GpibUi.cpp VclComboSyncText).
static void W906_ComboSyncText(TComboBox* cb)
{
    if(cb->ItemIndex>=0 && cb->ItemIndex<cb->Items->Count)
        cb->Text=AnsiString(cb->Items->Strings[cb->ItemIndex]);
    else
        cb->Text="";
}

//AI(W906-GB-P4) 20260926: golden SPComm TComm::StartComm (D:\HT9045\elec\Component\Spcomm.pas:371-388) raises
//   ECommsError "This serial port already opened" when the component is already open and "Error opening serial port"
//   when CreateFile fails; OpenTesterComm / OpenTesterComm_TTL catch(...) either into "Connect:FAIL".  vclcompat
//   TComm::StartComm never throws: it returns silently when already open and falls back to SIM when the port cannot
//   be opened (vclcompat/Comm.cpp:290-344).  This puts golden's two exceptions back (same scheme as
//   TesterComm/Gpib/GpibAux.cpp OpenTesterComm); a SIM the owner requested explicitly (SetSimMode(true) before the
//   call) is left alone.  Golden's CreateFile existence probe in front of it is kept verbatim, so a requested SIM still
//   needs the named port to exist.
static void W906_GoldenStartComm(TComm* Comm)
{
    if(Comm->IsOpen())
        throw std::runtime_error("This serial port already opened");
    const bool bSimRequested=Comm->IsSimMode();
    Comm->StartComm();
    if(Comm->IsSimMode() && !bSimRequested)
    {
        Comm->StopComm();
        throw std::runtime_error("Error opening serial port");
    }
}

//---------------------------------------------------------------------------
void TfRS232Main::CommTesterReceiveData(TObject *Sender,
      void* Buffer, WORD BufferLength)
{
    for(int i=0; i<BufferLength; i++)
    {
        ReceiveData.push_back(*((Byte*)Buffer+i));
    }

    ShowCommData("[T->H]", ReceiveData);
    AnsiString sCommand=GetAnalysisString(ReceiveData);                         //Jimmychiu 20240125 : Add AnalysisString function
    int iMaxDeal=10, iNowDeal=0;
    while(((sCommand.Pos("_None_")>0)==false) || iNowDeal>iMaxDeal)             //clear when get none or over deal
    {
        //AI(W906-GB-P4) 20260926: hang guard (rule 3, UB), same guard as golden ReceiveData_TCPIP in Rs232Parse.cpp.
        //   golden's `|| iNowDeal>iMaxDeal` (its comment "clear when get none or over deal" needed `&&`) keeps the loop
        //   alive once one receive held 11+ commands (after the 11th, iNowDeal==11 > iMaxDeal), and "_None_" is then a
        //   fixed point: DoRevCommand("_None_") matches no branch (it only counts iNowRevCycle, and returns before
        //   `delete List` on iRevCycleClear-1 of every iRevCycleClear calls, MainForm.cpp:3713-3718) and leaves
        //   ReceiveData alone, so GetAnalysisString keeps returning "_None_".  Golden RS232Standard.exe spun there
        //   forever, leaking, until iNowDeal++ overflowed int (UB).  In-process that would pin the shared TesterComm
        //   thread (Stop() could never join it) and grow the Handler's heap.  The loop is left exactly when the next
        //   command would be "_None_", before DoRevCommand sees it: every command golden dispatched is still
        //   dispatched, and for 0..10 commands the path is golden's (the while condition itself ends it there).
        if(iNowDeal>iMaxDeal && sCommand.Pos("_None_")>0)
            break;
        DoRevCommand(sCommand);
        sCommand=GetAnalysisString(ReceiveData);
        iNowDeal++;
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::CommTester_TTLReceiveData(TObject *Sender,                    //Isaac 20200903 :TTL RS232通訊
      void* Buffer, WORD BufferLength)
{
    bTTL1RS232Rev=true;
    static bool bDoubleSOTflag=false;                                           //Frank 20180723 :TTL RS232通訊
    int iStrLength;
    static int iPushback=0;
    static int iErrCount=0;                                                     //Isaac 20210510 : 板子接錯，alarm
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).  iPushback is pure scratch (assigned below before every read), so it needs none.
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        bDoubleSOTflag=false;
        iErrCount=0;
    }
    ReceiveData1.clear();
    vector<Byte>(ReceiveData1).swap(ReceiveData1);
    AnsiString strTemp="";
    for(int i=0; i<BufferLength; i++)
    {
        ReceiveData1.push_back(*((Byte*)Buffer+i));
    }

    if(iTTLBoardNum>=2 || bTTLBoardAddr)
    {
        iPushback=2;                                                            //有站號
    }
    else                                                                        //iTTLBoardNum==1
    {
        iPushback=0;                                                            //無站號
    }

    //AI(W906-GB-P4) 20260926: every fixed-offset ReceiveData1[k] test below reads through W906_RxAt (the golden
    //   byte when k < size(), 0 past the end); golden indexes the vector unchecked, so a chunk shorter than the frame it
    //   tests reads past the end.  The in-loop ReceiveData1[i] (i < size()) reads are golden text.
    if(iPushback==0)                                                            //TTL 一塊版，無站號
    {
        if(W906_RxAt(ReceiveData1, 1)=='0')                                     //有站號('0'0 or '0'1)的話，需報alarm
        {
            ShowCommData_TTL("[T->H]", ReceiveData1);
            ShowCommData("[TTL1]", "Should not get TTL1 board address. Please check TTL1 board version!");
            iErrCount++;                                                        //Isaac 20210510 : 板子接錯，alarm
            if(iErrCount>3)
            {
                iErrCount=0;
                GGpib2Handler.bOneCycle=false;                                  //Check TTL Board Version
                GGpib2Handler.bError=true;                                      //Check TTL Board Address
                GGpib2Handler.bEchoStop=false;                                  //Check TTL1 Reply
                GGpib2Handler.GPIBBin=0;                                        //Check TTL2 Reply

                SendMSG_CMD(MSG_CMD_Version, "[TTL Board] Should not get TTL1 board address. Please check TTL1 board version!");
            }
            return;
        }
    }
    else if(iPushback==2)                                                       //有站號，TTL1:00
    {
        if(W906_RxAt(ReceiveData1, 1)!='0' || (W906_RxAt(ReceiveData1, 2)!='0')) //新版本有站號:Pass,要判斷是哪張TTL板
        {
            ShowCommData("[TTL1]", "Should get TTL1 board address 00. Please check TTL board and Handler IPC TTL1 comport setting!");
            iErrCount++;                                                        //Isaac 20210510 : 板子接錯，alarm
            if(iErrCount>3)
            {
                iErrCount=0;

                GGpib2Handler.bOneCycle=false;                                  //Check TTL Board Version
                GGpib2Handler.bError=true;                                      //Check TTL Board Address
                GGpib2Handler.bEchoStop=false;                                  //Check TTL1 Reply
                GGpib2Handler.GPIBBin=0;                                        //Check TTL2 Reply

                SendMSG_CMD(MSG_CMD_Version, "[TTL1] Should get TTL1 board address 00. Please check TTL board and Handler IPC TTL1 comport setting!");
            }
            return;
        }
    }

    if(W906_RxAt(ReceiveData1, 1+iPushback)=='R' && W906_RxAt(ReceiveData1, 2+iPushback)=='B' && W906_RxAt(ReceiveData1, 3+iPushback)=='I' && W906_RxAt(ReceiveData1, 4+iPushback)=='N') //Frank 20180723 :TTL RS232通訊
    {
        ShowCommData_TTL("[T->H]", ReceiveData1, 1);
        int itest=ReceiveData1.size();                                          //testing
        if(itest!=16+iPushback)
        {
            ShowCommData("[TTL1]", "[TTL Board1] RBIN data length error!");
            for(int i=0; i<16; i++)
            {
                if(iStart[i]==true)
                {
                    GGpib2Handler.Result[i]=999;
                    iStart[i]=false;
                }
            }
        }
        else
        {
            if(bDoubleSOTflag==true)
            {
                AnsiString asTester;
                AnsiString asTest;
                char cStart[128];
                char cBuf[128];
                char cCRC[2];

                ShowCommData("[TTL1]", "Last Time Test Result to Ashbin!");

                for(int i=0; i<USE_SITE_COUNT; i++)
                {
                    MY_DUT_PAL[i]->cbSiteOn->Checked=iStart[i];
                    MY_DUT_PAL[i]->plSite->Caption=(MY_DUT_PAL[i]->cbSiteOn->Checked==true)?"T":"";
                    asTester+=AnsiString(iStart[i]);                            //AI(W906-GB-P4) 20260926: BCB6 `+=` has only the AnsiString operand -> AnsiString(int) "0"/"1"; vclcompat would bind +=(char)
                }

                asTester=asTester.SubString(1,8);

                if(iTTLBoardNum>1 || bTTLBoardAddr)
                {
                    asTest.sprintf("@00WSOTS%s", asTester);
                }
                else
                {
                    asTest.sprintf("@WSOTS%s", asTester);
                }

                const int iLength=asTest.Length();
                strcpy(cBuf, asTest.c_str());
                //AI(W906-GB-P4) 20260926: crc_chk takes unsigned char*; bcc32 accepted char* with a warning, C++ needs the cast.
                crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength, cCRC[0], cCRC[1]);
                strcpy(cStart, asTest.c_str());

                SendStartData.clear();
                for(int i=0; i<asTest.Length(); i++)
                {
                    SendStartData.push_back(cStart[i]);
                }
                SendStartData.push_back(cCRC[0]);
                SendStartData.push_back(cCRC[1]);
                SendStartData.push_back('#');
                SendCommandToTester_TTL(SendStartData, 0);

                bDoubleSOTflag=false;
                return;
            }

            AnsiString asReceiveData="";
            AnsiString asTestResult="";

            iStrLength=ReceiveData1.size();

            for(int i=0; i<iStrLength; i++)
            {
                if(bStartSOT[0])
                {
                    asReceiveData+=IntToHex((int)ReceiveData1[i], 2);

                    if(iTTLBoardNum>1)                                          //兩塊版(有站號)，第一塊板子，只取前四碼(1111 0000)
                    {
                        if(i>=5+iPushback && i<9+iPushback)
                        {
                            GGpib2Handler.Result[i-5-iPushback]=(int)ReceiveData1[i];
                            iStart[i-5-iPushback]=false;

                            asTestResult+=AnsiString((int)ReceiveData1[i]);     //AI(W906-GB-P4) 20260926: BCB6 AnsiString(int) = decimal text (see asTester above)
                            asTestResult+=",";
                        }
                    }
                    else                                                        //一塊板(有站號/無站號)，一次取八碼(1111 1111)
                    {
                        if(i>=5+iPushback && i<13+iPushback)
                        {
                            GGpib2Handler.Result[i-5-iPushback]=(int)ReceiveData1[i];
                            iStart[i-5-iPushback]=false;

                            asTestResult+=AnsiString((int)ReceiveData1[i]);     //AI(W906-GB-P4) 20260926: BCB6 AnsiString(int) = decimal text (see asTester above)
                            asTestResult+=",";
                        }
                    }
                }
                else
                {
                    //AI(W906-GB-P4) 20260926: golden runs these two stores for EVERY byte of the frame (i = 0 .. 15+iPushback),
                    //   so i<iPushback writes Result[-2..-1] (BCB6 layout: Result[-1] is GGpib2Handler.iCommand, Result[-2]
                    //   the object the linker put before GGpib2Handler) and i<5+iPushback writes iStart[-7..-1] (the globals
                    //   before iStart).  Only the in-range stores are kept -- in-process the stray ones would land in the
                    //   Handler's memory.  Result[0..15] / iStart[0..10] get exactly golden's values.
                    if(i-iPushback>=0 && i-iPushback<(int)(sizeof(GGpib2Handler.Result)/sizeof(GGpib2Handler.Result[0])))
                        GGpib2Handler.Result[i-iPushback]=999;
                    if(i-5-iPushback>=0 && i-5-iPushback<MAX_SITE_COUNT)
                        iStart[i-5-iPushback]=false;
                }
            }
            bStartSOT[0]=false;
            if(bStartSOT[0]==false && bStartSOT[1]==false)
            {
                SendResultFinish();
            }
            AddBinData(asTestResult);                                           //Steven 20171013 (wei) : 畫面顯示從測試機收到的Bin別Isaac
        }
    }
    else if(W906_RxAt(ReceiveData1, 1+iPushback)=='W' && W906_RxAt(ReceiveData1, 2+iPushback)=='I' && W906_RxAt(ReceiveData1, 3+iPushback)=='N' &&
            W906_RxAt(ReceiveData1, 4+iPushback)=='I' && W906_RxAt(ReceiveData1, 5+iPushback)=='T')
    {
        ShowCommData_TTL("[T->H]", ReceiveData1, 2);
    }
    else
    {
        ShowCommData_TTL("[T->H]", ReceiveData1);

        if(W906_RxAt(ReceiveData1, 1+iPushback)=='R' && W906_RxAt(ReceiveData1, 2+iPushback)=='u' && W906_RxAt(ReceiveData1, 3+iPushback)=='n' && W906_RxAt(ReceiveData1, 4+iPushback)=='i' && //有站號
           W906_RxAt(ReceiveData1, 5+iPushback)=='n' && W906_RxAt(ReceiveData1, 6+iPushback)=='g')
        {
            SendMSG_CMD(MSG_CMD_State_TTL);
        }
        else if(W906_RxAt(ReceiveData1, 1+iPushback)=='W' && W906_RxAt(ReceiveData1, 2+iPushback)=='C' && W906_RxAt(ReceiveData1, 3+iPushback)=='S' &&
                W906_RxAt(ReceiveData1, 4+iPushback)=='O' && W906_RxAt(ReceiveData1, 5+iPushback)=='T')
        {
            bDoubleSOTflag=false;
        }
        else if(W906_RxAt(ReceiveData1, 1+iPushback)=='E' && W906_RxAt(ReceiveData1, 2+iPushback)=='r' && W906_RxAt(ReceiveData1, 3+iPushback)=='r')
        {
            if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='F')
            {
                ShowCommData("[TTL1]", "1st char of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='C' && W906_RxAt(ReceiveData1, 5+iPushback)=='R' && W906_RxAt(ReceiveData1, 6+iPushback)=='C')
            {
                ShowCommData("[TTL1]", "CRC of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='E' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='F')
            {
                ShowCommData("[TTL1]", "Last char of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='C' && W906_RxAt(ReceiveData1, 5+iPushback)=='M' && W906_RxAt(ReceiveData1, 6+iPushback)=='D')
            {
                ShowCommData("[TTL1]", "Command not support error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                bDoubleSOTflag=true;
                ShowCommData("[TTL1]", "Double SOT error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='D' && W906_RxAt(ReceiveData1, 5+iPushback)=='A' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                ShowCommData("[TTL1]", "Command format error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='I' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                ShowCommData("[TTL1]", "Close site has bin error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='B' && W906_RxAt(ReceiveData1, 5+iPushback)=='I' && W906_RxAt(ReceiveData1, 6+iPushback)=='N')
            {
                ShowCommData("[TTL1]", "Bit Bit mode got 2 signal in one site error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='A' && W906_RxAt(ReceiveData1, 5+iPushback)=='R' && W906_RxAt(ReceiveData1, 6+iPushback)=='C')
            {
                ShowCommData("[TTL1]", "CRC of command error!");
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::btnUpdateClick(TObject *Sender)
{
    ComNameTester=cbDevice->Text;                                               //Isaac 20200903 :TTL RS232通訊
    CommTester->BaudRate=cbBaudRate->Text.ToInt();

    //AI(W906-GB-P4) 20260926: the SPComm enumerators are spelled Spcomm::... in this file (same as
    //   TesterComm/Gpib/GpibAux.cpp), so no other visible name can capture None / _1 / ...
    if(cbByteSize->ItemIndex==0)
        CommTester->ByteSize=Spcomm::_5;
    else if(cbByteSize->ItemIndex==1)
        CommTester->ByteSize=Spcomm::_6;
    else if(cbByteSize->ItemIndex==2)
        CommTester->ByteSize=Spcomm::_7;
    else if(cbByteSize->ItemIndex==3)
        CommTester->ByteSize=Spcomm::_8;

    if(cbStopBit->ItemIndex==0)
        CommTester->StopBits=Spcomm::_1;
    else if(cbStopBit->ItemIndex==1)
        CommTester->StopBits=Spcomm::_1_5;
    else if(cbStopBit->ItemIndex==2)
        CommTester->StopBits=Spcomm::_2;

    if(cbParity->ItemIndex==0)
        CommTester->Parity=Spcomm::None;
    else if(cbParity->ItemIndex==1)
        CommTester->Parity=Spcomm::Odd;
    else if(cbParity->ItemIndex==2)
        CommTester->Parity=Spcomm::Even;
    else if(cbParity->ItemIndex==3)
        CommTester->Parity=Spcomm::TParity(3);
    else if(cbParity->ItemIndex==4)
        CommTester->Parity=Spcomm::Space;

    SaveSetupData(CommTester);
}
//---------------------------------------------------------------------------
void TfRS232Main::btClearClick(TObject *Sender)
{
    MemoLog->Clear();
}
//---------------------------------------------------------------------------
void TfRS232Main::btManualTestClick(TObject *Sender)
{
    AnsiString Str="";
    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        iStart[i]=MY_DUT_PAL[i]->cbSiteOn->Checked;
    }

    bManualTest=true;

    // Show Result Data {
    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        MY_DUT_PAL[i]->plSite->Caption=(MY_DUT_PAL[i]->cbSiteOn->Checked==true)?"T":"";
    }

    ShowCommData("[Manual Access]", "Manual Test Button Pressed");
}
//---------------------------------------------------------------------------
void TfRS232Main::Timer1Timer(TObject *Sender)
{
    static bool flag=true;
    int sresult[MAX_SITE_COUNT], ct;
    char tstr[MAX_SITE_COUNT][16];
    static int iTTL1Count=0;
    static int iTTL2Count=0;
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        flag=true;
        iTTL1Count=0;
        iTTL2Count=0;
    }

    if(flag)                                                                    //得在Close() 之前做完
    {
        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {                                                                       //AI(W906-GB-P4) 20260926: braces for the sync line
            MY_DUT_PAL[i]->cbBin->ItemIndex=0;
            W906_ComboSyncText(MY_DUT_PAL[i]->cbBin);                           //AI(W906-GB-P4) 20260926: VCL SetItemIndex also sets ->Text
        }
        flag=false;
        //AI(W906-GB-P4) 20260926: window z-order only.  There is no RS232Standard window, and HMountWnd is a token (the
        //   mailbox address, Rs232Engine::Start), not a real HWND -- SetForegroundWindow must not see it.
#if 0 // TODO(W906-GB-P4): no form window; HMountWnd is not a real HWND in-process (golden MainForm.cpp:1720-1725)
        fRS232Main->SendToBack();

        if(HMountWnd!=NULL)
        {
            SetForegroundWindow(HMountWnd);
        }
#endif
    }

    dtPresent=Now();
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
    if(OldSystemSec!=SystemSec)
    {
        OldSystemSec=SystemSec;
        ProcessHandlerConnect();
    }

    if(bTTL1RS232Send==true && bTTL1RS232Rev==false)
    {
        iTTL1Count++;
        if(iTTL1Count>10)                                                       //300ms*10times=3sec
        {
            iTTL1Count=0;
            bTTL1RS232Send=false;

            GGpib2Handler.bOneCycle=false;                                      //Check TTL Board Version
            GGpib2Handler.bError=false;                                         //Check TTL Board Address
            GGpib2Handler.bEchoStop=true;                                       //Check TTL1 Reply
            SendMSG_CMD(MSG_CMD_Version, "Time Out!! No get TTL Board1 Reply!");
        }
    }
    else if(bTTL1RS232Send==true && bTTL1RS232Rev==true)
    {
        bTTL1RS232Send=false;
        bTTL1RS232Rev=false;
        iTTL1Count=0;

        GGpib2Handler.bOneCycle=false;                                          //Check TTL Board Version
        GGpib2Handler.bError=false;                                             //Check TTL Board Address
        GGpib2Handler.bEchoStop=false;                                          //Check TTL1 Reply
    }

    if(bTTL2RS232Send==true && bTTL2RS232Rev==false)
    {
        iTTL2Count++;
        if(iTTL2Count>10)
        {
            iTTL2Count=0;
            bTTL2RS232Send=false;

            GGpib2Handler.bOneCycle=false;                                      //Check TTL Board Version
            GGpib2Handler.bError=false;                                         //Check TTL Board Address
            GGpib2Handler.GPIBBin=1;                                            //Check TTL2 Reply

            SendMSG_CMD(MSG_CMD_Version, "Time Out!! No get TTL Board2 Reply!");
        }
    }
    else if(bTTL2RS232Send==true && bTTL2RS232Rev==true)
    {
        bTTL2RS232Send=false;
        bTTL2RS232Rev=false;
        iTTL2Count=0;

        GGpib2Handler.bOneCycle=false;                                          //Check TTL Board Version
        GGpib2Handler.bError=false;                                             //Check TTL Board Address
        GGpib2Handler.GPIBBin=0;                                                //Check TTL2 Reply
    }

    if(bSimulate)
    {
        if(bTwoArmTestMode && iTwoArmTestStep==0)                               //Steven 20141014 : 神盾測試模式
        {
            SendMSG_CMD(MSG_CMD_SwitchArm);
            iTwoArmTestStep=1;
            return;
        }

        for(int i=0; i<USE_SITE_COUNT; i++)                                     //jou 2015-03-23 use site
        {
            if(     MY_DUT_PAL[i]->cbBin->ItemIndex==0)                         //2012-10-11    Dell Fix
                ct=1;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==1)
                ct=2;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==2)
                ct=3;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==3)
                ct=4;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==4)
                ct=5;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==5)
                ct=6;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==6)
                ct=random(5)+1;
            else if(MY_DUT_PAL[i]->cbBin->ItemIndex==7)
                ct=random(10)+1;
            else
                ct=random(12)+1;

            if(iStart[i]==1)
            {
                iResult[i]=ct;
                sprintf(tstr[i], "%4d", ct);
            }
             else
            {
                iResult[i]=0;
                strcpy(tstr[i], "----");
            }

            GGpib2Handler.Result[i]=iResult[i];
            MY_DUT_PAL[i]->plSite->Caption=AnsiString(tstr[i]);
        }

        if(bTwoArmTestMode==false ||
           (bTwoArmTestMode==true && iTwoArmTestStep==2))                       //Steven 20141014 : 神盾測試模式
        {
            for(int i=0; i<USE_SITE_COUNT; i++)                                 //jou 2015-03-23 use site
            {
                iStart[i]=false;
            }

            SendResultFinish();
            bSimulate=false;
        }
    }
    #ifdef SOFT_SIMULTE
    uServer->DoOpenCommuncation();
    #endif
}
//---------------------------------------------------------------------------
bool TfRS232Main::CloseTesterComm()
{
    try
    {
        if(ComNameTester=="")
        {
            return false;
        }

        CommTester->StopComm();
        bCommConnect[0]=false;                                                  //Isaac 20200903 :TTL RS232通訊
        ShowCommData("[RS232]", "Disconnect:OK", ComNameTester);
    }
    catch(...)
    {
        ShowCommData("[RS232]", "Disconnect:FAIL", ComNameTester);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::CloseTesterComm_TTL(int iboard)                               //Isaac 20200903 :TTL RS232通訊
{
    try
    {
        if(iboard==0)
        {
            if(ComNameTTL=="")
            {
                return false;
            }

            CommTester_TTL->StopComm();
            bCommConnect[1]=false;
            ShowCommData("[TTL1]", "Disconnect:OK", ComNameTTL);
        }
        else                                                                    //iboard==1 //Isaac 20210309 :TTL RS232兩塊板子
        {
            if(ComNameTTL_2=="")
            {
                return false;
            }

            CommTester_TTL_2->StopComm();
            bCommConnect[2]=false;
            ShowCommData("[TTL2]", "Disconnect:OK", ComNameTTL_2);
        }
    }
    catch(...)
    {
        if(iboard==0)
        {
            ShowCommData("[TTL1]", "Disconnect:FAIL", ComNameTTL);
        }
        else                                                                    //iboard==1 //Isaac 20210309 :TTL RS232兩塊板子
        {
            ShowCommData("[TTL2]", "Disconnect:FAIL", ComNameTTL_2);
        }
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::OpenTesterComm()
{
    AnsiString Str;
    if(bCommConnect[0]==true)
    {
        CloseTesterComm();
    }

    try
    {
        if(ComNameTester=="")
        {
            ShowCommData("[RS232]", "Tester COM setting is NULL!!");
            bCommConnect[0]=false;
            return false;
        }

        HANDLE handle=INVALID_HANDLE_VALUE;
        AnsiString CN="\\\\.\\"+ComNameTester;
        //AI(W906-GB-P4) 20260926: golden `::CreateFile` (BCB6 ANSI build) is spelled ::CreateFileA in this file so a
        //   UNICODE build cannot turn it into CreateFileW.  Same open-and-close existence probe, same outcome.
        handle=::CreateFileA(CN.c_str(),
                          GENERIC_READ|GENERIC_WRITE,
                          0,
                          0,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                          0);
        if(handle==INVALID_HANDLE_VALUE)
        {
            CN="\\\\.\\"+ComNameTester;
            handle=::CreateFileA(CN.c_str(),
                          GENERIC_READ|GENERIC_WRITE,
                          0,
                          0,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                          0);
            if(handle==INVALID_HANDLE_VALUE)
            {
                int iErr=0;
                iErr=GetLastError();
                Str.sprintf("%s : %s", ComNameTester, GetErrorMessage(iErr));
                ShowCommData("[RS232]", "Connect:FAIL", Str);
                bCommConnect[0]=false;
                return false;
            }
            else
            {
                CloseHandle(handle);
            }
        }
        else
        {
            CloseHandle(handle);
        }

        fRS232Main->CommTester->CommName=CN;
        //AI(W906-GB-P4) 20260926: golden SPComm StartComm raises ECommsError when it cannot open the port (see
        //   W906_GoldenStartComm); vclcompat's silently falls back to SIM.  The helper restores the exception.
        W906_GoldenStartComm(fRS232Main->CommTester);
        bCommConnect[0]=true;
        ShowCommData("[RS232]", "Connect:OK", ComNameTester);
    }
    catch(...)
    {
        ShowCommData("[RS232]", "Connect:FAIL", ComNameTester);
        bCommConnect[0]=false;
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::OpenTesterComm_TTL(int iboard)                                //Isaac 20200903 :TTL RS232通訊 //Isaac 20210309 :TTL RS232兩塊板子
{
    AnsiString Str;
    if(bCommConnect[iboard+1]==true)
    {
        CloseTesterComm_TTL(iboard);
    }

    try
    {
        HANDLE handle=INVALID_HANDLE_VALUE;

        if(iboard==0)
        {
            if(ComNameTTL=="")
            {
                ShowCommData("[TTL1]", "TTL1 COM setting is NULL!!");
                bCommConnect[1]=false;
                return false;
            }

            AnsiString CN="\\\\.\\"+ComNameTTL;
            handle=::CreateFileA(CN.c_str(),
                              GENERIC_READ|GENERIC_WRITE,
                              0,
                              0,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                              0);
            if(handle==INVALID_HANDLE_VALUE)
            {
                CN="\\\\.\\"+ComNameTTL;
                handle=::CreateFileA(CN.c_str(),
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    0);
                if(handle==INVALID_HANDLE_VALUE)
                {
                    int iErr=0;
                    char cStr[50];
                    ZeroMemory(cStr, sizeof(cStr));                             //先清空記憶體再讀取資料, 避免舊資料出現
                    iErr=GetLastError();

                    Str.sprintf("TTL1 RS232 %s comport error, %s", ComNameTTL, GetErrorMessage(iErr));
                    //AI(W906-GB-P4) 20260926: rule 15 -- golden strcpy's Str into cStr[50], but GetErrorMessage alone is
                    //   ~80 chars ("Error Code : 0x02 ==> Error Message : ..."), so golden overran this stack buffer every
                    //   time the port was missing.  Bounded copy; the notice gets the full Str, which is the text golden's
                    //   MessageBox showed from cStr's address.
                    strncpy(cStr, Str.c_str(), sizeof(cStr)-1);
                    //AI(W906-GB-P4) 20260926: golden Application->MessageBox(cStr, "TTL1 RS232 Error", MB_OK) (modal, one OK
                    //   button) -> UiNotice (rule 4); the caption is not carried.
                    UiNotice(Str);

                    Str.sprintf("%s : %s", ComNameTTL, GetErrorMessage(iErr));
                    ShowCommData("[TTL1]", "Connect:FAIL", Str);
                    bCommConnect[1]=false;
                    return false;
                }
                else
                {
                    CloseHandle(handle);
                }
            }
            else
            {
                CloseHandle(handle);
            }
            fRS232Main->CommTester_TTL->CommName=CN;
            fRS232Main->CommTester_TTL->Parity=Spcomm::None;
            fRS232Main->CommTester_TTL->BaudRate=115200;
            fRS232Main->CommTester_TTL->ByteSize=Spcomm::_8;
            fRS232Main->CommTester_TTL->ParityCheck=false;
            fRS232Main->CommTester_TTL->StopBits=Spcomm::_1;

            W906_GoldenStartComm(fRS232Main->CommTester_TTL);            //AI(W906-GB-P4) 20260926: golden ->StartComm(), see OpenTesterComm
            bCommConnect[1]=true;

            ShowCommData("[TTL1]", "Connect:OK", ComNameTTL);
        }
        else                                                                    //iboard==1
        {
            if(ComNameTTL_2=="")
            {
                ShowCommData("[TTL2]", "TTL2 COM setting is NULL!!");
                //AI(W906-GB-P4) 20260926: golden clears bCommConnect[1] (TTL1's flag) here, not [2]; kept.
                bCommConnect[1]=false;
                return false;
            }

            AnsiString CN="\\\\.\\"+ComNameTTL_2;
            handle=::CreateFileA(CN.c_str(),
                              GENERIC_READ|GENERIC_WRITE,
                              0,
                              0,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                              0);
            if(handle==INVALID_HANDLE_VALUE)
            {
                CN="\\\\.\\"+ComNameTTL_2;
                handle=::CreateFileA(CN.c_str(),
                    GENERIC_READ|GENERIC_WRITE,
                    0,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    0);
                if(handle==INVALID_HANDLE_VALUE)
                {
                    int iErr=0;
                    char cStr[50];
                    ZeroMemory(cStr, sizeof(cStr));                             //先清空記憶體再讀取資料, 避免舊資料出現
                    iErr=GetLastError();

                    Str.sprintf("TTL2 RS232 : %s comport error, %s", ComNameTTL_2, GetErrorMessage(iErr));
                    //AI(W906-GB-P4) 20260926: rule 15 -- golden strcpy's Str into cStr[50], but GetErrorMessage alone is
                    //   ~80 chars ("Error Code : 0x02 ==> Error Message : ..."), so golden overran this stack buffer every
                    //   time the port was missing.  Bounded copy; the notice gets the full Str, which is the text golden's
                    //   MessageBox showed from cStr's address.
                    strncpy(cStr, Str.c_str(), sizeof(cStr)-1);
                    //AI(W906-GB-P4) 20260926: golden Application->MessageBox(cStr, "TTL2 RS232 Error", MB_OK) (modal, one OK
                    //   button) -> UiNotice (rule 4); the caption is not carried.
                    UiNotice(Str);

                    Str.sprintf("%s : %s", ComNameTTL_2, GetErrorMessage(iErr));
                    ShowCommData("[TTL2]", "Connect:FAIL", Str);
                    bCommConnect[2]=false;
                    return false;
                }
                else
                {
                    CloseHandle(handle);
                }
            }
            else
            {
                CloseHandle(handle);
            }
            fRS232Main->CommTester_TTL_2->CommName=CN;
            fRS232Main->CommTester_TTL_2->Parity=Spcomm::None;
            fRS232Main->CommTester_TTL_2->BaudRate=115200;
            fRS232Main->CommTester_TTL_2->ByteSize=Spcomm::_8;
            fRS232Main->CommTester_TTL_2->ParityCheck=false;
            fRS232Main->CommTester_TTL_2->StopBits=Spcomm::_1;

            W906_GoldenStartComm(fRS232Main->CommTester_TTL_2);          //AI(W906-GB-P4) 20260926: golden ->StartComm(), see OpenTesterComm
            bCommConnect[2]=true;

            ShowCommData("[TTL2]", "Connect:OK", ComNameTTL_2);
        }
    }
    catch(...)
    {
        if(iboard==0)
        {
            ShowCommData("[TTL1]", "Connect:FAIL", ComNameTTL);
            bCommConnect[1]=false;
        }
        else                                                                    //iboard==1
        {
            ShowCommData("[TTL2]", "Connect:FAIL", ComNameTTL_2);
            bCommConnect[2]=false;
        }
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void TfRS232Main::SendCommandToTester(vector<Byte>& Data)
{
    #ifdef SOFT_SIMULTE
    bCommConnect[0]=true;
    #endif
    if(bCommConnect[0]==false)                                                  //Isaac 20200903 :TTL RS232通訊
    {
        ShowCommData("[H->T]", Data);
        ShowCommData("[H->T]", "Connect:ERROR", "Data can not send!");
        return;
    }

    int iSize=Data.size();
    if(iSize<=0)
        return;

    char Buff[1024];
    //AI(W906-GB-P4) 20260926: rule 15 -- golden writes Buff[0..iSize] unchecked, so a Data of 1024+ bytes ran past this
    //   stack buffer (DoRevCommand builds replies from the Handler's strings, e.g. cTestBinCount[1024]).  Clamp to 1023
    //   data bytes (the log line below still shows all of Data); byte-identical below that.
    if(iSize>(int)sizeof(Buff)-1)
        iSize=(int)sizeof(Buff)-1;
    for(int i=0; i<iSize; i++)
    {
        Buff[i]=Data[i];
    }

    Buff[iSize]='\0';

    CommTester->WriteCommData(Buff, strlen(Buff));
    #ifdef SOFT_SIMULTE
    uServer->SendCommand(Buff, strlen(Buff));
    #endif
    ShowCommData("[H->T]", Data);

    Data.clear();
}
//---------------------------------------------------------------------------
void TfRS232Main::SendCommandToTester_TTL(vector<Byte>& Data, int iBoard)       //Isaac 20200903 :TTL RS232通訊   //@WSOTS11000000'CRC'#
{
    if(iBoard==0)
    {
        if(bCommConnect[1]==false)
        {
            ShowCommData("[H->TTL1]", Data);
            ShowCommData("[H->TTL1]", "Connect:ERROR", "Data can not send!");
            return;
        }
    }
    else                                                                        //iBoard==1 //Isaac 20210309 :TTL RS232兩塊板子
    {
        if(bCommConnect[2]==false)
        {
            ShowCommData("[H->TTL2]", Data);
            ShowCommData("[H->TTL2]", "Connect:ERROR", "Data can not send!");
            return;
        }
    }

    int iSize=Data.size();
    if(iSize<=0)
        return;

    char Buff[1024];
    ZeroMemory(Buff, sizeof(Buff));

    //AI(W906-GB-P4) 20260926: rule 15 -- same clamp as SendCommandToTester (TTL frames are 16-59 bytes, INIT the longest).
    if(iSize>(int)sizeof(Buff)-1)
        iSize=(int)sizeof(Buff)-1;
    for(int i=0; i<iSize; i++)
    {
        Buff[i]=Data[i];
    }

    Buff[iSize]='\0';

    if(iBoard==0)
    {
        CommTester_TTL->WriteCommData(Buff, iSize);                             //Frank 20220408 Add TTL ASE_JP Mode
        bTTL1RS232Send=true;
        bTTL1RS232Rev=false;
        ShowCommData_TTL("[H->TTL1]", Data, 0, iBoard);                         //Steven 20240913 : 變更RS232 log記錄方式
    }
    else
    {
        //AI(W906-GB-P4) 20260926: golden writes board 2's frame to CommTester_TTL (board 1's port), not
        //   CommTester_TTL_2; kept as golden (it looks like a copy-paste slip; see the final report).
        CommTester_TTL->WriteCommData(Buff, iSize);                             //Frank 20220408 Add TTL ASE_JP Mode
        bTTL2RS232Send=true;
        bTTL2RS232Rev=false;
        ShowCommData_TTL("[H->TTL2]", Data, 0, iBoard);                         //Steven 20240913 : 變更RS232 log記錄方式
    }

    Data.clear();
}
//---------------------------------------------------------------------------
void TfRS232Main::CommTester_TTL_2ReceiveData(TObject *Sender,
      void* Buffer, WORD BufferLength)
{
    bTTL2RS232Rev=true;
    static bool bDoubleSOTflag=false;                                           //Frank 20180723 :TTL RS232通訊
    int iStrLength;
    const int iPushback=2;                                                      //第二塊板子，程式直接寫定
    static int iErrCount=0;                                                     //Isaac 20210510 : 板子接錯，alarm
    //AI(W906-GB-P4) 20260926: re-arm the golden statics for a new program life (golden: a relaunched exe; see
    //   g_rs232Life in Rs232Bridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_rs232Life)
    {
        s_life=g_rs232Life;
        bDoubleSOTflag=false;
        iErrCount=0;
    }
    AnsiString asReceiveData="";
    AnsiString asTestResult="";

    if(iTTLBoardNum<2)
    return;

    ReceiveData1.clear();
    vector<Byte>(ReceiveData1).swap(ReceiveData1);

    for(int i=0; i<BufferLength; i++)
    {
        ReceiveData1.push_back(*((Byte*)Buffer+i));
    }

    //AI(W906-GB-P4) 20260926: fixed-offset ReceiveData1[k] tests read through W906_RxAt, as in
    //   CommTester_TTLReceiveData.
    if(W906_RxAt(ReceiveData1, 1)!='0' || W906_RxAt(ReceiveData1, 2)!='1')      //新版本有站號:Pass,要判斷是哪張TTL板
    {
        ShowCommData("[TTL2]", "Should get TTL2 board address 01. Please check TTL board and Handler IPC TTL2 comport setting!");
        iErrCount++;                                                            //Isaac 20210510 : 板子接錯，alarm
        if(iErrCount>3)
        {
            iErrCount=0;

            GGpib2Handler.bOneCycle=false;                                      //Check TTL Board Version
            GGpib2Handler.bError=true;                                          //Check TTL Board Address
            GGpib2Handler.bEchoStop=false;                                      //Check TTL1 Reply
            GGpib2Handler.GPIBBin=0;                                            //Check TTL2 Reply

            SendMSG_CMD(MSG_CMD_Version, "[TTL Board2] Should get TTL2 board address 01. Please check TTL board and Handler IPC TTL2 comport setting!");
        }
        return;
    }

    if(W906_RxAt(ReceiveData1, 1+iPushback)=='R' && W906_RxAt(ReceiveData1, 2+iPushback)=='B' && W906_RxAt(ReceiveData1, 3+iPushback)=='I' && W906_RxAt(ReceiveData1, 4+iPushback)=='N') //Frank 20180723 :TTL RS232通訊
    {
        ShowCommData_TTL("[T->H]", ReceiveData1, 1, 1);
        int itest=ReceiveData1.size();                                          //testing
        if(itest!=16+iPushback)
        {
            ShowCommData("[TTL2]", "RBIN data length error!");

            for(int i=0; i<16; i++)
            {
                if(iStart[i]==true)
                {
                    GGpib2Handler.Result[i]=999;
                    iStart[i]=false;
                }
            }
        }
        else
        {
            if(bDoubleSOTflag==true)
            {
                AnsiString asTester;
                AnsiString asTest;
                char cStart[128];
                char cBuf[128];
                char cCRC[2];

                ShowCommData("[TTL2]", "Last Time Test Result to Ashbin!");

                for(int i=0; i<USE_SITE_COUNT; i++)
                {
                    MY_DUT_PAL[i]->cbSiteOn->Checked=iStart[i];
                    MY_DUT_PAL[i]->plSite->Caption=(MY_DUT_PAL[i]->cbSiteOn->Checked==true)?"T":"";
                    asTester+=AnsiString(iStart[i]);                            //AI(W906-GB-P4) 20260926: BCB6 AnsiString(int), see CommTester_TTLReceiveData
                }

                asTester=asTester.SubString(1,8);

                if(iTTLBoardNum>1 || bTTLBoardAddr)
                {
                    asTest.sprintf("@01WSOTS%s", asTester);
                }
                else
                {
                    asTest.sprintf("@WSOTS%s", asTester);
                }

                const int iLength=asTest.Length();
                strcpy(cBuf, asTest.c_str());
                crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength, cCRC[0], cCRC[1]);   //AI(W906-GB-P4) 20260926: cast, see TTL1
                strcpy(cStart, asTest.c_str());

                SendStartData.clear();
                for(int i=0; i<asTest.Length(); i++)
                {
                    SendStartData.push_back(cStart[i]);
                }
                SendStartData.push_back(cCRC[0]);
                SendStartData.push_back(cCRC[1]);
                SendStartData.push_back('#');
                SendCommandToTester_TTL(SendStartData, 1);

                bDoubleSOTflag=false;
                return;
            }

            iStrLength=ReceiveData1.size();

            for(int i=0; i<iStrLength; i++)
            {
                if(bStartSOT[1])
                {
                    asReceiveData+=IntToHex((int)ReceiveData1[i], 2);
                    if(i>=5+iPushback && i<9+iPushback)                         //兩塊版(有站號)，第二塊板子，只取前四碼(1111 0000)
                    {
                        //AI(W906-GB-P4) 20260926: golden stores board 2's four bins in Result[0..3] / iStart[0..3],
                        //   the slots board 1 fills (board 2 serves sites 5-8, MainForm.cpp:1033); kept as golden.
                        GGpib2Handler.Result[i-5-iPushback]=(int)ReceiveData1[i];
                        iStart[i-5-iPushback]=false;

                        asTestResult+=AnsiString((int)ReceiveData1[i]);         //AI(W906-GB-P4) 20260926: BCB6 AnsiString(int) = decimal text
                        asTestResult+=",";
                    }
                }
                else
                {
                    if(i>=5+iPushback && i<9+iPushback)                         //4
                    {
                        GGpib2Handler.Result[i-5-iPushback]=999;
                        iStart[i-5-iPushback]=false;
                    }
                }
            }
            bStartSOT[1]=false;
            if(bStartSOT[0]==false && bStartSOT[1]==false)
            {
                SendResultFinish();
            }
            AddBinData(asTestResult);                                           //Steven 20171013 (wei) : 畫面顯示從測試機收到的Bin別Isaac
        }
    }
    else if(W906_RxAt(ReceiveData1, 1+iPushback)=='W' && W906_RxAt(ReceiveData1, 2+iPushback)=='I' && W906_RxAt(ReceiveData1, 3+iPushback)=='N' &&
            W906_RxAt(ReceiveData1, 4+iPushback)=='I' && W906_RxAt(ReceiveData1, 5+iPushback)=='T')
    {
        ShowCommData_TTL("[T->H]", ReceiveData1, 2, 1);
    }
    else
    {
        ShowCommData_TTL("[T->H]", ReceiveData1, 0, 1);

        if(W906_RxAt(ReceiveData1, 1+iPushback)=='R' && W906_RxAt(ReceiveData1, 2+iPushback)=='u' && W906_RxAt(ReceiveData1, 3+iPushback)=='n' && W906_RxAt(ReceiveData1, 4+iPushback)=='i' && //有站號
           W906_RxAt(ReceiveData1, 5+iPushback)=='n' && W906_RxAt(ReceiveData1, 6+iPushback)=='g')
        {
            SendMSG_CMD(MSG_CMD_State_TTL);
        }
        else if(W906_RxAt(ReceiveData1, 1+iPushback)=='W' && W906_RxAt(ReceiveData1, 2+iPushback)=='C' && W906_RxAt(ReceiveData1, 3+iPushback)=='S' &&
                W906_RxAt(ReceiveData1, 4+iPushback)=='O' && W906_RxAt(ReceiveData1, 5+iPushback)=='T')
        {
            bDoubleSOTflag=false;
        }
        else if(W906_RxAt(ReceiveData1, 1+iPushback)=='E' && W906_RxAt(ReceiveData1, 2+iPushback)=='r' && W906_RxAt(ReceiveData1, 3+iPushback)=='r')
        {
            if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='F')
            {
                ShowCommData("[TTL2]", "1st char of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='C' && W906_RxAt(ReceiveData1, 5+iPushback)=='R' && W906_RxAt(ReceiveData1, 6+iPushback)=='C')
            {
                ShowCommData("[TTL2]", "CRC of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='E' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='F')
            {
                ShowCommData("[TTL2]", "Last char of command error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='C' && W906_RxAt(ReceiveData1, 5+iPushback)=='M' && W906_RxAt(ReceiveData1, 6+iPushback)=='D')
            {
                ShowCommData("[TTL2]", "Command not support error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='O' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                bDoubleSOTflag=true;
                ShowCommData("[TTL2]", "Double SOT error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='D' && W906_RxAt(ReceiveData1, 5+iPushback)=='A' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                ShowCommData("[TTL2]", "Command format error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='S' && W906_RxAt(ReceiveData1, 5+iPushback)=='I' && W906_RxAt(ReceiveData1, 6+iPushback)=='T')
            {
                ShowCommData("[TTL2]", "Close site has bin error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='B' && W906_RxAt(ReceiveData1, 5+iPushback)=='I' && W906_RxAt(ReceiveData1, 6+iPushback)=='N')
            {
                ShowCommData("[TTL2]", "Bit Bit mode got 2 signal in one site error!");
            }
            else if(W906_RxAt(ReceiveData1, 4+iPushback)=='A' && W906_RxAt(ReceiveData1, 5+iPushback)=='R' && W906_RxAt(ReceiveData1, 6+iPushback)=='C')
            {
                ShowCommData("[TTL2]", "CRC of command error!");
            }
        }
    }
}
//---------------------------------------------------------------------------

}  // namespace rs232std
