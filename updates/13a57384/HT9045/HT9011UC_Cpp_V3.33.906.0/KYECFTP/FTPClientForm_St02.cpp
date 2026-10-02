// ===========================================================================
//  KYECFTP/FTPClientForm_St02.cpp -- golden TfFTPClient, F1 (no network).  See KYECFTP/FTPClientForm_St02.h.
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  Golden = 906_0625_Steven KYECFTP/FTPClient.cpp unless another file is named;
//  ".dfm" = 906_0625_Steven KYECFTP/FTPClient.dfm.  Lines are translated in golden order; each deviation is marked [W906].
//  Customer branches (S25): a branch that only refuses or only changes what is shown is kept as golden; a branch that
//  does customer work (XCOPY, auto download, ...) keeps its condition and its body is gated (W906_St02_FtpGated names it).
// ===========================================================================
#include "KYECFTP/FTPClientForm_St02.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"       // CC_*, bcSetupFile, rsmContinuStart
#include "cmydef.h"            // CUSTOMER_CODE, AccessLevel, SystemStart, bDownloadFTP, bSigurd*_Recipe, bHasFTPDownload
#include "Config.h"            // IniConfig
#include "CosFunction.h"       // CosFunction
#include "LastSet.h"           // LastSet.iRunStartMode (CC_GIGAS refusal, :1632)
#include "cprod.h"             // SaveTasterInfo (cprod.cpp:3413)
#include "common.h"            // AuthPath, DataPath, CheckAndReadIniData, MyForceDirectories, GetLastOpenFN, W906_SimNetPathBlocked
#include "canary_support.h"    // ShowMyMessage, RecordProcess (cMyDB.h repeats the same default arguments, so not both)
#include "BarcodeReader.h"     // Barcode_Reader (BarcodeReader.cpp:445)
#include "forms/fMain.h"       // fMain->cbSetupFileName
#include "W906FormShowing.h"   // W906_FormProgramShow
#include <winsock2.h>          // golden btSafeTasterNameClick :1919-1934.  After the umbrella's <windows.h> (as TesterComm/Rs232/
                               //   Rs232Support.cpp; vcl_compat.h:137-180 says why not first under MinGW) and after every project
                               //   header, so its macros (s_addr, h_addr ...) cannot reach them

// ---------------------------------------------------------------------------
//  golden file-scope globals (FTPClient.cpp:41-48)
// ---------------------------------------------------------------------------
static TfFTPClient g_fFTPClientForm;                                            // golden Application->CreateForm (HT9045.cpp)
TfFTPClient *fFTPClient = &g_fFTPClientForm;                                    // :41 (constant-initialised: safe in any static-init order)

namespace {
TStringList *tmpList = 0;                                                       // :42 Landam -- F1: this TU's own (header banner)
bool bCanExit = true;                                                           // :48 ChungHung 20150413 add for TSMC (only FTPClient.cpp uses it in golden)
}

TStringList* W906_St02_FtpTmpList() { return tmpList; }
bool W906_St02_FtpCanExit() { return bCanExit; }

W906_St02_FtpHooks W906_St02_Ftp = { 0, 0, 0, 0 };
W906_St02_FtpReport* W906_St02_FtpCurrentReport = 0;

void W906_St02_FtpGated(const std::string& what)
{
    std::printf("[LI9-F1] gated: %s\n", what.c_str());
    if (W906_St02_FtpCurrentReport) W906_St02_FtpCurrentReport->gated.push_back(what);
}

void W906_St02_FtpNote(const std::string& what)
{
    std::printf("[LI9-F1] %s\n", what.c_str());
    if (W906_St02_FtpCurrentReport) W906_St02_FtpCurrentReport->notes.push_back(what);
}

namespace {

// [W906] VCL raises where vclcompat returns quietly (header banner).  Texts = Delphi 6 RTLConsts / Classes.
std::string W906_Fmt(const char* fmt, const char* a)
{
    char buf[1024];
    std::snprintf(buf, sizeof(buf), fmt, a ? a : "");
    return std::string(buf);
}

AnsiString W906_Str(TStringList& l, int i)                                      // golden l->Strings[i]: TStringList.Get raises EStringListError
{
    if (i < 0 || i >= l.Count)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "List index out of bounds (%d)", i);
        throw W906_St02_VclError(buf);
    }
    return l.Strings[i];
}

void W906_LoadFromFileOrRaise(TStringList& l, const AnsiString& path)          // golden ->LoadFromFile: TFileStream fmOpenRead raises EFOpenError
{
    if (FileExists(path) == false)
        throw W906_St02_VclError(W906_Fmt("Cannot open file %s", path.c_str()));
    l.LoadFromFile(path);
}

void W906_SaveToFileOrRaise(TStringList& l, const AnsiString& path)            // golden ->SaveToFile: TFileStream fmCreate raises EFCreateError
{
    std::FILE* f = std::fopen(path.c_str(), "ab");                              // "ab": creates if missing, never truncates (the real write is SaveToFile)
    if (f == 0)
        throw W906_St02_VclError(W906_Fmt("Cannot create file %s", path.c_str()));
    std::fclose(f);
    l.SaveToFile(path);
}

// golden SysUtils.ExtractFileDir (not in vclcompat): up to the last '\' or ':'; that '\' is dropped unless it is the root's
AnsiString W906_ExtractFileDir(const AnsiString& FileName)
{
    int I = FileName.LastDelimiter("\\:");
    if (I > 1 && FileName[I] == '\\' && FileName[I - 1] != '\\' && FileName[I - 1] != ':')
        --I;
    return FileName.SubString(1, I);
}

