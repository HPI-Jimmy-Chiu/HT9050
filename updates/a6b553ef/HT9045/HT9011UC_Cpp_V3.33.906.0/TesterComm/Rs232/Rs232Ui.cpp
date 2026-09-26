// ===========================================================================
//  TesterComm/Rs232/Rs232Ui.cpp -- the form side of the translated RS232Standard program (TfRS232Main +
//  TMyDutPanel): constructor with the MainForm.dfm design-time widget values, destructor, ShowVersion,
//  FormCreate / FormDestroy / FormShow / FormClose, ShowInterface.
//
//  AI(W906-GB-P4) 20260926: golden D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410 (Big5 -> UTF-8)
//    MainForm.cpp   45-294  ShowVersion                 (builds slCmdList, sets bShowVersionOK)
//                  358-384  TfRS232Main::TfRS232Main    (+ MainForm.dfm values, + ~TfRS232Main)
//                  386-406  FormCreate / FormDestroy / AppException (text only, gated)
//                  407-536  FormShow
//                  537-556  FormClose
//                  557-579  ShowInterface
//    MyDutPanel.cpp  17-107 TMyDutPanel::~TMyDutPanel / TMyDutPanel::TMyDutPanel
//    MainForm.dfm   design-time values (Caption / Text / Items / ItemIndex / Checked / Visible / Enabled / Color /
//                   TabVisible / ActivePage / Timer Interval / TComm port settings)
//  Rules: TesterComm/Rs232/TRANSLATION_RULES.md.  Every `#if 0 // TODO(W906-GB-P4)` below is a gate.
//
//  Exit order (Rs232Engine.cpp): FormClose -> FormDestroy -> delete.  FormClose and FormDestroy NULL what golden
//  deletes there, and ~TfRS232Main frees only what is still allocated (Rs232Engine::Teardown also runs
//  FormDestroy + delete WITHOUT FormClose when FormCreate / FormShow threw).
//
//  REAL FILES / PORTS, BY DESIGN (golden paths kept; IniFileName / asHGeneralPath can be redirected by
//  Rs232Engine::OverrideIniPaths):
//    D:\RS232Standard\System\Setup.ini  [SystemSetup] iRevCycleClear / iTesterMode / bCheckClosedSiteHasBin
//        (FormShow; golden CheckAndReadIniData writes the default back when a key is missing)
//    d:\HT9045\system\Gerneral.ini      [System] TTL_CARD_TYPE / CUSTOMER_CODE (FormShow; same write-back of a
//        missing key -- this is the Handler's production Gerneral.ini)
//    D:\RS232Log\LOG\...                slRS232Log (TMyStringList "RS232_Log", TBy2Hour; its destructor, run by
//        FormDestroy, saves the log)
//    TComm CommTester / CommTester_TTL / CommTester_TTL_2 (COM ports opened by OpenTesterComm* from FormShow;
//        the .dfm default port is COM13, LoadSetupData* overwrites it), uSocketServer (SOFT_SIMULTE, TServerSocket
//        port 59999, created here, opened by OpenTesterComm)
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

// golden MainForm.cpp:16-17 (see the Rs232Bridge.h banner: the 902 snapshot has DEBUG uncommented, the releases
// do not)
#if RS232STD_GOLDEN_DEBUG
#define DEBUG
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif

