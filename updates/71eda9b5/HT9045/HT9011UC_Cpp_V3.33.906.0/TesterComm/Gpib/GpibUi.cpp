// ===========================================================================
//  TesterComm/Gpib/GpibUi.cpp -- the form side of the translated GPIB bridge (TSerialPoll + TMyDutPanel):
//  constructor with the Main.dfm design-time widget values, FormCreate / FormShow / FormClose, Timer1Timer,
//  the GPIB log (WriteLog / Save_Log), the general.ini LastData read/write, and the small checkbox / button
//  handlers.
//
//  AI(W906-GB-P1) 20260926: golden D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525 (Big5 -> UTF-8)
//    Main.cpp  109-304  TSerialPoll::TSerialPoll          (+ Main.dfm values, + ~TSerialPoll)
//              306-360  FormCreate / AppException (text only) / FormClose / GPIBSendToBack
//              362-617  FormShow
//              620-758  Timer1Timer / SaveResult / Save_Log / WriteLog / UpdateLed / ClearAllFlag
//             4289-4328 btnManualStartClick
//             4329-4619 ReadLastDataFile / WriteLastDataFile
//             4620-4637 chkUpperCaseClick / cbbGPIBTimoChange
//             4794-4810 chkStrLengthCheckClick / cbBinonEchoClick
//             4995-5002 cbFullSiteTimeOutClick
//             6582-6589 spbAutoRetestClick
//             7062-7097 ShowSimulateItem / btnRunModeClick
//             8096-8120 cbGPIBWriteWithout_r_nClick / btnUpdateClick / btnUpdateMouseDown / btnUpdateMouseUp
//             8294-8385 btnSaveLogClick / btnDiagZipClick
//    MyDutPanel.cpp 17-120 TMyDutPanel
//    Main.dfm   design-time values (Caption / Text / Items / ItemIndex / Checked / Visible / Interval / Enabled)
//  Rules: TesterComm/Gpib/TRANSLATION_RULES.md.  Every `#if 0 // TODO(W906-GB-P1)` below is a gate.
//
//  REAL FILES, BY DESIGN (golden paths kept): D:\GPIB9045\system\general.ini (read + write-back),
//  D:\GPIB9045\system\GpibString.dat (read), D:\HT9045\system\Gerneral.ini (read only), D:\GPIBLOG\Log\YYYY_MM\
//  (Save_Log), D:\GPIBLOG\Diag\ (btnDiagZipClick).  FormShow also does CreateDir("D:\gpib9045\system").
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"
#include "TesterComm/HandlerSettings.h"   // AI(W906-GB-P6) 20260926: ruling 4A (iUseGPIBFormat from the Handler)

#include <shellapi.h>     // golden Main.cpp:6 -- ShellExecute (btnDiagZipClick)
#include <cstdio>         // golden <stdio.h>  -- sprintf / fopen / fprintf (Save_Log, btnDiagZipClick)
#include <cstdlib>        // atoi (WriteLastDataFile)