// golden :1919-1934 (the host's first IPv4 address)
AnsiString W906_GoldenLocalIp()
{
    WSAData   wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);                                       // :1920 初始化WINSOCK?用
    char   HostName[80];                                                        // :1921 存放本主機名
    HostName[0] = 0;
    LPHOSTENT lpHostEnt;
    gethostname(HostName, sizeof(HostName));                                    // :1923 利用得到的主機名去取得主機結構
    lpHostEnt=gethostbyname(HostName);                                          // :1924 利用主機名去取主機結構
    AnsiString IP;
    if (lpHostEnt != 0 && lpHostEnt->h_addr_list[0] != 0)                       // [W906] golden :1928 dereferences without a check (an AV when the lookup fails)
    {
        struct in_addr *p=(struct in_addr *)(lpHostEnt->h_addr_list[0]);        // :1928
        IP=inet_ntoa(*p);                                                       // :1929
    }
    WSACleanup();                                                               // :1934
    return IP;
}

// [W906] golden fMain->ChangeSetUpFile(FileName) (V912 main.cpp:25655): the body is WebRecipeChange.cpp W906_RC_ChangeSetUpFile
//   (wb_serve only), reached through the installed hook.  Not installed / boot read chain not run = golden's "refused" (1).
int W906_ChangeSetUpFile(const AnsiString& FileName)
{
    if (W906_St02_Ftp.ChangeSetUpFile == 0)
    {
        W906_St02_FtpNote("ChangeSetUpFile is not installed (W906_St02_InstallLotInfoFtp not called) -- treated as refused (mode 1)");
        return 1;
    }
    if (W906_St02_Ftp.RecipeChainsReady != 0 && W906_St02_Ftp.RecipeChainsReady() == false)
    {
        W906_St02_FtpNote("boot-read-chain-not-run: tools/wb_serve.cpp W906_DoReadLastData(true) did not run -- the recipe is not changed "
                          "(the same port-only guard as WebRecipeChange.cpp recipe.change); treated as refused (mode 1)");
        return 1;
    }
    return W906_St02_Ftp.ChangeSetUpFile(FileName);
}

// [W906] the folders W906_RC_LookForFile would list (WebRecipeChange.cpp:155-216), read only -- see ShowFTPModal case 1
void W906_ListSetupFolders(std::vector<AnsiString>& out)
{
    WIN32_FIND_DATAA filedata;
    HANDLE filehandle = FindFirstFileA((DataPath + "*").c_str(), &filedata);
    if (filehandle == INVALID_HANDLE_VALUE) return;
    do
    {
        if ((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0 ||
            std::strcmp(filedata.cFileName, ".") == 0 || std::strcmp(filedata.cFileName, "..") == 0)
            continue;
        if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            out.push_back(ExtractFileName(filedata.cFileName));
    } while (FindNextFileA(filehandle, &filedata));
    FindClose(filehandle);
}

template <class T> T* Mk(bool visible = true, bool enabled = true)             // dfm defaults: VCL controls are Visible / Enabled
{                                                                               //   unless the .dfm says False (vclcompat's ctor says false/false)
    T* c = new T();
    c->Visible = visible;
    c->Enabled = enabled;
    return c;
}

}  // namespace

// ---------------------------------------------------------------------------
//  golden ctor :50-64 + the .dfm
// ---------------------------------------------------------------------------
TfFTPClient::TfFTPClient()
    : Left(260), Top(272), Caption("FTP"),                                      // .dfm :2-5
      enter(false), bListOk(false), iHD(0),
      bShow(false), bControlBySECSGEM(false), iErrorBySECSGEM(0), bError(false),
      bControlByGPIB(false), iErrorByGPIB(0),
      W906_bModalOpen(false), W906_iOpenerTag(-1), W906_ModalEnded(0), W906_iLastLoadMode(-1)
{
    // golden :53-54 bShow=false; bListOk=false; (the rest of a VCL form is zero-filled: TObject.NewInstance)
    // golden :55-63 bIsFtpRunning / slDailyTempLog / sl10MinTempLog (ChipMos FTP logs, N25) -- not F1, not created.
    PageControl1 = Mk<TfFTPClientPageControl>();
    TabSheet1 = Mk<TTabSheet>(); TabSheet1->Caption = "Network Setting"; TabSheet1->TabVisible = true;
    TabSheet2 = Mk<TTabSheet>(); TabSheet2->Caption = "Server";          TabSheet2->TabVisible = true;
    TabSheet3 = Mk<TTabSheet>(); TabSheet3->Caption = "HD";              TabSheet3->TabVisible = true;
    TabSheet4 = Mk<TTabSheet>(); TabSheet4->Caption = "Taster";          TabSheet4->TabVisible = true;
    PageControl1->ActivePage = TabSheet2;                                       // .dfm :26 ActivePage = TabSheet2
    PageControl1->ActivePageIndex = 1;                                          // .dfm :28 TabIndex = 1
    edtServerWaferName = Mk<TEdit>();
    lstServerFile      = Mk<TListBox>();  lstServerFile->ItemIndex = -1;
    edtHDWaferName     = Mk<TEdit>();
    lstHDFile          = Mk<TListBox>();  lstHDFile->ItemIndex = -1;
    ListBox1           = Mk<TListBox>(false);  ListBox1->ItemIndex = -1;       // .dfm :453 Visible = False
    plLoad      = Mk<TPanel>(); plLoad->Caption      = "Load from HD";
    plUnload    = Mk<TPanel>(); plUnload->Caption    = "Upload to Server";
    plSLoad     = Mk<TPanel>(); plSLoad->Caption     = "Download to Handler";
    Panel15     = Mk<TPanel>(); Panel15->Caption     = "Copy File Name";
    Panel16     = Mk<TPanel>(); Panel16->Caption     = "Clean File Name";
    plUnloadALL = Mk<TPanel>(false); plUnloadALL->Caption = "Upload All to Server";   // .dfm :756 Visible = False
    edHandlerType = Mk<TEdit>(true, false); edHandlerType->Text = "HT9045W";   // .dfm Enabled = False
    edHandlerID   = Mk<TEdit>(true, false); edHandlerID->Text   = "HT9045-001";
    btSafeTasterName = Mk<TButton>(); btSafeTasterName->Caption = "Save";
    Button3          = Mk<TButton>(); Button3->Caption          = "Exit";
    memoFTP = Mk<TMemo>(); memoFTP->Lines->Add("");                             // .dfm Lines.Strings = ('')
    rgInputMethod = Mk<TRadioGroup>();
    rgInputMethod->Items->Add("By List");                                       // .dfm Items.Strings
    rgInputMethod->Items->Add("Manually");
    rgInputMethod->ItemIndex = 0;                                               // .dfm ItemIndex = 0
    grpTesterMap  = Mk<TGroupBox>(); grpTesterMap->Caption  = "Taster Map";
    grpTesterName = Mk<TGroupBox>(); grpTesterName->Caption = "Taster Name";
    cbTesterType = Mk<TComboBox>(); cbTesterType->ItemIndex = -1;
    cbTesterID   = Mk<TComboBox>(); cbTesterID->ItemIndex   = -1;
    cbTasterIp   = Mk<TComboBox>(true, false); cbTasterIp->ItemIndex = -1;     // .dfm Enabled = False
    edTesterName = Mk<TEdit>();
    labN06_DownloadPath  = Mk<TLabel>(); labN06_DownloadPath->Caption  = "Download Path:";
    labN06_UploadPath    = Mk<TLabel>(); labN06_UploadPath->Caption    = "Upload Path:";
    labN06_DownloadPath1 = Mk<TLabel>(); labN06_DownloadPath1->Caption = "Download Path:";
    labN06_UploadPath1   = Mk<TLabel>(); labN06_UploadPath1->Caption   = "Upload Path:";
    FTP_DownPath  = Mk<TEdit>(true, false);                                     // .dfm Enabled = False (x4)
    FTP_UpLdPath  = Mk<TEdit>(true, false);
    FTP_DownPath1 = Mk<TEdit>(true, false);
    FTP_UpLdPath1 = Mk<TEdit>(true, false);
}

