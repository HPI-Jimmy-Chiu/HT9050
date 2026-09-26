// ===========================================================================
//  TesterComm/Rs232/Rs232Log.cpp -- TfRS232Main log lines, Setup.ini save/load, bin log, TTL manual SOT / clear-SOT
//  and a few small button handlers of the RS232Standard program (tester RS232 + TTL board) in the V906 process.
//
//  AI(W906-GB-P4) 20260926: faithful translation of golden
//  D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410\MainForm.cpp (Big5/cp950 -> UTF-8), per
//  TesterComm/Rs232/TRANSLATION_RULES.md.  Golden ranges, in file order:
//      2228-2494  ShowCommData (vector<Byte>), ShowCommData (text), ShowCommDataHToR, ShowCommDataRToH,
//                 ShowCommData_TTL
//      2495-2563  SaveSetupData, SaveSetupData_TTL, LoadSetupData, LoadSetupData_TTL
//                 (2564-2575 WriteDataToFile is not in this file)
//      2577-2588  btnUpdateMouseDown / btnUpdateMouseUp
//      2589-2606  btnAllUseClick / btnAllNoUseClick
//      2607-2675  SaveBinData / AddBinData
//      2676-2702  btnUpdate_TTLClick, btnUpdate_TTLMouseDown / btnUpdate_TTLMouseUp, btClear_TTLClick
//      2703-2838  btnTTL_ManualClick
//      2839-2913  btnClearSotClick
//      3115-3158  Button10Click / Button11Click, InitialBarcodeList, btnEnableSOTCSOTClick
//  Bodies are golden text line by line.  None of these functions has a function-local static, so rule 13 has
//  nothing to re-arm.  Deviations, each marked //AI(W906-GB-P4) in place:
//    * `__fastcall` dropped (rule 3); the four mouse handlers take Sender only (rule 4, header signature).
//    * The four mouse-handler bodies (TPanel::BevelOuter = bvLowered / bvRaised, visual only) are gated: vclcompat
//      TPanel has no BevelOuter, and bvLowered / bvRaised are not declared for this TU.
//    * MemoLog->Lines->Append -> ->Add (vclcompat TStringList has no Append; VCL Append == Add minus the index).
//    * slCmdList->Strings[i] wrapped in AnsiString(...) before sprintf (rule 11).
//    * btnTTL_ManualClick `asTester+=iStart[j]` -> `+=AnsiString(iStart[j])`: the operator BCB6 actually bound.
//    * crc_chk(cBuf, ...) -> crc_chk(reinterpret_cast<unsigned char*>(cBuf), ...) (char* vs unsigned char*).
//    * ShowCommData_TTL: golden reads past the end of its Data copy on every normal frame (see the notes there);
//      those reads go through W906_TtlAt (the golden byte in range, 0 past the end).
//    * ShowCommData_TTL `for(i; ...)` -> `for(; ...)` (the `i;` had no effect).
//  Golden quirks kept as they are (they look like bugs; the behaviour is golden's):
//    * ShowCommData_TTL CLEARS the member ReceiveData (the standard-mode receive buffer that CommTesterReceiveData
//      appends to) on every call, and refills it with the bytes "after the '#'", which for a normal frame do not
//      exist (past the end: BCB6 heap garbage, here 0 bytes).
//    * ShowCommData_TTL returns before the "VERS" check when neither cbShowLog_TTL nor cbSaveLog_TTL is checked, so
//      the TTL board version (sTTLVersion / MSG_CMD_Version) is only picked up while one of those boxes is ticked.
//    * ShowCommData_TTL with iDataType==2 and a >57-byte frame logs the frame twice (inside the branch and again at
//      the end), and sets GGpib2Handler.bOneCycle=false even when no "VERS" follows.
//    * ShowCommData_TTL: a CRC byte or bin value equal to '#' (0x23) cuts iSize short / re-triggers the copy.
//    * ShowCommDataRToH passes one sprintf argument more than the format uses.
//    * SaveBinData's "empty memo" early return only covers iUseRS232Mode==0 and >=InterfaceType_TTL; modes 1 / 2
//      with an empty MemoBinData still write an empty bin-log file.  File names use the System* time of the last
//      GetTimeInfo() call (SaveBinData does not refresh it).
//    * btClear_TTLClick clears MemoLog (the common log page), not MemoBinData_TTL.
//    * btnClearSotClick builds the board-2 "@01WCSOT" frame even with one board (it is sent only if iTTLBoardNum>1).
//    * btnAllUseClick / btnAllNoUseClick / btnTTL_ManualClick / InitialBarcodeList index MY_DUT_PAL[0..31]
//      unchecked (InitialBarcodeList only checks size()!=0); golden builds all 32 panels at start-up.
//  Real files / ports these bodies touch (directly or through the golden helpers they call):
//    * D:\RS232Standard\System\Setup.ini (golden IniFileName; Rs232Engine::OverrideIniPaths can point it elsewhere):
//      written by SaveSetupData / SaveSetupData_TTL (WriteIniData), and ALSO by LoadSetupData / LoadSetupData_TTL,
//      because golden CheckAndReadIniData writes the default back when a key is missing.
//    * D:\RS232Log\BinLog\YYYY_MM\YYYY_MM_DD_HH MM SS_Rs232_BinData.log and
//      D:\RS232Log\BinLog_TTL\YYYY_MM\YYYY_MM_DD_HH MM SS_TTL_Rs232_BinData.log (SaveBinData, which also creates the
//      month folder with MyForceDirectories); AddBinData calls SaveBinData once the memo passes 10000 lines.
//    * D:\RS232Log\LOG\YYYY\MM\RS232_Log_YYYYMMDD HH.log (slRS232Log, TMyStringList "RS232_Log", TBy2Hour,
//      MaxLineCount 1): every ShowCommData line goes through AddText / AddTextWithHex, which append the buffered
//      lines to that file whenever more than MaxLineCount are held, i.e. every second line (Rs232Support.cpp).  The change log written by WriteIniData (RecordChangeLogProcess) is Rs232Globals.cpp's.
//    * TTL board COM ports: btnTTL_ManualClick / btnClearSotClick send "@..WSOTS.." / "@..WCSOT.." frames through
//      SendCommandToTester_TTL (see that function for which TComm board 2 uses); Button10Click closes and
//      Button11Click opens both TTL ports (CloseTesterComm_TTL / OpenTesterComm_TTL).
//    * Handler: ShowCommData_TTL posts MSG_CMD_Version packets (SendMSG_CMD) when a board reports "VERS".
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