namespace gpibbridge {

namespace {

// AI(W906-GB-P1) 20260926: VCL TControl's constructor sets Visible:=True and Enabled:=True (TTimer: Enabled:=True);
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

// AI(W906-GB-P1) 20260926: .dfm `Items.Strings = ( ... )` (NULL-terminated list).
void DfmItems(TStringList* list, const char* const* items)
{
    list->Clear();
    for (; *items != NULL; ++items)
        list->Add(*items);
}

// AI(W906-GB-P1) 20260926: VCL TCustomComboBox.SetItemIndex (CB_SETCURSEL) also puts Items[ItemIndex] into the
// edit text, and an index outside the list clears it.  The stand-in's ItemIndex and Text are plain members, so
// where golden code itself sets ItemIndex (FormShow, Timer1Timer) this is called right after, keeping the ->Text
// that golden reads next (e.g. "Last Set ==> GPIB Time Out : %s") equal to what VCL would show.
void VclComboSyncText(TComboBox* cb)
{
    if (cb->ItemIndex >= 0 && cb->ItemIndex < cb->Items->Count)
        cb->Text = AnsiString(cb->Items->Strings[cb->ItemIndex]);
    else
        cb->Text = "";
}

}  // namespace

//---------------------------------------------------------------------------
//  golden MyDutPanel.cpp:17-120 -- TMyDutPanel (one per site; web: HTWidgets.makeDutPanel).
//  TfMyPal (MyDutPanel.cpp:13-16) is golden's design-time template form and has no runtime role here.
//---------------------------------------------------------------------------
TMyDutPanel::~TMyDutPanel()
{
//    gpSite->Parent=NULL;
    //AI(W906-GB-P1) 20260926: no Parent chain headless.  Golden creates the five children with this TComponent as
    //   their Owner (`new TGroupBox(this)` ...), so TComponent's destructor frees them; done explicitly here.
    delete labOcr;
    delete cbSiteOn;
    delete plSite;
    delete cbBin;
    delete gpSite;
}
//---------------------------------------------------------------------------
TMyDutPanel::TMyDutPanel(int index)
{
    //AI(W906-GB-P1) 20260926: golden `TMyDutPanel(TComponent* Owner, int index): TComponent(Owner)`.  Owner (palSite)
    //   was only the Parent of gpSite.  VCL zero-fills a new instance, so _Enable starts false (golden never
    //   writes it).  Geometry / Name / Parent / Font / Align / Alignment are layout only (the web page draws the
    //   panel) and stay as golden text in comments.
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
    cbBin->Items->Add("0");
    cbBin->Items->Add("1");
    cbBin->Items->Add("2");
    cbBin->Items->Add("3");
    cbBin->Items->Add("4");
    cbBin->Items->Add("5");
    cbBin->Items->Add("6");     //Steven 20220616 : 新增6~15bin
    cbBin->Items->Add("7");
    cbBin->Items->Add("8");
    cbBin->Items->Add("9");
    cbBin->Items->Add("10");
    cbBin->Items->Add("11");
    cbBin->Items->Add("12");
    cbBin->Items->Add("13");
    cbBin->Items->Add("14");
    cbBin->Items->Add("15");
    cbBin->Items->Add("1..6");
    cbBin->Items->Add("0..16");

    //if(SerialPoll->iBinSelect >15  )    //kevin 20140305 256 bin
    cbBin->Items->Add("0..255");
    cbBin->Text="1";
    cbBin->ItemIndex=1;
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

//    str.sprintf("labOcr%02d", index);           //Steven 20150713 : for 2D Code
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
TSerialPoll::TSerialPoll()
{
    //AI(W906-GB-P1) 20260926: golden `TSerialPoll(TComponent* Owner) : TForm(Owner)`.  Before the golden body runs,
    //   VCL has (1) zero-filled the instance (TObject::NewInstance) and (2) streamed Main.dfm, creating every
    //   __published widget with its design-time values (TCustomForm constructor).  Both are reproduced first, in
    //   that order; the golden body then follows verbatim.  FormCreate / FormShow are NOT called here: GpibEngine
    //   calls ctor -> FormCreate -> (fDummyART / fRS232Main ctors) -> FormShow in golden WinMain order.

    // ---- (1) zero-filled instance: plain members ----
    dwStart                = 0;
    dwEnd                  = 0;
    bFind                  = false;
    iBinSelect             = 0;
    oldiBinSelect          = 0;
    bRETURN_GPIB_VERSION   = false;
    iCurrentArm            = 0;
    bDummyArt              = false;
    bMessageFromHandler    = false;
    bStsMessageFromHandler = false;
    slCmdList              = NULL;
    GPIB_TYPE              = 0;
    GPIB_ERROR_BIN         = 0;
    closeRequested         = false;     // V906 plumbing
    mailbox                = NULL;      // V906 plumbing: set by GpibEngine before FormCreate

    // ---- (2) Main.dfm ----
    //   Only values the stand-ins can hold are applied: Caption / Text / Items / ItemIndex / Checked / Visible /
    //   Enabled / Color / GroupIndex / Tab state / Timer Interval / TComm port settings.  Geometry, Align, Font,
    //   BevelInner / BevelOuter, ReadOnly, ScrollBars, Glyph, ImeName are layout only (the web page draws them).
    //   VCL defaults that differ from the stand-ins: TComboBox / TListBox / TRadioGroup ItemIndex = -1 and
    //   TPageControl ActivePageIndex = -1 unless the .dfm stores one; TTabSheet TabVisible = true.
    Caption = "GPIB";                                               // Main.dfm:6 (the golden body overwrites it)

    StatusBar1   = VclNew<TStatusBar>();                            // Main.dfm:157 (7 panels, no Text)

    PageControl1 = VclNew<TPageControl>();                          // Main.dfm:187
    PageControl1->ActivePageIndex = 0;                              //   ActivePage = TabSheet1 (page 0)

    TabSheet1 = VclNew<TTabSheet>();                                // Main.dfm:196
    TabSheet1->Caption    = "Main";
    TabSheet1->TabVisible = true;

    palSite = VclNew<TPanel>();                                     // Main.dfm:198
    palSite->Color = 8421440;

    gbSite = VclNew<TGroupBox>();                                   // Main.dfm:209 (golden design-time template)
    gbSite->Caption = "Site01";
    gbSite->Visible = false;

    labOcr = VclNew<TLabel>();                                      // Main.dfm:223
    labOcr->Caption = "OCR Text";

    cbBin = VclNew<TComboBox>();                                    // Main.dfm:238
    {
        static const char* const items[] = { "1", "2", "3", "4", "5", "6", "1..5", "1..10", "0..11", "0..255", NULL };
        DfmItems(cbBin->Items, items);
    }
    cbBin->ItemIndex = -1;
    cbBin->Text      = "1";

    cbSiteOn = VclNew<TCheckBox>();                                 // Main.dfm:266
    cbSiteOn->Checked = true;

    plSite = VclNew<TPanel>();                                      // Main.dfm:275
    plSite->Caption = "01";
    plSite->Color   = 0x00C0C0C0;                                   //   clSilver

    palButton = VclNew<TPanel>();                                   // Main.dfm:293
    palButton->Color = 13619102;

    Panel12 = VclNew<TPanel>();                                     // Main.dfm:302
    Panel12->Color = 13619102;

    ALed1  = VclNew<TALed>();  Label11 = VclNew<TLabel>();  Label11->Caption = "ERR";     // Main.dfm:311-324
    ALed2  = VclNew<TALed>();  Label12 = VclNew<TLabel>();  Label12->Caption = "TIMO";
    ALed3  = VclNew<TALed>();  Label13 = VclNew<TLabel>();  Label13->Caption = "END";
    ALed4  = VclNew<TALed>();  Label14 = VclNew<TLabel>();  Label14->Caption = "SRQI";
    ALed5  = VclNew<TALed>();  Label15 = VclNew<TLabel>();  Label15->Caption = "CMPL";
    ALed6  = VclNew<TALed>();  Label16 = VclNew<TLabel>();  Label16->Caption = "LOK";
    ALed7  = VclNew<TALed>();  Label17 = VclNew<TLabel>();  Label17->Caption = "REM";
    ALed8  = VclNew<TALed>();  Label18 = VclNew<TLabel>();  Label18->Caption = "CIC";
    ALed9  = VclNew<TALed>();  Label19 = VclNew<TLabel>();  Label19->Caption = "ATN";
    ALed10 = VclNew<TALed>();  Label20 = VclNew<TLabel>();  Label20->Caption = "TACS";
    ALed11 = VclNew<TALed>();  Label21 = VclNew<TLabel>();  Label21->Caption = "LACS";
    ALed12 = VclNew<TALed>();  Label22 = VclNew<TLabel>();  Label22->Caption = "DTAS";
    ALed13 = VclNew<TALed>();  Label23 = VclNew<TLabel>();  Label23->Caption = "DCAS";   // Main.dfm:479-492

    labPackageType_1 = VclNew<TLabel>();                            // Main.dfm:493
    labPackageType_1->Caption = "Package Type";
    labPackageType_1->Visible = false;

    labPackageType = VclNew<TLabel>();                              // Main.dfm:507
    labPackageType->Caption = "M1P";
    labPackageType->Visible = false;

    btnSaveLog = VclNew<TButton>();                                 // Main.dfm:521
    btnSaveLog->Caption = "Save Log";

    btnDiagZip = VclNew<TButton>();                                 // Main.dfm:530
    btnDiagZip->Caption = "Diag ZIP";

    Panel1 = VclNew<TPanel>();                                      // Main.dfm:540
    Panel1->Color = 13619102;

    lblTestTime = VclNew<TLabel>();                                 // Main.dfm:549
    lblTestTime->Caption = "Test Time :";

    Label1 = VclNew<TLabel>();                                      // Main.dfm:556
    Label1->Caption = "S";
    Label1->Visible = false;

    labDebugMode = VclNew<TLabel>();                                // Main.dfm:564
    labDebugMode->Caption = "!!DEBUG MODE!!";

    btnManualStart = VclNew<TButton>();                             // Main.dfm:579
    btnManualStart->Caption = "START";

    chkUpperCase = VclNew<TCheckBox>();                             // Main.dfm:588
    chkUpperCase->Caption = "UpperCase";

    cbbGPIBTimo = VclNew<TComboBox>();                              // Main.dfm:597
    {
        static const char* const items[] = { "T30ms", "T100ms", "T300ms", "T1s", "T3s", NULL };
        DfmItems(cbbGPIBTimo->Items, items);
    }
    cbbGPIBTimo->ItemIndex = 1;
    cbbGPIBTimo->Text      = "T100ms";

    chkStrLengthCheck = VclNew<TCheckBox>();                        // Main.dfm:614
    chkStrLengthCheck->Caption = "String length error";

    cbBinonEcho = VclNew<TCheckBox>();                              // Main.dfm:623
    cbBinonEcho->Caption = "ECHO BINON without SRQ0x41";
    cbBinonEcho->Checked = true;

    cbFullSiteTimeOut = VclNew<TCheckBox>();                        // Main.dfm:640
    cbFullSiteTimeOut->Caption = "Full Site Time Out";
    cbFullSiteTimeOut->Visible = false;

    cbbFullsiteTimeOut = VclNew<TComboBox>();                       // Main.dfm:656 (OnChange = cbbGPIBTimoChange, golden quirk)
    {
        static const char* const items[] = { "1", "3", "5", "10", "15", "20", "30", "40", "50", "60", "70", "80",
                                             "90", "100", "", NULL };
        DfmItems(cbbFullsiteTimeOut->Items, items);
    }
    cbbFullsiteTimeOut->ItemIndex = 3;
    cbbFullsiteTimeOut->Text      = "10";
    cbbFullsiteTimeOut->Visible   = false;

    pnlAMDCmd = VclNew<TPanel>();                                   // Main.dfm:684
    pnlAMDCmd->Color   = 13619102;
    pnlAMDCmd->Visible = false;

    Label24 = VclNew<TLabel>();                                     // Main.dfm:693
    Label24->Caption = "Label24";

    rgController = VclNew<TRadioGroup>();                           // Main.dfm:700 (Caption 'Tester Type': no stand-in member)
    {
        static const char* const items[] = { "Phase I", "Phase II", NULL };
        DfmItems(rgController->Items, items);
    }
    rgController->ItemIndex = 0;

    cbMode = VclNew<TComboBox>();                                   // Main.dfm:712
    {
        static const char* const items[] = { "TC", "TJ", "TS", NULL };
        DfmItems(cbMode->Items, items);
    }
    cbMode->ItemIndex = -1;
    cbMode->Text      = "cbMode";

    btnSendTemp = VclNew<TButton>();                                // Main.dfm:725
    btnSendTemp->Caption = "Send TC/TJ";

    chkTempReady = VclNew<TCheckBox>();                             // Main.dfm:734
    chkTempReady->Caption = "Temp Ready";
    chkTempReady->Visible = false;

    chkCMDLog = VclNew<TCheckBox>();                                // Main.dfm:743
    chkCMDLog->Caption = "CMD Log";
    chkCMDLog->Visible = false;

    btnRunMode = VclNew<TButton>();                                 // Main.dfm:753
    btnRunMode->Caption = "Send to Handler";

    cbbRunMode = VclNew<TComboBox>();                               // Main.dfm:762 (no ItemIndex stored -> -1)
    {
        static const char* const items[] = { "RECONTACT", "PLACELOAD", "TRAYFEED", "HANA", NULL };
        DfmItems(cbbRunMode->Items, items);
    }
    cbbRunMode->ItemIndex = -1;
    cbbRunMode->Text      = "TRAYFEED";

    cbGPIBWriteWithout_r_n = VclNew<TCheckBox>();                   // Main.dfm:776 (.dfm text is a literal backslash-r backslash-n)
    cbGPIBWriteWithout_r_n->Caption = "GPIB Write without \\r\\n";

    Button1 = VclNew<TButton>();                                    // Main.dfm:791
    Button1->Caption = "Button1";

    Panel2 = VclNew<TPanel>();                                      // Main.dfm:800
    Panel2->Color = 13619102;

    mmoBINON = VclNew<TMemo>();                                     // Main.dfm:809

    lstRecord = VclNew<TListBox>();                                 // Main.dfm:818
    lstRecord->ItemIndex = -1;

    palSCKART = VclNew<TPanel>();                                   // Main.dfm:828
    palSCKART->Color = 13619102;

    labLotID_1     = VclNew<TLabel>();  labLotID_1->Caption     = "Lot No. :";      // Main.dfm:837
    labLotID       = VclNew<TLabel>();  labLotID->Caption       = "0";              // Main.dfm:850
    labLotCount    = VclNew<TLabel>();  labLotCount->Caption    = "0";              // Main.dfm:863
    labLotCount_1  = VclNew<TLabel>();  labLotCount_1->Caption  = "Lot Count :";    // Main.dfm:876
    labSts_1       = VclNew<TLabel>();  labSts_1->Caption       = "Status :";       // Main.dfm:889
    labStatus      = VclNew<TLabel>();  labStatus->Caption      = "NONE";           // Main.dfm:902
    labTestCount_1 = VclNew<TLabel>();  labTestCount_1->Caption = "Test Count :";   // Main.dfm:915
    labTestCount   = VclNew<TLabel>();  labTestCount->Caption   = "0";              // Main.dfm:928

    spbAutoRetest = VclNew<TSpeedButton>();                         // Main.dfm:941 (AllowAllUp = True: no stand-in member)
    spbAutoRetest->Caption    = "Dummy ART";
    spbAutoRetest->GroupIndex = 1;

    gbAlarmCode = VclNew<TGroupBox>();                              // Main.dfm:1060
    gbAlarmCode->Caption = "Alarm Code";

    memoAlarmCode = VclNew<TMemo>();                                // Main.dfm:1074

    edHANACmd = VclNew<TEdit>();                                    // Main.dfm:1084

    pnlHanaART = VclNew<TPanel>();                                  // Main.dfm:1093
    pnlHanaART->Color = 8421440;

    grpHanaARTToHandler = VclNew<TGroupBox>();                      // Main.dfm:1104
    grpHanaARTToHandler->Caption = "To Handler";

    cbCmdHANAART = VclNew<TComboBox>();                             // Main.dfm:1120
    cbCmdHANAART->ItemIndex = -1;

    edCmdHANAART = VclNew<TEdit>();                                 // Main.dfm:1128

    btnSend_ED = VclNew<TButton>();                                 // Main.dfm:1135
    btnSend_ED->Caption = "Send_ED";

    btnHANA_SendCB = VclNew<TButton>();                             // Main.dfm:1144
    btnHANA_SendCB->Caption = "Send_CB";

    grpHanaARTToTester = VclNew<TGroupBox>();                       // Main.dfm:1154
    grpHanaARTToTester->Caption = "To Tester";

    cbCmdHANAARTtoTester = VclNew<TComboBox>();                     // Main.dfm:1170
    cbCmdHANAARTtoTester->ItemIndex = -1;

    edCmdHANAARTtoTester = VclNew<TEdit>();                         // Main.dfm:1178

    btnSend_EDToTester = VclNew<TButton>();                         // Main.dfm:1185
    btnSend_EDToTester->Caption = "Send_ED";

    btnHANA_SendCBTotester = VclNew<TButton>();                     // Main.dfm:1194
    btnHANA_SendCBTotester->Caption = "Send_CB";

    tsRS232 = VclNew<TTabSheet>();                                  // Main.dfm:1206
    tsRS232->Caption    = "RS232";
    tsRS232->TabVisible = true;

    PageControl2 = VclNew<TPageControl>();                          // Main.dfm:1209 (no pages)
    PageControl2->ActivePageIndex = -1;

    pgcRS232 = VclNew<TPageControl>();                              // Main.dfm:1225
    pgcRS232->ActivePageIndex = 0;                                  //   ActivePage = ts_STD_Log (page 0)

    ts_STD_Log = VclNew<TTabSheet>();                               // Main.dfm:1235
    ts_STD_Log->Caption    = "COM Log";
    ts_STD_Log->TabVisible = true;

    MemoLog = VclNew<TMemo>();                                      // Main.dfm:1237 (ReadOnly = True: no stand-in member)

    ts_STD_Setup = VclNew<TTabSheet>();                             // Main.dfm:1254
    ts_STD_Setup->Caption    = "Setup";
    ts_STD_Setup->TabVisible = true;

    GroupBox1 = VclNew<TGroupBox>();                                // Main.dfm:1257
    GroupBox1->Caption = "COM Port Setting";

    Label2 = VclNew<TLabel>();  Label2->Caption = "Baud Rate：";    // Main.dfm:1265 ('Baud Rate'#65306)
    Label3 = VclNew<TLabel>();  Label3->Caption = "Byte Size：";    // Main.dfm:1280
    Label4 = VclNew<TLabel>();  Label4->Caption = "Stop Bit：";     // Main.dfm:1295
    Label5 = VclNew<TLabel>();  Label5->Caption = "Parity：";       // Main.dfm:1310
    Label6 = VclNew<TLabel>();  Label6->Caption = "Device：";       // Main.dfm:1325

    cbBaudRate = VclNew<TComboBox>();                               // Main.dfm:1340
    {
        static const char* const items[] = { "4800", "7200", "9600", "14400", "19200", NULL };
        DfmItems(cbBaudRate->Items, items);
    }
    cbBaudRate->ItemIndex = -1;
    cbBaudRate->Text      = "cbBaudRate";

    cbByteSize = VclNew<TComboBox>();                               // Main.dfm:1361
    {
        static const char* const items[] = { "5", "6", "7", "8", NULL };
        DfmItems(cbByteSize->Items, items);
    }
    cbByteSize->ItemIndex = -1;
    cbByteSize->Text      = "cbByteSize";

    cbStopBit = VclNew<TComboBox>();                                // Main.dfm:1381
    {
        static const char* const items[] = { "1", "1.5", "2", NULL };
        DfmItems(cbStopBit->Items, items);
    }
    cbStopBit->ItemIndex = -1;
    cbStopBit->Text      = "cbStopBit";

    cbParity = VclNew<TComboBox>();                                 // Main.dfm:1400
    {
        static const char* const items[] = { "None", "Odd", "Even", "Mark", "Space", NULL };
        DfmItems(cbParity->Items, items);
    }
    cbParity->ItemIndex = -1;
    cbParity->Text      = "cbParity";

    cbDevice = VclNew<TComboBox>();                                 // Main.dfm:1421
    {
        static const char* const items[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                                             "COM10", "COM11", "COM12", "COM13", "COM14", "COM15", "COM16", "COM17",
                                             "COM18", NULL };
        DfmItems(cbDevice->Items, items);
    }
    cbDevice->ItemIndex = -1;
    cbDevice->Text      = "cbCOM";

    btnUpdate = VclNew<TPanel>();                                   // Main.dfm:1456 (a TPanel used as a button)
    btnUpdate->Caption = "Update";

    GroupBox14 = VclNew<TGroupBox>();                               // Main.dfm:1475
    GroupBox14->Caption = "Detail Settng";

    Label7 = VclNew<TLabel>();                                      // Main.dfm:1483 ('ReadIntervalTimeout '#65306)
    Label7->Caption = "ReadIntervalTimeout ：";

    edReadIntervalTimeout = VclNew<TEdit>();                        // Main.dfm:1498
    edReadIntervalTimeout->Text = "50";

    TabSheet3 = VclNew<TTabSheet>();                                // Main.dfm:1510
    TabSheet3->Caption    = "Version";
    TabSheet3->TabVisible = true;

    lblGPIBWnd = VclNew<TLabel>();                                  // Main.dfm:1513
    lblGPIBWnd->Caption = "lblGPIBWnd";

    Memo1 = VclNew<TMemo>();                                        // Main.dfm:1520

    Timer1 = VclNew<TTimer>();                                      // Main.dfm:1531 (OnTimer = Timer1Timer)
    Timer1->Enabled  = false;
    Timer1->Interval = 300;

    // Main.dfm:1537 / :1568.  DtrControl / RtsControl / Xon-Xoff limits and chars / Outx_Cts/DsrFlow /
    // total timeouts have no stand-in member (the .dfm values are the SPComm defaults the shim uses).
    CommAMD = new TComm(NULL);
    CommAMD->CommName            = "COM2";
    CommAMD->BaudRate            = 115200;
    CommAMD->ParityCheck         = false;
    CommAMD->Outx_XonXoffFlow    = true;
    CommAMD->Inx_XonXoffFlow     = true;
    CommAMD->ByteSize            = Spcomm::_8;
    CommAMD->Parity              = Spcomm::None;
    CommAMD->StopBits            = Spcomm::_1;
    CommAMD->ReadIntervalTimeout = 10;

    CommAMD2 = new TComm(NULL);
    CommAMD2->CommName            = "COM2";
    CommAMD2->BaudRate            = 115200;
    CommAMD2->ParityCheck         = false;
    CommAMD2->Outx_XonXoffFlow    = true;
    CommAMD2->Inx_XonXoffFlow     = true;
    CommAMD2->ByteSize            = Spcomm::_8;
    CommAMD2->Parity              = Spcomm::None;
    CommAMD2->StopBits            = Spcomm::_1;
    CommAMD2->ReadIntervalTimeout = 10;

    // Main.dfm:1564 / :1595 OnReceiveData = CommAMDReceiveData (both ports).
    //AI(W906-GB-P1) 20260926: rule 11 -- TComm fires OnReceiveData on its own reader thread, golden handles it on the
    //   form thread.  The callbacks only QueueRx(); GpibEngine::RunOnce -> DrainRx() dispatches to
    //   CommAMDReceiveData on the TesterComm thread.
    CommAMD->OnReceiveData  = [this](TObject*, void* b, Word n){ QueueRx(kRxCommAMD, b, n); };
    CommAMD2->OnReceiveData = [this](TObject*, void* b, Word n){ QueueRx(kRxCommAMD2, b, n); };

    TimerTMode = VclNew<TTimer>();                                  // Main.dfm:1599 (OnTimer = TimerTModeTimer)
    TimerTMode->Enabled  = false;
    TimerTMode->Interval = 10;

    // ---- golden body (Main.cpp:112-303) ----
    bFind=false;
    sBarCode=new TStringList();
    sBarCode_ASE_CL=new TStringList();                                          //KaiChen 20191126 ：中壢日月光，2D 回傳格式

    bFind           = false;                                                    //JerryYang 20160630 RS232加入GPIB
    Caption         = "RS232Standard";                                          //JerryYang 20160630 RS232加入GPIB

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
    slCmdList->Add("GETAICCD?");                                                //Sam 20240826 : Add GPIB GETAICCD?
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

    bHasSiteMapping=false;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden has no destructor body -- FormClose frees the lists and the dut panels, and
//   TComponent's destructor frees every owned widget.  Here FormClose NULLs what it frees (GpibEngine may call
//   FormClose and then delete the form, and a later engine start re-creates the lists), so this deletes only
//   what is still allocated: never a double free after FormClose, no leak when FormClose never ran.
//   CommAMD / CommAMD2 go first: their reader threads call QueueRx(this) until StopComm (in ~TComm) joins them.
TSerialPoll::~TSerialPoll()
{
    delete CommAMD;   CommAMD  = NULL;
    delete CommAMD2;  CommAMD2 = NULL;

    for (size_t i = 0; i < MY_DUT_PAL.size(); ++i)
    {
        if (MY_DUT_PAL[i] != NULL)
            delete MY_DUT_PAL[i];
    }
    MY_DUT_PAL.clear();
    if (slCmdList != NULL)
    {
        delete slCmdList;
        slCmdList = NULL;
    }
    if (sBarCode != NULL)               // created by this form's ctor (golden Main.cpp:113)
    {
        delete sBarCode;
        sBarCode = NULL;
    }
    if (sBarCode_ASE_CL != NULL)        // golden Main.cpp:114
    {
        delete sBarCode_ASE_CL;
        sBarCode_ASE_CL = NULL;
    }

    delete StatusBar1;
    delete PageControl1;
    delete TabSheet1;
    delete palSite;
    delete gbSite;
    delete labOcr;
    delete cbBin;
    delete cbSiteOn;
    delete plSite;
    delete palButton;
    delete Panel12;
    delete ALed1;   delete Label11;
    delete ALed2;   delete Label12;
    delete ALed3;   delete Label13;
    delete ALed4;   delete Label14;
    delete ALed5;   delete Label15;
    delete ALed6;   delete Label16;
    delete ALed7;   delete Label17;
    delete ALed8;   delete Label18;
    delete ALed9;   delete Label19;
    delete ALed10;  delete Label20;
    delete ALed11;  delete Label21;
    delete ALed12;  delete Label22;
    delete ALed13;  delete Label23;
    delete labPackageType_1;
    delete labPackageType;
    delete btnSaveLog;
    delete btnDiagZip;
    delete Panel1;
    delete lblTestTime;
    delete Label1;
    delete labDebugMode;
    delete btnManualStart;
    delete chkUpperCase;
    delete cbbGPIBTimo;
    delete chkStrLengthCheck;
    delete cbBinonEcho;
    delete cbFullSiteTimeOut;
    delete cbbFullsiteTimeOut;
    delete pnlAMDCmd;
    delete Label24;
    delete rgController;
    delete cbMode;
    delete btnSendTemp;
    delete chkTempReady;
    delete chkCMDLog;
    delete btnRunMode;
    delete cbbRunMode;
    delete cbGPIBWriteWithout_r_n;
    delete Button1;
    delete Panel2;
    delete mmoBINON;
    delete lstRecord;
    delete palSCKART;
    delete labLotID_1;
    delete labLotID;
    delete labLotCount;
    delete labLotCount_1;
    delete labSts_1;
    delete labStatus;
    delete labTestCount_1;
    delete labTestCount;
    delete spbAutoRetest;
    delete gbAlarmCode;
    delete memoAlarmCode;
    delete edHANACmd;
    delete pnlHanaART;
    delete grpHanaARTToHandler;
    delete cbCmdHANAART;
    delete edCmdHANAART;
    delete btnSend_ED;
    delete btnHANA_SendCB;
    delete grpHanaARTToTester;
    delete cbCmdHANAARTtoTester;
    delete edCmdHANAARTtoTester;
    delete btnSend_EDToTester;
    delete btnHANA_SendCBTotester;
    delete tsRS232;
    delete PageControl2;
    delete pgcRS232;
    delete ts_STD_Log;
    delete MemoLog;
    delete ts_STD_Setup;
    delete GroupBox1;
    delete Label2;
    delete Label3;
    delete Label4;
    delete Label5;
    delete Label6;
    delete cbBaudRate;
    delete cbByteSize;
    delete cbStopBit;
    delete cbParity;
    delete cbDevice;
    delete btnUpdate;
    delete GroupBox14;
    delete Label7;
    delete edReadIntervalTimeout;
    delete TabSheet3;
    delete lblGPIBWnd;
    delete Memo1;
    delete Timer1;
    delete TimerTMode;
}
//------------------------------------------------------------------------------
void TSerialPoll::FormCreate(TObject *Sender)
{
//    ReadLastDataFile();
    iCurrentArm=0;
    bDummyArt=false;                                                            //Steven 20170413 (wei) : Add ART simulator
    bMessageFromHandler=false;
    bStsMessageFromHandler=false;
//    Application->OnException=AppException;                                    //ChungHung 20141226 add catch exception
    //AI(W906-GB-P1) 20260926: dropped -- there is no VCL Application in-process; see AppException below.
}
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: AppException is the Application->OnException hook (dropped in FormCreate); it is not
//   declared in GpibBridge.h and has no VCL Exception type to receive.  Golden text kept.
#if 0 // TODO(W906-GB-P1): no VCL Application->OnException / Exception in-process (golden Main.cpp:316-322)
void __fastcall TSerialPoll::AppException(TObject *Sender, Exception *E)        //ChungHung 20141226 add catch exception
{
    AnsiString Str="";
    Str.sprintf("[Exception Error] %d: %s", E->HelpContext, E->Message);
    WriteLog(Str);
    return;
}
#endif
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden (TObject *Sender, TCloseAction &Action); Action is never used by golden.
void TSerialPoll::FormClose(TObject *Sender)
{
    AnsiString Str;
    #ifdef DEBUG
        Str.sprintf("xxx Program be Closed with DEBUG Mode @ Revision %s xxx", GPIBVersion);
    #else
        Str.sprintf("xxx Program be Closed @ Revision %s xxx", GPIBVersion);
    #endif
    WriteLog(Str);                                                              //Steven 20170413 (wei) : Add GPIB close log

    SaveResult();
    fRS232Main->SaveResult();
    WriteLastDataFile();
    WriteGpibString();
//    Thread1->Terminate();
    //AI(W906-GB-P1) 20260926: TMyThread is emulated by GpibEngine::RunOnce; after FormClose the engine
    //   stops calling Process (golden Unit2.cpp:40-66).
    for(std::vector<TMyDutPanel *>::iterator iter=MY_DUT_PAL.begin(); iter!=MY_DUT_PAL.end(); ++iter)   //AI(W906-GB-P1) 20260926: vector -> std::vector
    {
        delete *iter;
        *iter=NULL;                                                             //AI(W906-GB-P1) 20260926: freed here, ~TSerialPoll skips it
    }
    MY_DUT_PAL.clear();
    sBarCode->Clear();                                                          //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
    sBarCode_ASE_CL->Clear();                                                   //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    delete sBarCode;
    sBarCode=NULL;                                                              //AI(W906-GB-P1) 20260926: the engine may delete the form after FormClose, and a later
                                                                                //   engine start re-creates the list; ~TSerialPoll frees only non-NULL
    delete sBarCode_ASE_CL;                                                     //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    sBarCode_ASE_CL=NULL;                                                       //AI(W906-GB-P1) 20260926: see sBarCode above
    slCmdList->Clear();                                                         //Steven 20190520 : Add Command Log from Handler
    delete slCmdList;
    slCmdList=NULL;                                                             //AI(W906-GB-P1) 20260926: see sBarCode above
}
//------------------------------------------------------------------------------
void TSerialPoll::GPIBSendToBack()
{
    //AI(W906-GB-P1) 20260926: window z-order only.  There is no bridge window, and HMountWnd is a token (the
    //   mailbox address, GpibEngine::Start), not a real HWND -- SetForegroundWindow must not see it.
#if 0 // TODO(W906-GB-P1): no form window; HMountWnd is not a real HWND in-process (golden Main.cpp:353-360)
    SerialPoll->SendToBack();
    if(HMountWnd!=NULL)
    {
        SetForegroundWindow(HMountWnd);
    }
#endif
}
//------------------------------------------------------------------------------
void TSerialPoll::FormShow(TObject *Sender)
{
    AnsiString Buffer1, Buffer2, Str="";
    int Lenth=0;
    InitialOK=false;

    if(FileExists(LastSet.sLastFile))                                           //jou 2011-11-22 error  LastSet.sLastFile=Null
    {
        Lenth  =LastSet.sLastFile.Length();
        Buffer1=LastSet.sLastFile.SubString(12, Lenth);
    }
    else
    {
        Buffer1="";
    }

    GPIBVersion=VerInfo().GetFileVersion();
    CreateDir("D:\\gpib9045\\system");                                          //jou 2011-11-22 error mkdir -> CreateDir
    Buffer2 = "D:\\gpib9045"+ Buffer1;

    Memo1->Lines->Add("2026.09.26");                                            //Steven 20260926 : For HT9050
    Memo1->Lines->Add("* Add Handler Type : HT-9050.");
    Memo1->Lines->Add("2024.10.10");
    Memo1->Lines->Add("* Add Handler Type : HT-9046AU.");
    Memo1->Lines->Add("* Add Handler Type : HT-9046CR.");
    Memo1->Lines->Add("2023.03.23");
    Memo1->Lines->Add("* Add Handler Type : HT-1032.");
    Memo1->Lines->Add("* Add Handler Type : HT-1032AT.");
    Memo1->Lines->Add("2022.04.06");
    Memo1->Lines->Add("* Add Handler Type : HT-7080.");                         //Steven 20230323 : For HT7080
    Memo1->Lines->Add("2022.07.04");                                            // ben 20220704 : for HT505
    Memo1->Lines->Add("* Add Handler Type : HT-505.");
    Memo1->Lines->Add("2021.06.23");
    Memo1->Lines->Add("* Add Handler Type : HT-502.");
    Memo1->Lines->Add("2020.10.22");
    Memo1->Lines->Add("* Support Qorvo protocol.");
    Memo1->Lines->Add("2020.09.23");
    Memo1->Lines->Add("* Support HonPrec TTL Board communication");
    Memo1->Lines->Add("2019.11.12");
    Memo1->Lines->Add("* Support Samsung protocol.");
    Memo1->Lines->Add("2019.04.09");
    Memo1->Lines->Add("* Support RS232 communication");
    Memo1->Lines->Add("2018.12.14");
    Memo1->Lines->Add("* Support T6577 protocol.");
    Memo1->Lines->Add("2017.09.12");                                            //wei 20170911 (steven) State Record
    Memo1->Lines->Add("* State Record ");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2017.08.01");                                            //kevin 20170801
    Memo1->Lines->Add("* empty char 'G' LISTEN:BINON:GGGGGGGG,GGGGGGGG,GGGGGGGG,GGGGGGG2; ");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2017.01.24");
    Memo1->Lines->Add("* Support GS 16 and 32 bin protocol.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2016.10.25");
    Memo1->Lines->Add("* Support ART function.");
    Memo1->Lines->Add("2016.03.01");
    Memo1->Lines->Add("* Support Test Arm Pos and EP function.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2015.12.25");
    Memo1->Lines->Add("* Support 2D code/OCR function.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2012.10.03");
    Memo1->Lines->Add("* Support 32 Site.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2012.07.26");
    Memo1->Lines->Add("* add Handler ID funtion.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2011.07.28");
    Memo1->Lines->Add("* Save GpibString at anotherfile.");
    Memo1->Lines->Add("");
    Memo1->Lines->Add("2011.04.19");
    Memo1->Lines->Add("* Integrate GPIB9045 & GPIB9046.");
    Memo1->Lines->Add("* Use INI file as database.");
    Memo1->Lines->Add("");

    for(int i=0; i<TOTAL_SITE; i++)
    {
        MY_DUT_PAL.push_back(new TMyDutPanel(i));                              //AI(W906-GB-P1) 20260926: golden (palSite, i): Owner/Parent is layout only
    }

    LastSet.sLastFile=Buffer2;
    try
    {
        ReadLastDataFile();
    }
    catch(...)
    {
    }

    GetComputerName(PcName, &PcNameLen);                                        //Steven 20110131
    StatusBar1->Panels->Items[0]->Text="Revision: "+GPIBVersion;                //Steven 20120828 : 改用Revision當版號

    switch(LastSet.iTimeOut)                                                    //Steven 20110419 Start
    {
        case 8:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 30ms";
            break;
        case 9:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 100ms";
            break;
        case 10:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 300ms";
            break;
        case 11:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 1s";
            break;
        case 12:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 3s";
            break;
        case 13:
            StatusBar1->Panels->Items[1]->Text=" Time Out: 10s";
            break;
    }
    //cbbGPIBTimo->ItemIndex=LastSet.iTimeOut-11;                               //jou 2013-05-21 不能低於11，會出現錯誤
    cbbGPIBTimo->ItemIndex=LastSet.iTimeOut-8;                                  // ChungHung 20141103 fix GPIB can not change time out blow 1s , 會出現錯誤主要是Time out 不吻合 不能卡掉 TSMC 需要設定300ms 不然要會有GPIB time out 產生
    VclComboSyncText(cbbGPIBTimo);                                              //AI(W906-GB-P1) 20260926: VCL SetItemIndex also sets ->Text (read below)

    if(LastSet.GpibAddress<=0 || LastSet.GpibAddress>=31)
        LastSet.GpibAddress=1;
    GpibAddress=LastSet.GpibAddress;

    StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);

    if(LastSet.bStringLength)
        StatusBar1->Panels->Items[3]->Text="Enable check string length.";
    else
        StatusBar1->Panels->Items[3]->Text="Disable check string length.";

    if(LastSet.bUpperCase)
        StatusBar1->Panels->Items[4]->Text="Use upper case.";
    else
        StatusBar1->Panels->Items[4]->Text="Use Lower case.";

    StatusBar1->Panels->Items[5]->Text=IntToStr(SerialPoll->iBinSelect)+" Bin"; //kevin 20140318
    oldiBinSelect=SerialPoll->iBinSelect;

    StatusBar1->Panels->Items[6]->Text="Enable record data.";
    chkUpperCase->Checked=LastSet.bUpperCase;
    chkStrLengthCheck   ->Checked=LastSet.bStringLength;
    cbBinonEcho->Visible=(CustomerCode!=CC_ASE_KaohSiung);                      //kevin 20120828 不顯示  //Steven 20141212 : 改用客戶代碼
    if(cbBinonEcho->Visible==false)
    {
        LastSet.bBinonEcho=true;
        cbBinonEcho->Checked=true;
    }
    else
    {
        cbBinonEcho->Checked=LastSet.bBinonEcho;
    }

    cbGPIBWriteWithout_r_n->Checked=LastSet.bGPIBWriteWithout_r_n;              //Jason 20221005 : 增加 GPIB Write結尾不要加/r/n的選項//

    palSCKART->Visible=CheckAndReadIniData("D:\\GPIB9045\\system\\general.ini", "Auto Retest", "ART Simulator", 0);    //Steven 20171017 : For支援SCK ART的通知GPIB顯示模擬畫面
    InitialStartValue();

    if(bEnableThread==false)
    {
        bEnableThread=true;
//        Thread1 = new TMyThread(true);
//        Thread1->Resume();
//        Thread1->Priority=tpTimeCritical;
        //AI(W906-GB-P1) 20260926: TMyThread is emulated by GpibEngine::RunOnce on the TesterComm thread
        //   (runs Process while bEnableThread).
    }
    Timer1->Enabled=true;
    #ifndef DEBUG
    GPIBSendToBack();
    #endif

#if 0 // TODO(W906-GB-P1): form position -- there is no form window (golden Main.cpp:528-529)
    Top=0;
    Left=0;
#endif
//    PageControl1->ActivePage=TabSheet1;
    PageControl1->ActivePageIndex=0;                                            //AI(W906-GB-P1) 20260926: the stand-in has only ActivePageIndex;
                                                                                //   TabSheet1 is page 0 (Main.dfm:196)
    InitialOK=true;

#if 0 // TODO(W906-GB-P1): listbox horizontal scroll extent -- no list box window (golden Main.cpp:533)
    SendMessage(lstRecord->Handle, LB_SETHORIZONTALEXTENT, 900, 0);             //kevin 20140305 list ADD X SCROSS
#endif

    szMachineType.sprintf("%s\r\n", CheckAndReadIniData(asHGeneralPath, "Version", "Model", AnsiString("HT-9046")));
    szHadnlerID.sprintf("%s\r\n",   CheckAndReadIniData(asHGeneralPath, "Version", "Machine ID", AnsiString("29828")));
    //AI(W906-GB-P6) 20260926: user ruling 4A (decision #5 Q4 暫照建議 A) -- only the Handler's config decides
    //  (IniConfig.iI25UseGPIBFormat, published through HandlerSettings.h); the engine's own general.ini copy is no longer
    //  read.  No Handler publisher (-1, engine-only ctest) -> golden's default 0.  GpibCommands.cpp HANDLERID? also reads
    //  the live value (W906_UseGPIBFormat).
    { const int hs=testercomm::HsUseGPIBFormat().load(); iUseGPIBFormat=(hs>=0) ? hs : 0; }
#if 0 // AI(W906-GB-P6) 20260926: golden text, replaced by the line above (ruling 4A)
    iUseGPIBFormat=CheckAndReadIniData(asGeneralPath, "Temperature", "iUseGPIBFormat", 0); //kevin 20130705 台積電通訊規格  0:HT    1:NS    //Steven 20140920 : For 矽格阮瑋民的要求,不能用Handler ID
#endif
    sGPIBVersion.sprintf("NEWOI-%s-HT9045W", GPIBVersion);                      //wei 20160115 矽格版本

    #ifdef DEBUG
        labDebugMode->Visible=true;                                             //Steven 20150410 : 紀錄使用版本與是否為Debug Mode
        if(LastSet.i2DIDFormat==eAMD)                                           //JerryYang 20200422 2DID format選項改用下拉選單
        {
            chkTempReady->Visible=true;                                         //Ifor 20181225 : add 直接回覆 Temp Ready 不等ATC
            chkCMDLog->Visible=true;
            pnlAMDCmd->Visible=true;                                            //Ifor 20190304 :add TJ/TC 手動切換命令僅Debug 模式下可執行
            labDebugMode->Caption="ATC Connect Err";
        }
        Str.sprintf("=== Program On with DEBUG Mode @ Revision %s ===", GPIBVersion);       //steven 20150626 %d-->%s
    #else
        labDebugMode->Visible=false;
        Str.sprintf("=== Program On @ Revision %s ===", GPIBVersion);           //steven 20150626 %d-->%s
        pnlAMDCmd->Visible=false;                                               //Ifor 20190304 :add TJ/TC 手動切換命令僅Debug 模式下可執行
    #endif
    WriteLog(Str);

    Str.sprintf("Last Set ==> GPIB Time Out : %s", cbbGPIBTimo->Text);          //wei 20150409 Add Close GPIB Command
    WriteLog(Str);

    if(bSimulate)
        WriteLog("Last Set ==> Simulate GPIB");
    else
        WriteLog("Last Set ==> Normal GPIB");

    WriteLog(AnsiString("Last Set ==> ADDR : "+AnsiString(LastSet.GpibAddress)));

    if(bGpibMode)
        WriteLog("Last Set ==> GPIB MODE");
    else
        WriteLog("Last Set ==> Other MODE");

    WriteLog(AnsiString("Last Set ==> Support Bin : 0 ~ "+AnsiString(iBinSelect-1)));

    if(LastSet.bUpperCase)
        WriteLog("Last Set ==> Upper Case is On");
    else
        WriteLog("Last Set ==> Upper Case is Off");

    WriteLog("Last Set ==> Record Data is On");

    if(LastSet.bStringLength)
        WriteLog("Last Set ==> String Length is On");
    else
        WriteLog("Last Set ==> String Length is Off");

    if(LastSet.bBinonEcho)
        WriteLog("Last Set ==> BINON ECHO is On");
    else
        WriteLog("Last Set ==> BINON ECHO is Off");

    if(LastSet.bFullSiteTimeOut)
        WriteLog("Last Set ==> Full Site Time Out is On");
    else
        WriteLog("Last Set ==> Full Site Time Out is Off");

    InitialBarcodeList();

    if(LastSet.i2DIDFormat==eAMD)                                               //JerryYang 20200422 2DID format選項改用下拉選單
    {                                                                           //Ifor 20190304 : AMD 需開啟COM PORT 18 與ATC 通訊
        try
        {
            CommAMD->StopComm();
            CommAMD->CommName="\\\\.\\"+ATC_AMD_COM;                                // ben 20220704 : 改為讀 Ini
            CommAMD->StartComm();

            TimerTMode->Enabled=true;
            ZeroMemory(bCheckDiodeThermal, sizeof(bCheckDiodeThermal));             //Ifor 20190528 : add Diode Thermal Check
            labDebugMode->Visible=false;
        }
        catch(...)
        {
        }
    }

    fRS232Main->LoadSetupData();
    fRS232Main->OpenTesterComm();
}
//------------------------------------------------------------------------------
// golden Main.cpp:619 `Word OldSystemSec;` is defined in GpibGlobals.cpp (extern in GpibBridge.h).
void TSerialPoll::Timer1Timer(TObject *Sender)
{
    static bool flag=true;
    //AI(W906-GB-P1) 20260926: re-arm the golden statics for a new bridge life (golden: a relaunched exe; see
    //   g_bridgeLife in GpibBridge.h).
    static unsigned long s_life=0;
    if(s_life!=g_bridgeLife)
    {
        s_life=g_bridgeLife;
        flag=true;
    }

    if(flag)                                                                    //得在Close() 之前做完
    {
        for(int i=0; i<TOTAL_SITE; i++)                                         //kevin 20140305
        {
            MY_DUT_PAL[i]->cbBin->ItemIndex=1;                                  //Steven 20220616 : 0 --> 1
            VclComboSyncText(MY_DUT_PAL[i]->cbBin);                             //AI(W906-GB-P1) 20260926: VCL SetItemIndex also sets ->Text
        }
        flag=false;
        GPIBSendToBack();
    }

    if(LastSet.MachName=="9046_32GPIB"      ||
       LastSet.MachName=="9045GPIB"         ||
       LastSet.MachName=="9046GPIB"         ||
       LastSet.MachName=="9045GPIB_12Site"  ||                                  //ChungHung 20130507 add HT9045 updata for 12site
       LastSet.MachName=="2601GPIB"         ||                                  //2012-03-02    Dell for HT-2601
       LastSet.MachName=="9055GPIB"         ||                                  //Steven 20180205 : Add for HT9055
       LastSet.MachName=="502GPIB"          ||
       LastSet.MachName=="7080GPIB"         ||                                  //Steven 20230323 : For HT7080
       LastSet.MachName=="9050GPIB"         ||                                  //Steven 20260926 : For HT9050
       LastSet.MachName=="1032GPIB")                                            //Steven 20230323 : For HT1032
    {
        Caption=LastSet.MachName;
    }
    else
    {                                                                           //如果改錯機台名稱 會強制改9045
        Caption=LastSet.MachName;
//        WriteIniValueData(asGeneralPath, "Version", "Model", "9045GPIB");     //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
        WriteLog("GPIB : No handler window, close GPIB,  _Timer1_");
        RequestClose("Timer1Timer");                                            //golden Close();
    }

    TDateTime dtPresent = Now();
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
    if(OldSystemSec!=SystemSec)
    {
        OldSystemSec=SystemSec;
        ProcessHMountConnect();
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::SaveResult()
{
    try
    {
        Save_Log();                                                             //kevin 20130516 add record
    }
    catch(...)
    {
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::Save_Log()                                                    //kevin 20130516 add record
{
    TDateTime tt= Now();
    Word SystemYear, SystemMonth, SystemDate;
    DecodeDate(tt, SystemYear, SystemMonth, SystemDate);
    AnsiString str1, str2;

    str1.sprintf("%04d%s%02d%s", SystemYear, "_", SystemMonth, "\\");
    GetTimeInfo();
    sprintf(cAlarmTime, "%04d-%02d-%02d %02d %02d %02d",
                SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    FILE * pFile;
    AnsiString fp="", s_cFolder;
    sFolder.sprintf("%s%s", "D:\\GPIBLOG\\","Log\\");

    if(sFolder.SubString(sFolder.Length(),1)!="\\")
        sFolder=sFolder+"\\";

    s_cFolder= sFolder + str1;
    fp= s_cFolder + str2;

    if(!(DirectoryExists(s_cFolder)))
        ForceDirectories(s_cFolder);

    LastSet.sLastFile=s_cFolder+AnsiString(cAlarmTime)+".txt";                  //kevin
    lstRecord->Items->SaveToFile(LastSet.sLastFile);
}
//------------------------------------------------------------------------------
void TSerialPoll::WriteLog(AnsiString str1)                          //Steven 20170111 (Jou) : 把WriteResult & WriteLog分開
{
    AnsiString str2;                                                            //Steven 20160215 : 把 char 改成 AnsiString, 避免記憶體用太多或是溢位
    AnsiString str=str1.Trim();

    if(str!="")
    {
        GetTimeInfo();
        str2.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, str);   //Steven 20150330 : GPIB Log時間格式改跟Handler Event Log一樣
        lstRecord->Items->Add(str2);
        lstRecord->ItemIndex=lstRecord->Items->Count-1;
    }
    else
    {
        lstRecord->Items->Add(str);
        lstRecord->ItemIndex=lstRecord->Items->Count-1;
    }

    if(lstRecord->Items->Count>10240)
    {
        SaveResult();
        lstRecord->Clear();
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::UpdateLed()
{
    ALed1->Value =ibsta&0x8000;
    ALed2->Value =ibsta&0x4000;
    ALed3->Value =ibsta&0x2000;
    ALed4->Value =ibsta&0x1000;
    ALed5->Value =ibsta&0x100;
    ALed6->Value =ibsta&0x80;
    ALed7->Value =ibsta&0x40;                                                   //REM
    ALed8->Value =ibsta&0x20;                                                   //CIC
    ALed8->Value =ibsta&0x10;                                                   //ATN
    ALed9->Value =ibsta&0x8;
    ALed10->Value=ibsta&0x4;                                                    //TACS
    ALed11->Value=ibsta&0x2;                                                    //LACS
    ALed11->Value=ibsta&0x1;
}
//------------------------------------------------------------------------------
void TSerialPoll::ClearAllFlag()                                                //Steven 20170215 (wei) 清除所有flag
{
    WriteLog("Clear All Flag");
    iMainTask=1;
    iGbibTask=1;
    LastSet.bFULLSITES=false;                                                   //kevin 20130516 //Steven 20150303 : 避免停止測試還收到Binon
    LastSet.bSQR41=false;                                                       //Steven 20161025 : 確保設定的資料有存起來
    LastSet.bHasBarCode=false;
    LastSet.bSRQC0=false;                                                       //jou 2015-09-21 Auto Retest function
    LastSet.bFlagRCMD=false;                                                    //jou 2015-09-21 Auto Retest function
    LastSet.bFlagSVID=false;                                                    //jou 2015-09-21 Auto Retest function
    LastSet.bFlagECID=false;                                                    //jou 2015-09-21 Auto Retest function
    IsTest=false;
    InitialBarcodeList();
    WriteLastDataFile();                                                        //Steven 20161025 : 確保設定的資料有存起來
}
//------------------------------------------------------------------------------
void TSerialPoll::btnManualStartClick(TObject *Sender)
{
    AnsiString Str;
    if(IsTest==true)
        return;

    TDateTime dtPresent = Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
    for(int i=0; i<TOTAL_SITE; i++)
    {
        if(MY_DUT_PAL[i]->cbSiteOn->Checked)
        {
            Str.sprintf("Y%04dM%02dD%02dH%02dM%02dS%02dSiteNo%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, i+1);
            sBarCode->Strings[(TOTAL_SITE-1)-i]=Str;
            MY_DUT_PAL[i]->labOcr->Caption=Str;
            iStart[i]=1;
        }
        else
        {
            iStart[i]=0;
            sBarCode->Strings[(TOTAL_SITE-1)-i]="0";
            MY_DUT_PAL[i]->labOcr->Caption="0";
        }
    }

    bSimulate=false;
    bNeedInital=false;
    IsTest=true;
    WriteLog("Manual Start Test");                                              //ChungHung 20150213 add record manual send Start Test

    if(GpibAddress!=oldGpibAddress)
    {
        LastSet.GpibAddress=GpibAddress;
        StatusBar1->Panels->Items[2]->Text=" Address: "+AnsiString(LastSet.GpibAddress);
        WriteLastDataFile();
    }
    dwStart=GetTickCount();
    lstRecord->SetFocus();                                                      //kevin 20140307
}
//------------------------------------------------------------------------------
//  讀取設定資料檔
//------------------------------------------------------------------------------
bool TSerialPoll::ReadLastDataFile()
{
    bool bTempAMD=false;

    CustomerCode=CheckAndReadIniData(asHGeneralPath, "System", "CUSTOMER_CODE", 0);//kevin 20141218
    LastSet.MachName        = CheckAndReadIniData(asGeneralPath, "Version",     "Model",        AnsiString("ModelNG")); //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    LastSet.bStringLength   = CheckAndReadIniData(asGeneralPath, "SystemSetup", "StringLength", true);

    if(FindWindow("TfMain", "HT505")==NULL)
    {
        ATC_AMD_COM=CheckAndReadIniData(asGeneralPath, "ATC",     "AMD COM",    AnsiString("COM18"));   // ben 20220704 : for HT505
    }
    else
    {
        ATC_AMD_COM=CheckAndReadIniData(asGeneralPath, "ATC",     "AMD COM",    AnsiString("COM3"));   // ben 20220704 : for HT505
    }

    LastSet.iTesterMode     = CheckAndReadIniData(asGeneralPath, "SystemSetup", "iTesterMode",  InterfaceType_ADVAN_Type1);
    if(GPIBVersionCheck<6 &&                                                    //jou 20170209 (Steven) : 修正Tester mode 存檔異常
       (LastSet.iTesterMode==InterfaceType_16BinGS ||
        LastSet.iTesterMode==InterfaceType_32BinGS))
    {
        RequestClose("ReadLastDataFile");                                       //golden Close();
    }
    else if(GPIBVersionCheck<8 &&
            LastSet.iTesterMode==InterfaceType_15BinT6577)                      //Steven 20181214 : ASE-CL add T6577
    {
        RequestClose("ReadLastDataFile");                                       //golden Close();
    }
    else if(GPIBVersionCheck<12.06 &&
            LastSet.iTesterMode==InterfaceType_15BinQorvo)                      //Steven 20201022 : Add Qorvo protocol
    {
        RequestClose("ReadLastDataFile");                                       //golden Close();
    }

    iATC_Use_Heat_Count=CheckAndReadIniData(asHGeneralPath, "ATC", "ATC_SYSTEM_USEHEAT", 4);    //Ifor 20201023 add: ATC 控制器數量

    LastSet.bGpib8080       = CheckAndReadIniData(asGeneralPath, "SystemSetup", "b8080",        true);
    LastSet.GpibAddress     = CheckAndReadIniData(asGeneralPath, "SystemSetup", "GpibAddress",  1);
    LastSet.iTimeOut        = CheckAndReadIniData(asGeneralPath, "SystemSetup", "TimeOut",      11);
    LastSet.sLastFile       = CheckAndReadIniData(asGeneralPath, "SystemSetup", "LastFile",     AnsiString(""));
//    LastSet.bRecordDatd     = CheckAndReadIniData(asGeneralPath, "SystemSetup", "RecordDatd",   false);
    LastSet.bUpperCase      = CheckAndReadIniData(asGeneralPath, "SystemSetup", "Upper Case",   true);      //kevin 20110502
    LastSet.bSQR41          = CheckAndReadIniData(asGeneralPath, "Run Status",  "SRQ41",        false);     //Steven 20110909
    LastSet.bBinonEcho      = CheckAndReadIniData(asGeneralPath, "SystemSetup", "Binon Echo",   true);      //Steven 20111219
    SerialPoll->iBinSelect  = CheckAndReadIniData(asGeneralPath, "SystemSetup", "BinTotal",  16);           //kevin 20140318 add bin
    LastSet.bUseBarcodeFunction=CheckAndReadIniData(asGeneralPath, "SystemSetup", "bUseBarcodeFunction",      false);       //Steven 20150713 : Add 2D code
    LastSet.bAlarmWhen2DFuncMisMatch=CheckAndReadIniData(asGeneralPath, "Run Status", "bAlarmWhen2DFuncMisMatch", true);    //Steven 20211109 : 當測試機問BARCODE?, 但是機台沒有開功能時, 要跳ALARM
    LastSet.bSRQC0          = CheckAndReadIniData(asGeneralPath, "Run Status",  "SRQC0",        false);     //jou 2015-09-21 Auto Retest function
    LastSet.bFlagRCMD       = CheckAndReadIniData(asGeneralPath, "Run Status",  "bFlagRCMD",    false);     //jou 2015-09-21 Auto Retest function
    LastSet.bFlagSVID       = CheckAndReadIniData(asGeneralPath, "Run Status",  "bFlagSVID",    false);     //jou 2015-09-21 Auto Retest function
    LastSet.bFlagECID       = CheckAndReadIniData(asGeneralPath, "Run Status",  "bFlagECID",    false);     //jou 2015-09-21 Auto Retest function
    LastSet.iTesterType     = CheckAndReadIniData(asGeneralPath, "ART",         "iTesterType",  0);
    LastSet.iTACS_ATNTimeOut= CheckAndReadIniData(asGeneralPath, "SystemSetup", "iTACS_ATNTimeOut",  10);   //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
    LastSet.iMyGpibWriteRetry      = CheckAndReadIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteRetry",      20);   //Steven 20260428 : MyGPIBWrite 總重試次數
    LastSet.iMyGpibWriteThreshold  = CheckAndReadIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteThreshold",  10);   //Steven 20260428 : MyGPIBWrite 進入 LACS/Reset 判斷的門檻
    LastSet.iMyGpibWriteWaitMS     = CheckAndReadIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteWaitMS",     100);  //Steven 20260428 : MyGPIBWrite 每次重試 SleepEx 毫秒
    LastSet.bMyGpibWriteVerboseLog = CheckAndReadIniData(asGeneralPath, "SystemSetup", "bMyGpibWriteVerboseLog", false);//Steven 20260428 : MyGPIBWrite 詳細 log
    LastSet.bDummyART       = CheckAndReadIniData(asGeneralPath, "Dummy ART",  "Running Status", false);
    LastSet.iDummyARTTask   = CheckAndReadIniData(asGeneralPath, "Dummy ART",  "ART Task",       1);
    LastSet.iLotCount       = CheckAndReadIniData(asGeneralPath, "Dummy ART",  "Lot Count",      0);
    LastSet.sLotID          = CheckAndReadIniData(asGeneralPath, "Dummy ART",  "Lot ID", AnsiString(""));
    bTempAMD                = CheckAndReadIniData(asGeneralPath, "SystemSetup", "bUseAMDFunction",      false);
    LastSet.i2DIDFormat     = CheckAndReadIniData(asGeneralPath, "SystemSetup", "i2DIDFormat",      int(bTempAMD)); //JerryYang 20200422 2DID format
    LastSet.bConfigureSRQ      =CheckAndReadIniData(asGeneralPath, "Configure", "SRQ",                 false);     //Steven 20201022 : For RFMD
    LastSet.bHaveContactorInfo =CheckAndReadIniData(asGeneralPath, "Configure", "bHaveContactorInfo",  false);     //Steven 20201022 : For RFMD
    LastSet.bEnableManualStart =CheckAndReadIniData(asGeneralPath, "SystemSetup", "Enable_Start_Button", true);    //Steven 20210415 : 加入手動Start的限制
    LastSet.bRunHANA_ART    =CheckAndReadIniData(asGeneralPath, "ART", "bRunHANA_ART", false);      //JimmyChiu 20241023 HANA ART Function

    if(CustomerCode==CC_HANA_MICRON)                                            //JimmyChiu 20250214 : For Hana ART
    {
        LastSet.bRunHANA_ART=true;
        LastSet.bTryHANA_ART=CheckAndReadIniData(asGeneralPath, "Hana ART", "Try Run ART", false);      //Steven 20250414 : HANA ART Function, 手動測試用
    }
    else
    {
        LastSet.bTryHANA_ART=false;
    }

    InitHANA_ART();                                                             //Steven 20250414 : HANA ART Function
    btnManualStart->Enabled    =LastSet.bEnableManualStart;

    LastSet.bGPIBWriteWithout_r_n   = CheckAndReadIniData(asGeneralPath, "SystemSetup", "bGPIBWriteWithout_r_n",   false);      //Jason 20221005 : 增加 GPIB Write結尾不要加/r/n的選項//

    fDummyART->edDummyCount->Text=LastSet.iLotCount;
    fDummyART->edLotID->Text=LastSet.sLotID;

    iBinSelect=SerialPoll->iBinSelect;                                          //kevin 20140305 256 bin
    bRETURN_GPIB_VERSION    = CheckAndReadIniData(asGeneralPath, "SystemSetup", "RETURN_GPIB_VERSION",  0);     //wei 20160115 矽格版本

    LastSet.sPackageType    = CheckAndReadIniData(asGeneralPath, "SystemSetup", "PackageType",     AnsiString("M1P"));//add: 海光要求Package Type
    labPackageType->Caption =LastSet.sPackageType;
    bA10_3_Enable           = CheckAndReadIniData(asGeneralPath, "SystemSetup",  "bA10_3_Enable",        false);    //Jimmychiu 20231205 : 借用變數bTimeOutProcess，當作A10-3開啟判斷
    if(LastSet.iTACS_ATNTimeOut<1)
    {
        LastSet.iTACS_ATNTimeOut=1;
    }
    else if(LastSet.iTACS_ATNTimeOut>120)
    {
        LastSet.iTACS_ATNTimeOut=120;
    }
    WriteIniData(asGeneralPath, "SystemSetup", "iTACS_ATNTimeOut",  LastSet.iTACS_ATNTimeOut);

    //Steven 20260428 : MyGPIBWrite 參數 range guard + write-back
    if(LastSet.iMyGpibWriteRetry < 5)        LastSet.iMyGpibWriteRetry = 5;
    else if(LastSet.iMyGpibWriteRetry > 100) LastSet.iMyGpibWriteRetry = 100;
    if(LastSet.iMyGpibWriteThreshold < 1)                                    LastSet.iMyGpibWriteThreshold = 1;
    else if(LastSet.iMyGpibWriteThreshold >= LastSet.iMyGpibWriteRetry)      LastSet.iMyGpibWriteThreshold = LastSet.iMyGpibWriteRetry - 1;
    if(LastSet.iMyGpibWriteWaitMS < 10)       LastSet.iMyGpibWriteWaitMS = 10;
    else if(LastSet.iMyGpibWriteWaitMS > 1000)LastSet.iMyGpibWriteWaitMS = 1000;
    WriteIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteRetry",      LastSet.iMyGpibWriteRetry);
    WriteIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteThreshold",  LastSet.iMyGpibWriteThreshold);
    WriteIniData(asGeneralPath, "SystemSetup", "iMyGpibWriteWaitMS",     LastSet.iMyGpibWriteWaitMS);
    WriteIniData(asGeneralPath, "SystemSetup", "bMyGpibWriteVerboseLog", LastSet.bMyGpibWriteVerboseLog);

    LastSet.bFullSiteTimeOut=false;                                             //Steven 20150410 : 強制關閉Fullsite Time Out
    cbFullSiteTimeOut->Checked=LastSet.bFullSiteTimeOut;

    cbbFullsiteTimeOut->Text= CheckAndReadIniData(asGeneralPath, "SystemSetup", "FullSiteTimeOut Time",  5);

    if(LastSet.iTimeOut<8)                                                      //jou 2013-05-21 不能低於11，會出現錯誤
        LastSet.iTimeOut=8;                                                     // ChungHung 20141103 fix GPIB can not change time out blow 1s , 會出現錯誤主要是Time out 不吻合 不能卡掉 TSMC 需要設定300ms 不然要會有GPIB time out 產生

    if(LastSet.iTimeOut>12)                                                     //Steven 20151020 : 最大改成3S, 避免設定為10S會影響機台速度
        LastSet.iTimeOut=12;

    //AI(W906-GB-P1) 20260926: every Application->Title below is comment-only (rule 4): no VCL Application.
    if(LastSet.MachName=="9045GPIB")
    {
       LastSet.MachineType=0;
//       Application->Title="H9045GPIB";
    }
    else if(LastSet.MachName=="9046GPIB")
    {
       LastSet.MachineType=1;
//       Application->Title="H9046GPIB";
    }
    else if(LastSet.MachName=="9046_32GPIB")
    {
       LastSet.MachineType=2;
//       Application->Title="H9046GPIB";
    }
    else if(LastSet.MachName=="2601GPIB")                                       //2012-03-02    Dell for HT-2601
    {
       LastSet.MachineType=0;
//       Application->Title="H9045GPIB";
    }
    else if(LastSet.MachName=="9045GPIB_12Site")
    {
        LastSet.MachineType=1;
//        Application->Title="H9045GPIB_12Site";
    }
    else if(LastSet.MachName=="502GPIB")
    {
        LastSet.MachineType=0;
//        Application->Title="502GPIB";
    }
    else if(LastSet.MachName=="7080GPIB")
    {
        LastSet.MachineType=1;
//        Application->Title="7080GPIB";
    }
    else if(LastSet.MachName=="1032GPIB")
    {
        LastSet.MachineType=2;
//        Application->Title="1032GPIB";
    }
    else if(LastSet.MachName=="9050GPIB")                                       //Steven 20260926 : For HT9050, 比照 9046_32GPIB (F/B 各 2x8 吸嘴 = 32 Site)
    {
        LastSet.MachineType=2;
//        Application->Title="9050GPIB";
    }
    else                                                                        //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    {
        AnsiString asString="D:\\GPIB9045\\system\\general.ini \"Model\" read error!!";
        WriteLog(asString);
//        MessageDlg(asString, mtConfirmation, TMsgDlgButtons()<<mbOK, 0);
        UiNotice(asString);                                                     //AI(W906-GB-P1) 20260926: MessageDlg -> UiNotice (non-blocking)
        RequestClose("ReadLastDataFile");                                       //golden Close();
    }

    TStringList *myTList=new TStringList();
    if(FileExists("d:\\gpib9045\\system\\GpibString.dat"))
    {
        myTList->LoadFromFile("d:\\gpib9045\\system\\GpibString.dat");
        if(myTList->Count!=0 &&                                                 //KevinYang 20241122 : add protection
           myTList->Strings[0]!="")
            LastSet.sGpibString=myTList->Strings[0];
        else
            LastSet.sGpibString="";
    }
    else
    {
        LastSet.sGpibString="";
    }

    if(FileExists(asGeneralPath))
    {
        sGPIBSetting=new TStringList();                                         //Steven 20160912 : 加上保護
        sGPIBSetting->LoadFromFile(asGeneralPath);                              //Steven 20160912 : 加上保護
        sGPIBSetting->Clear();
        delete sGPIBSetting;
    }
    myTList->Clear();                                                           //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
    delete myTList;

    return true;
}
//------------------------------------------------------------------------------
//  寫入設定資料檔
//------------------------------------------------------------------------------
bool TSerialPoll::WriteLastDataFile()
{
    WriteIniData(asGeneralPath, "SystemSetup", "b8080",        LastSet.bGpib8080);
    WriteIniData(asGeneralPath, "SystemSetup", "GpibAddress",  LastSet.GpibAddress);
    WriteIniData(asGeneralPath, "SystemSetup", "LastFile",     LastSet.sLastFile);
    WriteIniData(asGeneralPath, "SystemSetup", "TimeOut",      LastSet.iTimeOut);
    WriteIniData(asGeneralPath, "SystemSetup", "Binon Echo",   LastSet.bBinonEcho); //Steven 20111219
    WriteIniData(asGeneralPath, "ART",         "iTesterType",  LastSet.iTesterType);

    WriteIniData(asGeneralPath, "SystemSetup", "bGPIBWriteWithout_r_n",   LastSet.bGPIBWriteWithout_r_n);       //Jason 20221005 : 增加 GPIB Write結尾不要加/r/n的選項//

//Steven Function不用存檔
//    WriteIniData(asGeneralPath, "Version",     "Model",        LastSet.MachName);
//    WriteIniData(asGeneralPath, "SystemSetup", "StringLength", LastSet.bStringLength);
//    WriteIniData(asGeneralPath, "SystemSetup", "Upper Case",   LastSet.bUpperCase);
//    WriteIniData(asGeneralPath, "SystemSetup", "RecordDatd",   LastSet.bRecordDatd);
    WriteIniData(asGeneralPath, "SystemSetup", "Upper Case",   LastSet.bUpperCase);
//    WriteIniData(asGeneralPath, "SystemSetup", "RecordDatd",   LastSet.bRecordDatd);
    WriteIniData(asGeneralPath, "SystemSetup", "StringLength", LastSet.bStringLength);
    WriteIniData(asGeneralPath, "Run Status",  "SRQ41",        LastSet.bSQR41); //Steven 20110909
    WriteIniData(asGeneralPath, "SystemSetup", "BinTotal",     SerialPoll->iBinSelect);//kevin 20140319
    WriteIniData(asGeneralPath, "SystemSetup", "bUseBarcodeFunction", LastSet.bUseBarcodeFunction);       //Steven 20150713 : Add 2D code

    //ChungHung 20141029 add for SCK want to enable/disable and setting time
    WriteIniData(asGeneralPath, "SystemSetup", "bFullSiteTimeOut",  cbFullSiteTimeOut->Checked);
    WriteIniData(asGeneralPath, "SystemSetup", "FullSiteTimeOut Time",  atoi(cbbFullsiteTimeOut->Text.c_str()));

    WriteIniData(asGeneralPath, "Run Status",  "SRQC0",        LastSet.bSRQC0);     //jou 2015-09-21 Auto Retest function
    WriteIniData(asGeneralPath, "Run Status",  "bFlagRCMD",    LastSet.bFlagRCMD);  //jou 2015-09-21 Auto Retest function
    WriteIniData(asGeneralPath, "Run Status",  "bFlagSVID",    LastSet.bFlagSVID);  //jou 2015-09-21 Auto Retest function
    WriteIniData(asGeneralPath, "Run Status",  "bFlagECID",    LastSet.bFlagECID);  //jou 2015-09-21 Auto Retest function

    WriteIniData(asGeneralPath, "Dummy ART",  "Running Status", LastSet.bDummyART);
    WriteIniData(asGeneralPath, "Dummy ART",  "ART Task",       LastSet.iDummyARTTask);
    WriteIniData(asGeneralPath, "Dummy ART",  "Lot Count",      LastSet.iLotCount);
    WriteIniData(asGeneralPath, "Dummy ART",  "Lot ID",         LastSet.sLotID);
    WriteIniData(asGeneralPath, "SystemSetup", "i2DIDFormat",   LastSet.i2DIDFormat);   //JerryYang 20200422 2DID format
    WriteIniData(asGeneralPath, "Configure",  "SRQ",            LastSet.bConfigureSRQ);                 //Steven 20201022 : For RFMD
    WriteIniData(asGeneralPath, "Configure",  "bHaveContactorInfo",      LastSet.bHaveContactorInfo);   //Steven 20201022 : For RFMD
    WriteIniData(asGeneralPath, "ART",  "bRunHANA_ART",      LastSet.bRunHANA_ART);     //JimmyChiu 20241023 HANA ART Function

    if(CustomerCode==CC_HANA_MICRON)                                                    //JimmyChiu 20250214 : For Hana ART
    {
        WriteIniData(asGeneralPath, "Hana ART", "Try Run ART", LastSet.bTryHANA_ART);   //Steven 20250414 : HANA ART Function, 手動測試用
    }

    if(GPIBVersionCheck<6 &&                                                    //jou 20170209 (Steven) : 修正Tester mode 存檔異常
       (LastSet.iTesterMode==InterfaceType_16BinGS ||
        LastSet.iTesterMode==InterfaceType_32BinGS))
    {
        RequestClose("WriteLastDataFile");                                      //golden Close();
    }
    else if(GPIBVersionCheck<8 &&
            LastSet.iTesterMode==InterfaceType_15BinT6577)                      //Steven 20181214 : ASE-CL add T6577
    {
        RequestClose("WriteLastDataFile");                                      //golden Close();
    }
    else if(GPIBVersionCheck<12.06 &&
            LastSet.iTesterMode==InterfaceType_15BinQorvo)                      //Steven 20201022 : Add Qorvo protocol
    {
        RequestClose("WriteLastDataFile");                                      //golden Close();
    }
    else
    {
        WriteIniData(asGeneralPath, "SystemSetup", "iTesterMode",  LastSet.iTesterMode);
    }
    if(LastSet.iTesterMode==InterfaceType_Delta_Castle)
    {
        labPackageType_1->Visible=true;
        labPackageType->Visible=true;
    }
    else
    {
        labPackageType_1->Visible=false;
        labPackageType->Visible=false;
    }
    WriteIniData(asGeneralPath, "SystemSetup",  "bA10_3_Enable", bA10_3_Enable);//Jimmychiu 20231205 : 借用變數bTimeOutProcess，當作A10-3開啟判斷
    return true;
}
//------------------------------------------------------------------------------
void TSerialPoll::chkUpperCaseClick(TObject *Sender)
{
    LastSet.bUpperCase=chkUpperCase->Checked;
    if(LastSet.bUpperCase)
        WriteLog("Change Set ==> Upper Case is On");
    else
        WriteLog("Change Set ==> Upper Case is Off");
}
//------------------------------------------------------------------------------
void TSerialPoll::cbbGPIBTimoChange(TObject *Sender)
{
    LastSet.iTimeOut=cbbGPIBTimo->ItemIndex+8;                                  // ChungHung 20141103 fix GPIB can not change time out blow 1s , 會出現錯誤主要是Time out 不吻合 不能卡掉 TSMC 需要設定300ms 不然要會有GPIB time out 產生
    WriteLastDataFile();
    AnsiString Str;
    Str.sprintf("Change Set ==> GPIB Time Out : %s", cbbGPIBTimo->Text);
    WriteLog(Str);
}
//------------------------------------------------------------------------------
void TSerialPoll::chkStrLengthCheckClick(TObject *Sender)
{
    LastSet.bStringLength=chkStrLengthCheck->Checked;
    if(LastSet.bStringLength)
        WriteLog("Change Set ==> String Length is On");
    else
        WriteLog("Change Set ==> String Length is Off");
}
//------------------------------------------------------------------------------
void TSerialPoll::cbBinonEchoClick(TObject *Sender)
{
    LastSet.bBinonEcho=cbBinonEcho->Checked;
    if(LastSet.bBinonEcho)
        WriteLog("Change Set ==> BINON ECHO is On");
    else
        WriteLog("Change Set ==> BINON ECHO is Off");
}
//------------------------------------------------------------------------------
void TSerialPoll::cbFullSiteTimeOutClick(TObject *Sender)
{
    LastSet.bFullSiteTimeOut=cbFullSiteTimeOut->Checked;                        //ChungHung 20150409 add
    if(LastSet.bFullSiteTimeOut)
        WriteLog("Change Set ==> Full Site Time Out is On");
    else
        WriteLog("Change Set ==> Full Site Time Out is Off");
}
//------------------------------------------------------------------------------
void TSerialPoll::spbAutoRetestClick(TObject *Sender)
{
    if(bSimulate)                                                               //Steven 20170512 (wei) : ART Simulator加上保護
    {
//        fDummyART->Show();                                                      //Steven 20170413 (wei) : Add ART simulator
        fDummyART->FormShow(NULL);                                              //AI(W906-GB-P1) 20260926: no window; Show() fires OnShow = FormShow (DummyArt.dfm:17)
        spbAutoRetest->Down=false;
    }
}
//------------------------------------------------------------------------------
void TSerialPoll::ShowSimulateItem(bool bShow)
{
    cbbRunMode->Visible=bShow;
    btnRunMode->Visible=bShow;
}
//---------------------------------------------------------------------------
void TSerialPoll::btnRunModeClick(TObject *Sender)
{
    AnsiString Str;
    if(cbbRunMode->ItemIndex==0)
    {
        SendMSG_CMD(MSG_CMD_ReContact);
    }
    else if(cbbRunMode->ItemIndex==1)
    {
        SendMSG_CMD(MSG_CMD_PlaceLoad);
    }
    else if(cbbRunMode->ItemIndex==2)
    {
        SendMSG_CMD(MSG_CMD_TrayFeed);
    }
    else if(cbbRunMode->ItemIndex==3)                                           //JimmyChiu 20250214 : For Hana ART
    {
        if(LastSet.bRunHANA_ART)
        {
            AnsiString Str=edHANACmd->Text;
            SendMSG_CMD(MSG_CMD_HANA_ART, Str);
        }
    }

//    else
//    {
//        Str="PICKLOAD2;1,4,3;2,3,3;3,2,3;4,1,3;";
//        SendMSG_CMD(MSG_CMD_PickLoad, Str);
//    }
}
//---------------------------------------------------------------------------
void TSerialPoll::cbGPIBWriteWithout_r_nClick(TObject *Sender)
{
    LastSet.bGPIBWriteWithout_r_n=cbGPIBWriteWithout_r_n->Checked;              //Jason 20221005 : 增加 GPIB Write結尾不要加/r/n的選項//
    if(LastSet.bGPIBWriteWithout_r_n)
        WriteLog("Change Set ==> GPIB Write Without /r/n");
    else
        WriteLog("Change Set ==> GPIB Write has /r/n");
}
//------------------------------------------------------------------------------
void TSerialPoll::btnUpdateClick(TObject *Sender)
{
    fRS232Main->SetFormToData();
}
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden (Sender, TMouseButton Button, TShiftState Shift, int X, int Y) -- the extra
//   arguments are unused; the body only redraws the panel bevel.
void TSerialPoll::btnUpdateMouseDown(TObject *Sender)
{
#if 0 // TODO(W906-GB-P1): TPanel stand-in has no BevelOuter (visual press effect only; golden Main.cpp:8110-8114)
    btnUpdate->BevelOuter=bvLowered;
#endif
}
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: see btnUpdateMouseDown.
void TSerialPoll::btnUpdateMouseUp(TObject *Sender)
{
#if 0 // TODO(W906-GB-P1): TPanel stand-in has no BevelOuter (visual press effect only; golden Main.cpp:8116-8120)
    btnUpdate->BevelOuter=bvRaised;
#endif
}
//------------------------------------------------------------------------------
void TSerialPoll::btnSaveLogClick(TObject *Sender)
{
    SaveResult();
    lstRecord->Clear();
}
//------------------------------------------------------------------------------
//Steven 20260428 : 收集 GPIB 診斷檔案 (log + ini + ibsta 摘要) 並打包 ZIP
void TSerialPoll::btnDiagZipClick(TObject *Sender)
{
    try
    {
        SaveResult();                                                           //先把目前 lstRecord 倒到檔案
    }
    catch(...) {}

    GetTimeInfo();
    Word SystemYear, SystemMonth, SystemDate;
    DecodeDate(Now(), SystemYear, SystemMonth, SystemDate);

    AnsiString sStamp;
    sStamp.sprintf("%04d%02d%02d_%02d%02d%02d",
                   SystemYear, SystemMonth, SystemDate,
                   SystemHour, SystemMin, SystemSec);

    AnsiString sDiagDir = "D:\\GPIBLOG\\Diag\\" + sStamp + "\\";
    if(!DirectoryExists(sDiagDir))
        ForceDirectories(sDiagDir);

    //摘要檔: 機台資訊 + ibsta 即時值 + 重要 ini 設定
    AnsiString sSummary = sDiagDir + "summary.txt";
    FILE *fp = fopen(sSummary.c_str(), "w");
    if(fp)
    {
        fprintf(fp, "=== GPIB Diagnostic Summary ===\r\n");
        fprintf(fp, "Time           : %04d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
                SystemYear, SystemMonth, SystemDate,
                SystemHour, SystemMin, SystemSec, SystemMSec);
        fprintf(fp, "Machine        : %s\r\n", LastSet.MachName.c_str());
        fprintf(fp, "GPIB Address   : %d\r\n", LastSet.GpibAddress);
        fprintf(fp, "GPIB TimeOut   : %d (cbbGPIBTimo idx=%d)\r\n",
                LastSet.iTimeOut, cbbGPIBTimo->ItemIndex);
        fprintf(fp, "Tester Mode    : %d\r\n", LastSet.iTesterMode);
        fprintf(fp, "Bin Total      : %d\r\n", iBinSelect);
        fprintf(fp, "BinonEcho      : %d\r\n", LastSet.bBinonEcho);
        fprintf(fp, "TACS_ATN TmOut : %d sec\r\n", LastSet.iTACS_ATNTimeOut);
        fprintf(fp, "MyGpibWrite Retry/Threshold/WaitMS/Verbose : %d / %d / %d / %d\r\n",
                LastSet.iMyGpibWriteRetry, LastSet.iMyGpibWriteThreshold,
                LastSet.iMyGpibWriteWaitMS, (int)LastSet.bMyGpibWriteVerboseLog);
        fprintf(fp, "\r\n=== Live ibsta @ snapshot ===\r\n");
        fprintf(fp, "ibsta = 0x%04X\r\n", ibsta);
        fprintf(fp, "  TACS=%d  LACS=%d  ATN=%d  CIC=%d  ERR=%d  TIMO=%d  END=%d  SRQI=%d\r\n",
                (ibsta&TACS)?1:0, (ibsta&LACS)?1:0, (ibsta&ATN)?1:0,
                (ibsta&CIC) ?1:0, (ibsta&ERR) ?1:0, (ibsta&TIMO)?1:0,
                (ibsta&END) ?1:0, (ibsta&SRQI)?1:0);
        fprintf(fp, "iberr = %d\r\n", iberr);
        fprintf(fp, "ibcnt = %d\r\n", (int)ibcnt);                              //AI(W906-GB-P1) 20260926: NI Decl-32 ibcnt is int, gpibbridge::ibcnt is long
        fclose(fp);
    }

    //複製 general.ini 到診斷資料夾
    CopyFile(asGeneralPath.c_str(), AnsiString(sDiagDir+"general.ini").c_str(), FALSE);

    //複製最新 log 檔
    if(LastSet.sLastFile!="" && FileExists(LastSet.sLastFile))
    {
        AnsiString sLogDest = sDiagDir + ExtractFileName(LastSet.sLastFile);
        CopyFile(LastSet.sLastFile.c_str(), sLogDest.c_str(), FALSE);
    }

    //用 PowerShell Compress-Archive 打包 (Win7+ 內建)
    AnsiString sZip = "D:\\GPIBLOG\\Diag\\GPIB_Diag_" + sStamp + ".zip";
    AnsiString sCmd;
    sCmd.sprintf("-NoProfile -Command \"Compress-Archive -Force -Path '%s*' -DestinationPath '%s'\"",
                 sDiagDir.c_str(), sZip.c_str());
    HINSTANCE h = ShellExecute(NULL, "open", "powershell.exe",
                               sCmd.c_str(), NULL, SW_HIDE);

    AnsiString sLog;
    //AI(W906-GB-P1) 20260926: (int)h -> (int)(INT_PTR)h: same value on Win32, and no pointer-truncation error on x64.
    if((int)(INT_PTR)h > 32)
        sLog.sprintf("Diag ZIP ==> Collecting to %s ...", sZip.c_str());
    else
        sLog.sprintf("Diag ZIP ==> ShellExecute fail (ret=%d), files saved at %s",
                     (int)(INT_PTR)h, sDiagDir.c_str());
    WriteLog(sLog);

//    Application->MessageBoxA(
//        AnsiString("GPIB 診斷檔案已收集到:\n"+sDiagDir+
//                   "\n\n壓縮檔:\n"+sZip+
//                   "\n(壓縮約需數秒)").c_str(),
//        "Diag ZIP", MB_OK | MB_ICONINFORMATION);
    //AI(W906-GB-P1) 20260926: MessageBox -> UiNotice (non-blocking), same text; caption "Diag ZIP" dropped.
    UiNotice(AnsiString("GPIB 診斷檔案已收集到:\n"+sDiagDir+
                        "\n\n壓縮檔:\n"+sZip+
                        "\n(壓縮約需數秒)"));
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