// ---------------------------------------------------------------------------
//  [PORT-ONLY] VCL property semantics
// ---------------------------------------------------------------------------
void TfFTPClient::W906_SetText(TEdit* e, const AnsiString& v)
{
    if (e == 0 || e->Text == v) return;                                         // VCL TControl.SetText: same text -> no OnChange
    e->Text = v;
    if (e == edtHDWaferName)          edtHDWaferNameChange(e);                  // .dfm :648 OnChange = edtHDWaferNameChange
    else if (e == edtServerWaferName) edtServerWaferNameChange(e);              // .dfm :496 OnChange = edtServerWaferNameChange
}

void TfFTPClient::W906_ListClear(TListBox* l)                                   // VCL TListBox.Clear: items gone, ItemIndex -1
{
    l->Clear();
    l->ItemIndex = -1;
}

void TfFTPClient::W906_ComboClear(TComboBox* c)                                 // VCL TComboBox.Clear / Items->Clear (CB_RESETCONTENT): text and selection too
{
    c->Clear();
    c->Text = "";
    c->ItemIndex = -1;
}

void TfFTPClient::W906_ComboSetItemIndex(TComboBox* c, int v)                   // VCL SetItemIndex: CB_SETCURSEL only when it differs; the edit
{                                                                               //   shows that item, or is cleared for -1 / out of range
    if (c->ItemIndex == v) return;
    if (v >= 0 && v < c->Items->Count)
    {
        c->ItemIndex = v;
        c->Text = c->Items->Strings[v];
    }
    else
    {
        c->ItemIndex = -1;
        c->Text = "";
    }
}

void TfFTPClient::W906_RadioSetItemIndex(TRadioGroup* r, int v)                 // VCL TCustomRadioGroup.SetItemIndex: clamped; a new checked
{                                                                               //   button clicks -> ButtonClick -> OnClick
    if (v < -1) v = -1;
    if (v >= r->Items->Count) v = r->Items->Count - 1;
    if (r->ItemIndex == v) return;
    r->ItemIndex = v;
    if (v >= 0 && r == rgInputMethod)
        rgInputMethodClick(this);                                               // .dfm :848 OnClick = rgInputMethodClick
}

bool TfFTPClient::W906_BarcodeKeys() const
{
    return CosFunction.bFTPUseBarcodeReader==true && IniConfig.bN06_UseBarcode==true;
}

// ---------------------------------------------------------------------------
//  [W906] golden TCustomForm::Close (no TForm here): CloseQuery -> OnClose; a modal form then returns from ShowModal
// ---------------------------------------------------------------------------
void TfFTPClient::Close()
{
    bool CanClose = true;
    FormCloseQuery(this, CanClose);                                             // .dfm: no OnCloseQuery line, but golden FTPClient.h:128 / :2258
    if (!CanClose) return;                                                      //   (TfFTPClient::FormCloseQuery) -- VCL: CloseQuery false = stays open
    FormClose(this);                                                            // .dfm :16 OnClose = FormClose
    if (W906_bModalOpen)
    {
        W906_bModalOpen = false;
        W906_FormProgramShow("fFTPClient", false, "FTPClient.cpp Close (ShowModal returns)");
        void (*fn)(int) = W906_ModalEnded;
        const int tag = W906_iOpenerTag;
        W906_ModalEnded = 0;
        W906_iOpenerTag = -1;
        if (fn) fn(tag);                                                        // the caller's lines after ShowModal returned
    }
}