namespace rs232std {

namespace {

// AI(W906-GB-P4) 20260926: VCL TControl's constructor sets Visible:=True and Enabled:=True (TTimer: Enabled:=True);
// the headless stand-ins default both to false.  Every widget is created through this so it starts where VCL
// starts, before the .dfm values are applied on top (and the web page shows the same visibility golden does).
template <class T>
T* VclNew()
{
    T* p = new T();
    p->Visible = true;
    p->Enabled = true;
    return p;
}

// AI(W906-GB-P4) 20260926: .dfm `Items.Strings = ( ... )` (NULL-terminated list).
void DfmItems(TStringList* list, const char* const* items)
{
    list->Clear();
    for (; *items != NULL; ++items)
        list->Add(*items);
}

// AI(W906-GB-P4) 20260926: VCL TCustomComboBox.SetItemIndex (CB_SETCURSEL) also puts Items[ItemIndex] into the
// edit text, and an index outside the list clears it.  The stand-in's ItemIndex and Text are plain members, so
// where golden code itself sets ItemIndex (FormShow) this is called right after, keeping ->Text equal to what VCL
// would show (the web page draws ->Text).
void VclComboSyncText(TComboBox* cb)
{
    if (cb->ItemIndex >= 0 && cb->ItemIndex < cb->Items->Count)
        cb->Text = AnsiString(cb->Items->Strings[cb->ItemIndex]);
    else
        cb->Text = "";
}

}  // namespace

//---------------------------------------------------------------------------
//  golden MyDutPanel.cpp:17-107 -- TMyDutPanel (one per site; web: HTWidgets.makeDutPanel).
//  TfMyPal (MyDutPanel.cpp:11-16, MyDutPanel.dfm) is golden's design-time template form and has no runtime role.
//---------------------------------------------------------------------------
TMyDutPanel::~TMyDutPanel()
{
//    gpSite->Parent=NULL;
    //AI(W906-GB-P4) 20260926: no Parent chain headless.  Golden creates the five children with this TComponent as
    //   their Owner (`new TGroupBox(this)` ...), so TComponent's destructor frees them; done explicitly here.
    delete labOcr;
    delete cbSiteOn;
    delete plSite;
    delete cbBin;
    delete gpSite;
}
//------------------------------------------------------------------------------
TMyDutPanel::TMyDutPanel(int index)
{
    //AI(W906-GB-P4) 20260926: golden `TMyDutPanel(TComponent* Owner, int index): TComponent(Owner)`.  Owner
    //   (palSite) was only the Parent of gpSite.  VCL zero-fills a new instance, so _Enable starts false (golden
    //   never writes it).  Name / Parent / geometry / Font / Align / Alignment are layout only (the web page draws
    //   the panel) and stay as golden text in comments.
    _Enable=false;

    AnsiString str;
//    int iTop    =0;
//    int iLeft   =4;
//    int iTPitch =110;
//    int iLPitch =154;

    _Index=index;
    gpSite  =VclNew<TGroupBox>();       // golden new TGroupBox  (this);
    cbBin   =VclNew<TComboBox>();       // golden new TComboBox  (this);
    plSite  =VclNew<TPanel>();          // golden new TPanel     (this);
    cbSiteOn=VclNew<TCheckBox>();       // golden new TCheckBox  (this);
    labOcr  =VclNew<TLabel>();          // golden new TLabel     (this);

//    str.sprintf("gpSite%02d", index);
//    gpSite->Name        =str;
//    gpSite->Parent      =dynamic_cast <TPanel *>(Owner);
//    gpSite->Height      =110;
//    gpSite->Width       =150;
//    gpSite->Left        =iLeft+(index%8)*iLPitch;
//    gpSite->Top         =iTop +(index/8)*iTPitch;
//    gpSite->Font->Color =clWhite;
//    gpSite->Font->Name  ="Arial";
//    gpSite->Font->Size  =12;
    str.sprintf("Site %02d", index+1);
    gpSite->Caption     =str;

//    str.sprintf("cbBin%02d", index);
//    cbBin->Name         =str;
//    cbBin->Parent       =gpSite;
//    cbBin->Height       =28;
//    cbBin->Left         =35;
//    cbBin->Top          =68;
//    cbBin->Width        =106;
//    cbBin->Font->Color  =clBlack;
//    cbBin->Font->Name   ="Arial";
//    cbBin->Font->Size   =12;
    cbBin->Tag          =index;
    cbBin->Items->Add("1");
    cbBin->Items->Add("2");
    cbBin->Items->Add("3");
    cbBin->Items->Add("4");
    cbBin->Items->Add("5");
    cbBin->Items->Add("6");
    cbBin->Items->Add("1..5");
    cbBin->Items->Add("0..11");
    cbBin->Text         ="1";
    cbBin->ItemIndex    =-1;            //AI(W906-GB-P4) 20260926: not golden text.  VCL: CB_ADDSTRING selects nothing
                                        //   and WM_SETTEXT on a csDropDown combo only sets the edit text, so golden's
                                        //   ItemIndex is -1 here; the stand-in's default is 0.

//    str.sprintf("cbSiteOn%02d", index);
//    cbSiteOn->Name      =str;
//    cbSiteOn->Parent    =gpSite;
//    cbSiteOn->Height    =17;
//    cbSiteOn->Left      =15;
//    cbSiteOn->Top       =72;
//    cbSiteOn->Width     =14;
    cbSiteOn->Caption   ="";
    cbSiteOn->Tag       =index;
    cbSiteOn->Checked   =false;

//    str.sprintf("plSite%02d", index);
//    plSite->Name         =str;
//    plSite->Parent       =gpSite;
    plSite->Color        =0x00C0C0C0;   // golden clSilver
//    plSite->Height       =49;
//    plSite->Left         =12;
//    plSite->Top          =18;
//    plSite->Width        =129;
//    plSite->Font->Color  =clWhite;
//    plSite->Font->Name   ="Impact";
//    plSite->Font->Size   =26;
    plSite->Caption      ="01";
    plSite->Tag          =index;

//    str.sprintf("labOcr%02d", index);                                           //Steven 20150713 : for 2D Code
//    labOcr->Name         =str;
//    labOcr->Parent       =gpSite;
//    labOcr->Color        =TColor(0x00808040);
//    labOcr->Font->Color  =clWhite;
//    labOcr->Font->Name   ="Arial";
//    labOcr->Font->Size   =8;
    labOcr->Caption      ="";
    labOcr->Tag          =index;
//    labOcr->Align        =alBottom;
//    labOcr->Alignment    =taCenter;
}
//------------------------------------------------------------------------------
void TfRS232Main::ShowVersion()
{
    StatusBar1->Panels->Items[0]->Text = RS232Version;                          //<<重要:請記得更換>>

    MemoVer->Clear();
    MemoVer->Lines->Add("V12.12.847.0 : Last Edit: 2024/10/10");
    MemoVer->Lines->Add("  [1] Support HT-9046AU Handler.");
    MemoVer->Lines->Add("  [2] Support HT-9046CR Handler.");
    MemoVer->Lines->Add("V12.11.843.0 : Last Edit: 2024/09/13");
    MemoVer->Lines->Add("  [1] Add Protection of got BA without CE.");
    MemoVer->Lines->Add("  [2] Add Protection of close site have bin.");
    MemoVer->Lines->Add("V12.08.704.0 : Last Edit: 2021/10/12");
    MemoVer->Lines->Add("  [1] Add command : BARCODE?");
    MemoVer->Lines->Add("  [2] Add command : GET2D?");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.07 : Last Edit: 2014/08/27");
    MemoVer->Lines->Add("  [1] fix RS232 Receive Data error");
    MemoVer->Lines->Add("  [2] Add ReadIntervalTimeout");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.06 : Last Edit: 2014/01/03");
    MemoVer->Lines->Add("  [1] Correct Data Structure from GPIB.");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.05 : Last Edit: 2014/01/03");
    MemoVer->Lines->Add("  [1] Support HT9046LS 32 Site test.");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.04 : Last Edit: 2012/11/08");
    MemoVer->Lines->Add("  [1] If bin is 16~32, set to bin 16 (as error bin).");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.03 : Last Edit: 2012/09/18");
    MemoVer->Lines->Add("  [1] Support HT9045/HT9046 16 site test.");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.02 : Last Edit: 2012/09/18");
    MemoVer->Lines->Add("  [1] Support HT9045/HT9046 4 site test.");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.01 : Last Edit: 2010/06/28");
    MemoVer->Lines->Add("  [1] Support Bin 10~15(A~F).");
    MemoVer->Lines->Add("  ");
    MemoVer->Lines->Add("V1.00 : Last Edit: 2010/06/09");
    MemoVer->Lines->Add("  [1] RS232 Standard.");
    MemoVer->Lines->Add("  ");

    slCmdList=new TStringList();                                                //Steven 20190520 : Add Command Log from Handler

    slCmdList->Add("NONE");
    slCmdList->Add("CatalystSimpleGPIB");
    slCmdList->Add("SwitchArm");
    slCmdList->Add("SwitchArmOK");
    slCmdList->Add("AbortTest");
    slCmdList->Add("AskArmTestMode");       //5
    slCmdList->Add("2ArmTestMode");
    slCmdList->Add("1ArmTestMode");
    slCmdList->Add("NoFullSiteRespon");
    slCmdList->Add("ECHONG Error!");
    slCmdList->Add("DoubleContact");        //10
    slCmdList->Add("TimeOutSkip");
    slCmdList->Add("TimeOutRetryWait");
    slCmdList->Add("TimeOutRetrySend");
    slCmdList->Add("HandlerHomeStart");
    slCmdList->Add("HandlerHomeFinish");    //15
    slCmdList->Add("Arm1Down");
    slCmdList->Add("Arm2Down");
    slCmdList->Add("ContactTestArm1");
    slCmdList->Add("ContactTestArm2");
    slCmdList->Add("ContactTestAbort");     //20
    slCmdList->Add("CloseGpib");
    slCmdList->Add("LotStatus");
    slCmdList->Add("ChangeGpib");
    slCmdList->Add("Version");
    slCmdList->Add("RCMD");                 //25
    slCmdList->Add("SVID");
    slCmdList->Add("ECID");
    slCmdList->Add("RetestFlag");
    slCmdList->Add("CEIDON");
    slCmdList->Add("CEIDOFF");              //30
    slCmdList->Add("EnableBarCode");
    slCmdList->Add("DisableBarCode");
    slCmdList->Add("BarCodeFlowErr");
    slCmdList->Add("MachineState");
    slCmdList->Add("OverDrive");            //35
    slCmdList->Add("ReContact");
    slCmdList->Add("TesterBin");
    slCmdList->Add("SoakTime");
    slCmdList->Add("JamCode");
    slCmdList->Add("SiteMap");              //40
    slCmdList->Add("AllMassTemp");
    slCmdList->Add("BarcodeOFF");
    slCmdList->Add("ART_LOTCLEAR");
    slCmdList->Add("ART_LOTRTCLEAR");
    slCmdList->Add("ART_INPUTQTY");         //45
    slCmdList->Add("ART_LOTSTATUS");
    slCmdList->Add("ART_Alarm");
    slCmdList->Add("ART_QTY");
    slCmdList->Add("ART_INITIAL");
    slCmdList->Add("ART_SRQMASK");          //50
    slCmdList->Add("TesterMode");
    slCmdList->Add("State_Record");
    slCmdList->Add("ART_RunDummy");
    slCmdList->Add("Auto_Clean");
    slCmdList->Add("Pause");                //55
    slCmdList->Add("TempArm");
    slCmdList->Add("TestArm");
    slCmdList->Add("ContactForce");
    slCmdList->Add("ActualTemp");
    slCmdList->Add("Assign");               //60
    slCmdList->Add("StartMode");
    slCmdList->Add("HandlerID");
    slCmdList->Add("HandlerSiteMap");
    slCmdList->Add("HandlerSoakTime");
    slCmdList->Add("HandlerTemperature");   //65
    slCmdList->Add("Force");
    slCmdList->Add("BinMap");
    slCmdList->Add("TestMode");
    slCmdList->Add("GetNowAllTemp");
    slCmdList->Add("ChkSetup");             //70
    slCmdList->Add("GetTestArmPos");
    slCmdList->Add("GetTestArmEP");
    slCmdList->Add("SetTemp");
    slCmdList->Add("SetSoakTime");
    slCmdList->Add("SetTJ");                //75
    slCmdList->Add("SetSiteMapData");
    slCmdList->Add("SetAlarmSetup");
    slCmdList->Add("EnableAMDFunction");
    slCmdList->Add("DisableAMDFunction");
    slCmdList->Add("AMDNextStep1");         //80
    slCmdList->Add("AMDNextStep2");
    slCmdList->Add("HandlerIDRS232");
    slCmdList->Add("GetSiteOnOff");
    slCmdList->Add("GetNumOfSites");
    slCmdList->Add("DeviceMapSRQ");         //85
    slCmdList->Add("PickLoad");
    slCmdList->Add("PlaceLoad");
    slCmdList->Add("TrayFeed");
    slCmdList->Add("TMP?");
    slCmdList->Add("MAP?");                 //90
    slCmdList->Add("SOAK?");
    slCmdList->Add("AMDRS232Connect");
    slCmdList->Add("2DIDFormat");
    slCmdList->Add("State_TTL");
    slCmdList->Add("Command_TTL");          //95
    slCmdList->Add("ESC Function");
    slCmdList->Add("RESUME");
    slCmdList->Add("TestAlarm");
    slCmdList->Add("POWERFOLLOWING");
    slCmdList->Add("SetPID");               //100
    slCmdList->Add("GET_PFC_PARAMETER");                                        //讀取單一溫度的PF參數值
    slCmdList->Add("SET_PFC_PARAMETER");                                        //設定單一溫度的PF參數值
    slCmdList->Add("DoubleContactCount");                                       //Steven 20220517 : 補上
    slCmdList->Add("Set Test Temp");
    slCmdList->Add("CHKSTATUS?");           //105                               //KaiChen 20180910 ：Add GPIB CHKSTATUS?
    slCmdList->Add("ONECYCLE");                                                 //KaiChen 20180910 ：Add GPIB ONECYCLE
    slCmdList->Add("ECHOOK_ONECYCLE");                                          //KaiChen 20181114 ：Add GPIB ECHOOK:ONECYCLE
    slCmdList->Add("GETBINCATEGORY?");                                          //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
    slCmdList->Add("SETUPFILENAME?");                                           //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
    slCmdList->Add("HANDLERID?");           //110                               //KaiChen 20200507 ：Add GPIB HANDLERID?
    slCmdList->Add("SGSETUP_");                                                 //KaiChen 20190613 ：Add GPIB SGSETUP_
    slCmdList->Add("SETSTARTMODE_");                                            //KaiChen 20180910 ：Add GPIB SetStartMode_
    slCmdList->Add("CHECKLIST?");                                               //KaiChen 20190613 ：Add GPIB CHECKLIST?
    slCmdList->Add("BINPOS_");                                                  //KaiChen 20190706 ：Add GPIB BINPOS_
    slCmdList->Add("SGFTP_");               //115                               //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
    slCmdList->Add("SGFTP_STATUS");                                             //Sam 20210329 : Add GPIB SGFTP_STATUS
    slCmdList->Add("NONDOUBLEBIN_");                                            //Sam 20210329 : Add GPIB NONDOUBLEBIN_
    slCmdList->Add("BINCOUNT_");                                                //Sam 20210329 : Add GPIB BINCOUNT_
    slCmdList->Add("SGOSBIN_");                                                 //Sam 20210406 : Add GPIB SGOSBIN_
    slCmdList->Add("SGCONTFAIL_");          //120                               //Sam 20210422 : Add GPIB SGCONTFAIL_
    slCmdList->Add("SETTESTERID_");                                             //Sam 20210617 : Add GPIB SETTESTERID
    slCmdList->Add("SETTESTERID?");                                             //Sam 20210617 : Add GPIB SETTESTERID
    slCmdList->Add("SOFTBIN");                                                  //Steven 20220120 : Amlogic需要收SBIN
    slCmdList->Add("PAUSE_01");                                                 //Steven 20220517 : Add for GIGA
    slCmdList->Add("STOP_01");              //125                               //Steven 20220517 : Add for GIGA
    slCmdList->Add("AUTOCLEAN?");                                               //Sam 20220408 : Novatek 新增 AUTOCLEAN?
    slCmdList->Add("DEVICEFORCEPERPIN?");                                       //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
    slCmdList->Add("ARMCONTACTHIGHVALUE?");                                     //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
    slCmdList->Add("YIELDCONTINUESFAIL?");                                      //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
    slCmdList->Add("YIELDSITEUNBALANCE?");  //130                               //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
    slCmdList->Add("DUTSTATUS?");                                               //Sam 20220408 : Novatek 新增 DUTSTATUS?
    slCmdList->Add("UPH?");                                                     //Sam 20220408 : Novatek 新增 UPH?
    slCmdList->Add("INDEXCYCLETIME?");                                          //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
    slCmdList->Add("GETTEMPOFFSET?");                                           //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
    slCmdList->Add("GETTEMPERATURETOLERANCE?");                                 //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
    slCmdList->Add("VACUUMAIR?");                                               //Sam 20220408 : Novatek 新增 VACUUMAIR?
    slCmdList->Add("SET_ALL?");                                                 //Sam 20220408 : Novatek 新增 SET_ALL?
    slCmdList->Add("HANDLERVERSION?");                                          //Sam 20220408 : Novatek 新增 HANDLERVERSION?
    slCmdList->Add("PPSELECT");                                                 //Richard 20220929 :Add for UTAC 讀檔
    slCmdList->Add("ASKPPSELECT");          //140                               //Richard 20220929 :Add for UTAC 讀檔
    slCmdList->Add("Set Bin Map");
    slCmdList->Add("GETSHUTTLEMODE?");                                          //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
    slCmdList->Add("SETMAXTEST_");                                              //Sam 20230201 : Add GPIB SETMAXTEST_
    slCmdList->Add("GETMAXTEST?");                                              //Sam 20230201 : Add GPIB GETMAXTEST
    slCmdList->Add("SETINITIALMAXTEST_");   //145                               //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
    slCmdList->Add("GETINITIALMAXTEST?");                                       //Sam 20230201 : Add GPIB GETINITIALMAXTEST
    slCmdList->Add("READYNEXTSHOT");                                            //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
    slCmdList->Add("NEXT2DID");                                                 //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
    slCmdList->Add("CloseSiteHaveBin");                                         //Steven 20231017 : GPIB flow error need alarm
    slCmdList->Add("BinonWithout0x41");     //150                               //Steven 20231017 : GPIB flow error need alarm
    slCmdList->Add("BinonWithoutFullsite");                                     //Steven 20231017 : GPIB flow error need alarm
    slCmdList->Add("SETAICCD_");                                                //Sam 20231108 : Add GPIB SETAICCD_
    slCmdList->Add("ASIF_TJ_EFUSED");                                           //Steven 20240903 : for MTK ASIF data
    slCmdList->Add("ASIF_TJ_REQUEST");                                          //Steven 20240903 : for MTK ASIF data
    slCmdList->Add("ASIF_TJ_FB");           //155                               //Steven 20240903 : for MTK ASIF data
    slCmdList->Add("GETAICCD");                                                 //Sam 20240826 : Add GPIB GETAICCD?
    slCmdList->Add("ART Enabled?");                                             //Steven 20241004 : Qorvo check ART enable
    slCmdList->Add("RUN_HANA_ART");                                             //JimmyChiu 20241023 HANA ART Function
    slCmdList->Add("HANA_ART");
    slCmdList->Add("SETOSBIN_");            //160                               //Sam 20250115 : Add GPIB SETOSBIN_
    slCmdList->Add("GETOSBIN?");                                                //Sam 20250115 : Add GPIB GETOSBIN?
    slCmdList->Add("DUTCHK?");                                                  //Steven 20250701 : for DOOSAN TESNA
    slCmdList->Add("GetFFC?");                                                  //Steven 20250701 : for Ampere
    slCmdList->Add("GetTJFunction?");
    slCmdList->Add("GetPowerFollowing?");   //165
    slCmdList->Add("SetSiteOnOff");
    slCmdList->Add("EnableFTPFunction");                                        //Ifor 20231101 add:FTP Function
    slCmdList->Add("DisableFTPFunction");                                       //Ifor 20231101 add:FTP Function
    slCmdList->Add("SetupFileChange");                                          //Ifor 20231101 add:FTP Function
    slCmdList->Add("GetContactCount");      //170                               //Ifor 20240510 add: Get Head Contact Count
    slCmdList->Add("EnablePin1Function");                                       //Ifor 20240528 add:Pin1 Function
    slCmdList->Add("DisablePin1Function");                                      //Ifor 20240528 add:Pin1 Function
    slCmdList->Add("BarcodePin1ON");                                            //Ifor 20240528 add:Pin1 Function
    slCmdList->Add("GetSocketCounter");                                         //Ifor 20250607 add:Get Socket Counter
    slCmdList->Add("GetTIMCounter");        //175                               //Ifor 20250607 add:Get TIM Counter
    slCmdList->Add("FTPDownLoad");                                              //Ifor 20231101 add:FTP Function
    slCmdList->Add("SetSocketToBinR");                                          //Ifor 20210911: Add KLT 要求BINON WITHOUT 0x41 ERROR直接分ErrBin
    slCmdList->Add("MultiZoneTemp");                                            //wei 20240617 Multi Zone Temp
    slCmdList->Add("MultiZoneEnable");                                          //wei 20240617 Multi Zone Temp
    slCmdList->Add("GET_SLOPE_OFFSET");     //180                               //Get the Slope/Offset value            //Eliot 20210412
    slCmdList->Add("SET_SLOPE_OFFSET");                                         //Set the Slope/Offset value            //Eliot 20210412
    slCmdList->Add("READTJ");
    slCmdList->Add("GET_VOLTAGE");
    slCmdList->Add("SET_ATCCONTROLMODE");
    slCmdList->Add("SET_ATC_TEMP");         //185
    slCmdList->Add("RECODETJ");
    slCmdList->Add("QUERYTJ");
    slCmdList->Add("GET_ATCCONTROLMODE");
    slCmdList->Add("GET_ATCERROR");
    slCmdList->Add("RESET_ATCALARM");       //190
    slCmdList->Add("READ_WATER_VALUE");
    slCmdList->Add("SET_WATER_VALUE");
    slCmdList->Add("SET_DYNAMIC_PID");
    slCmdList->Add("READ_DYNAMIC_PID");
    slCmdList->Add("AUTOZSTART");           //195
    slCmdList->Add("AUTOZMOVE");
    slCmdList->Add("AUTOZMOVEOK");
    slCmdList->Add("AUTOZPASS");
    slCmdList->Add("AUTOZREACHLIMIT");
    slCmdList->Add("AUTOZOK");              //200
    slCmdList->Add("READAUTOZLIMIT");
    slCmdList->Add("READZPOS");
    slCmdList->Add("READZTORQUE");

    bShowVersionOK=true;                                                        //Isaac 20210510 : 避免還沒new出來就讀count造成錯誤
}
//---------------------------------------------------------------------------
//  golden MainForm.cpp:296-355 (HMountWnd, bSimulate, ... iRevCycleClear) are defined in Rs232Globals.cpp
//  (extern in Rs232Bridge.h).
//---------------------------------------------------------------------------
TfRS232Main::TfRS232Main()
{
    //AI(W906-GB-P4) 20260926: golden `TfRS232Main(TComponent* Owner) : TForm(Owner)`.  Before the golden body runs,
    //   VCL has (1) zero-filled the instance (TObject::NewInstance) and (2) streamed MainForm.dfm, creating every
    //   __published widget with its design-time values (TCustomForm constructor).  Both are reproduced first, in
    //   that order, after (0) every pointer member is set NULL before anything can throw (engine owner's rule: the
    //   destructor and FormDestroy only ever see NULL or a live object).  The golden body then follows verbatim.
    //   FormCreate / FormShow are NOT called here: Rs232Engine::Start calls ctor -> FormCreate -> FormShow in
    //   golden WinMain order (RS232Standard.cpp).
    //   Note: unlike VCL, C++ does not run ~TfRS232Main when a constructor throws; only operator new / TStringList
    //   allocation can throw below, and nothing here starts a thread or opens a port.

    // ---- (0) every pointer member NULL (Rs232Bridge.h declaration order) ----
    CommTester = NULL; Timer1 = NULL; PageControl1 = NULL; tsVersion = NULL; MemoVer = NULL; StatusBar1 = NULL;
    palSite = NULL; gbSite = NULL; cbBin = NULL; cbSiteOn = NULL; plSite = NULL; tsSPRD = NULL; PageControl5 = NULL;
    TabSheet3 = NULL; Pc_SPRDSite1Control = NULL; ts_SPRDLogSite1 = NULL; mm_SPRDLogSite1 = NULL;
    cb_SPRDShowLogSite1 = NULL; bt_SPRDClearLogSite1 = NULL; cb_SPRDSaveLogSite1 = NULL; ts_SPRDSetupSite1 = NULL;
    Button2 = NULL; GroupBox6 = NULL; Label44 = NULL; Label45 = NULL; Label46 = NULL; Label47 = NULL; Label48 = NULL;
    cb_SPRDBaudSite1 = NULL; cb_SPRDSizeSite1 = NULL; cb_SPRDStopSite1 = NULL; cb_SPRDParitySite1 = NULL;
    cb_SPRDComSite1 = NULL; GroupBox7 = NULL; Label49 = NULL; Label50 = NULL; Label51 = NULL; Label52 = NULL;
    cb_SPRDDTRSite1 = NULL; cb_SPRDRTSSite1 = NULL; cb_SPRDTxCSite1 = NULL; ed_SPRDTimeOutTotalSite1 = NULL;
    ed_SPRDTimeOutSite1 = NULL; TabSheet10 = NULL; Pc_SPRDSite2Control = NULL; ts_SPRDLogSite2 = NULL;
    mm_SPRDLogSite2 = NULL; cb_SPRDShowLogSite2 = NULL; bt_SPRDClearLogSite2 = NULL; cb_SPRDSaveLogSite2 = NULL;
    ts_SPRDSetupSite2 = NULL; Button4 = NULL; GroupBox8 = NULL; Label53 = NULL; Label54 = NULL; Label55 = NULL;
    Label56 = NULL; Label57 = NULL; cb_SPRDBaudSite2 = NULL; cb_SPRDSizeSite2 = NULL; cb_SPRDStopSite2 = NULL;
    cb_SPRDParitySite2 = NULL; cb_SPRDComSite2 = NULL; GroupBox9 = NULL; Label58 = NULL; Label59 = NULL;
    Label60 = NULL; Label61 = NULL; cb_SPRDDTRSite2 = NULL; cb_SPRDRTSSite2 = NULL; cb_SPRDTxCSite2 = NULL;
    ed_SPRDTimeOutTotalSite2 = NULL; ed_SPRDTimeOutSite2 = NULL; TabSheet8 = NULL; Pc_SPRDSite3Control = NULL;
    ts_SPRDLogSite3 = NULL; mm_SPRDLogSite3 = NULL; cb_SPRDShowLogSite3 = NULL; bt_SPRDClearLogSite3 = NULL;
    cb_SPRDSaveLogSite3 = NULL; ts_SPRDSetupSite3 = NULL; Button3 = NULL; GroupBox10 = NULL; Label80 = NULL;
    Label81 = NULL; Label82 = NULL; Label83 = NULL; Label84 = NULL; cb_SPRDBaudSite3 = NULL; cb_SPRDSizeSite3 = NULL;
    cb_SPRDStopSite3 = NULL; cb_SPRDParitySite3 = NULL; cb_SPRDComSite3 = NULL; GroupBox11 = NULL; Label85 = NULL;
    Label86 = NULL; Label87 = NULL; Label88 = NULL; cb_SPRDDTRSite3 = NULL; cb_SPRDRTSSite3 = NULL;
    cb_SPRDTxCSite3 = NULL; ed_SPRDTimeOutTotalSite3 = NULL; ed_SPRDTimeOutSite3 = NULL; TabSheet11 = NULL;
    Pc_SPRDSite4Control = NULL; ts_SPRDLogSite4 = NULL; mm_SPRDLogSite4 = NULL; cb_SPRDShowLogSite4 = NULL;
    bt_SPRDClearLogSite4 = NULL; cb_SPRDSaveLogSite4 = NULL; ts_SPRDSetupSite4 = NULL; Button6 = NULL;
    GroupBox12 = NULL; Label89 = NULL; Label90 = NULL; Label91 = NULL; Label92 = NULL; Label93 = NULL;
    cb_SPRDBaudSite4 = NULL; cb_SPRDSizeSite4 = NULL; cb_SPRDStopSite4 = NULL; cb_SPRDParitySite4 = NULL;
    cb_SPRDComSite4 = NULL; GroupBox13 = NULL; Label94 = NULL; Label95 = NULL; Label96 = NULL; Label97 = NULL;
    cb_SPRDDTRSite4 = NULL; cb_SPRDRTSSite4 = NULL; cb_SPRDTxCSite4 = NULL; ed_SPRDTimeOutTotalSite4 = NULL;
    ed_SPRDTimeOutSite4 = NULL; ts_BinCode = NULL; Label76 = NULL; Label77 = NULL; Label78 = NULL; Label79 = NULL;
    Panel15 = NULL; Label62 = NULL; Label63 = NULL; Label64 = NULL; Label65 = NULL; Label66 = NULL; pn_WSBin11 = NULL;
    pn_WSBin12 = NULL; pn_WSBin13 = NULL; pn_WSBin14 = NULL; pn_WSBin15 = NULL; Panel9 = NULL; Label67 = NULL;
    Label68 = NULL; Label69 = NULL; Label70 = NULL; Label71 = NULL; pn_WSBin6 = NULL; pn_WSBin7 = NULL;
    pn_WSBin8 = NULL; pn_WSBin9 = NULL; pn_WSBin10 = NULL; Panel3 = NULL; lbWS3000Bin1 = NULL; Label72 = NULL;
    Label73 = NULL; Label74 = NULL; Label75 = NULL; pn_WSBin1 = NULL; pn_WSBin2 = NULL; pn_WSBin3 = NULL;
    pn_WSBin4 = NULL; pn_WSBin5 = NULL; cb_SPRDSendSOTString = NULL; Pan_TimeOutBin = NULL; ed_TimeOutTime = NULL;
    bt_ChangeTimeOut = NULL; Pan_NotDefinedtBin = NULL; Button1 = NULL; ts_STD = NULL; PageControl2 = NULL;
    ts_STD_Log = NULL; ts_STD_BinLog = NULL; ts_STD_Setup = NULL; ts_STD_Simu = NULL; MemoLog = NULL;
    MemoBinData = NULL; Panel2 = NULL; cbShowLog = NULL; btClear = NULL; cbSaveLog = NULL; GroupBox1 = NULL;
    Label1 = NULL; Label2 = NULL; Label3 = NULL; Label4 = NULL; Label5 = NULL; cbBaudRate = NULL; cbByteSize = NULL;
    cbStopBit = NULL; cbParity = NULL; cbDevice = NULL; btnUpdate = NULL; btManualTest = NULL; btnAllUse = NULL;
    btnAllNoUse = NULL; ts_AVAGO = NULL; PageControl3 = NULL; TabSheet4 = NULL; PageControl4 = NULL;
    ts_AVAGOLogSite1 = NULL; mm_AvagoLogSite1 = NULL; cb_AvagoShowLogSite1 = NULL; bt_AvagoClearLogSite1 = NULL;
    cb_AvagoSaveLogSite1 = NULL; TabSheet2 = NULL; bt_UpdateSite1 = NULL; GroupBox2 = NULL; Label17 = NULL;
    Label18 = NULL; Label25 = NULL; Label26 = NULL; Label27 = NULL; cb_AvagoBaudSite1 = NULL;
    cb_AvagoSizeSite1 = NULL; cb_AvagoStopSite1 = NULL; cb_AvagoParitySite1 = NULL; cb_AvagoComSite1 = NULL;
    GroupBox3 = NULL; Label28 = NULL; Label29 = NULL; Label30 = NULL; Label31 = NULL; cb_AvagoDTRSite1 = NULL;
    cb_AvagoRTSSite1 = NULL; cb_AvagoTxCSite1 = NULL; ed_AvagoTimeOutTotalSite1 = NULL; ed_AvagoTimeOutSite1 = NULL;
    TabSheet5 = NULL; PageControl6 = NULL; TabSheet6 = NULL; mm_AvagoLogSite2 = NULL; cb_AvagoShowLogSite2 = NULL;
    bt_AvagoClearLogSite2 = NULL; cb_AvagoSaveLogSite2 = NULL; TabSheet7 = NULL; bt_UpdateSite2 = NULL;
    GroupBox4 = NULL; Label32 = NULL; Label33 = NULL; Label34 = NULL; Label35 = NULL; Label36 = NULL;
    cb_AvagoBaudSite2 = NULL; cb_AvagoSizeSite2 = NULL; cb_AvagoStopSite2 = NULL; cb_AvagoParitySite2 = NULL;
    cb_AvagoComSite2 = NULL; GroupBox5 = NULL; Label38 = NULL; Label39 = NULL; Label40 = NULL; Label41 = NULL;
    cb_AvagoDTRSite2 = NULL; cb_AvagoRTSSite2 = NULL; cb_AvagoTxCSite2 = NULL; ed_AvagoTimeOutTotalSite2 = NULL;
    ed_AvagoTimeOutSite2 = NULL; TabSheet1 = NULL; Label42 = NULL; Label43 = NULL; bt_SimulateManualTestSite2 = NULL;
    ed_SimulateNu = NULL; ed_Number = NULL; GroupBox14 = NULL; Label6 = NULL; edReadIntervalTimeout = NULL;
    labDebugMode = NULL; tsTTL = NULL; PageControl7 = NULL; ts_TTL_BinLog = NULL; MemoBinData_TTL = NULL;
    Panel1 = NULL; cbShowLog_TTL = NULL; btClear_TTL = NULL; cbSaveLog_TTL = NULL; ts_TTL_Setup = NULL;
    GroupBox15 = NULL; Label7 = NULL; Label8 = NULL; Label9 = NULL; Label10 = NULL; Label11 = NULL;
    cbBaudRate_TTL = NULL; cbByteSize_TTL = NULL; cbStopBit_TTL = NULL; cbParity_TTL = NULL; cbDevice_TTL = NULL;
    btnUpdate_TTL = NULL; GroupBox16 = NULL; Label12 = NULL; edReadIntervalTimeout_TTL = NULL; ts_TTL_Simu = NULL;
    Button8 = NULL; Button9 = NULL; btnTTL_Manual = NULL; btnClearSot = NULL; tsLog = NULL; CommTester_TTL = NULL;
    CommTester_TTL_2 = NULL; Label13 = NULL; cbDevice_TTL_2 = NULL; Button10 = NULL; Button11 = NULL; labOcr = NULL;
    btnEnableSOTCSOT = NULL; slCmdList = NULL; uServer = NULL; slRS232Log = NULL;

    // ---- (1) zero-filled instance: plain members ----
    iMaxBinCount           = 0;
    CustomerCode           = 0;
    bCommConnect[0]        = false;
    bCommConnect[1]        = false;
    bCommConnect[2]        = false;
    bFind                  = false;
    dtPresent              = 0.0;
    OldSystemSec           = 0;
    bTwoArmTestMode        = false;
    iTwoArmTestStep        = 0;
    iHasCE                 = 0;
    iCheckClosedSiteHasBin = 0;
    closeRequested         = false;     // V906 plumbing
    mailbox                = NULL;      // V906 plumbing: set by Rs232Engine::Start right after the ctor
    //   AnsiString / std::vector members (Caption, ComNameTester, ComNameTTL, ComNameTTL_2, closeReason, lastNotice,
    //   ReceiveData ... SendStartData1, MY_DUT_PAL, rxQueue) start empty, as a zero-filled VCL instance does.

    // ---- (2) MainForm.dfm ----
    //   Only values the stand-ins can hold are applied.  Geometry, Align, Alignment, AutoSize, Font, BevelInner /
    //   BevelOuter, ParentFont, TabOrder, TabWidth, Style (tsButtons), ReadOnly, ImeMode / ImeName, ItemHeight and
    //   ImageIndex are layout only (the web page draws them).  Event wiring (OnClick = ...) is noted per widget:
    //   the stand-ins have no event slots, the engine / web command path calls the golden handler directly.
    //   VCL defaults that differ from the stand-ins: TComboBox ItemIndex = -1 (none of the .dfm combos stores
    //   one); TTabSheet TabVisible = true; TPageControl ActivePageIndex = the page index of the stored ActivePage;
    //   TPanel / TForm Color clBtnFace is not applied (golden never reads ->Color; only stored Colors are set).
    //   The `static const char* const items[]` tables below are constant data, not golden statics (rule 13: no
    //   re-arm needed).
    Caption = "RS232 Standard";                                    // MainForm.dfm:6 (the golden body overwrites it)

    labDebugMode = VclNew<TLabel>();                                // MainForm.dfm:47
    labDebugMode->Caption = "!! Debug Mode !!";

    PageControl1 = VclNew<TPageControl>();                          // MainForm.dfm:62
    PageControl1->ActivePageIndex = 0;                              //   ActivePage = ts_STD (page 0; .dfm TabIndex = 0)

    ts_STD = VclNew<TTabSheet>();                                   // MainForm.dfm:79
    ts_STD->Caption = "Standard";
    ts_STD->TabVisible = true;

    PageControl2 = VclNew<TPageControl>();                          // MainForm.dfm:82
    PageControl2->ActivePageIndex = 3;                              //   ActivePage = ts_STD_Simu (page 3; .dfm TabIndex = 3)

    ts_STD_Log = VclNew<TTabSheet>();                               // MainForm.dfm:92
    ts_STD_Log->Caption = "COM Log";
    ts_STD_Log->TabVisible = true;

    ts_STD_BinLog = VclNew<TTabSheet>();                            // MainForm.dfm:95
    ts_STD_BinLog->Caption = "Bin Log";
    ts_STD_BinLog->TabVisible = true;

    MemoBinData = VclNew<TMemo>();                                  // MainForm.dfm:98

    Panel2 = VclNew<TPanel>();                                      // MainForm.dfm:106

    cbShowLog = VclNew<TCheckBox>();                                // MainForm.dfm:114
    cbShowLog->Caption = "Show Log";

    btClear = VclNew<TButton>();                                    // MainForm.dfm:128 (OnClick = btClearClick)
    btClear->Caption = "CLEAR ";

    cbSaveLog = VclNew<TCheckBox>();                                // MainForm.dfm:137
    cbSaveLog->Caption = "Save Log";

    ts_STD_Setup = VclNew<TTabSheet>();                             // MainForm.dfm:153
    ts_STD_Setup->Caption = "Setup";
    ts_STD_Setup->TabVisible = true;

    GroupBox1 = VclNew<TGroupBox>();                                // MainForm.dfm:156
    GroupBox1->Caption = "COM Port Setting";

    Label1 = VclNew<TLabel>();                                      // MainForm.dfm:164
    Label1->Caption = "Baud Rate：";

    Label2 = VclNew<TLabel>();                                      // MainForm.dfm:179
    Label2->Caption = "Byte Size：";

    Label3 = VclNew<TLabel>();                                      // MainForm.dfm:194
    Label3->Caption = "Stop Bit：";

    Label4 = VclNew<TLabel>();                                      // MainForm.dfm:209
    Label4->Caption = "Parity：";

    Label5 = VclNew<TLabel>();                                      // MainForm.dfm:224
    Label5->Caption = "Device：";

    cbBaudRate = VclNew<TComboBox>();                               // MainForm.dfm:239
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "115200", NULL };
        DfmItems(cbBaudRate->Items, items);
    }
    cbBaudRate->ItemIndex = -1;                                     //   no ItemIndex stored -> VCL -1
    cbBaudRate->Text      = "cbBaudRate";

    cbByteSize = VclNew<TComboBox>();                               // MainForm.dfm:261
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cbByteSize->Items, items);
    }
    cbByteSize->ItemIndex = -1;                                     //   no ItemIndex stored -> VCL -1
    cbByteSize->Text      = "cbByteSize";

    cbStopBit = VclNew<TComboBox>();                                // MainForm.dfm:281
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cbStopBit->Items, items);
    }
    cbStopBit->ItemIndex = -1;                                      //   no ItemIndex stored -> VCL -1
    cbStopBit->Text      = "cbStopBit";

    cbParity = VclNew<TComboBox>();                                 // MainForm.dfm:300
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cbParity->Items, items);
    }
    cbParity->ItemIndex = -1;                                       //   no ItemIndex stored -> VCL -1
    cbParity->Text      = "cbParity";

    cbDevice = VclNew<TComboBox>();                                 // MainForm.dfm:321
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", "COM10", "COM11", "COM12", "COM13", "COM14", "COM15", "COM16",
                                             "COM17", "COM18", NULL };
        DfmItems(cbDevice->Items, items);
    }
    cbDevice->ItemIndex = -1;                                       //   no ItemIndex stored -> VCL -1
    cbDevice->Text      = "cbCOM";

    btnUpdate = VclNew<TPanel>();                                   // MainForm.dfm:356 (OnClick = btnUpdateClick, OnMouseDown = btnUpdateMouseDown, OnMouseUp = btnUpdateMouseUp)
    btnUpdate->Caption = "Update";

    GroupBox14 = VclNew<TGroupBox>();                               // MainForm.dfm:375
    GroupBox14->Caption = "Detail Settng";

    Label6 = VclNew<TLabel>();                                      // MainForm.dfm:383
    Label6->Caption = "ReadIntervalTimeout ：";

    edReadIntervalTimeout = VclNew<TEdit>();                        // MainForm.dfm:398
    edReadIntervalTimeout->Text = "50";

    ts_STD_Simu = VclNew<TTabSheet>();                              // MainForm.dfm:408
    ts_STD_Simu->Caption = "Simulate";
    ts_STD_Simu->TabVisible = true;

    btManualTest = VclNew<TButton>();                               // MainForm.dfm:411 (OnClick = btManualTestClick)
    btManualTest->Caption = "Manual Test";

    btnAllUse = VclNew<TButton>();                                  // MainForm.dfm:420 (OnClick = btnAllUseClick)
    btnAllUse->Caption = "All Use";

    btnAllNoUse = VclNew<TButton>();                                // MainForm.dfm:429 (OnClick = btnAllNoUseClick)
    btnAllNoUse->Caption = "All No Use";

    ts_AVAGO = VclNew<TTabSheet>();                                 // MainForm.dfm:441
    ts_AVAGO->Caption = "AVAGO";
    ts_AVAGO->TabVisible = true;

    PageControl3 = VclNew<TPageControl>();                          // MainForm.dfm:444
    PageControl3->ActivePageIndex = 0;                              //   ActivePage = TabSheet4 (page 0; .dfm TabIndex = 0)

    TabSheet4 = VclNew<TTabSheet>();                                // MainForm.dfm:455
    TabSheet4->Caption = "Site1";
    TabSheet4->TabVisible = true;

    PageControl4 = VclNew<TPageControl>();                          // MainForm.dfm:457
    PageControl4->ActivePageIndex = 0;                              //   ActivePage = ts_AVAGOLogSite1 (page 0; .dfm TabIndex = 0)

    ts_AVAGOLogSite1 = VclNew<TTabSheet>();                         // MainForm.dfm:473
    ts_AVAGOLogSite1->Caption = "Log";
    ts_AVAGOLogSite1->TabVisible = true;

    mm_AvagoLogSite1 = VclNew<TMemo>();                             // MainForm.dfm:475 (ReadOnly = True: no stand-in member)

    cb_AvagoShowLogSite1 = VclNew<TCheckBox>();                     // MainForm.dfm:491
    cb_AvagoShowLogSite1->Caption = "Show Log (Site1 )";

    bt_AvagoClearLogSite1 = VclNew<TButton>();                      // MainForm.dfm:505
    bt_AvagoClearLogSite1->Caption = "CLEAR ";

    cb_AvagoSaveLogSite1 = VclNew<TCheckBox>();                     // MainForm.dfm:513
    cb_AvagoSaveLogSite1->Caption = "Save Log (Site1 ) ";

    TabSheet2 = VclNew<TTabSheet>();                                // MainForm.dfm:528
    TabSheet2->Caption = "Setup";
    TabSheet2->TabVisible = true;

    bt_UpdateSite1 = VclNew<TButton>();                             // MainForm.dfm:531
    bt_UpdateSite1->Caption = "Update";

    GroupBox2 = VclNew<TGroupBox>();                                // MainForm.dfm:545
    GroupBox2->Caption = "COM Port Setting";

    Label17 = VclNew<TLabel>();                                     // MainForm.dfm:552
    Label17->Caption = "Baud Rate：";

    Label18 = VclNew<TLabel>();                                     // MainForm.dfm:567
    Label18->Caption = "Byte Size：";

    Label25 = VclNew<TLabel>();                                     // MainForm.dfm:582
    Label25->Caption = "Stop Bit：";

    Label26 = VclNew<TLabel>();                                     // MainForm.dfm:597
    Label26->Caption = "Parity：";

    Label27 = VclNew<TLabel>();                                     // MainForm.dfm:612
    Label27->Caption = "Device：";

    cb_AvagoBaudSite1 = VclNew<TComboBox>();                        // MainForm.dfm:627
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", NULL };
        DfmItems(cb_AvagoBaudSite1->Items, items);
    }
    cb_AvagoBaudSite1->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoBaudSite1->Text      = "cbBaudRate";

    cb_AvagoSizeSite1 = VclNew<TComboBox>();                        // MainForm.dfm:648
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_AvagoSizeSite1->Items, items);
    }
    cb_AvagoSizeSite1->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoSizeSite1->Text      = "cbByteSize";

    cb_AvagoStopSite1 = VclNew<TComboBox>();                        // MainForm.dfm:668
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_AvagoStopSite1->Items, items);
    }
    cb_AvagoStopSite1->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoStopSite1->Text      = "cbStopBit";

    cb_AvagoParitySite1 = VclNew<TComboBox>();                      // MainForm.dfm:687
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_AvagoParitySite1->Items, items);
    }
    cb_AvagoParitySite1->ItemIndex = -1;                            //   no ItemIndex stored -> VCL -1
    cb_AvagoParitySite1->Text      = "cbParity";

    cb_AvagoComSite1 = VclNew<TComboBox>();                         // MainForm.dfm:708
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_AvagoComSite1->Items, items);
    }
    cb_AvagoComSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoComSite1->Text      = "cbCOM";

    GroupBox3 = VclNew<TGroupBox>();                                // MainForm.dfm:734
    GroupBox3->Caption = "Detail Setting";

    Label28 = VclNew<TLabel>();                                     // MainForm.dfm:741
    Label28->Caption = "DTR Ctrl：";

    Label29 = VclNew<TLabel>();                                     // MainForm.dfm:755
    Label29->Caption = "RTS Ctrll：";

    Label30 = VclNew<TLabel>();                                     // MainForm.dfm:769
    Label30->Caption = "ReadIntervalTimeout：";

    Label31 = VclNew<TLabel>();                                     // MainForm.dfm:782
    Label31->Caption = "WriteTotalTimeoutMultiplier：";

    cb_AvagoDTRSite1 = VclNew<TComboBox>();                         // MainForm.dfm:795
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_AvagoDTRSite1->Items, items);
    }
    cb_AvagoDTRSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoDTRSite1->Text      = "DTR";

    cb_AvagoRTSSite1 = VclNew<TComboBox>();                         // MainForm.dfm:814
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_AvagoRTSSite1->Items, items);
    }
    cb_AvagoRTSSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoRTSSite1->Text      = "RTS";

    cb_AvagoTxCSite1 = VclNew<TCheckBox>();                         // MainForm.dfm:834
    cb_AvagoTxCSite1->Caption = "TxContinueOnXoff";

    ed_AvagoTimeOutTotalSite1 = VclNew<TEdit>();                    // MainForm.dfm:842

    ed_AvagoTimeOutSite1 = VclNew<TEdit>();                         // MainForm.dfm:849

    TabSheet5 = VclNew<TTabSheet>();                                // MainForm.dfm:860
    TabSheet5->Caption = "Site2";
    TabSheet5->TabVisible = true;

    PageControl6 = VclNew<TPageControl>();                          // MainForm.dfm:863
    PageControl6->ActivePageIndex = 0;                              //   ActivePage = TabSheet6 (page 0; .dfm TabIndex = 0)

    TabSheet6 = VclNew<TTabSheet>();                                // MainForm.dfm:879
    TabSheet6->Caption = "Log";
    TabSheet6->TabVisible = true;

    mm_AvagoLogSite2 = VclNew<TMemo>();                             // MainForm.dfm:881 (ReadOnly = True: no stand-in member)

    cb_AvagoShowLogSite2 = VclNew<TCheckBox>();                     // MainForm.dfm:897
    cb_AvagoShowLogSite2->Caption = "Show Log (Site2 )";

    bt_AvagoClearLogSite2 = VclNew<TButton>();                      // MainForm.dfm:911
    bt_AvagoClearLogSite2->Caption = "CLEAR ";

    cb_AvagoSaveLogSite2 = VclNew<TCheckBox>();                     // MainForm.dfm:919
    cb_AvagoSaveLogSite2->Caption = "Save Log (Site2 )";

    TabSheet7 = VclNew<TTabSheet>();                                // MainForm.dfm:934
    TabSheet7->Caption = "Setup";
    TabSheet7->TabVisible = true;

    bt_UpdateSite2 = VclNew<TButton>();                             // MainForm.dfm:937
    bt_UpdateSite2->Caption = "Update";

    GroupBox4 = VclNew<TGroupBox>();                                // MainForm.dfm:951
    GroupBox4->Caption = "COM Port Setting";

    Label32 = VclNew<TLabel>();                                     // MainForm.dfm:958
    Label32->Caption = "Baud Rate：";

    Label33 = VclNew<TLabel>();                                     // MainForm.dfm:973
    Label33->Caption = "Byte Size：";

    Label34 = VclNew<TLabel>();                                     // MainForm.dfm:988
    Label34->Caption = "Stop Bit：";

    Label35 = VclNew<TLabel>();                                     // MainForm.dfm:1003
    Label35->Caption = "Parity：";

    Label36 = VclNew<TLabel>();                                     // MainForm.dfm:1018
    Label36->Caption = "Device：";

    cb_AvagoBaudSite2 = VclNew<TComboBox>();                        // MainForm.dfm:1033
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", NULL };
        DfmItems(cb_AvagoBaudSite2->Items, items);
    }
    cb_AvagoBaudSite2->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoBaudSite2->Text      = "cbBaudRate";

    cb_AvagoSizeSite2 = VclNew<TComboBox>();                        // MainForm.dfm:1054
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_AvagoSizeSite2->Items, items);
    }
    cb_AvagoSizeSite2->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoSizeSite2->Text      = "cbByteSize";

    cb_AvagoStopSite2 = VclNew<TComboBox>();                        // MainForm.dfm:1074
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_AvagoStopSite2->Items, items);
    }
    cb_AvagoStopSite2->ItemIndex = -1;                              //   no ItemIndex stored -> VCL -1
    cb_AvagoStopSite2->Text      = "cbStopBit";

    cb_AvagoParitySite2 = VclNew<TComboBox>();                      // MainForm.dfm:1093
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_AvagoParitySite2->Items, items);
    }
    cb_AvagoParitySite2->ItemIndex = -1;                            //   no ItemIndex stored -> VCL -1
    cb_AvagoParitySite2->Text      = "cbParity";

    cb_AvagoComSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1114
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_AvagoComSite2->Items, items);
    }
    cb_AvagoComSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoComSite2->Text      = "cbCOM";

    GroupBox5 = VclNew<TGroupBox>();                                // MainForm.dfm:1140
    GroupBox5->Caption = "Detail Setting";

    Label38 = VclNew<TLabel>();                                     // MainForm.dfm:1147
    Label38->Caption = "DTR Ctrl：";

    Label39 = VclNew<TLabel>();                                     // MainForm.dfm:1161
    Label39->Caption = "RTS Ctrll：";

    Label40 = VclNew<TLabel>();                                     // MainForm.dfm:1175
    Label40->Caption = "ReadIntervalTimeout：";

    Label41 = VclNew<TLabel>();                                     // MainForm.dfm:1188
    Label41->Caption = "WriteTotalTimeoutMultiplier：";

    cb_AvagoDTRSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1201
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_AvagoDTRSite2->Items, items);
    }
    cb_AvagoDTRSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoDTRSite2->Text      = "DTR";

    cb_AvagoRTSSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1220
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_AvagoRTSSite2->Items, items);
    }
    cb_AvagoRTSSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_AvagoRTSSite2->Text      = "RTS";

    cb_AvagoTxCSite2 = VclNew<TCheckBox>();                         // MainForm.dfm:1240
    cb_AvagoTxCSite2->Caption = "TxContinueOnXoff";

    ed_AvagoTimeOutTotalSite2 = VclNew<TEdit>();                    // MainForm.dfm:1248

    ed_AvagoTimeOutSite2 = VclNew<TEdit>();                         // MainForm.dfm:1255

    TabSheet1 = VclNew<TTabSheet>();                                // MainForm.dfm:1266
    TabSheet1->Caption = "Simulate";
    TabSheet1->TabVisible = true;

    Label42 = VclNew<TLabel>();                                     // MainForm.dfm:1269
    Label42->Caption = "Simulate Device Number";

    Label43 = VclNew<TLabel>();                                     // MainForm.dfm:1276
    Label43->Caption = "Number";

    bt_SimulateManualTestSite2 = VclNew<TButton>();                 // MainForm.dfm:1283
    bt_SimulateManualTestSite2->Caption = "Manual Test";

    ed_SimulateNu = VclNew<TEdit>();                                // MainForm.dfm:1291
    ed_SimulateNu->Text = "ABCD";

    ed_Number = VclNew<TEdit>();                                    // MainForm.dfm:1299
    ed_Number->Text = "0";

    tsSPRD = VclNew<TTabSheet>();                                   // MainForm.dfm:1310
    tsSPRD->Caption = "SPRD";
    tsSPRD->TabVisible = true;

    PageControl5 = VclNew<TPageControl>();                          // MainForm.dfm:1313
    PageControl5->ActivePageIndex = 0;                              //   ActivePage = TabSheet3 (page 0; .dfm TabIndex = 0)

    TabSheet3 = VclNew<TTabSheet>();                                // MainForm.dfm:1323
    TabSheet3->Caption = "Site1";
    TabSheet3->TabVisible = true;

    Pc_SPRDSite1Control = VclNew<TPageControl>();                   // MainForm.dfm:1325
    Pc_SPRDSite1Control->ActivePageIndex = 1;                       //   ActivePage = ts_SPRDSetupSite1 (page 1; .dfm TabIndex = 1)

    ts_SPRDLogSite1 = VclNew<TTabSheet>();                          // MainForm.dfm:1341
    ts_SPRDLogSite1->Caption = "Log";
    ts_SPRDLogSite1->TabVisible = true;

    mm_SPRDLogSite1 = VclNew<TMemo>();                              // MainForm.dfm:1343 (ReadOnly = True: no stand-in member)

    cb_SPRDShowLogSite1 = VclNew<TCheckBox>();                      // MainForm.dfm:1359
    cb_SPRDShowLogSite1->Caption = "Show Log (Site1 )";

    bt_SPRDClearLogSite1 = VclNew<TButton>();                       // MainForm.dfm:1373
    bt_SPRDClearLogSite1->Caption = "CLEAR ";

    cb_SPRDSaveLogSite1 = VclNew<TCheckBox>();                      // MainForm.dfm:1381
    cb_SPRDSaveLogSite1->Caption = "Save Log (Site1 ) ";

    ts_SPRDSetupSite1 = VclNew<TTabSheet>();                        // MainForm.dfm:1396
    ts_SPRDSetupSite1->Caption = "Setup";
    ts_SPRDSetupSite1->TabVisible = true;

    Button2 = VclNew<TButton>();                                    // MainForm.dfm:1399
    Button2->Caption = "Update";

    GroupBox6 = VclNew<TGroupBox>();                                // MainForm.dfm:1413
    GroupBox6->Caption = "COM Port Setting";

    Label44 = VclNew<TLabel>();                                     // MainForm.dfm:1420
    Label44->Caption = "Baud Rate：";

    Label45 = VclNew<TLabel>();                                     // MainForm.dfm:1435
    Label45->Caption = "Byte Size：";

    Label46 = VclNew<TLabel>();                                     // MainForm.dfm:1450
    Label46->Caption = "Stop Bit：";

    Label47 = VclNew<TLabel>();                                     // MainForm.dfm:1465
    Label47->Caption = "Parity：";

    Label48 = VclNew<TLabel>();                                     // MainForm.dfm:1480
    Label48->Caption = "Device：";

    cb_SPRDBaudSite1 = VclNew<TComboBox>();                         // MainForm.dfm:1495
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "38400", "115200", NULL };
        DfmItems(cb_SPRDBaudSite1->Items, items);
    }
    cb_SPRDBaudSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDBaudSite1->Text      = "cbBaudRate";

    cb_SPRDSizeSite1 = VclNew<TComboBox>();                         // MainForm.dfm:1518
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_SPRDSizeSite1->Items, items);
    }
    cb_SPRDSizeSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDSizeSite1->Text      = "cbByteSize";

    cb_SPRDStopSite1 = VclNew<TComboBox>();                         // MainForm.dfm:1538
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_SPRDStopSite1->Items, items);
    }
    cb_SPRDStopSite1->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDStopSite1->Text      = "cbStopBit";

    cb_SPRDParitySite1 = VclNew<TComboBox>();                       // MainForm.dfm:1557
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_SPRDParitySite1->Items, items);
    }
    cb_SPRDParitySite1->ItemIndex = -1;                             //   no ItemIndex stored -> VCL -1
    cb_SPRDParitySite1->Text      = "cbParity";

    cb_SPRDComSite1 = VclNew<TComboBox>();                          // MainForm.dfm:1578
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_SPRDComSite1->Items, items);
    }
    cb_SPRDComSite1->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDComSite1->Text      = "cbCOM";

    GroupBox7 = VclNew<TGroupBox>();                                // MainForm.dfm:1604
    GroupBox7->Caption = "Detail Setting";

    Label49 = VclNew<TLabel>();                                     // MainForm.dfm:1611
    Label49->Caption = "DTR Ctrl：";

    Label50 = VclNew<TLabel>();                                     // MainForm.dfm:1625
    Label50->Caption = "RTS Ctrll：";

    Label51 = VclNew<TLabel>();                                     // MainForm.dfm:1639
    Label51->Caption = "ReadIntervalTimeout：";

    Label52 = VclNew<TLabel>();                                     // MainForm.dfm:1652
    Label52->Caption = "WriteTotalTimeoutMultiplier：";

    cb_SPRDDTRSite1 = VclNew<TComboBox>();                          // MainForm.dfm:1665
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_SPRDDTRSite1->Items, items);
    }
    cb_SPRDDTRSite1->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDDTRSite1->Text      = "DTR";

    cb_SPRDRTSSite1 = VclNew<TComboBox>();                          // MainForm.dfm:1684
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_SPRDRTSSite1->Items, items);
    }
    cb_SPRDRTSSite1->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDRTSSite1->Text      = "RTS";

    cb_SPRDTxCSite1 = VclNew<TCheckBox>();                          // MainForm.dfm:1704
    cb_SPRDTxCSite1->Caption = "TxContinueOnXoff";

    ed_SPRDTimeOutTotalSite1 = VclNew<TEdit>();                     // MainForm.dfm:1712

    ed_SPRDTimeOutSite1 = VclNew<TEdit>();                          // MainForm.dfm:1719

    TabSheet10 = VclNew<TTabSheet>();                               // MainForm.dfm:1730
    TabSheet10->Caption = "Site2";
    TabSheet10->TabVisible = true;

    Pc_SPRDSite2Control = VclNew<TPageControl>();                   // MainForm.dfm:1733
    Pc_SPRDSite2Control->ActivePageIndex = 0;                       //   ActivePage = ts_SPRDLogSite2 (page 0; .dfm TabIndex = 0)

    ts_SPRDLogSite2 = VclNew<TTabSheet>();                          // MainForm.dfm:1749
    ts_SPRDLogSite2->Caption = "Log";
    ts_SPRDLogSite2->TabVisible = true;

    mm_SPRDLogSite2 = VclNew<TMemo>();                              // MainForm.dfm:1751 (ReadOnly = True: no stand-in member)

    cb_SPRDShowLogSite2 = VclNew<TCheckBox>();                      // MainForm.dfm:1767
    cb_SPRDShowLogSite2->Caption = "Show Log (Site2 )";

    bt_SPRDClearLogSite2 = VclNew<TButton>();                       // MainForm.dfm:1781
    bt_SPRDClearLogSite2->Caption = "CLEAR ";

    cb_SPRDSaveLogSite2 = VclNew<TCheckBox>();                      // MainForm.dfm:1789
    cb_SPRDSaveLogSite2->Caption = "Save Log (Site2 )";

    ts_SPRDSetupSite2 = VclNew<TTabSheet>();                        // MainForm.dfm:1804
    ts_SPRDSetupSite2->Caption = "Setup";
    ts_SPRDSetupSite2->TabVisible = true;

    Button4 = VclNew<TButton>();                                    // MainForm.dfm:1807
    Button4->Caption = "Update";

    GroupBox8 = VclNew<TGroupBox>();                                // MainForm.dfm:1821
    GroupBox8->Caption = "COM Port Setting";

    Label53 = VclNew<TLabel>();                                     // MainForm.dfm:1828
    Label53->Caption = "Baud Rate：";

    Label54 = VclNew<TLabel>();                                     // MainForm.dfm:1843
    Label54->Caption = "Byte Size：";

    Label55 = VclNew<TLabel>();                                     // MainForm.dfm:1858
    Label55->Caption = "Stop Bit：";

    Label56 = VclNew<TLabel>();                                     // MainForm.dfm:1873
    Label56->Caption = "Parity：";

    Label57 = VclNew<TLabel>();                                     // MainForm.dfm:1888
    Label57->Caption = "Device：";

    cb_SPRDBaudSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1903
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "38400", "115200", NULL };
        DfmItems(cb_SPRDBaudSite2->Items, items);
    }
    cb_SPRDBaudSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDBaudSite2->Text      = "cbBaudRate";

    cb_SPRDSizeSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1926
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_SPRDSizeSite2->Items, items);
    }
    cb_SPRDSizeSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDSizeSite2->Text      = "cbByteSize";

    cb_SPRDStopSite2 = VclNew<TComboBox>();                         // MainForm.dfm:1946
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_SPRDStopSite2->Items, items);
    }
    cb_SPRDStopSite2->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDStopSite2->Text      = "cbStopBit";

    cb_SPRDParitySite2 = VclNew<TComboBox>();                       // MainForm.dfm:1965
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_SPRDParitySite2->Items, items);
    }
    cb_SPRDParitySite2->ItemIndex = -1;                             //   no ItemIndex stored -> VCL -1
    cb_SPRDParitySite2->Text      = "cbParity";

    cb_SPRDComSite2 = VclNew<TComboBox>();                          // MainForm.dfm:1986
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_SPRDComSite2->Items, items);
    }
    cb_SPRDComSite2->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDComSite2->Text      = "cbCOM";

    GroupBox9 = VclNew<TGroupBox>();                                // MainForm.dfm:2012
    GroupBox9->Caption = "Detail Setting";

    Label58 = VclNew<TLabel>();                                     // MainForm.dfm:2019
    Label58->Caption = "DTR Ctrl：";

    Label59 = VclNew<TLabel>();                                     // MainForm.dfm:2033
    Label59->Caption = "RTS Ctrll：";

    Label60 = VclNew<TLabel>();                                     // MainForm.dfm:2047
    Label60->Caption = "ReadIntervalTimeout：";

    Label61 = VclNew<TLabel>();                                     // MainForm.dfm:2060
    Label61->Caption = "WriteTotalTimeoutMultiplier：";

    cb_SPRDDTRSite2 = VclNew<TComboBox>();                          // MainForm.dfm:2073
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_SPRDDTRSite2->Items, items);
    }
    cb_SPRDDTRSite2->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDDTRSite2->Text      = "DTR";

    cb_SPRDRTSSite2 = VclNew<TComboBox>();                          // MainForm.dfm:2092
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_SPRDRTSSite2->Items, items);
    }
    cb_SPRDRTSSite2->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDRTSSite2->Text      = "RTS";

    cb_SPRDTxCSite2 = VclNew<TCheckBox>();                          // MainForm.dfm:2112
    cb_SPRDTxCSite2->Caption = "TxContinueOnXoff";

    ed_SPRDTimeOutTotalSite2 = VclNew<TEdit>();                     // MainForm.dfm:2120

    ed_SPRDTimeOutSite2 = VclNew<TEdit>();                          // MainForm.dfm:2127

    TabSheet8 = VclNew<TTabSheet>();                                // MainForm.dfm:2138
    TabSheet8->Caption = "Site3";
    TabSheet8->TabVisible = true;

    Pc_SPRDSite3Control = VclNew<TPageControl>();                   // MainForm.dfm:2141
    Pc_SPRDSite3Control->ActivePageIndex = 0;                       //   ActivePage = ts_SPRDLogSite3 (page 0; .dfm TabIndex = 0)

    ts_SPRDLogSite3 = VclNew<TTabSheet>();                          // MainForm.dfm:2157
    ts_SPRDLogSite3->Caption = "Log";
    ts_SPRDLogSite3->TabVisible = true;

    mm_SPRDLogSite3 = VclNew<TMemo>();                              // MainForm.dfm:2159 (ReadOnly = True: no stand-in member)

    cb_SPRDShowLogSite3 = VclNew<TCheckBox>();                      // MainForm.dfm:2175
    cb_SPRDShowLogSite3->Caption = "Show Log (Site3 )";

    bt_SPRDClearLogSite3 = VclNew<TButton>();                       // MainForm.dfm:2189
    bt_SPRDClearLogSite3->Caption = "CLEAR ";

    cb_SPRDSaveLogSite3 = VclNew<TCheckBox>();                      // MainForm.dfm:2197
    cb_SPRDSaveLogSite3->Caption = "Save Log (Site3 )";

    ts_SPRDSetupSite3 = VclNew<TTabSheet>();                        // MainForm.dfm:2212
    ts_SPRDSetupSite3->Caption = "Setup";
    ts_SPRDSetupSite3->TabVisible = true;

    Button3 = VclNew<TButton>();                                    // MainForm.dfm:2215
    Button3->Caption = "Update";

    GroupBox10 = VclNew<TGroupBox>();                               // MainForm.dfm:2229
    GroupBox10->Caption = "COM Port Setting";

    Label80 = VclNew<TLabel>();                                     // MainForm.dfm:2236
    Label80->Caption = "Baud Rate：";

    Label81 = VclNew<TLabel>();                                     // MainForm.dfm:2251
    Label81->Caption = "Byte Size：";

    Label82 = VclNew<TLabel>();                                     // MainForm.dfm:2266
    Label82->Caption = "Stop Bit：";

    Label83 = VclNew<TLabel>();                                     // MainForm.dfm:2281
    Label83->Caption = "Parity：";

    Label84 = VclNew<TLabel>();                                     // MainForm.dfm:2296
    Label84->Caption = "Device：";

    cb_SPRDBaudSite3 = VclNew<TComboBox>();                         // MainForm.dfm:2311
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "38400", "115200", NULL };
        DfmItems(cb_SPRDBaudSite3->Items, items);
    }
    cb_SPRDBaudSite3->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDBaudSite3->Text      = "cbBaudRate";

    cb_SPRDSizeSite3 = VclNew<TComboBox>();                         // MainForm.dfm:2334
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_SPRDSizeSite3->Items, items);
    }
    cb_SPRDSizeSite3->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDSizeSite3->Text      = "cbByteSize";

    cb_SPRDStopSite3 = VclNew<TComboBox>();                         // MainForm.dfm:2354
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_SPRDStopSite3->Items, items);
    }
    cb_SPRDStopSite3->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDStopSite3->Text      = "cbStopBit";

    cb_SPRDParitySite3 = VclNew<TComboBox>();                       // MainForm.dfm:2373
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_SPRDParitySite3->Items, items);
    }
    cb_SPRDParitySite3->ItemIndex = -1;                             //   no ItemIndex stored -> VCL -1
    cb_SPRDParitySite3->Text      = "cbParity";

    cb_SPRDComSite3 = VclNew<TComboBox>();                          // MainForm.dfm:2394
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_SPRDComSite3->Items, items);
    }
    cb_SPRDComSite3->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDComSite3->Text      = "cbCOM";

    GroupBox11 = VclNew<TGroupBox>();                               // MainForm.dfm:2420
    GroupBox11->Caption = "Detail Setting";

    Label85 = VclNew<TLabel>();                                     // MainForm.dfm:2427
    Label85->Caption = "DTR Ctrl：";

    Label86 = VclNew<TLabel>();                                     // MainForm.dfm:2441
    Label86->Caption = "RTS Ctrll：";

    Label87 = VclNew<TLabel>();                                     // MainForm.dfm:2455
    Label87->Caption = "ReadIntervalTimeout：";

    Label88 = VclNew<TLabel>();                                     // MainForm.dfm:2468
    Label88->Caption = "WriteTotalTimeoutMultiplier：";

    cb_SPRDDTRSite3 = VclNew<TComboBox>();                          // MainForm.dfm:2481
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_SPRDDTRSite3->Items, items);
    }
    cb_SPRDDTRSite3->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDDTRSite3->Text      = "DTR";

    cb_SPRDRTSSite3 = VclNew<TComboBox>();                          // MainForm.dfm:2500
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_SPRDRTSSite3->Items, items);
    }
    cb_SPRDRTSSite3->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDRTSSite3->Text      = "RTS";

    cb_SPRDTxCSite3 = VclNew<TCheckBox>();                          // MainForm.dfm:2520
    cb_SPRDTxCSite3->Caption = "TxContinueOnXoff";

    ed_SPRDTimeOutTotalSite3 = VclNew<TEdit>();                     // MainForm.dfm:2528

    ed_SPRDTimeOutSite3 = VclNew<TEdit>();                          // MainForm.dfm:2535

    TabSheet11 = VclNew<TTabSheet>();                               // MainForm.dfm:2546
    TabSheet11->Caption = "Site4";
    TabSheet11->TabVisible = true;

    Pc_SPRDSite4Control = VclNew<TPageControl>();                   // MainForm.dfm:2549
    Pc_SPRDSite4Control->ActivePageIndex = 0;                       //   ActivePage = ts_SPRDLogSite4 (page 0; .dfm TabIndex = 0)

    ts_SPRDLogSite4 = VclNew<TTabSheet>();                          // MainForm.dfm:2565
    ts_SPRDLogSite4->Caption = "Log";
    ts_SPRDLogSite4->TabVisible = true;

    mm_SPRDLogSite4 = VclNew<TMemo>();                              // MainForm.dfm:2567 (ReadOnly = True: no stand-in member)

    cb_SPRDShowLogSite4 = VclNew<TCheckBox>();                      // MainForm.dfm:2583
    cb_SPRDShowLogSite4->Caption = "Show Log (Site4 )";

    bt_SPRDClearLogSite4 = VclNew<TButton>();                       // MainForm.dfm:2597
    bt_SPRDClearLogSite4->Caption = "CLEAR ";

    cb_SPRDSaveLogSite4 = VclNew<TCheckBox>();                      // MainForm.dfm:2605
    cb_SPRDSaveLogSite4->Caption = "Save Log (Site4 )";

    ts_SPRDSetupSite4 = VclNew<TTabSheet>();                        // MainForm.dfm:2620
    ts_SPRDSetupSite4->Caption = "Setup";
    ts_SPRDSetupSite4->TabVisible = true;

    Button6 = VclNew<TButton>();                                    // MainForm.dfm:2623
    Button6->Caption = "Update";

    GroupBox12 = VclNew<TGroupBox>();                               // MainForm.dfm:2637
    GroupBox12->Caption = "COM Port Setting";

    Label89 = VclNew<TLabel>();                                     // MainForm.dfm:2644
    Label89->Caption = "Baud Rate：";

    Label90 = VclNew<TLabel>();                                     // MainForm.dfm:2659
    Label90->Caption = "Byte Size：";

    Label91 = VclNew<TLabel>();                                     // MainForm.dfm:2674
    Label91->Caption = "Stop Bit：";

    Label92 = VclNew<TLabel>();                                     // MainForm.dfm:2689
    Label92->Caption = "Parity：";

    Label93 = VclNew<TLabel>();                                     // MainForm.dfm:2704
    Label93->Caption = "Device：";

    cb_SPRDBaudSite4 = VclNew<TComboBox>();                         // MainForm.dfm:2719
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "38400", "115200", NULL };
        DfmItems(cb_SPRDBaudSite4->Items, items);
    }
    cb_SPRDBaudSite4->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDBaudSite4->Text      = "cbBaudRate";

    cb_SPRDSizeSite4 = VclNew<TComboBox>();                         // MainForm.dfm:2742
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cb_SPRDSizeSite4->Items, items);
    }
    cb_SPRDSizeSite4->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDSizeSite4->Text      = "cbByteSize";

    cb_SPRDStopSite4 = VclNew<TComboBox>();                         // MainForm.dfm:2762
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cb_SPRDStopSite4->Items, items);
    }
    cb_SPRDStopSite4->ItemIndex = -1;                               //   no ItemIndex stored -> VCL -1
    cb_SPRDStopSite4->Text      = "cbStopBit";

    cb_SPRDParitySite4 = VclNew<TComboBox>();                       // MainForm.dfm:2781
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cb_SPRDParitySite4->Items, items);
    }
    cb_SPRDParitySite4->ItemIndex = -1;                             //   no ItemIndex stored -> VCL -1
    cb_SPRDParitySite4->Text      = "cbParity";

    cb_SPRDComSite4 = VclNew<TComboBox>();                          // MainForm.dfm:2802
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", NULL };
        DfmItems(cb_SPRDComSite4->Items, items);
    }
    cb_SPRDComSite4->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDComSite4->Text      = "cbCOM";

    GroupBox13 = VclNew<TGroupBox>();                               // MainForm.dfm:2828
    GroupBox13->Caption = "Detail Setting";

    Label94 = VclNew<TLabel>();                                     // MainForm.dfm:2835
    Label94->Caption = "DTR Ctrl：";

    Label95 = VclNew<TLabel>();                                     // MainForm.dfm:2849
    Label95->Caption = "RTS Ctrll：";

    Label96 = VclNew<TLabel>();                                     // MainForm.dfm:2863
    Label96->Caption = "ReadIntervalTimeout：";

    Label97 = VclNew<TLabel>();                                     // MainForm.dfm:2876
    Label97->Caption = "WriteTotalTimeoutMultiplier：";

    cb_SPRDDTRSite4 = VclNew<TComboBox>();                          // MainForm.dfm:2889
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", NULL };
        DfmItems(cb_SPRDDTRSite4->Items, items);
    }
    cb_SPRDDTRSite4->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDDTRSite4->Text      = "DTR";

    cb_SPRDRTSSite4 = VclNew<TComboBox>();                          // MainForm.dfm:2908
    {
        static const char* const items[] = { "Enable", "Disable", "Handshake", "TransmissionAvailable", NULL };
        DfmItems(cb_SPRDRTSSite4->Items, items);
    }
    cb_SPRDRTSSite4->ItemIndex = -1;                                //   no ItemIndex stored -> VCL -1
    cb_SPRDRTSSite4->Text      = "RTS";

    cb_SPRDTxCSite4 = VclNew<TCheckBox>();                          // MainForm.dfm:2928
    cb_SPRDTxCSite4->Caption = "TxContinueOnXoff";

    ed_SPRDTimeOutTotalSite4 = VclNew<TEdit>();                     // MainForm.dfm:2936

    ed_SPRDTimeOutSite4 = VclNew<TEdit>();                          // MainForm.dfm:2943

    ts_BinCode = VclNew<TTabSheet>();                               // MainForm.dfm:2954
    ts_BinCode->Caption = "BinCode";
    ts_BinCode->TabVisible = true;

    Label76 = VclNew<TLabel>();                                     // MainForm.dfm:2957
    Label76->Caption = "TIMEOUT BIN";

    Label77 = VclNew<TLabel>();                                     // MainForm.dfm:2971
    Label77->Caption = "TIMEOUT TIME";

    Label78 = VclNew<TLabel>();                                     // MainForm.dfm:2985
    Label78->Caption = " 0.1 sc";

    Label79 = VclNew<TLabel>();                                     // MainForm.dfm:2992
    Label79->Caption = "NOTDEF BIN";

    Panel15 = VclNew<TPanel>();                                     // MainForm.dfm:3006

    Label62 = VclNew<TLabel>();                                     // MainForm.dfm:3013
    Label62->Caption = "Bin 11";

    Label63 = VclNew<TLabel>();                                     // MainForm.dfm:3026
    Label63->Caption = "Bin 12";

    Label64 = VclNew<TLabel>();                                     // MainForm.dfm:3039
    Label64->Caption = "Bin 13";

    Label65 = VclNew<TLabel>();                                     // MainForm.dfm:3052
    Label65->Caption = "Bin 14";

    Label66 = VclNew<TLabel>();                                     // MainForm.dfm:3065
    Label66->Caption = "Bin 15";

    pn_WSBin11 = VclNew<TPanel>();                                  // MainForm.dfm:3078

    pn_WSBin12 = VclNew<TPanel>();                                  // MainForm.dfm:3086

    pn_WSBin13 = VclNew<TPanel>();                                  // MainForm.dfm:3094

    pn_WSBin14 = VclNew<TPanel>();                                  // MainForm.dfm:3102

    pn_WSBin15 = VclNew<TPanel>();                                  // MainForm.dfm:3110

    Panel9 = VclNew<TPanel>();                                      // MainForm.dfm:3119

    Label67 = VclNew<TLabel>();                                     // MainForm.dfm:3126
    Label67->Caption = "Bin 6";

    Label68 = VclNew<TLabel>();                                     // MainForm.dfm:3139
    Label68->Caption = "Bin 7";

    Label69 = VclNew<TLabel>();                                     // MainForm.dfm:3152
    Label69->Caption = "Bin 8";

    Label70 = VclNew<TLabel>();                                     // MainForm.dfm:3165
    Label70->Caption = "Bin 9";

    Label71 = VclNew<TLabel>();                                     // MainForm.dfm:3178
    Label71->Caption = "Bin 10";

    pn_WSBin6 = VclNew<TPanel>();                                   // MainForm.dfm:3191

    pn_WSBin7 = VclNew<TPanel>();                                   // MainForm.dfm:3199

    pn_WSBin8 = VclNew<TPanel>();                                   // MainForm.dfm:3207

    pn_WSBin9 = VclNew<TPanel>();                                   // MainForm.dfm:3215

    pn_WSBin10 = VclNew<TPanel>();                                  // MainForm.dfm:3223

    Panel3 = VclNew<TPanel>();                                      // MainForm.dfm:3232

    lbWS3000Bin1 = VclNew<TLabel>();                                // MainForm.dfm:3239
    lbWS3000Bin1->Caption = "Bin 1";

    Label72 = VclNew<TLabel>();                                     // MainForm.dfm:3252
    Label72->Caption = "Bin 2";

    Label73 = VclNew<TLabel>();                                     // MainForm.dfm:3265
    Label73->Caption = "Bin 3";

    Label74 = VclNew<TLabel>();                                     // MainForm.dfm:3278
    Label74->Caption = "Bin 4";

    Label75 = VclNew<TLabel>();                                     // MainForm.dfm:3291
    Label75->Caption = "Bin 5";

    pn_WSBin1 = VclNew<TPanel>();                                   // MainForm.dfm:3304

    pn_WSBin2 = VclNew<TPanel>();                                   // MainForm.dfm:3312

    pn_WSBin3 = VclNew<TPanel>();                                   // MainForm.dfm:3320

    pn_WSBin4 = VclNew<TPanel>();                                   // MainForm.dfm:3328

    pn_WSBin5 = VclNew<TPanel>();                                   // MainForm.dfm:3336

    cb_SPRDSendSOTString = VclNew<TCheckBox>();                     // MainForm.dfm:3345
    cb_SPRDSendSOTString->Caption = "Simulate";

    Pan_TimeOutBin = VclNew<TPanel>();                              // MainForm.dfm:3353
    Pan_TimeOutBin->Color = 0x0000FFFF;                             //   clYellow

    ed_TimeOutTime = VclNew<TEdit>();                               // MainForm.dfm:3362
    ed_TimeOutTime->Text = "10";

    bt_ChangeTimeOut = VclNew<TButton>();                           // MainForm.dfm:3370
    bt_ChangeTimeOut->Caption = "Set";

    Pan_NotDefinedtBin = VclNew<TPanel>();                          // MainForm.dfm:3378
    Pan_NotDefinedtBin->Color = 0x0000FFFF;                         //   clYellow

    Button1 = VclNew<TButton>();                                    // MainForm.dfm:3387
    Button1->Caption = "Manual Test";

    tsVersion = VclNew<TTabSheet>();                                // MainForm.dfm:3398
    tsVersion->Caption = "Version";
    tsVersion->TabVisible = true;

    MemoVer = VclNew<TMemo>();                                      // MainForm.dfm:3401 (ReadOnly = True: no stand-in member)

    tsTTL = VclNew<TTabSheet>();                                    // MainForm.dfm:3412
    tsTTL->Caption = "TTL";
    tsTTL->TabVisible = true;

    PageControl7 = VclNew<TPageControl>();                          // MainForm.dfm:3415
    PageControl7->ActivePageIndex = 2;                              //   ActivePage = ts_TTL_Simu (page 2; .dfm TabIndex = 2)

    ts_TTL_BinLog = VclNew<TTabSheet>();                            // MainForm.dfm:3425
    ts_TTL_BinLog->Caption = "Bin Log";
    ts_TTL_BinLog->TabVisible = true;

    MemoBinData_TTL = VclNew<TMemo>();                              // MainForm.dfm:3428

    Panel1 = VclNew<TPanel>();                                      // MainForm.dfm:3436

    cbShowLog_TTL = VclNew<TCheckBox>();                            // MainForm.dfm:3444
    cbShowLog_TTL->Caption = "Show Log";

    btClear_TTL = VclNew<TButton>();                                // MainForm.dfm:3458 (OnClick = btClear_TTLClick)
    btClear_TTL->Caption = "CLEAR ";

    cbSaveLog_TTL = VclNew<TCheckBox>();                            // MainForm.dfm:3467
    cbSaveLog_TTL->Caption = "Save Log";

    ts_TTL_Setup = VclNew<TTabSheet>();                             // MainForm.dfm:3483
    ts_TTL_Setup->Caption = "Setup";
    ts_TTL_Setup->TabVisible = true;

    GroupBox15 = VclNew<TGroupBox>();                               // MainForm.dfm:3486
    GroupBox15->Caption = "COM Port Setting";

    Label7 = VclNew<TLabel>();                                      // MainForm.dfm:3494
    Label7->Caption = "Baud Rate：";

    Label8 = VclNew<TLabel>();                                      // MainForm.dfm:3509
    Label8->Caption = "Byte Size：";

    Label9 = VclNew<TLabel>();                                      // MainForm.dfm:3524
    Label9->Caption = "Stop Bit：";

    Label10 = VclNew<TLabel>();                                     // MainForm.dfm:3539
    Label10->Caption = "Parity：";

    Label11 = VclNew<TLabel>();                                     // MainForm.dfm:3554
    Label11->Caption = "Device1：";

    Label13 = VclNew<TLabel>();                                     // MainForm.dfm:3569
    Label13->Caption = "Device2：";

    cbBaudRate_TTL = VclNew<TComboBox>();                           // MainForm.dfm:3584
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", "115200", NULL };
        DfmItems(cbBaudRate_TTL->Items, items);
    }
    cbBaudRate_TTL->ItemIndex = -1;                                 //   no ItemIndex stored -> VCL -1
    cbBaudRate_TTL->Text      = "cbBaudRate";
    cbBaudRate_TTL->Enabled = false;

    cbByteSize_TTL = VclNew<TComboBox>();                           // MainForm.dfm:3607
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cbByteSize_TTL->Items, items);
    }
    cbByteSize_TTL->ItemIndex = -1;                                 //   no ItemIndex stored -> VCL -1
    cbByteSize_TTL->Text      = "cbByteSize";
    cbByteSize_TTL->Enabled = false;

    cbStopBit_TTL = VclNew<TComboBox>();                            // MainForm.dfm:3628
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cbStopBit_TTL->Items, items);
    }
    cbStopBit_TTL->ItemIndex = -1;                                  //   no ItemIndex stored -> VCL -1
    cbStopBit_TTL->Text      = "cbStopBit";
    cbStopBit_TTL->Enabled = false;

    cbParity_TTL = VclNew<TComboBox>();                             // MainForm.dfm:3648
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cbParity_TTL->Items, items);
    }
    cbParity_TTL->ItemIndex = -1;                                   //   no ItemIndex stored -> VCL -1
    cbParity_TTL->Text      = "cbParity";
    cbParity_TTL->Enabled = false;

    cbDevice_TTL = VclNew<TComboBox>();                             // MainForm.dfm:3670
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", "COM10", "COM11", "COM12", "COM13", "COM14", "COM15", "COM16",
                                             "COM17", "COM18", NULL };
        DfmItems(cbDevice_TTL->Items, items);
    }
    cbDevice_TTL->ItemIndex = -1;                                   //   no ItemIndex stored -> VCL -1
    cbDevice_TTL->Text      = "cbCOM";

    cbDevice_TTL_2 = VclNew<TComboBox>();                           // MainForm.dfm:3704
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                             "COM9", "COM10", "COM11", "COM12", "COM13", "COM14", "COM15", "COM16",
                                             "COM17", "COM18", NULL };
        DfmItems(cbDevice_TTL_2->Items, items);
    }
    cbDevice_TTL_2->ItemIndex = -1;                                 //   no ItemIndex stored -> VCL -1
    cbDevice_TTL_2->Text      = "cbCOM";

    btnUpdate_TTL = VclNew<TPanel>();                               // MainForm.dfm:3739 (OnClick = btnUpdate_TTLClick, OnMouseDown = btnUpdate_TTLMouseDown, OnMouseUp = btnUpdate_TTLMouseUp)
    btnUpdate_TTL->Caption = "Update";

    GroupBox16 = VclNew<TGroupBox>();                               // MainForm.dfm:3758
    GroupBox16->Caption = "Detail Settng";

    Label12 = VclNew<TLabel>();                                     // MainForm.dfm:3766
    Label12->Caption = "ReadIntervalTimeout ：";

    edReadIntervalTimeout_TTL = VclNew<TEdit>();                    // MainForm.dfm:3781
    edReadIntervalTimeout_TTL->Text = "50";

    ts_TTL_Simu = VclNew<TTabSheet>();                              // MainForm.dfm:3791
    ts_TTL_Simu->Caption = "Simulate";
    ts_TTL_Simu->TabVisible = true;

    Button8 = VclNew<TButton>();                                    // MainForm.dfm:3794 (OnClick = btnAllUseClick)
    Button8->Caption = "All Use";

    Button9 = VclNew<TButton>();                                    // MainForm.dfm:3803 (OnClick = btnAllNoUseClick)
    Button9->Caption = "All No Use";

    btnTTL_Manual = VclNew<TButton>();                              // MainForm.dfm:3812 (OnClick = btnTTL_ManualClick)
    btnTTL_Manual->Caption = "Manual Test";

    btnClearSot = VclNew<TButton>();                                // MainForm.dfm:3821 (OnClick = btnClearSotClick)
    btnClearSot->Caption = "Clear SOT";

    Button10 = VclNew<TButton>();                                   // MainForm.dfm:3830 (OnClick = Button10Click)
    Button10->Caption = "Close Port";
    Button10->Visible = false;

    Button11 = VclNew<TButton>();                                   // MainForm.dfm:3840 (OnClick = Button11Click)
    Button11->Caption = "Open Port";
    Button11->Visible = false;

    btnEnableSOTCSOT = VclNew<TButton>();                           // MainForm.dfm:3850 (OnClick = btnEnableSOTCSOTClick)
    btnEnableSOTCSOT->Caption = "Enable Clear SOT";

    tsLog = VclNew<TTabSheet>();                                    // MainForm.dfm:3862
    tsLog->Caption = "Log";
    tsLog->TabVisible = true;

    MemoLog = VclNew<TMemo>();                                      // MainForm.dfm:3865 (ReadOnly = True: no stand-in member)

    StatusBar1 = VclNew<TStatusBar>();                              // MainForm.dfm:3882 (Panels = 1 item, Width 50, no Text; the stand-in carries 8)

    palSite = VclNew<TPanel>();                                     // MainForm.dfm:3893
    palSite->Color = 8421440;

    gbSite = VclNew<TGroupBox>();                                   // MainForm.dfm:3902
    gbSite->Caption = "Site01";
    gbSite->Visible = false;

    labOcr = VclNew<TLabel>();                                      // MainForm.dfm:3916
    labOcr->Caption = "OCR Text";

    cbBin = VclNew<TComboBox>();                                    // MainForm.dfm:3931 (ImeMode / ImeName: no stand-in member)
    {
        static const char* const items[] = { "1", "2", "3", "4", "5", "6", "1..5", "1..10", "0..11", NULL };
        DfmItems(cbBin->Items, items);
    }
    cbBin->ItemIndex = -1;                                          //   no ItemIndex stored -> VCL -1
    cbBin->Text      = "1";

    cbSiteOn = VclNew<TCheckBox>();                                 // MainForm.dfm:3958
    cbSiteOn->Checked = true;

    plSite = VclNew<TPanel>();                                      // MainForm.dfm:3967
    plSite->Caption = "01";
    plSite->Color = 0x00C0C0C0;                                     //   clSilver

    Timer1 = VclNew<TTimer>();                                      // MainForm.dfm:4016 (OnTimer = Timer1Timer)
    Timer1->Interval = 300;                                         //   Enabled not stored -> VCL default True

    // MainForm.dfm:3985 / :4022 / :4053.  Outx_CtsFlow=False, Outx_DsrFlow=False, DtrControl=DtrEnable,
    // DsrSensitivity=False, TxContinueOnXoff=True, ReplaceWhenParityError=False, IgnoreNullChar=False,
    // RtsControl=RtsEnable, XonLimit=500, XoffLimit=500, XonChar=#17, XoffChar=#19, ReplacedChar=#0 and
    // Read/WriteTotalTimeoutMultiplier/Constant (all 0) have no vclcompat TComm member (vclcompat/Comm.h).
    // FormShow's LoadSetupData / LoadSetupData_TTL overwrite the port settings from Setup.ini.
    CommTester = new TComm(NULL);                                   // MainForm.dfm:3985 (OnReceiveData = CommTesterReceiveData)
    CommTester->CommName            = "COM13";
    CommTester->BaudRate            = 9600;
    CommTester->ParityCheck         = false;
    CommTester->Outx_XonXoffFlow    = false;
    CommTester->Inx_XonXoffFlow     = false;
    CommTester->ByteSize            = Spcomm::_7;
    CommTester->Parity              = Spcomm::Even;
    CommTester->StopBits            = Spcomm::_1;
    CommTester->ReadIntervalTimeout = 1;

    CommTester_TTL = new TComm(NULL);                               // MainForm.dfm:4022 (OnReceiveData = CommTester_TTLReceiveData)
    CommTester_TTL->CommName            = "COM13";
    CommTester_TTL->BaudRate            = 115200;
    CommTester_TTL->ParityCheck         = false;
    CommTester_TTL->Outx_XonXoffFlow    = false;
    CommTester_TTL->Inx_XonXoffFlow     = false;
    CommTester_TTL->ByteSize            = Spcomm::_8;
    CommTester_TTL->Parity              = Spcomm::None;
    CommTester_TTL->StopBits            = Spcomm::_1;
    CommTester_TTL->ReadIntervalTimeout = 10;

    CommTester_TTL_2 = new TComm(NULL);                             // MainForm.dfm:4053 (OnReceiveData = CommTester_TTL_2ReceiveData)
    CommTester_TTL_2->CommName            = "COM13";
    CommTester_TTL_2->BaudRate            = 115200;
    CommTester_TTL_2->ParityCheck         = false;
    CommTester_TTL_2->Outx_XonXoffFlow    = false;
    CommTester_TTL_2->Inx_XonXoffFlow     = false;
    CommTester_TTL_2->ByteSize            = Spcomm::_8;
    CommTester_TTL_2->Parity              = Spcomm::None;
    CommTester_TTL_2->StopBits            = Spcomm::_1;
    CommTester_TTL_2->ReadIntervalTimeout = 10;

    //AI(W906-GB-P4) 20260926: rule 14 -- vclcompat TComm fires OnReceiveData on its own reader thread; golden
    //   handles it on the form thread.  The callbacks only QueueRx(); Rs232Engine::RunOnce -> DrainRx() dispatches
    //   to CommTesterReceiveData / CommTester_TTLReceiveData / CommTester_TTL_2ReceiveData on the TesterComm thread.
    CommTester->OnReceiveData       = [this](TObject*, void* b, Word n){ QueueRx(kRxTester, b, n); };
    CommTester_TTL->OnReceiveData   = [this](TObject*, void* b, Word n){ QueueRx(kRxTtl1, b, n); };
    CommTester_TTL_2->OnReceiveData = [this](TObject*, void* b, Word n){ QueueRx(kRxTtl2, b, n); };

    // ---- golden body (MainForm.cpp:361-383) ----
    slRS232Log=new TMyStringList("D:\\RS232Log\\LOG",
                                 "RS232_Log",
                                 "Date, Time, Action, Message, Hex");           //Steven 20240913 : 變更RS232 log記錄方式
    slRS232Log->SaveType=TBy2Hour;

    InitialOK=true;

    bFind           =false;
    Caption         ="RS232Standard";
    bCommConnect[0] =false;                                                     //Isaac 20200903 :TTL RS232通訊，多一個RS232元件
    bCommConnect[1] =false;                                                     //Isaac 20210309 :TTL RS232兩塊板子
    bCommConnect[2] =false;                                                     //Isaac 20210309 :TTL RS232兩塊板子
    iMaxBinCount    =1;
    bTwoArmTestMode =false;                                                     //Steven 20141014 : 神盾測試模式
    iTwoArmTestStep =0;
    sBarCode        =new TStringList();
    sBarCode_ASE_CL =new TStringList();                                         //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    iHasCE          =iNoneTest;                                                 //Steven 20231205 : 判斷RS232流程是否異常
    #ifdef SOFT_SIMULTE
    uServer=new uSocketServer();
