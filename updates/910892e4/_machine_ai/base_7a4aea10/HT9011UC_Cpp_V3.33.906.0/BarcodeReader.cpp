//---------------------------------------------------------------------------
//  BarcodeReader.cpp -- faithful headless port of golden BarcodeReader.cpp
//  (445 lines, cp950). Facade/deviation/gate registers: BarcodeReader.h.
//
//  AI(W906-FW-BARCODE1) 20260824: entity wave only -- the G-Barcode /
//  E-B1/E-B2 / WB-9/WB-10 / W8-5 / (6-B) call sites stay gated until the
//  next (behaviour-change) wave.
//
//  GOLDEN QUIRK (BR-q1): BarcodeChange[] initialises 15 entries against
//  bcTotal==16 (MachineType.h:933) -- BarcodeChange[bcSECSGEM==15] is a
//  default-constructed empty AnsiString, so a SECSGEM-triggered login line
//  logs "... Change " with no suffix. Preserved verbatim.
//
//  GOLDEN NOTE (BR-n1): FormShow centres against hard-coded 1280x1024
//  (golden :54-55), not the actual screen metrics. Preserved verbatim.
//
//  GOLDEN NOTE (BR-n2): TimerKeyInTimer's KYEC tail (golden :353-401)
//  accepts 6-digit codes whose 2-digit head is 85..120 -- a 2-digit head
//  can never exceed 99, so codes 100..120 are only reachable via the
//  7-digit/3-digit-head arm. Golden's own comment says "85~120 (85~120)".
//  Preserved verbatim.
//---------------------------------------------------------------------------
#include "MachineDefine.h"
#pragma hdrstop

#include "BarcodeReader.h"
#include "MachineType.h"
#include "cmydef.h"          // USE_BARCODE_AS_KEYBOARD/bBarcodeReader/ReEnterBarcode/asSecsGemBarCode/
                             // bEnable_KLT_Function/SPIL_FOR_QLE/bBarCoderAutoLogin/bBarCoderSetupFile/
                             // InitialOK/AccessLevel/bOffsetEnterBarcode
#include "cMyDB.h"           // RecordProcess
#include "cprod.h"           // TestIF_File (b2DIDAllowList), CosFunction reach   // golden :8 uLotInfo.h equivalent surface
#include "Config.h"          // IniConfig (bO23_InputLotIDByBarcode/bVTESTFunction/bSPILFunction/bN06_UseBarcode)
#include "CosFunction.h"     // CosFunction (bUseBarCoderAutoLogin/bUseBarCoderChangeSetupFile/bFTPUseBarcodeReader/bHiSiliconFunction)
#include "SECSGEM/SecsEventType.h"    // SECS_EVENT.BarcodeReaderEnter  // golden :9 uHGemHT9045.h equivalent
#include "SECSGEM/SecsEventReport.h"  // EventReport(unsigned)
// golden :10 #include "FTPClient.h" -- NOT ported (TfFTPClient has no port
// anywhere in this tree, grep 20260824) -- see GATE (B-F1) in the header.
//---------------------------------------------------------------------------
TFormBarcodeReader *FormBarcodeReader;          // golden :14 (zero-init NULL; nothing constructs at static-init)
//---------------------------------------------------------------------------
AnsiString sLastKeyin = "";
// 20140103 wei KYEC Barcode Reader
AnsiString BarcodeChange[bcTotal]={" User Name", " Tray Form", " Plate Form", " TrayAssign", " Temperature",
                                   " Contact", " Tester IF", " Bin", " Setup", " Yield",
                                   " Auto Clean", " Builder", " Offset", " Setup File",
                                   " Exit"};                                    // GOLDEN QUIRK (BR-q1): 15 of bcTotal==16