// ---------------------------------------------------------------------------
//  golden ShowFTPModal :1215-1573
// ---------------------------------------------------------------------------
void TfFTPClient::ShowFTPModal(int HD)
{
    int i=0;                                                                    // :1217
    (void)i;

    if(bShow)                                                                   // :1219
    {
        ShowMyMessage("FTP Form already Opened!!");                             // :1221
        iErrorBySECSGEM=1;                                                      //ChungHung 20150515 add Control by SECSGEM
        bControlBySECSGEM=false;                                                //ChungHung 20150515 add Control by SECSGEM
        iErrorByGPIB=1;                                                         //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        bControlByGPIB=false;                                                   //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        return;
    }

    bShow=true;                                                                 // :1229
    bError=false;
    if (tmpList) delete tmpList;                                                // [W906] golden :1231 news a fresh list each time (the old one is
    tmpList = new TStringList;                                                  //   freed by FormClose; the remote-download arm leaks it -- freed here)
    AnsiString cDir="", Cur="", DirName="";
    (void)cDir; (void)Cur;
    iHD=HD;                                                                     // :1233
    switch(HD)
    {
        case 0:                                                                 // :1236 Server
            PageControl1->ActivePage=TabSheet2;  PageControl1->ActivePageIndex=1;   // :1260
            TabSheet1->TabVisible=false;
            TabSheet2->TabVisible=true;
            TabSheet3->TabVisible=false;
            TabSheet4->TabVisible=false;
            Panel15->Visible=!bSigurdDownload_Recipe;                           //KaiChen 20190530 ：Sigurd FTP Automation
            W906_ListClear(lstServerFile);                                      // :1266
            // GATE (W906-LI9-F2): golden :1237-1258 new TNMFTP and :1268-1440 connect to [FTP] Host / CWD / NLST / wait for the
            //   list are the network part (card F2).  No socket here: the case ends the way golden's own "could not list" path
            //   does -- bError=true, then :1566-1570 bShow=false; Close() -- without golden's "FTP Server is not connected" box.
            W906_St02_FtpGated("golden FTPClient.cpp:1237-1441 the Server list (TNMFTP connect / CWD / NLST) -- card F2, not available yet");
            bError=true;
            break;
        case 1: //HD                                                            // :1442
            PageControl1->ActivePage=TabSheet3;  PageControl1->ActivePageIndex=2;   // :1443
            TabSheet1->TabVisible=false;
            TabSheet2->TabVisible=false;
            TabSheet3->TabVisible=true;
            TabSheet4->TabVisible=false;
            plLoad->Visible=!bSigurdUpload_Recipe;                              //KaiChen 20190530 ：Sigurd FTP Automation
            W906_ListClear(lstHDFile);                                          // :1449

            if(CUSTOMER_CODE==CC_TSMC_TAINAN ||                                 //ChungHung 20150413 add for TSMC
               CUSTOMER_CODE==CC_Greatek)                                       //Sam 20171019 (wei) : Setup File Download 完成後刪除原本的 Setup File，本機只留一個
            {
                W906_St02_FtpGated("golden FTPClient.cpp:1454-1469 CC_TSMC_TAINAN / CC_Greatek: plUnload by level + list D:\\HT9045\\IniData\\Data\\ folders (S25)");
            }
            else
            {
                // [W906] golden lists fMain->cbSetupFileName->Items, which golden fills at boot (TfMain::FormShow -> LookForFile) and
                //   after every change.  The port fills it only in recipe.change "list" (WebRecipeChange.cpp:660; fMain.cpp:509 is a
                //   counting stub), so when it is empty the same folders are read here, read only (no Offset move, no ghost-file
                //   delete, no bSetTempChange) -- and fMain->cbSetupFileName is not touched.
                std::vector<AnsiString> folders;
                if(fMain->cbSetupFileName->Items->Count==0)
                {
                    W906_ListSetupFolders(folders);
                    W906_St02_FtpNote("fMain->cbSetupFileName->Items is empty (no LookForFile since boot): the HD list is DataPath's folders, read only");
                }
                else
                {
                    for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
                        folders.push_back(fMain->cbSetupFileName->Items->Strings[i]);
                }
                for(int i=0; i<(int)folders.size(); i++)                        // :1473
                {
                    lstHDFile->Items->Add(folders[i]);                          // :1475
                    tmpList->Add(folders[i]);                                   // :1476 Landam
                }
            }
            break;
        case 2: //Taster                                                        // :1480
            PageControl1->ActivePage=TabSheet4;  PageControl1->ActivePageIndex=3;   // :1481
            TabSheet1->TabVisible=false;
            TabSheet2->TabVisible=false;
            TabSheet3->TabVisible=false;
            TabSheet4->TabVisible=true;
            if(IniConfig.N06_TasterListFile!="")                                // :1486
            {
                if(FileExists(IniConfig.N06_TasterListFile))
                {
                    W906_ComboClear(cbTesterType);                              // :1490 cbTesterType->Clear();
                    GetTesterType();

                    cbTesterType->Text=IniConfig.TasterType;                    // :1493
                    W906_ComboSetItemIndex(cbTesterType, cbTesterType->Items->IndexOf(IniConfig.TasterType));
                    cbTesterTypeChange(this);
                    W906_RadioSetItemIndex(rgInputMethod, IniConfig.TasterInputMethod);   // :1496 (VCL: OnClick when it changes)
                    if(rgInputMethod->ItemIndex==0)
                    {
                        grpTesterMap->Visible=true;
                        grpTesterName->Enabled=false;
                        cbTesterID->Text=IniConfig.TasterNo;                    // :1501
                        W906_ComboSetItemIndex(cbTesterID, cbTesterID->Items->IndexOf(IniConfig.TasterNo));
                        cbTesterIDChange(this);
                    }
                    else
                    {
                        grpTesterMap->Visible=false;
                        grpTesterName->Enabled=true;
                        edTesterName->Text=IniConfig.TasterName;                // :1509
                    }
                    edHandlerType->Text=IniConfig.sMachineType;                 // :1511
                    edHandlerID->Text  =IniConfig.SocketHandlerID;
                }
            }
            break;
    }
    plSLoad->Enabled=true;                                                      // :1517
    plUnload->Enabled=true;

    if(bError==false)                                                           // :1520
    {
        iErrorBySECSGEM=0;                                                      //ChungHung 20150515 add Control by SECSGEM
        iErrorByGPIB=0;                                                         //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        static const bool kW906_bAutoDownloadSetupFile = false;                 // [W906] fProductionInfo->bAutoDownloadSetupFile is not in the port (S25 CC_Greatek)
        if(bControlBySECSGEM==true)                                             // :1524 ChungHung 20150515 add Control by SECSGEM
        {
            W906_SetText(edtServerWaferName, aSetUpNameBySECSGEM);              // :1526
            W906_St02_FtpGated("golden FTPClient.cpp:1527 plSLoadClick(this) -- the SECS remote download, card F2");
            bShow=false;                                                        // :1528
        }
        else if(bControlByGPIB==true)                                           // :1530 KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        {
            W906_SetText(edtServerWaferName, asSetUpNameByGPIB);                // :1532
            W906_St02_FtpGated("golden FTPClient.cpp:1533 plSLoadClick(this) -- the GPIB remote download, card F2");
            bShow=false;                                                        // :1534
        }
        else if(kW906_bAutoDownloadSetupFile==true &&                           // :1536 Sam 20190801 : AutoDown SetupFile
                CUSTOMER_CODE==CC_Greatek)
        {
            W906_St02_FtpGated("golden FTPClient.cpp:1538-1559 CC_Greatek auto download (S25 + F2)");
            bShow=false;                                                        // :1549
            Close();                                                            // :1558
        }
        else
        {
            // [W906] golden :1563 ShowModal(): VCL runs OnShow (FormShow) and then waits in the modal loop.  Here the page table
            //   opens the web window and this returns; the rest of golden's caller runs when Close() ends the "modal".
            FormShow(this);
            W906_bModalOpen = true;
            W906_FormProgramShow("fFTPClient", true, "FTPClient.cpp:1563 ShowModal");
        }
    }
    else
    {
        bShow=false;                                                            // :1568
        Close();
    }
    bControlBySECSGEM=false;                                                    // :1571 ChungHung 20150515 add Control by SECSGEM
    bControlByGPIB=false;                                                       // :1572 (FormShow :1715 / :1717 already cleared both)
}