//    uServer->SetReceiveFunc(ReceiveData_TCPIP);
    //AI(W906-GB-P4) 20260926: rule 14 -- vclcompat TServerSocket fires OnClientRead (-> uSocketServer::ServerSocketRead
    //   -> tpvReceive) on its own thread; the callback only queues a copy, DrainRx() calls ReceiveData_TCPIP on the
    //   TesterComm thread.
    uServer->SetReceiveFunc([this](char* c, int n){ QueueRx(kRxTcp, c, (Word)n); });
    bCommConnect[0]=true;
    #endif
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden has no destructor body -- FormClose frees the dut panels and the barcode lists,
//   FormDestroy frees slRS232Log, and TComponent's destructor frees every owned widget (slCmdList and uServer are
//   never freed in golden: the process exits).  FormClose / FormDestroy NULL what they free, so this deletes only
//   what is still allocated: no double free after FormClose / FormDestroy, no leak when they never ran.
//   CommTester / CommTester_TTL / CommTester_TTL_2 and uServer go first: their threads call QueueRx(this) until
//   ~TComm (StopComm) / ~uSocketServer (TServerSocket) joins them.
TfRS232Main::~TfRS232Main()
{
    delete CommTester;        CommTester       = NULL;
    delete CommTester_TTL;    CommTester_TTL   = NULL;
    delete CommTester_TTL_2;  CommTester_TTL_2 = NULL;
    if (uServer != NULL)
    {
        delete uServer;
        uServer = NULL;
    }

    for (size_t i = 0; i < MY_DUT_PAL.size(); ++i)
    {
        if (MY_DUT_PAL[i] != NULL)
            delete MY_DUT_PAL[i];
    }
    MY_DUT_PAL.clear();
    if (slCmdList != NULL)              // created by ShowVersion (golden MainForm.cpp:86)
    {
        delete slCmdList;
        slCmdList = NULL;
    }
    if (slRS232Log != NULL)             // golden ctor :361, freed by FormDestroy :395
    {
        delete slRS232Log;
        slRS232Log = NULL;
    }
    if (sBarCode != NULL)               // golden ctor :376, freed by FormClose :552
    {
        delete sBarCode;
        sBarCode = NULL;
    }
    if (sBarCode_ASE_CL != NULL)        // golden ctor :377, freed by FormClose :553
    {
        delete sBarCode_ASE_CL;
        sBarCode_ASE_CL = NULL;
    }

    // MainForm.dfm widgets (dfm order)
    delete labDebugMode;
    delete PageControl1;
    delete ts_STD;
    delete PageControl2;
    delete ts_STD_Log;
    delete ts_STD_BinLog;
    delete MemoBinData;
    delete Panel2;
    delete cbShowLog;
    delete btClear;
    delete cbSaveLog;
    delete ts_STD_Setup;
    delete GroupBox1;
    delete Label1;
    delete Label2;
    delete Label3;
    delete Label4;
    delete Label5;
    delete cbBaudRate;
    delete cbByteSize;
    delete cbStopBit;
    delete cbParity;
    delete cbDevice;
    delete btnUpdate;
    delete GroupBox14;
    delete Label6;
    delete edReadIntervalTimeout;
    delete ts_STD_Simu;
    delete btManualTest;
    delete btnAllUse;
    delete btnAllNoUse;
    delete ts_AVAGO;
    delete PageControl3;
    delete TabSheet4;
    delete PageControl4;
    delete ts_AVAGOLogSite1;
    delete mm_AvagoLogSite1;
    delete cb_AvagoShowLogSite1;
    delete bt_AvagoClearLogSite1;
    delete cb_AvagoSaveLogSite1;
    delete TabSheet2;
    delete bt_UpdateSite1;
    delete GroupBox2;
    delete Label17;
    delete Label18;
    delete Label25;
    delete Label26;
    delete Label27;
    delete cb_AvagoBaudSite1;
    delete cb_AvagoSizeSite1;
    delete cb_AvagoStopSite1;
    delete cb_AvagoParitySite1;
    delete cb_AvagoComSite1;
    delete GroupBox3;
    delete Label28;
    delete Label29;
    delete Label30;
    delete Label31;
    delete cb_AvagoDTRSite1;
    delete cb_AvagoRTSSite1;
    delete cb_AvagoTxCSite1;
    delete ed_AvagoTimeOutTotalSite1;
    delete ed_AvagoTimeOutSite1;
    delete TabSheet5;
    delete PageControl6;
    delete TabSheet6;
    delete mm_AvagoLogSite2;
    delete cb_AvagoShowLogSite2;
    delete bt_AvagoClearLogSite2;
    delete cb_AvagoSaveLogSite2;
    delete TabSheet7;
    delete bt_UpdateSite2;
    delete GroupBox4;
    delete Label32;
    delete Label33;
    delete Label34;
    delete Label35;
    delete Label36;
    delete cb_AvagoBaudSite2;
    delete cb_AvagoSizeSite2;
    delete cb_AvagoStopSite2;
    delete cb_AvagoParitySite2;
    delete cb_AvagoComSite2;
    delete GroupBox5;
    delete Label38;
    delete Label39;
    delete Label40;
    delete Label41;
    delete cb_AvagoDTRSite2;
    delete cb_AvagoRTSSite2;
    delete cb_AvagoTxCSite2;
    delete ed_AvagoTimeOutTotalSite2;
    delete ed_AvagoTimeOutSite2;
    delete TabSheet1;
    delete Label42;
    delete Label43;
    delete bt_SimulateManualTestSite2;
    delete ed_SimulateNu;
    delete ed_Number;
    delete tsSPRD;
    delete PageControl5;
    delete TabSheet3;
    delete Pc_SPRDSite1Control;
    delete ts_SPRDLogSite1;
    delete mm_SPRDLogSite1;
    delete cb_SPRDShowLogSite1;
    delete bt_SPRDClearLogSite1;
    delete cb_SPRDSaveLogSite1;
    delete ts_SPRDSetupSite1;
    delete Button2;
    delete GroupBox6;
    delete Label44;
    delete Label45;
    delete Label46;
    delete Label47;
    delete Label48;
    delete cb_SPRDBaudSite1;
    delete cb_SPRDSizeSite1;
    delete cb_SPRDStopSite1;
    delete cb_SPRDParitySite1;
    delete cb_SPRDComSite1;
    delete GroupBox7;
    delete Label49;
    delete Label50;
    delete Label51;
    delete Label52;
    delete cb_SPRDDTRSite1;
    delete cb_SPRDRTSSite1;
    delete cb_SPRDTxCSite1;
    delete ed_SPRDTimeOutTotalSite1;
    delete ed_SPRDTimeOutSite1;
    delete TabSheet10;
    delete Pc_SPRDSite2Control;
    delete ts_SPRDLogSite2;
    delete mm_SPRDLogSite2;
    delete cb_SPRDShowLogSite2;
    delete bt_SPRDClearLogSite2;
    delete cb_SPRDSaveLogSite2;
    delete ts_SPRDSetupSite2;
    delete Button4;
    delete GroupBox8;
    delete Label53;
    delete Label54;
    delete Label55;
    delete Label56;
    delete Label57;
    delete cb_SPRDBaudSite2;
    delete cb_SPRDSizeSite2;
    delete cb_SPRDStopSite2;
    delete cb_SPRDParitySite2;
    delete cb_SPRDComSite2;
    delete GroupBox9;
    delete Label58;
    delete Label59;
    delete Label60;
    delete Label61;
    delete cb_SPRDDTRSite2;
    delete cb_SPRDRTSSite2;
    delete cb_SPRDTxCSite2;
    delete ed_SPRDTimeOutTotalSite2;
    delete ed_SPRDTimeOutSite2;
    delete TabSheet8;
    delete Pc_SPRDSite3Control;
    delete ts_SPRDLogSite3;
    delete mm_SPRDLogSite3;
    delete cb_SPRDShowLogSite3;
    delete bt_SPRDClearLogSite3;
    delete cb_SPRDSaveLogSite3;
    delete ts_SPRDSetupSite3;
    delete Button3;
    delete GroupBox10;
    delete Label80;
    delete Label81;
    delete Label82;
    delete Label83;
    delete Label84;
    delete cb_SPRDBaudSite3;
    delete cb_SPRDSizeSite3;
    delete cb_SPRDStopSite3;
    delete cb_SPRDParitySite3;
    delete cb_SPRDComSite3;
    delete GroupBox11;
    delete Label85;
    delete Label86;
    delete Label87;
    delete Label88;
    delete cb_SPRDDTRSite3;
    delete cb_SPRDRTSSite3;
    delete cb_SPRDTxCSite3;
    delete ed_SPRDTimeOutTotalSite3;
    delete ed_SPRDTimeOutSite3;
    delete TabSheet11;
    delete Pc_SPRDSite4Control;
    delete ts_SPRDLogSite4;
    delete mm_SPRDLogSite4;
    delete cb_SPRDShowLogSite4;
    delete bt_SPRDClearLogSite4;
    delete cb_SPRDSaveLogSite4;
    delete ts_SPRDSetupSite4;
    delete Button6;
    delete GroupBox12;
    delete Label89;
    delete Label90;
    delete Label91;
    delete Label92;
    delete Label93;
    delete cb_SPRDBaudSite4;
    delete cb_SPRDSizeSite4;
    delete cb_SPRDStopSite4;
    delete cb_SPRDParitySite4;
    delete cb_SPRDComSite4;
    delete GroupBox13;
    delete Label94;
    delete Label95;
    delete Label96;
    delete Label97;
    delete cb_SPRDDTRSite4;
    delete cb_SPRDRTSSite4;
    delete cb_SPRDTxCSite4;
    delete ed_SPRDTimeOutTotalSite4;
    delete ed_SPRDTimeOutSite4;
    delete ts_BinCode;
    delete Label76;
    delete Label77;
    delete Label78;
    delete Label79;
    delete Panel15;
    delete Label62;
    delete Label63;
    delete Label64;
    delete Label65;
    delete Label66;
    delete pn_WSBin11;
    delete pn_WSBin12;
    delete pn_WSBin13;
    delete pn_WSBin14;
    delete pn_WSBin15;
    delete Panel9;
    delete Label67;
    delete Label68;
    delete Label69;
    delete Label70;
    delete Label71;
    delete pn_WSBin6;
    delete pn_WSBin7;
    delete pn_WSBin8;
    delete pn_WSBin9;
    delete pn_WSBin10;
    delete Panel3;
    delete lbWS3000Bin1;
    delete Label72;
    delete Label73;
    delete Label74;
    delete Label75;
    delete pn_WSBin1;
    delete pn_WSBin2;
    delete pn_WSBin3;
    delete pn_WSBin4;
    delete pn_WSBin5;
    delete cb_SPRDSendSOTString;
    delete Pan_TimeOutBin;
    delete ed_TimeOutTime;
    delete bt_ChangeTimeOut;
    delete Pan_NotDefinedtBin;
    delete Button1;
    delete tsVersion;
    delete MemoVer;
    delete tsTTL;
    delete PageControl7;
    delete ts_TTL_BinLog;
    delete MemoBinData_TTL;
    delete Panel1;
    delete cbShowLog_TTL;
    delete btClear_TTL;
    delete cbSaveLog_TTL;
    delete ts_TTL_Setup;
    delete GroupBox15;
    delete Label7;
    delete Label8;
    delete Label9;
    delete Label10;
    delete Label11;
    delete Label13;
    delete cbBaudRate_TTL;
    delete cbByteSize_TTL;
    delete cbStopBit_TTL;
    delete cbParity_TTL;
    delete cbDevice_TTL;
    delete cbDevice_TTL_2;
    delete btnUpdate_TTL;
    delete GroupBox16;
    delete Label12;
    delete edReadIntervalTimeout_TTL;
    delete ts_TTL_Simu;
    delete Button8;
    delete Button9;
    delete btnTTL_Manual;
    delete btnClearSot;
    delete Button10;
    delete Button11;
    delete btnEnableSOTCSOT;
    delete tsLog;
    delete MemoLog;
    delete StatusBar1;
    delete palSite;
    delete gbSite;
    delete labOcr;
    delete cbBin;
    delete cbSiteOn;
    delete plSite;
    delete Timer1;
}
//---------------------------------------------------------------------------
void TfRS232Main::FormCreate(TObject *Sender)
{
//    Application->OnException=AppException;                                      //ChungHung 20141226 add catch exception
    //AI(W906-GB-P4) 20260926: dropped -- there is no VCL Application in-process; see AppException below.
}
//---------------------------------------------------------------------------
void TfRS232Main::FormDestroy(TObject *Sender)
{
    InitialOK=false;
    slRS232Log->Clear();
    delete slRS232Log;
    slRS232Log=NULL;                                                            //AI(W906-GB-P4) 20260926: the engine deletes the form after FormDestroy
                                                                                //   (Rs232Engine::Teardown); ~TfRS232Main frees only non-NULL
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: AppException is the Application->OnException hook (dropped in FormCreate); it is not
//   declared in Rs232Bridge.h and there is no VCL Exception type to receive.  Golden text kept.
#if 0 // TODO(W906-GB-P4): no VCL Application->OnException / Exception in-process (golden MainForm.cpp:398-405)
void __fastcall TfRS232Main::AppException(TObject *Sender, Exception *E)        //ChungHung 20141226 add catch exception
{
    AnsiString Str1, Str2;
    Str1.sprintf("Exception Code : %d", E->HelpContext);
    Str2.sprintf("Exception Message : %s", E->Message);
    ShowCommData("[Exception]",  Str1, Str2);
    return;
}
#endif
//---------------------------------------------------------------------------
void TfRS232Main::FormShow(TObject *Sender)
{
    AnsiString Str1, Str2="";                                                   //Jimmychiu 20240122 : add Rev cycle clear
    RS232Version=VerInfo().GetFileVersion();

    for(int i=0; i<USE_SITE_COUNT; i++)                                         //jou 2015-03-23 use site
    {
        MY_DUT_PAL.push_back(new TMyDutPanel(i));                               //AI(W906-GB-P4) 20260926: golden (palSite, i): Owner/Parent is layout only
    }
    ts_AVAGO->TabVisible=false;
    tsSPRD->TabVisible=false;

    iRevCycleClear          =CheckAndReadIniData(IniFileName, "SystemSetup", "iRevCycleClear", 5);  //Jimmychiu 20240122 : add Rev cycle clear
    iUseRS232Mode           =CheckAndReadIniData(IniFileName, "SystemSetup", "iTesterMode", 0);     //Isaac 20200903 :TTL RS232通訊
    TTL_CARD_TYPE           =CheckAndReadIniData(asHGeneralPath, "System", "TTL_CARD_TYPE", 0);
    CustomerCode            =CheckAndReadIniData(asHGeneralPath, "System", "CUSTOMER_CODE", 0);     //kevin 20141218
    iCheckClosedSiteHasBin  =CheckAndReadIniData(IniFileName, "SystemSetup", "bCheckClosedSiteHasBin", 1);     //Steven 20241205 : 啟用關site有bin檢查

    if(iUseRS232Mode>=InterfaceType_TTL)
    {
        if(TTL_CARD_TYPE==3)                                                    //20210920 Isaac : TTL RS232 兩塊板子(有站號)
        {
            iTTLBoardNum=2;
        }
        else if(TTL_CARD_TYPE==2)                                               //一塊板子
        {
            iTTLBoardNum=1;
        }
        else
        {
            iTTLBoardNum=0;
        }
    }
    else
    {
        iTTLBoardNum=0;
    }

    if(iTTLBoardNum==0)
    {
        LoadSetupData(CommTester);                                              //RS232模式
    }
    else if(iTTLBoardNum>=1)
    {
        LoadSetupData_TTL();                                                    //Isaac 20200903 :TTL RS232單板通訊
    }
    //=>standard
    cbDevice->Text=CommTester->CommName;

    cbBaudRate->Text=IntToStr(CommTester->BaudRate);

    //AI(W906-GB-P4) 20260926: the SPComm enumerators are spelled Spcomm::_5 ... Spcomm::Space here (golden: bare
    //   _5 ... Space via SPComm.hpp's `using namespace Spcomm`); same values, the qualification only keeps the short
    //   names from ever meeting another declaration inside namespace rs232std.
    if(CommTester->ByteSize==Spcomm::_5)
        cbByteSize->ItemIndex=0;
    else if(CommTester->ByteSize==Spcomm::_6)
        cbByteSize->ItemIndex=1;
    else if(CommTester->ByteSize==Spcomm::_7)
        cbByteSize->ItemIndex=2;
    else if(CommTester->ByteSize==Spcomm::_8)
        cbByteSize->ItemIndex=3;
    if(cbByteSize->ItemIndex>=0) VclComboSyncText(cbByteSize);                  //AI(W906-GB-P4) 20260926: VCL SetItemIndex also sets ->Text; -1 (.dfm) means
                                                                                //   no branch above ran, and VCL then leaves the .dfm Text alone

    if(CommTester->StopBits==Spcomm::_1)
        cbStopBit->ItemIndex=0;
    else if(CommTester->StopBits==Spcomm::_1_5)
        cbStopBit->ItemIndex=1;
    else if(CommTester->StopBits==Spcomm::_2)
        cbStopBit->ItemIndex=2;
    if(cbStopBit->ItemIndex>=0) VclComboSyncText(cbStopBit);                    //AI(W906-GB-P4) 20260926: see cbByteSize above

    if(CommTester->Parity==Spcomm::None)
        cbParity->ItemIndex=0;
    else if(CommTester->Parity==Spcomm::Odd)
        cbParity->ItemIndex=1;
    else if(CommTester->Parity==Spcomm::Even)
        cbParity->ItemIndex=2;
    else if(CommTester->Parity==Spcomm::TParity(3))                             //golden TParity(3) == Mark
        cbParity->ItemIndex=3;
    else if(CommTester->Parity==Spcomm::Space)
        cbParity->ItemIndex=4;
    if(cbParity->ItemIndex>=0) VclComboSyncText(cbParity);                      //AI(W906-GB-P4) 20260926: see cbByteSize above

    edReadIntervalTimeout->Text=CommTester->ReadIntervalTimeout;                //wei 20150212  add
    //<=standard

    //=>TTL用RS232通訊--------------------------------------------------------------------------------------------
    cbDevice_TTL->Text=ComNameTTL;                                              //Isaac 20200903 :TTL RS232通訊
    cbDevice_TTL_2->Text=ComNameTTL_2;                                          //Isaac 20210309 :TTL RS232兩塊板子

    cbBaudRate_TTL->Text="115200";                                              //TTL板子寫定115200    IntToStr(CommTester_TTL->BaudRate);
    cbBaudRate_TTL->Enabled=false;

    cbByteSize_TTL->ItemIndex=3;                                                //_8
    VclComboSyncText(cbByteSize_TTL);                                           //AI(W906-GB-P4) 20260926: VCL SetItemIndex also sets ->Text
    cbStopBit_TTL->ItemIndex=0;                                                 //_1
    VclComboSyncText(cbStopBit_TTL);                                            //AI(W906-GB-P4) 20260926: VCL SetItemIndex also sets ->Text
    cbParity_TTL->ItemIndex=0;                                                  //None
    VclComboSyncText(cbParity_TTL);                                             //AI(W906-GB-P4) 20260926: VCL SetItemIndex also sets ->Text
    edReadIntervalTimeout_TTL->Text=CommTester_TTL->ReadIntervalTimeout;        //wei 20150212  add
    //<=TTL用RS232通訊--------------------------------------------------------------------------------------------
    if(iTTLBoardNum==0)
    {
        OpenTesterComm();                                                       //Standard
    }

    if(iTTLBoardNum>=1)
    {
        OpenTesterComm_TTL(0);                                                  //Isaac 20200903 :TTL RS232通訊
    }

    if(iTTLBoardNum>=2)
    {
        OpenTesterComm_TTL(1);                                                  //Isaac 20210309 :TTL RS232兩塊板子
    }

    ShowVersion();
    ShowInterface();                                                            //Isaac 20200903 :TTL RS232通訊

#ifdef DEBUG                                                                    //Steven 20150410 : 新增Debug Mode資訊於畫面中
    labDebugMode->Visible=true;
    Str2.sprintf("DEBUG MODE");
#else
    labDebugMode->Visible=false;
#endif
    ShowCommData("[Program Start]",  RS232Version, Str2);

    if(CustomerCode==CC_ChipOn)
    {
        cbSaveLog->Checked=true;
        cbSaveLog->Enabled=true;
    }
    else
    {
        cbSaveLog->Checked=true;
    }
}
//---------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden (TObject *Sender, TCloseAction &Action); Action is never used by golden.
void TfRS232Main::FormClose(TObject *Sender)
{
    CloseTesterComm();
    CloseTesterComm_TTL(0);                                                     //Isaac 20200903 :TTL RS232通訊
    CloseTesterComm_TTL(1);                                                     //Isaac 20210309 :TTL RS232兩塊板子

    SaveBinData();

    for(std::vector<TMyDutPanel *>::iterator iter=MY_DUT_PAL.begin(); iter!=MY_DUT_PAL.end(); ++iter)   //AI(W906-GB-P4) 20260926: vector -> std::vector
    {
        delete *iter;
        *iter=NULL;                                                             //AI(W906-GB-P4) 20260926: freed here, ~TfRS232Main skips it
    }
    sBarCode->Clear();                                                          //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
    sBarCode_ASE_CL->Clear();                                                   //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    delete sBarCode;
    sBarCode=NULL;                                                              //AI(W906-GB-P4) 20260926: the engine deletes the form after FormClose, and a later
                                                                                //   engine start re-creates the list; ~TfRS232Main frees only non-NULL
    delete sBarCode_ASE_CL;                                                     //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    sBarCode_ASE_CL=NULL;                                                       //AI(W906-GB-P4) 20260926: see sBarCode above
    MY_DUT_PAL.clear();
}
//---------------------------------------------------------------------------
void TfRS232Main::ShowInterface()                                               //Isaac 20200903 :TTL RS232通訊
{
//    PageControl1->ActivePage=tsLog;
    PageControl1->ActivePageIndex=5;                                            //AI(W906-GB-P4) 20260926: the stand-in has only ActivePageIndex; tsLog is
                                                                                //   page 5 of PageControl1 (MainForm.dfm:62 pages ts_STD, ts_AVAGO, tsSPRD,
                                                                                //   tsVersion, tsTTL, tsLog; PageIndex counts hidden tabs too)
    if(iUseRS232Mode>=InterfaceType_TTL)
    {
        cbSaveLog_TTL->Checked=true;
        cbShowLog_TTL->Checked=true;
        cbSaveLog_TTL->Enabled=false;
        cbShowLog_TTL->Enabled=false;
        ts_STD->TabVisible=false;
        tsTTL->TabVisible=true;
    }
    else
    {
        cbSaveLog->Checked=true;                                                //wei 20150330 強制打開存檔不可修改
        cbShowLog->Checked=true;
        cbSaveLog->Enabled=false;
        cbShowLog->Enabled=false;
        ts_STD->TabVisible=true;
        tsTTL->TabVisible=false;
    }
}
//---------------------------------------------------------------------------

}  // namespace rs232std