//---------------------------------------------------------------------------
TFormBarcodeReader::TFormBarcodeReader()                                        // golden :24-30 -- fields only (D-2)
{
    iKeyType=KEY_NONE;
    sBarcodeInfo="";
    bShow=false;                                                                //Steven 20140508
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::btnEnterClick()                                        // golden :32-44
{
    if(_sInputType=="RunMode")                                                  //RogerYang 20251215 : JCET 2D
    {
    }
    else if(edtBarcodeNumber->Text.Length()<4)                                  //Ifor 20160830 add 避免客戶端誤輸入不符範圍
    {
        return;
    }

    iKeyType=KEY_ENTER;
    Close();
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::FormShow()                                             // golden :46-91
{
    bShow=true;                                                                 //Steven 20140508
    sLastKeyin="";

    iKeyType=KEY_NONE;
    edtBarcodeNumber->Text="";
    edtBarcodeNumber->Enabled=true;                                             //KevinCheng 20250913 : fixed
    Left=(1280-Width)/2;                                                        // GOLDEN NOTE (BR-n1)
    Top =(1024-Height)/2;
    edtBarcodeNumber->SetFocus();

    #ifdef SOFT_SIMULTE
        TimerKeyIn->Enabled=false;
    #else
        if(CUSTOMER_CODE==CC_PTI)                                               //RogerYang 20170327 (Steven)
        {
            //lblInputType->Caption="Input Lot ID:";
            if((CosFunction.bUseBarCoderAutoLogin && bBarCoderAutoLogin) ||     //Sam 20230320 : 使用 BarCodeReader
               (CosFunction.bUseBarCoderChangeSetupFile && bBarCoderSetupFile))
            {
                TimerKeyIn->Enabled=true;
            }
            else
            {
                if(AccessLevel>0)
                    TimerKeyIn->Enabled=false;
                else
                    TimerKeyIn->Enabled=true;
            }
        }
        else if(CosFunction.bFTPUseBarcodeReader==true)
        {
#if 0   // GATE (B-F1) -- fFTPClient (TfFTPClient) has no port anywhere in this tree; see header
            if((IniConfig.bN06_UseBarcode==false && fFTPClient->bShow==false) ||
               (CUSTOMER_CODE==CC_CYUEAN && fFTPClient->bShow))                 //Sam 20230706 : 依安客戶 FTP Setup
            {
                TimerKeyIn->Enabled=true;
            }
#endif  // GATE (B-F1)
        }
        else
        {
            TimerKeyIn->Enabled=true;
        }
#if 0   // GATE (B-W1) -- RegisterHotKey needs a real HWND (golden Handle); none exists headless
        RegisterHotKey(Handle, 2, MOD_CONTROL, 'V');                            //wei 20151120 內容用ctrl+V貼上
#endif  // GATE (B-W1)
    #endif
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::FormClose()                                            // golden :93-120
{
    TimerKeyIn->Enabled=false;

    if(iKeyType==KEY_ENTER)
    {
        sBarcodeInfo = edtBarcodeNumber->Text;
        asSecsGemBarCode =sBarcodeInfo;                                         //wei 20150811 SecsGem BarcodeReader
        EventReport(SECS_EVENT.BarcodeReaderEnter);                             //wei 20150811 SecsGem BarcodeReader
    }
    else
    {
        sBarcodeInfo="";
    }

    if(CUSTOMER_CODE==CC_SJ_Semiconductor ||                                    //Steven 20240612 : 去除換行
       CUSTOMER_CODE==CC_SJ_Semiconductor_OS)
    {
        sBarcodeInfo=StringReplace(sBarcodeInfo, "\r", "", TReplaceFlags()<<rfReplaceAll);
        sBarcodeInfo=StringReplace(sBarcodeInfo, "\n", "", TReplaceFlags()<<rfReplaceAll);
    }
    sBarcodeInfo=sBarcodeInfo.Trim();

    bShow=false;                                                                //Steven 20140508
#if 0   // GATE (B-W1) -- see FormShow
    UnregisterHotKey(Handle, 2);                                                //wei 20151120 內容用ctrl+V貼上
#endif  // GATE (B-W1)
    _sInputType="";
}
//---------------------------------------------------------------------------
AnsiString InputBarcodeNumber(AnsiString sLabelCaption, AnsiString sInputType)  // golden :122-145
{
    AnsiString sBarcode;

    FormBarcodeReader->lblInputType->Caption=sLabelCaption;
    FormBarcodeReader->_sInputType=sInputType;
    if(sInputType=="Password")                                                  //Steven 20200313
    {
        FormBarcodeReader->edtBarcodeNumber->PasswordChar='*';
    }
    FormBarcodeReader->ShowModal();                                             // (D-1) offline instant-submit no-op
    FormBarcodeReader->edtBarcodeNumber->PasswordChar=0;                        // golden: =NULL (char)

    sBarcode=FormBarcodeReader->sBarcodeInfo;
//    FormBarcodeReader->lblInputType->Caption="Input Operator ID:";
    if(sInputType=="SetupFile")                                                 //Sam 20230320 : 使用 BarCodeReader
    {
        return sBarcode.UpperCase();
    }
    else
    {
        return sBarcode;
    }
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::edtBarcodeNumberKeyDown(unsigned short &Key)           // golden :147-157 (D-4)
{
    if(IniConfig.bSPILFunction && TestIF_File.b2DIDAllowList)                   //JerryYang 20241104 : 支援2DID白名單
    {
        if(edtBarcodeNumber->Text.Length()>=8 && Key=='\r')
        {
            edtBarcodeNumber->Enabled=false;
        }
    }
}
//---------------------------------------------------------------------------
bool bBarcodeFirstKeyIn=false;                                                  //Ifor 20190924 : add Barcode 輸入
void TFormBarcodeReader::edtBarcodeNumberKeyUp(unsigned short &Key)             // golden :160-180 (D-4)
{
    if(Key==17)
    {
        edtBarcodeNumber->Text="";
        sLastKeyin="";
    }

    if(CUSTOMER_CODE==CC_PTI ||
       CUSTOMER_CODE==CC_SCC ||                                                 //Ifor 20181029 add :SCC Barcode
       CUSTOMER_CODE==CC_TFME_CHINA)                                            //RogerYang 20170407 (Steven)
    {
        HDelayTime.SetMSAndOn(80);
    }

    if(bBarcodeFirstKeyIn==false)                                               //Ifor 20190924 : add Barcode 輸入
    {
        bBarcodeFirstKeyIn=true;
    }
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::TimerKeyInTimer()                                      // golden :182-402
{
    static int iCount=0;

    if(InitialOK==false)
        return;

    if(bBarcodeFirstKeyIn==true)                                                //Ifor 20190924 : add Barcode 輸入
    {
        bBarcodeFirstKeyIn=false;
        return;
    }

    if(bShow)
        FormBarcodeReader->BringToFront();                                      //Steven 20140508
    else
        return;

    if(IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
    {
        if(_sInputType=="RunMode")                                              //RogerYang 20251215 : JCET 2D
        {
        }
        else if(edtBarcodeNumber->Text.Length()>30 ||                           //Steven 20250217 : 華中芯這邊限制5~30字
           edtBarcodeNumber->Text.Length()<5)                                   //Steven 20240612 : SJSM要求Lot ID
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
    else if(CUSTOMER_CODE==CC_JCET)                                             //Steven 20210304 : 江陰長電要求Barcode
    {
        if(_sInputType=="RunMode")                                              //RogerYang 20251215 : JCET 2D
        {
        }
        else if(edtBarcodeNumber->Text.Length()<15)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }

        if(edtBarcodeNumber->Text!="")
        {
            btnEnter->Click();
        }
    }
    else if(CUSTOMER_CODE==CC_SJ_Semiconductor ||
            CUSTOMER_CODE==CC_SJ_Semiconductor_OS)
    {
        if((_sInputType=="LotID" || _sInputType=="UserName") &&
           (edtBarcodeNumber->Text.Length()>15 ||                               //Steven 20240612 : SJSM要求Lot ID
            edtBarcodeNumber->Text.Length()<6))
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
        else if(_sInputType=="")                                                //工作檔
        {
            if(edtBarcodeNumber->Text.Length()>100 ||
               edtBarcodeNumber->Text.Length()<15)
            {
                edtBarcodeNumber->Text="";
                sLastKeyin="";
            }
        }

        if(edtBarcodeNumber->Text!="")
        {
            btnEnter->Click();
        }
    }
    else if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_TFME_CHINA)
    {
        if((_sInputType=="LotID" || _sInputType=="UserName") &&
           (edtBarcodeNumber->Text.Length()>30 ||                               //Steven 20200306 : SCC要求Lot ID
            edtBarcodeNumber->Text.Length()<5))
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
        else if(_sInputType=="")                                                //工作檔
        {
            if(CosFunction.bHiSiliconFunction==true)
            {
                if(edtBarcodeNumber->Text.Length()>12 &&
                   !(edtBarcodeNumber->Text.Pos("LAC")==1 ||
                     edtBarcodeNumber->Text.Pos("KL")==1))                      //Ifor 20181224 : add SCC 工作檔卡控
                {
                    edtBarcodeNumber->Text="";
                    sLastKeyin="";
                }
            }
            else
            {
                if(edtBarcodeNumber->Text.Length()>30 ||
                   edtBarcodeNumber->Text.Length()<5)
                {
                    edtBarcodeNumber->Text="";
                    sLastKeyin="";
                }
            }
        }

        if(edtBarcodeNumber->Text!="")
        {
            btnEnter->Click();
        }
    }
    else if(IniConfig.bVTESTFunction==true ||                                   //jou 20220912 : 增加VTEST時可以按鍵盤輸入
           (CosFunction.bUseBarCoderAutoLogin && bBarCoderAutoLogin) ||         //Sam 20221101 : 使用 BarCoder 自動登錄
           (CosFunction.bUseBarCoderChangeSetupFile && bBarCoderSetupFile))     //Sam 20230320 : 使用 BarCodeReader
    {
        if(edtBarcodeNumber->Text.Length()<=3)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
            iCount=0;
        }
        else
        {
            iCount++;
            if(iCount>3)
            {
                iCount=0;
                btnEnter->Click();
            }
        }
    }
    else if(CUSTOMER_CODE==CC_PTI)                                              //Sam 20221101 註冊時
    {
        if(edtBarcodeNumber->Text.Length()<3 ||
          (edtBarcodeNumber->Text.Length()!=12 && HDelayTime.Off()))
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
    else if(CUSTOMER_CODE==CC_AMD_M)
    {
        if(edtBarcodeNumber->Text.Length()<=3)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
    else if(IniConfig.bSPILFunction && TestIF_File.b2DIDAllowList &&            //JerryYang 20241104 : 支援2DID白名單
            SPIL_FOR_QLE==0)
    {
        if(edtBarcodeNumber->Text.Length()<=8)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
    else if(IniConfig.bSPILFunction &&
            TestIF_File.b2DIDAllowList && SPIL_FOR_QLE==1)                      //KevinCheng 20250912 : 掃描Lot ID
    {
        if(edtBarcodeNumber->Text.Length()<8)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
#if 0   // GATE (B-F1) -- fFTPClient has no port; see header
    else if(CUSTOMER_CODE==CC_CYUEAN && fFTPClient->bShow)                      //Sam 20230706 : 依安客戶 FTP Setup
    {
        if(edtBarcodeNumber->Text.Length()<10 || edtBarcodeNumber->Text.Length()>45)
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
    }
#endif  // GATE (B-F1)
    else
    {
        if(edtBarcodeNumber->Text.Length()<6 ||
           edtBarcodeNumber->Text.Length()>7)                                   // 2013.12.17 , Joye , KYEC Barcode
        {
            edtBarcodeNumber->Text="";
            sLastKeyin="";
        }
        else
        {
            if(sLastKeyin=="")
            {
                sLastKeyin=edtBarcodeNumber->Text;
                AnsiString sHeadNum;
                int iHeadNum=0;
                if(sLastKeyin.Length()==6)                                      // 2013.12.17 , Joye , KYEC Barcode
                {
                    sHeadNum=sLastKeyin.SubString(1, 2);
                    iHeadNum=atoi(sHeadNum.c_str());
                }
                else if(sLastKeyin.Length()==7)
                {
                    sHeadNum=sLastKeyin.SubString(1, 3);
                    iHeadNum=atoi(sHeadNum.c_str());
                }

                if(bEnable_KLT_Function==true)                                  //Ifor 20180802 (Steven) : add KLT
                {                                                               //Ifor 20180517 : add BarCode 編碼
                    if(iHeadNum<3 || iHeadNum>31)                               // KYEC 3~31 (2003~2031)
                    {
                        edtBarcodeNumber->Text="";
                        sLastKeyin="";
                    }
                }
                else
                {
                    if(iHeadNum<85 || iHeadNum>120)                             // KYEC 85~120 (85~120) -- GOLDEN NOTE (BR-n2)
                    {
                        edtBarcodeNumber->Text="";
                        sLastKeyin="";
                    }
                }
            }
            else
            {
                edtBarcodeNumber->Text=sLastKeyin;
            }
        }
    }
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::miClearClick()                                         // golden :404-408
{
    edtBarcodeNumber->Text="";
    sLastKeyin="";
}
//---------------------------------------------------------------------------
void TFormBarcodeReader::miCloseClick()                                         // golden :410-413
{
    Close();
}
//------------------------------------------------------------------------------
int Barcode_Reader(int Barcode)                                                 // golden :415-444 -- 20140103 wei KYEC
{
    int iFlag=2;

    if(USE_BARCODE_AS_KEYBOARD!=0 && bBarcodeReader && ReEnterBarcode[Barcode]==false &&
       (CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX))
    {
        AnsiString sBarcodeID=InputBarcodeNumber("Input Operator ID:");
        if(sBarcodeID=="")
        {
            iFlag=0;
        }
        else
        {
            AnsiString sRecord="ID:"+sBarcodeID+" Login --- Change "+BarcodeChange[Barcode];
            RecordProcess(sRecord.c_str());

            if(Barcode==bcOffset)
            {
                bOffsetEnterBarcode=true;
            }
            else if(Barcode!=bcSECSGEM)                                         //wei 20150904 SECSGEM 每次都要輸入
            {
                ReEnterBarcode[Barcode]=true;
            }
            iFlag=1;
        }
    }
    return iFlag;
}
//------------------------------------------------------------------------------