// ---------------------------------------------------------------------------
//  golden plLoadClick :1575-1653 (change the setup file from the local disk; no network)
// ---------------------------------------------------------------------------
void TfFTPClient::plLoadClick(TObject *Sender)
{
    (void)Sender;
    W906_iLastLoadMode = -1;
    if(Barcode_Reader(bcSetupFile)==0)                                          // :1577 20140103 wei KYEC Barcode Reader
    {
        return;
    }

    if(lstHDFile->Items->Count==0)                                              // :1582 Landam  選對一定會剩一條record
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //ChungHung 20150413 add for TSMC
            ShowMyMessage("未選取產品名稱");
        else
            ShowMyMessage("未選擇這路徑");
        return;
    }

    if(edtHDWaferName->Text!=lstHDFile->Items->Strings[0])                      // :1591 Landam  選對名稱會一樣
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //ChungHung 20150413 add for TSMC
            ShowMyMessage("未選取產品名稱");
        else
            ShowMyMessage("未選擇這路徑");
        return;
    }

    bCanExit=false;  //Steven 20260618 fix: == -> = (P11 bug fix)               // :1600 ChungHung 20150413 add for TSMC
    AnsiString Backup,str;
    (void)Backup; (void)str;
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           // :1602 ChungHung 20150413 add for TSMC
    {
        W906_St02_FtpGated("golden FTPClient.cpp:1604-1627 CC_TSMC_TAINAN: Data / DataFTP swap of DataPath + XCOPY (S25)");
    }

    if(CUSTOMER_CODE==CC_GIGAS)                                                 // :1630 Isaac 20200710 : 全智科技，從FTP下載，如果是continue start而且檔案名稱相同，不進行下載
    {
        if(LastSet.iRunStartMode==rsmContinuStart &&
           edtHDWaferName->Text==fMain->cbSetupFileName->Text)
        {
            ShowMyMessage("Test mode is ContinuStart and the selected filename is the same!\r\nNot download from HD");
            return;                                                             // :1636 (golden leaves bCanExit false here -- kept)
        }
    }

    int mode=0;                                                                 // :1640

    RecordProcess("plLoadClick");                                               // :1642
    mode=W906_ChangeSetUpFile(edtHDWaferName->Text);                            // :1643 fMain->ChangeSetUpFile (see W906_ChangeSetUpFile)
    W906_iLastLoadMode = mode;
    if(mode==1)
    {
        fMain->cbSetupFileName->Text=GetLastOpenFN();                           // :1646
    }
    fMain->cbSetupFileName->Text=edtHDWaferName->Text;                          // :1648 (golden: also after a refused change -- kept)

    W906_SetText(edtHDWaferName, "");                                           // :1650
    bCanExit=true;   //Steven 20260618 fix: == -> = (P11 bug fix)               // :1651 ChungHung 20150413 add for TSMC
    Close();                                                                    // :1652
}