#include <cstring>   // strcpy (btnTTL_ManualClick / btnClearSotClick)
#include <vector>

// golden MainForm.cpp:16-17 build switches (Rs232Bridge.h banner, rule 2; golden defines them after its includes, so
// no header sees them).  None of the text in this file is conditional on them.
#if RS232STD_GOLDEN_DEBUG
#define DEBUG
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif

namespace rs232std {

//AI(W906-GB-P4) 20260926: golden MainForm.h:17-18 has `#include <vector>` + `using namespace std;`; the bodies spell
//   `vector<Byte>`.  This using-declaration (this TU only, just the one name) keeps that text unchanged.
using std::vector;

//AI(W906-GB-P4) 20260926: bounds-safe element read for ShowCommData_TTL.  Golden indexes its by-value copy of Data
//   past the end on every normal TTL frame (see the notes in ShowCommData_TTL); BCB6 read whatever heap bytes followed,
//   in C++ that is undefined behaviour.  In range the golden byte is returned unchanged; past the end, 0.
static Byte W906_TtlAt(const vector<Byte>& Data, int i)
{
    if(i>=0 && i<(int)Data.size())
        return Data[i];
    return 0;
}

//---------------------------------------------------------------------------
//Steven 20171017 (wei) : 修改存檔格式,加入ASCII碼方便判讀
//---------------------------------------------------------------------------
void TfRS232Main::ShowCommData(AnsiString sUnitName, vector<Byte>bHex)          //Steven 20240913 : 變更RS232 log記錄方式
{
    if(MemoLog->Lines->Count>10240)
    {
        MemoLog->Clear();
    }

    //AI(W906-GB-P4) 20260926: VCL TStrings::Append(S) is `Add(S);` minus the returned index (Classes.pas); vclcompat
    //   TStringList has no Append, so the Add it forwards to is called directly (same line appended).
    MemoLog->Lines->Add(slRS232Log->AddTextWithHex(sUnitName, bHex));
}
//---------------------------------------------------------------------------
void TfRS232Main::ShowCommData(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2)    //Steven 20240913 : 變更RS232 log記錄方式
{
    if(MemoLog->Lines->Count>10240)
    {
        MemoLog->Clear();
    }

    //AI(W906-GB-P4) 20260926: Append -> Add, as above.
    MemoLog->Lines->Add(slRS232Log->AddText(sUnitName, sMsg1, sMsg2));
}
//---------------------------------------------------------------------------
void TfRS232Main::ShowCommDataHToR(int iRecvCommand, AnsiString sMsg)           //Steven 20240913 : 變更RS232 log記錄方式
{
    AnsiString Str1="", Str2="";
    if(bShowVersionOK==true)                                                    //Isaac 20210510 : 避免還沒new出來就讀count造成錯誤
    {
        if(iRecvCommand==0)
        {
            Str1=sMsg;
        }
        else
        {
            //AI(W906-GB-P4) 20260926: Strings[i] is a vclcompat proxy; wrapped before it goes through sprintf's
            //   varargs (rule 11).
            if((int)iRecvCommand>=slCmdList->Count)                             //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
                Str1.sprintf("CMD %d", iRecvCommand);
            else
                Str1.sprintf("%s", AnsiString(slCmdList->Strings[iRecvCommand]));

            Str2=sMsg;
        }

        if(iUseRS232Mode>=InterfaceType_TTL)                                    //Isaac 20200903 :TTL RS232通訊
        {
            ShowCommData("[Handler ==> RS232_TTL]", Str1, Str2);
        }
        else
        {
            ShowCommData("[Handler ==> RS232]", Str1, Str2);
        }
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::ShowCommDataRToH(int iRecvCommand, AnsiString sMsg)           //Steven 20240913 : 變更RS232 log記錄方式
{
    AnsiString Str1="", Str2="";
    if(bShowVersionOK==true)                                                    //Isaac 20210510 : 避免還沒new出來就讀count造成錯誤
    {
        if(iRecvCommand==0)
        {
            Str1=sMsg;
        }
        else
        {
            //AI(W906-GB-P4) 20260926: golden passes one argument more than each format uses (sMsg); kept -- the
            //   surplus vararg is ignored.  Strings[i] wrapped for the varargs as in HToR (rule 11).
            if((int)iRecvCommand>=slCmdList->Count)                             //Steven 20190823 : 避免忘記加陣列,產生記憶體破壞
                Str1.sprintf("CMD %d", iRecvCommand, sMsg);
            else
                Str1.sprintf("%s", AnsiString(slCmdList->Strings[iRecvCommand]), sMsg);
            Str2=sMsg;
        }

        if(iUseRS232Mode>=InterfaceType_TTL)                                    //Isaac 20200903 :TTL RS232通訊
        {
            ShowCommData("[RS232_TTL ==> Handler]", Str1, Str2);
        }
        else
        {
            ShowCommData("[RS232 ==> Handler]", Str1, Str2);
        }
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::ShowCommData_TTL(AnsiString sType, vector<Byte> Data, int iDataType, int iboard)      //Isaac 20200903 :TTL RS232通訊   //Isaac 20210309 :TTL RS232兩塊板子
{
    int iSize=Data.size();

    if(cbShowLog_TTL->Checked==false && cbSaveLog_TTL->Checked==false && iSize!=0)
        return;

    AnsiString cFile="";
    AnsiString sDataHex="";
    AnsiString sDataAscii="";

    ReceiveData.clear();
    int ilen=Data.size();
    int iShowPushback=0;

    if(iTTLBoardNum>1 || bTTLBoardAddr)
    {
        iShowPushback=2;
    }

    if(iDataType==2 && ilen>57)
    {
        if(iTTLBoardNum>1 || bTTLBoardAddr)
        {
            iSize=59;
        }
        else
        {
            iSize=57;
        }

        //AI(W906-GB-P4) 20260926: with two boards (iSize=59) a 58-byte frame passes the ilen>57 test and golden reads
        //   Data[58], one past the end.  W906_TtlAt (top of file): in-range reads identical, past the end 0.
        //   The strings built here are reset at golden :2375-2376 before use, so only the read is guarded.
        for(int i=0; i<iSize; i++)
        {
            sDataHex+=IntToHex((int)W906_TtlAt(Data, i), 2)+" ";
            if(i<iSize-3 || i==iSize-1)
            {
                sDataAscii+=MyDeCodeASCII((int)W906_TtlAt(Data, i));
            }
            else
            {
                if(i==iSize-3)
                    sDataAscii+="[";
                sDataAscii+=IntToHex((int)W906_TtlAt(Data, i), 2)+" ";
                if(i==iSize-2)
                    sDataAscii+="]";
            }
        }

        if(iSize==0)
        {
            ShowCommData("[TTL Board]", sType);
        }
        else
        {
            if(iboard==0)
            {
                ShowCommData("[TTL1] "+sType, Data);
            }
            else                                                                //Isaac 20210309 :TTL RS232兩塊板子
            {
                ShowCommData("[TTL2] "+sType, Data);
            }
        }

        iSize=Data.size();
        sDataHex="";
        sDataAscii="";

        int i=0;
        if(iTTLBoardNum>1 || bTTLBoardAddr)
        {
            i=59;
        }
        else
        {
            i=57;
        }
        //AI(W906-GB-P4) 20260926: golden `for(i; i<iSize; i++)`: the init-statement `i;` is an expression with no
        //   effect (i was set just above); written as an empty init-statement, same loop, no -Wunused-value.
        for(; i<iSize; i++)
        {
            sDataHex+=IntToHex((int)Data[i], 2)+" ";
            if(i<iSize-3 || i==iSize-1)
            {
                sDataAscii+=MyDeCodeASCII((int)Data[i]);
            }
            else
            {
                if(i==iSize-3)
                    sDataAscii+="[";
                sDataAscii+=IntToHex((int)Data[i], 2)+" ";
                if(i==iSize-2)
                    sDataAscii+="]";
            }
        }

        GGpib2Handler.bOneCycle=false;

        if(sDataAscii.AnsiPos("VERS")!=0)                                       //Isaac 20210511 : TTLRS232板子版本檢查
        {
            if(bTTLBoardAddr==false)                                            //@VERS07071601[53 B5 ]#
            {
                if(iboard==0)
                    sTTLVersion=sDataAscii.SubString(6, 8);
                else                                                            //iboard==1
                    sTTLVersion2=sDataAscii.SubString(6, 8);
            }
            else                                                                //@00VERS07071601[53 B5 ]#     or  @01VERS07071601[53 B5 ]#
            {
                if(iboard==0)
                    sTTLVersion=sDataAscii.SubString(8, 8);
                else                                                            //iboard==1
                    sTTLVersion2=sDataAscii.SubString(8, 8);
            }

            GGpib2Handler.bOneCycle=true;                                       //Check TTL Board Version
            GGpib2Handler.bError=false;                                         //Check TTL Board Address
            GGpib2Handler.bEchoStop=false;                                      //Check TTL1 Reply
            GGpib2Handler.GPIBBin=0;                                            //Check TTL2 Reply

            SendMSG_CMD(MSG_CMD_Version, sTTLVersion);

            if(sTTLVersion.Length()!=0 && sTTLVersion2.Length()!=0)
            {
                if(sTTLVersion!=sTTLVersion2)
                    SendMSG_CMD(MSG_CMD_Version, "[TTL Board] Board1 and Board2 version is not match. Please check TTL board version!");
            }
        }
    }
    else
    {
        //AI(W906-GB-P4) 20260926: this branch reads past the end of Data on EVERY normal frame: once '#' (the last
        //   byte) is found golden copies the 16+iShowPushback bytes AFTER it into the member ReceiveData, and
        //   with iShowPushback==2 iSize becomes i+3 so the '#' test and the hex loop below also read Data[size]
        //   and Data[size+1].  BCB6 read heap garbage there; in C++ it is undefined behaviour.  Every such read
        //   goes through W906_TtlAt (in range: the golden byte; past the end: 0).  Kept the golden shape:
        //   ReceiveData gets exactly as many push_backs as golden, the loop bounds are untouched, and the hex /
        //   ASCII strings stay unused (the log line below is built from Data itself).
        for(int i=0; i<iSize; i++)
        {
            if(W906_TtlAt(Data, i)=='#')
            {
                for(int j=0; j<16+iShowPushback; j++)
                {
                    ReceiveData.push_back(W906_TtlAt(Data, i+1+j+iShowPushback));
                }
                iSize=i+1+iShowPushback;
            }
        }

        for(int i=0; i<iSize; i++)
        {
            sDataHex+=IntToHex((int)W906_TtlAt(Data, i), 2)+" ";
            if(i<iSize-3-iShowPushback || i==iSize-1-iShowPushback)
            {
                if(iDataType==0)
                {
                    sDataAscii+=MyDeCodeASCII((int)W906_TtlAt(Data, i));
                }
                else
                {
                    if(i<5+iShowPushback || i==iSize-1-iShowPushback)
                        sDataAscii+=MyDeCodeASCII((int)W906_TtlAt(Data, i));
                    else
                        sDataAscii+=IntToHex((int)W906_TtlAt(Data, i), 1);
                }
            }
            else
            {
                if(i==iSize-3-iShowPushback)
                    sDataAscii+="[";
                sDataAscii+=IntToHex((int)W906_TtlAt(Data, i), 2)+" ";
                if(i==iSize-2-iShowPushback)
                    sDataAscii+="]";
            }
        }
    }

    if(iSize==0)
    {
        ShowCommData("[TTL Board]", sType);
    }
    else
    {
        if(iboard==0)
        {
            ShowCommData("[TTL1] "+sType, Data);
        }
        else                                                                    //Isaac 20210309 :TTL RS232兩塊板子
        {
            ShowCommData("[TTL2] "+sType, Data);
        }
    }
}
//---------------------------------------------------------------------------
bool TfRS232Main::SaveSetupData(const TComm* Comm)                              //Isaac 20200903 :TTL RS232通訊
{
    WriteIniData(IniFileName, "COMPort", "CommName", ComNameTester );
    WriteIniData(IniFileName, "COMPort", "BaudRate", Comm->BaudRate );
    WriteIniData(IniFileName, "COMPort", "ByteSize", Comm->ByteSize );
    WriteIniData(IniFileName, "COMPort", "StopBits", Comm->StopBits );
    WriteIniData(IniFileName, "COMPort", "Parity"  , Comm->Parity   );
    WriteIniData(IniFileName, "Detail Settng", "ReadIntervalTimeout", edReadIntervalTimeout->Text);   //wei 20150212  add
    WriteIniData(IniFileName, "SystemSetup", "iTesterMode", iUseRS232Mode);     //Frank 20180723 :TTL RS232通訊

    ts_STD->TabVisible=true;
    tsTTL->TabVisible=false;

    LoadSetupData(CommTester);

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::SaveSetupData_TTL()                                           //Isaac 20200903 :TTL RS232通訊
{
    WriteIniData(IniFileName, "COMPort_TTL",    "CommName", ComNameTTL);
    WriteIniData(IniFileName, "COMPort_TTL_2",  "CommName", ComNameTTL_2);
    WriteIniData(IniFileName, "Detail Settng",  "ReadIntervalTimeout_TTL", edReadIntervalTimeout_TTL->Text);   //wei 20150212  add
    WriteIniData(IniFileName, "SystemSetup",    "iTesterMode", iUseRS232Mode);  //Frank 20180723 :TTL RS232通訊

    ts_STD->TabVisible=false;
    tsTTL->TabVisible=true;

    cbDevice_TTL->Text=ComNameTTL;                                              //Isaac 20200903 :TTL RS232通訊
    cbDevice_TTL_2->Text=ComNameTTL_2;                                          //Isaac 20210309 :TTL RS232兩塊板子
    cbBaudRate_TTL->Text="115200";                                              //TTL板子寫定115200    IntToStr(CommTester_TTL->BaudRate);
    cbBaudRate_TTL->Enabled=false;
    cbByteSize_TTL->ItemIndex=3;                                                //_8
    cbStopBit_TTL->ItemIndex=0;                                                 //_1
    cbParity_TTL->ItemIndex=0;                                                  //None
    edReadIntervalTimeout_TTL->Text=CommTester_TTL->ReadIntervalTimeout ;       //wei 20150212  add

    if(iTTLBoardNum>=1)                                                         //Isaac 20210309 :TTL RS232兩塊板通訊
    {
        LoadSetupData_TTL();
    }

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::LoadSetupData(TComm* Comm)
{
    Comm->CommName = CheckAndReadIniData(IniFileName, "COMPort", "CommName", AnsiString("COM3"));
    ComNameTester  = CheckAndReadIniData(IniFileName, "COMPort", "CommName", AnsiString("COM3"));
    Comm->BaudRate = CheckAndReadIniData(IniFileName, "COMPort", "BaudRate", 9600);
    Comm->ByteSize = (TByteSize)CheckAndReadIniData(IniFileName, "COMPort", "ByteSize", _7);
    Comm->StopBits = (TStopBits)CheckAndReadIniData(IniFileName, "COMPort", "StopBits", _1);
    Comm->Parity   = (TParity  )CheckAndReadIniData(IniFileName, "COMPort", "Parity", Even);
    Comm->ReadIntervalTimeout =CheckAndReadIniData(IniFileName, "Detail Settng", "ReadIntervalTimeout", 70);    //wei 20150212  add

    return true;
}
//---------------------------------------------------------------------------
bool TfRS232Main::LoadSetupData_TTL()                                           //Isaac 20200903 :TTL RS232通訊
{
    ComNameTTL  =CheckAndReadIniData(IniFileName, "COMPort_TTL",   "CommName", AnsiString("COM3"));
    ComNameTTL_2=CheckAndReadIniData(IniFileName, "COMPort_TTL_2", "CommName", AnsiString("COM8"));

    CommTester_TTL->ReadIntervalTimeout  =CheckAndReadIniData(IniFileName, "Detail Settng", "ReadIntervalTimeout_TTL", 70);
    CommTester_TTL_2->ReadIntervalTimeout=CheckAndReadIniData(IniFileName, "Detail Settng", "ReadIntervalTimeout_TTL", 70);
    return true;
}
//---------------------------------------------------------------------------

//AI(W906-GB-P4) 20260926: golden (Sender, TMouseButton Button, TShiftState Shift, int X, int Y); Rs232Bridge.h
//   declares the mouse handlers with Sender only (rule 4) -- the body never used the others.
void TfRS232Main::btnUpdateMouseDown(TObject *Sender)
{
#if 0 // TODO(W906-GB-P4): TPanel::BevelOuter / bvLowered / bvRaised are not in vclcompat Controls.h; visual only (golden MainForm.cpp:2580)
    btnUpdate->BevelOuter=bvLowered;
#endif
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden (Sender, TMouseButton Button, TShiftState Shift, int X, int Y); Rs232Bridge.h
//   declares the mouse handlers with Sender only (rule 4) -- the body never used the others.
void TfRS232Main::btnUpdateMouseUp(TObject *Sender)
{
#if 0 // TODO(W906-GB-P4): TPanel::BevelOuter / bvLowered / bvRaised are not in vclcompat Controls.h; visual only (golden MainForm.cpp:2586)
    btnUpdate->BevelOuter=bvRaised;
#endif
}
//---------------------------------------------------------------------------
void TfRS232Main::btnAllUseClick(TObject *Sender)
{
    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        MY_DUT_PAL[i]->cbSiteOn->Checked=true;
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::btnAllNoUseClick(TObject *Sender)
{
    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        MY_DUT_PAL[i]->cbSiteOn->Checked=false;
    }
}
//------------------------------------------------------------------------------
//Steven 20171013 (wei) : 畫面顯示從測試機收到的Bin別
//------------------------------------------------------------------------------
void TfRS232Main::SaveBinData()
{
    if(iUseRS232Mode==0 && MemoBinData->Lines->Count==0)                        //Isaac 20200903 :TTL RS232通訊
        return;
    if(iUseRS232Mode>=InterfaceType_TTL && MemoBinData_TTL->Lines->Count==0)
        return;
    AnsiString FileName;

    if(iUseRS232Mode>=InterfaceType_TTL)                                        //Isaac 20200903 :TTL RS232通訊
    {
        FileName.sprintf("D:\\RS232Log\\BinLog_TTL\\%04d_%02d", SystemYear, SystemMonth);
        MyForceDirectories(FileName);

        FileName.sprintf("D:\\RS232Log\\BinLog_TTL\\%04d_%02d\\%04d_%02d_%02d_%02d %02d %02d_TTL_Rs232_BinData.log",
                         SystemYear, SystemMonth,
                         SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        MemoBinData_TTL->Lines->SaveToFile(FileName);
        MemoBinData_TTL->Clear();
    }
    else //if(iUseRS232Mode==0)
    {
        FileName.sprintf("D:\\RS232Log\\BinLog\\%04d_%02d", SystemYear, SystemMonth);
        MyForceDirectories(FileName);

        FileName.sprintf("D:\\RS232Log\\BinLog\\%04d_%02d\\%04d_%02d_%02d_%02d %02d %02d_Rs232_BinData.log",
                         SystemYear, SystemMonth,
                         SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        MemoBinData->Lines->SaveToFile(FileName);
        MemoBinData->Clear();
    }
}
//------------------------------------------------------------------------------
void TfRS232Main::AddBinData(AnsiString sBinData)
{
    AnsiString Str;
    if(iUseRS232Mode>=InterfaceType_TTL)                                        //Isaac 20200903 :TTL RS232通訊
    {
        if(MemoBinData_TTL->Lines->Count>10000)
        {
            SaveBinData();
        }
        Str.sprintf("%04d-%02d-%02d,%02d:%02d:%02d.%03d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, sBinData);
        MemoBinData_TTL->Lines->Add(Str);
    }
    else if(iUseRS232Mode==2)
    {
        if(MemoBinData->Lines->Count>10000)
        {
            SaveBinData();
        }

        Str.sprintf("%04d-%02d-%02d,%02d:%02d:%02d.%03d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, sBinData);
        MemoBinData->Lines->Add(Str);
    }
    else //if(iUseRS232Mode==0) ==1
    {
        if(MemoBinData->Lines->Count>10000)
        {
            SaveBinData();
        }

        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, sBinData);
        MemoBinData->Lines->Add(Str);
    }
}
//------------------------------------------------------------------------------
void TfRS232Main::btnUpdate_TTLClick(TObject *Sender)                           //Isaac 20200903 :TTL RS232通訊
{
    ComNameTTL=cbDevice_TTL->Text;
    ComNameTTL_2=cbDevice_TTL_2->Text;

    CommTester_TTL->BaudRate  =115200;
    CommTester_TTL_2->BaudRate=115200;
    SaveSetupData_TTL();
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden (Sender, TMouseButton Button, TShiftState Shift, int X, int Y); Rs232Bridge.h
//   declares the mouse handlers with Sender only (rule 4) -- the body never used the others.
void TfRS232Main::btnUpdate_TTLMouseDown(TObject *Sender)                       //Isaac 20200903 :TTL RS232通訊
{
#if 0 // TODO(W906-GB-P4): TPanel::BevelOuter / bvLowered / bvRaised are not in vclcompat Controls.h; visual only (golden MainForm.cpp:2689)
    btnUpdate_TTL->BevelOuter=bvLowered;
#endif
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden (Sender, TMouseButton Button, TShiftState Shift, int X, int Y); Rs232Bridge.h
//   declares the mouse handlers with Sender only (rule 4) -- the body never used the others.
void TfRS232Main::btnUpdate_TTLMouseUp(TObject *Sender)                         //Isaac 20200903 :TTL RS232通訊
{
#if 0 // TODO(W906-GB-P4): TPanel::BevelOuter / bvLowered / bvRaised are not in vclcompat Controls.h; visual only (golden MainForm.cpp:2695)
    btnUpdate_TTL->BevelOuter=bvRaised;
#endif
}
//---------------------------------------------------------------------------
void TfRS232Main::btClear_TTLClick(TObject *Sender)                             //Isaac 20200903 :TTL RS232通訊
{
     MemoLog->Clear();
}
//---------------------------------------------------------------------------
void TfRS232Main::btnTTL_ManualClick(TObject *Sender)                           //Isaac 20200903 :TTL RS232通訊
{
    AnsiString asTester="", asTest="", asTest2="";
    AnsiString asSendtoBoard1="";
    AnsiString asSendtoBoard2="";
    AnsiString Str="";
    char cStart[128];
    char cBuf[128];
    char cCRC[2];
    bool bNeedTest[2]={true,true};

    ZeroMemory(cStart, sizeof(cStart));                                         //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cBuf, sizeof(cBuf));                                             //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cCRC, sizeof(cCRC));                                             //先清空記憶體再讀取資料, 避免舊資料出現

    ShowCommData("[Manual Access]", "Manual Test Button Pressed");

    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        iStart[i]=MY_DUT_PAL[i]->cbSiteOn->Checked;
    }

    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        MY_DUT_PAL[i]->plSite->Caption=(MY_DUT_PAL[i]->cbSiteOn->Checked==true)?"T":"";
    }

    for(int j=0; j<8; j++)
    {
        //AI(W906-GB-P4) 20260926: BCB6 AnsiString has only operator+=(const AnsiString&), so the int went through
        //   AnsiString(int) and appended "0"/"1" (hence the "11000000" comment).  vclcompat also has
        //   operator+=(char), which an int would pick (standard conversion) and append the byte 0x00/0x01.
        asTester+=AnsiString(iStart[j]);                                        //11000000
    }

    if(iTTLBoardNum>1)
    {
        asSendtoBoard1=asTester.SubString(1,4);
        if(asSendtoBoard1=="0000")
        {
            bNeedTest[0]=false;
        }
        asSendtoBoard1+="0000";

        if(iTTLBoardNum>1 || bTTLBoardAddr)
        {
            asTest.sprintf("@00WSOTS%s", asSendtoBoard1);
        }
        else
        {
            asTest.sprintf("@WSOTS%s", asSendtoBoard1);
        }

        asSendtoBoard2=asTester.SubString(5,4);
        if(asSendtoBoard2=="0000")
        {
            bNeedTest[1]=false;
        }

        asSendtoBoard2+="0000";
        if(iTTLBoardNum>1 || bTTLBoardAddr)
        {
            asTest2.sprintf("@01WSOTS%s", asSendtoBoard2);
        }
        else
        {
            asTest2.sprintf("@WSOTS%s", asSendtoBoard2);
        }
    }
    else
    {
        if(iTTLBoardNum>1 || bTTLBoardAddr)
        {
            asTest.sprintf("@00WSOTS%s", asTester);                             //@WSOTS11000000
        }
        else
        {
            asTest.sprintf("@WSOTS%s", asTester);                               //@WSOTS11000000
        }
        if(asTester=="00000000")
        {
            bNeedTest[0]=false;
        }
        bNeedTest[1]=false;
    }

    const int iLength=asTest.Length();
    strcpy(cBuf, asTest.c_str());
    //AI(W906-GB-P4) 20260926: crc_chk takes unsigned char*; bcc32 accepted char* with warning W8079 (mixing pointers
    //   to different char types), C++ needs the cast.  Same at every crc_chk call in this file.
    crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength, cCRC[0], cCRC[1]);
    strcpy(cStart, asTest.c_str());                                             //@WSOTS11000000

    SendStartData.clear();
    vector<Byte>(SendStartData).swap(SendStartData);

    for(int i=0; i<asTest.Length(); i++)
    {
        SendStartData.push_back(cStart[i]);                                     //@WSOTS11000000
    }
    SendStartData.push_back(cCRC[0]);
    SendStartData.push_back(cCRC[1]);                                           //@WSOTS11000000'CRC'
    SendStartData.push_back('#');                                               //@WSOTS11000000'CRC'#

    if(bNeedTest[0]==true)
    {
        SendCommandToTester_TTL(SendStartData, 0);                              //@WSOTS11000000'CRC'#  送測試訊號
        bStartSOT[0]=true;
    }
    else
    {
        bStartSOT[0]=false;
    }
    if(iTTLBoardNum>1)
    {
        const int iLength2=asTest2.Length();
        strcpy(cBuf, asTest2.c_str());
        crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength2, cCRC[0], cCRC[1]);
        strcpy(cStart, asTest2.c_str());                                        //@WSOTS11000000

        SendStartData.clear();
        for(int i=0; i<asTest2.Length(); i++)
        {
            SendStartData.push_back(cStart[i]);                                 //@WSOTS11000000
        }
        SendStartData.push_back(cCRC[0]);
        SendStartData.push_back(cCRC[1]);                                       //@WSOTS11000000'CRC'
        SendStartData.push_back('#');                                           //@WSOTS11000000'CRC'#
        if(bNeedTest[1]==true)
        {
            SendCommandToTester_TTL(SendStartData, 1);
            bStartSOT[1]=true;
        }
        else
        {
            bStartSOT[1]=false;
        }
    }
    btnTTL_Manual->Enabled=false;
}
//---------------------------------------------------------------------------
void TfRS232Main::btnClearSotClick(TObject *Sender)                             //Isaac 20200903 :TTL RS232通訊
{
    AnsiString asTester="", asTest="",Str="";
    char cWCOST[128];
    char cBuf[128];
    char cCRC[2];

    ZeroMemory(cWCOST, sizeof(cWCOST));                                         //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cBuf, sizeof(cBuf));                                             //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cCRC, sizeof(cCRC));                                             //先清空記憶體再讀取資料, 避免舊資料出現

    if(iTTLBoardNum>1 || bTTLBoardAddr)
    {
        asTest="@00WCSOT00000000";                                              //@WCSOT00000000
    }
    else
    {
        asTest="@WCSOT00000000";                                                //@WCSOT00000000
    }

    ShowCommData("[Manual Access]", "Clear SOT Button Pressed");

    const int iLength=asTest.Length();
    strcpy(cBuf, asTest.c_str());
    crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength, cCRC[0], cCRC[1]);
    strcpy(cWCOST, asTest.c_str());                                             //@WCSOT00000000

    SendStartData.clear();
    for(int i=0; i<asTest.Length(); i++)
    {
        SendStartData.push_back(cWCOST[i]);                                     //@WCSOT00000000
    }
    SendStartData.push_back(cCRC[0]);
    SendStartData.push_back(cCRC[1]);                                           //@WCSOT00000000'CRC'
    SendStartData.push_back('#');                                               //@WCSOT00000000'CRC'#
    SendCommandToTester_TTL(SendStartData, 0);                                  //@WCSOT00000000'CRC'#  送測試訊號


    ZeroMemory(cWCOST, sizeof(cWCOST));                                         //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cBuf, sizeof(cBuf));                                             //先清空記憶體再讀取資料, 避免舊資料出現
    ZeroMemory(cCRC, sizeof(cCRC));                                             //先清空記憶體再讀取資料, 避免舊資料出現

    if(iTTLBoardNum>1 || bTTLBoardAddr)
    {
        asTest="@01WCSOT00000000";                                              //@WCSOT00000000
    }
    else
    {
        asTest="@WCSOT00000000";                                                //@WCSOT00000000
    }

    const int iLength2=asTest.Length();
    strcpy(cBuf, asTest.c_str());
    crc_chk(reinterpret_cast<unsigned char*>(cBuf), iLength2, cCRC[0], cCRC[1]);
    strcpy(cWCOST, asTest.c_str());                                             //@WCSOT00000000

    SendStartData.clear();
    for(int i=0; i<asTest.Length(); i++)
    {
        SendStartData.push_back(cWCOST[i]);                                     //@WCSOT00000000
    }
    SendStartData.push_back(cCRC[0]);
    SendStartData.push_back(cCRC[1]);                                           //@WCSOT00000000'CRC'
    SendStartData.push_back('#');                                               //@WCSOT00000000'CRC'#
    if(iTTLBoardNum>1)
    {
        SendCommandToTester_TTL(SendStartData, 1);
    }
    bStartSOT[0]=false;
    bStartSOT[1]=false;

    btnClearSot->Enabled=false;
    btnTTL_Manual->Enabled=true;
}
//---------------------------------------------------------------------------

void TfRS232Main::Button10Click(TObject *Sender)
{
    CloseTesterComm_TTL(0);
    CloseTesterComm_TTL(1);
}
//---------------------------------------------------------------------------
void TfRS232Main::Button11Click(TObject *Sender)
{
    OpenTesterComm_TTL(0);
    OpenTesterComm_TTL(1);
}
//---------------------------------------------------------------------------
void TfRS232Main::InitialBarcodeList()                                          //JimmyChiu 20211004 : add Barcode?指令
{
    if(sBarCode->Count==MAX_SITE_COUNT)
    {
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            sBarCode->Strings[i]="0";
        }
    }
    else
    {
        sBarCode->Clear();
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            sBarCode->Add("0");
        }
    }

    if(MY_DUT_PAL.size()!=0)
    {
        for(int i=0; i<MAX_SITE_COUNT; i++)
        {
            MY_DUT_PAL[i]->labOcr->Caption="0";
        }
    }
}
//---------------------------------------------------------------------------
void TfRS232Main::btnEnableSOTCSOTClick(TObject *Sender)
{
    btnClearSot->Enabled=true;
}
//---------------------------------------------------------------------------

}  // namespace rs232std