// ---------------------------------------------------------------------------
//  golden FormClose :1655-1701
// ---------------------------------------------------------------------------
void TfFTPClient::FormClose(TObject *Sender)
{
    (void)Sender;
    memoFTP->Lines->Add("Process -- Close-----------");                         // :1658
    memoFTP->Lines->Add("");
    if(memoFTP->Lines->Count>1024)
        memoFTP->Clear();
    bShow=false;                                                                // :1662 JerryYang 20200416 : add
    if(tmpList)                                                                 // [W906] golden :1663-1664 deletes and leaves it dangling (a second
    {                                                                           //   Close would crash); here it is NULL afterwards
        tmpList->Clear();
        delete tmpList;
        tmpList = 0;
    }
    // GATE (W906-LI9-F2): golden :1665-1697 `if(iHD==0 && NMFTP3!=NULL)` aborts / closes the Server connection.  NMFTP3 is
    //   KYECFTP/FTPClient_Transfer.cpp's (ht9045_kyecftp, not linked into wb_serve) and F1's case 0 never creates one, so the
    //   condition is false in F1; card F2 brings it back.
    bSigurdUpload_Recipe=false;                                                 // :1698 KaiChen 20190530 ：Sigurd FTP Automation
    bSigurdDownload_Recipe=false;
    bHasFTPDownload=false;                                                      // :1700 Ifor 20240422 add:FTP 工作檔下載
}

// golden Button3Click :1703-1706 ('Exit' on the Taster tab)
void TfFTPClient::Button3Click(TObject *Sender)
{
    (void)Sender;
    Close();
}

// ---------------------------------------------------------------------------
//  golden FormShow :1708-1797
// ---------------------------------------------------------------------------
void TfFTPClient::FormShow(TObject *Sender)
{
    (void)Sender;
    enter=false;                                                                // :1710
    fFTPClient->Left=10;                                                        // :1711 [W906] stored only: the page table places the window
    fFTPClient->Top=100;

    iErrorBySECSGEM=0;          //ChungHung 20150515 add Control by SECSGEM    // :1714
    bControlBySECSGEM=false;    //ChungHung 20150515 add Control by SECSGEM
    iErrorByGPIB=0;             //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
    bControlByGPIB=false;       //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
    bDownloadFTP=false;         //wei 20160512 避免Download失敗，下次就無法Download // :1718
    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             // :1719 20140310 WEI 在FTP頁面顯示 Download 和 Uplaod路徑 KYEC
    {
        labN06_DownloadPath->Visible    =true;
        labN06_UploadPath->Visible      =true;
        FTP_DownPath->Visible           =true;
        FTP_DownPath->Text              =IniConfig.FtpDownloadPath;
        FTP_UpLdPath->Visible           =true;
        FTP_UpLdPath->Text              =IniConfig.FtpUplaodPath ;

        labN06_DownloadPath1->Visible   =true;                      //wei 20150325  在FTP頁面顯示 Download 和 Uplaod路徑     KYEC
        labN06_UploadPath1->Visible     =true;
        FTP_DownPath1->Visible          =true;
        FTP_DownPath1->Text             =IniConfig.FtpDownloadPath;
        FTP_UpLdPath1->Visible          =true;
        FTP_UpLdPath1->Text             =IniConfig.FtpUplaodPath ;
    }
    else
    {
        labN06_DownloadPath->Visible    =false;                                 // :1737
        labN06_UploadPath->Visible      =false;
        FTP_DownPath->Visible           =false;
        FTP_UpLdPath->Visible           =false;

        labN06_DownloadPath1->Visible   =false;                     //wei 20150325  在FTP頁面顯示 Download 和 Uplaod路徑     KYEC
        labN06_UploadPath1->Visible     =false;
        FTP_DownPath1->Visible          =false;
        FTP_UpLdPath1->Visible          =false;
    }

    if(CosFunction.bHiSiliconFunction==true && (CUSTOMER_CODE==CC_JCET || CUSTOMER_CODE==CC_SCC))    // :1748 jou 20170921 (Steven) : center要求海思版hontech權限也不能選擇工作檔
    {
        plLoad->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_N) //Steven 20201231 : FTP HD選項 setup file是要保留的，loadfrom HD功能不需要了   // :1753
    {
        plLoad->Visible=false;
    }

    Panel16->Visible=(CUSTOMER_CODE==CC_TSMC_TAINAN);               //wei 20151208 清除FTP檔案名稱   // :1758

    if(CUSTOMER_CODE==CC_Greatek)                                   //Sam 20171006 (wei) : 超豐要求不顯示。   // :1760
    {
        Panel15->Visible =false;
    }
    else if(CUSTOMER_CODE==CC_GIGAS)    //Isaac 20200710 : 全智科技FTP   // :1764
    {
        if(PageControl1->ActivePageIndex==1)
            edtServerWaferName->SetFocus();                                     // (vclcompat: no-op)
        else
            edtHDWaferName->SetFocus();

        plUnloadALL->Visible=true;  //一鍵上傳的按鈕
    }

    if(CosFunction.bFTPUseBarcodeReader==true)                                  // :1774
    {
        if(IniConfig.bN06_UseBarcode==true)
        {
            if(CUSTOMER_CODE==CC_CYUEAN)                                        //Sam 20230706 : 确安科技上傳不需要用 Barcode reader
            {
                lstServerFile->Enabled=false;
                lstHDFile->Enabled=true;
            }
            else
            {
                lstServerFile->Enabled=false;
                lstHDFile->Enabled=false;
            }
        }
        else
        {
            lstServerFile->Enabled=true;
            lstHDFile->Enabled=true;
        }
    }
    W906_SetText(edtServerWaferName, "");                                       // :1795 (VCL: OnChange -> FilterList when it was not empty)
    W906_SetText(edtHDWaferName, "");                                           // :1796
}

// ---------------------------------------------------------------------------
//  The Taster tab
// ---------------------------------------------------------------------------
void TfFTPClient::cbTesterTypeChange(TObject *Sender)                           // :1831-1866
{
    (void)Sender;
    bool bAlreadyIn;
    TStringList sListY;                                                         // :1834 golden new TStringList() (here on the stack: a VCL
    TStringList sListX;                                                         //   exception below cannot leak them)

    W906_LoadFromFileOrRaise(sListY, IniConfig.N06_TasterListFile);             // :1837 sListY->LoadFromFile (EFOpenError when missing)

    W906_ComboClear(cbTesterID);                                                // :1839 cbTesterID->Items->Clear(); (CB_RESETCONTENT)
    W906_ComboClear(cbTasterIp);                                                // :1840
    cbTesterID->Text="";
    cbTasterIp->Text="";

    for(int i=1; i<sListY.Count; i++)                                           // :1844
    {
        sListX.CommaText=sListY.Strings[i]; //取得ROW
        bAlreadyIn=false;

        for(int j=0; j<cbTesterID->Items->Count; j++)
        {
            if(W906_Str(sListX, 0)!=cbTesterType->Text || W906_Str(sListX, 1)==cbTesterID->Items->Strings[j])
                bAlreadyIn=true;
        }

        if(bAlreadyIn==false && W906_Str(sListX, 0)==cbTesterType->Text)       // :1855
        {
            cbTesterID->Items->Add(W906_Str(sListX, 1));
            cbTasterIp->Items->Add(W906_Str(sListX, 2));
        }
    }
    sListY.Clear();                                                             //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    sListX.Clear();                                                             //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    edTesterName->Text=cbTesterType->Text+"-"+cbTesterID->Text;                 // :1865
}

void TfFTPClient::btSafeTasterNameClick(TObject *Sender)                        // :1868-1952
{
    (void)Sender;
//    AnsiString sPath=AuthPath+"config.ini";
    IniConfig.TasterInputMethod=rgInputMethod->ItemIndex;                       // :1871
    IniConfig.TasterType=cbTesterType->Text;
    IniConfig.TasterNo=cbTesterID->Text;
    IniConfig.TasterName=edTesterName->Text;

    //Landam 檢查Tester 不存在   IniConfig.TasterName=""
    int ipos=edTesterName->Text.Pos("-");                                       // :1877
    if(ipos!=0)                                                                 //無-
    {
        // Tester 為 Type-ID
        AnsiString strType=edTesterName->Text.SubString(1, ipos-1);
        AnsiString strID=edTesterName->Text.SubString(ipos+1, edTesterName->Text.Length());

        if(cbTesterType->Items->Count==0)                                       // :1884
        {
            GetTesterType();
            if(cbTesterType->Items->Count==0)                                   //檔案不存在
            {
                edTesterName->Text="";
            }
        }

        if(cbTesterType->Items->Count!=0)                                       //因為上面會重新Load資料,所以這邊不能用else
        {
            int iType=cbTesterType->Items->IndexOf(strType);                    // :1895

            W906_ComboSetItemIndex(cbTesterType, iType);                        // :1897 cbTesterType->ItemIndex=iType; (no OnChange)

            int iID=cbTesterID->Items->IndexOf(strID);

            if(iType<0 || iID<0)     // Type 存在
            {
                edTesterName->Text="";
            }
        }
    }
    else
    {
        edTesterName->Text="";                                                  // :1909
    }

    if(edTesterName->Text=="")                                                  // :1912
        ShowMyMessage("Tester Name 錯誤", "");

    AnsiString str="";
    TStringList sList;                                                          // :1916 golden new TStringList()
    sList.Add("Handler Name, Handler Address, Tester Name, Tester Address");

    //    for(int i=0; lpHostEnt->h_addr_list[i]!=0; i++)                           //迴圈取得全部的IP位置
    {
        AnsiString IP = W906_St02_Ftp.LocalIp ? W906_St02_Ftp.LocalIp()        // [W906] ctest seam (no socket API there);
                                              : W906_GoldenLocalIp();           //   NULL = golden :1919-1929 / :1934
        str.sprintf("%s,%s,%s,%s", edHandlerID->Text, IP, edTesterName->Text, cbTasterIp->Text);   // :1931
        sList.Add(str);
    }

    str=W906_ExtractFileDir(IniConfig.N06_TasterListMap);                       // :1936 取出資料夾
    if(W906_SimNetPathBlocked(IniConfig.N06_TasterListMap))                     // [W906] W58 Q5: the SIM build skips a save under a network path
    {                                                                           //   (common.cpp:2785; W906_SIM_NET_PATHS=1 allows it)
        W906_St02_FtpGated("golden FTPClient.cpp:1937-1938 Taster map save skipped: " + std::string(IniConfig.N06_TasterListMap.c_str()) +
                           " is a network path (W58 Q5, SIM build)");
    }
    else
    {
        MyForceDirectories(str);                                                // :1937 不存在就建立資料夾
        W906_SaveToFileOrRaise(sList, IniConfig.N06_TasterListMap);             // :1938 sList->SaveToFile (EFCreateError when it cannot)
    }
    sList.Clear();                                                              //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    //--------------------------
    IniConfig.TasterInputMethod =rgInputMethod->ItemIndex;                      // :1942
    IniConfig.TasterType        =cbTesterType->Text;
    IniConfig.TasterNo          =cbTesterID->Text;
    IniConfig.TasterName        =edTesterName->Text;
    SaveTasterInfo();                                                           // :1946

    str.sprintf("%s is connect with %s", edHandlerID->Text, edTesterName->Text);   // :1948
    RecordProcess(str);                                                         //Steven 20091004
    if(edTesterName->Text!="")
        Close();                                                                //Landam
}

void TfFTPClient::cbTesterIDChange(TObject *Sender)                             // :1954-1958
{
    (void)Sender;
    W906_ComboSetItemIndex(cbTasterIp, cbTesterID->ItemIndex);                  // :1956 cbTasterIp->ItemIndex=cbTesterID->ItemIndex;
    edTesterName->Text=cbTesterType->Text+"-"+cbTesterID->Text;
}

void TfFTPClient::memoFTPDblClick(TObject *Sender)                              // :2062-2065
{
    (void)Sender;
    memoFTP->Clear();
}

void TfFTPClient::rgInputMethodClick(TObject *Sender)                           // :2067-2084
{
    (void)Sender;
    if(rgInputMethod->ItemIndex==0)
    {
        grpTesterMap->Visible=true;
        grpTesterName->Enabled=false;
        cbTesterID->Text=IniConfig.TasterNo;                                    // :2073
        W906_ComboSetItemIndex(cbTesterID, cbTesterID->Items->IndexOf(IniConfig.TasterNo));
        cbTesterIDChange(this);
    }
    else
    {
        AnsiString sPath=AuthPath+"config.ini";                                 // :2079
        grpTesterMap->Visible=false;
        grpTesterName->Enabled=true;
        edTesterName->Text=CheckAndReadIniData(sPath, "Taster", "Taster Name",  AnsiString(""));
    }
}

// ---------------------------------------------------------------------------
//  Filter / pick (:2098-2145)
// ---------------------------------------------------------------------------
void TfFTPClient::FilterList(TObject *Sender)                                   // :2098 Landam
{
    TEdit *edit=dynamic_cast <TEdit *>   (Sender);
    if(edit==0 || tmpList==0)                                                   // [W906] golden would AV (never reached while the dialog is open)
        return;
    AnsiString str1, str2;
    W906_ListClear(lstHDFile);                                                  // :2102 lstHDFile->Clear();
    W906_ListClear(lstServerFile);
    str2= edit->Text.UpperCase();
    for(int i=0; i<tmpList->Count; ++i)
    {
        str1=AnsiString(tmpList->Strings[i]).UpperCase();
        if(str1==str2 || str2=="")                                  //Sam 20190925 : 修正 FTP 相似檔名無法下載問題
        {
            str1=tmpList->Strings[i];
            lstHDFile->Items->Add(str1);
            lstServerFile->Items->Add(str1);
        }
    }
}

void TfFTPClient::edtHDWaferNameChange(TObject *Sender)                         // :2117
{
    FilterList(Sender);
}

void TfFTPClient::edtServerWaferNameChange(TObject *Sender)                     // :2122
{
    FilterList(Sender);
}

void TfFTPClient::lstServerFileDblClick(TObject *Sender)                        // :2127-2137 (Server tab, F2)
{
    (void)Sender;
    int index=lstServerFile->ItemIndex;
    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20200803 : 全智要求不能從選單連點選取工作檔
        return;
    if(index<0)
        return;

    AnsiString str=lstServerFile->Items->Strings[index];
    W906_SetText(edtServerWaferName, str);                                      // :2136
}

void TfFTPClient::lstHDFileDblClick(TObject *Sender)                            // :2139-2145
{
    (void)Sender;
    int index=lstHDFile->ItemIndex;
    if(index<0)
        return;
    W906_SetText(edtHDWaferName, lstHDFile->Items->Strings[index]);             // :2144
}

void TfFTPClient::GetTesterType()                                               // :2147-2188 Steven 20121018 : 取消不用MDB, 改用直接讀取文字檔
{
    int i, j;
    bool bAlreadyIn;
    TStringList sListY;
    TStringList sListX;

    if(FileExists(IniConfig.N06_TasterListFile)==false)                         // :2154
    {
        // golden :2155-2157 `return` has no ';' -> `return cbTesterType->Clear();` (QUIRK kept: the combo is cleared ONLY when
        //   the file is missing; with the file there golden never clears cbTesterType, it only adds types not in it yet)
        W906_ComboClear(cbTesterType);
        return;
    }
    W906_ComboClear(cbTesterID);                                                // :2158
    W906_ComboClear(cbTasterIp);

    cbTesterType->Text="";                                                      // :2161
    cbTesterID  ->Text="";
    cbTasterIp  ->Text="";
    edTesterName->Text="";

    sListY.LoadFromFile(IniConfig.N06_TasterListFile); //讀檔                  // :2166

    for(i=1; i<sListY.Count; i++)
    {
        sListX.CommaText=sListY.Strings[i]; //取得ROW
        bAlreadyIn=false;

        for(j=0; j<cbTesterType->Items->Count; j++)
        {
            if(W906_Str(sListX, 0)==cbTesterType->Items->Strings[j])
                bAlreadyIn=true;
        }

        if(bAlreadyIn==false)
        {
            cbTesterType->Items->Add(W906_Str(sListX, 0));                      // :2181
        }
    }
    sListY.Clear();    //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    sListX.Clear();    //Ifor 20170603 (wei) TStringList 刪除前先 Clean
}

// ---------------------------------------------------------------------------
//  KeyPress (:2190-2220) and CloseQuery (:2258-2263)
// ---------------------------------------------------------------------------
void TfFTPClient::edtServerWaferNameKeyPress(TObject *Sender, char &Key)
{
    (void)Sender;
    if(CosFunction.bFTPUseBarcodeReader==true && IniConfig.bN06_UseBarcode==true)
    {
        Key=0;                                                                  // golden Key=NULL
    }
    else
    {
        if(Key==VK_RETURN && edtServerWaferName->Text.Length()!=0)
        {
            W906_St02_FtpGated("golden FTPClient.cpp:2201 plSLoadClick(this) -- the Server download, card F2");
        }
    }
}

void TfFTPClient::edtHDWaferNameKeyPress(TObject *Sender, char &Key)
{
    (void)Sender;
    if(CosFunction.bFTPUseBarcodeReader==true && IniConfig.bN06_UseBarcode==true)
    {
        Key=0;                                                                  // golden Key=NULL
    }
    else
    {
        if(Key==VK_RETURN && edtHDWaferName->Text.Length()!=0)
        {
            plLoadClick(this);                                                  // :2217
        }
    }
}

void TfFTPClient::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    (void)Sender;
     if(bCanExit==false)  //ChungHung 20150413 add for TSMC                     // :2261
        CanClose=false;
}
