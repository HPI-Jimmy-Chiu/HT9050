// ===========================================================================
//  KYECFTP/FTPClientForm_St02.h -- golden TfFTPClient (the KYEC FTP dialog), F1 = the parts with no network.
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  Card LI-9 F1 (docs/C7_FTPSAVE_SGJAM_PLAN_20261002.md 3.4-3.8, 4.3 F1).
//  Golden = 906_0625_Steven (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven) KYECFTP/FTPClient.h / .cpp / .dfm;
//  every "golden :N" below is FTPClient.cpp of that tree unless another file is named.
//
//  WHAT IS HERE (F1)
//    ShowFTPModal (:1215-1573) for HD = 1 (HD) and 2 (Tester); HD = 0 (Server) is network = F2 (GATE LI9-F2 below).
//    FormShow (:1708-1797), FormClose (:1655-1701), FormCloseQuery (:2258-2263), Button3Click (:1703-1706),
//    plLoadClick (:1575-1653), FilterList / edt*Change / lst*DblClick (:2098-2145), edt*KeyPress (:2190-2220),
//    the Tester tab: cbTesterTypeChange (:1831-1866), btSafeTasterNameClick (:1868-1952), cbTesterIDChange (:1954-1958),
//    rgInputMethodClick (:2067-2084), GetTesterType (:2147-2188), memoFTPDblClick (:2062-2065).
//  F2-3 (AI(W906-W202) 20261009 (St02-E)): plUnloadClick 'Upload to Server' = golden 913 :1128-1229, body in its own TU
//    KYECFTP/FTPClientUpload_St02.cpp (it needs KYECFTP/FTPClient_Transfer.h, i.e. ht9045_kyecftp).
//  NOT HERE: plSLoadClick / LoadFileFormServer2 / DownloadPasswordFormServer / NMFTP1* wiring (F2-down),
//    Panel15Click / Panel16Click (Server tab, F2), the barcode KeyDown / MouseDown (:3734-3821, InputBarcodeNumber),
//    every other TfFTPClient method (N25 / N31 / N32 / ChipMos / JHT / 2DID ... -- other cards).
//
//  THE FACADE.  The members are golden FTPClient.h's names and VCL types (vclcompat/Controls.h).  Only what F1 reads or
//    writes is declared.  Deviations, each marked [W906] where it happens:
//    * ShowModal cannot block the wb_serve tick: it opens the web window (page table W906_FormProgramShow("fFTPClient"))
//      and returns; the code golden runs after ShowModal returned runs when the dialog closes (Close() below calls
//      W906_ModalEnded, set by the caller -- forms/fLotInfo_Ftp_St02.cpp btnFtpServerClick's tail :5110-5125).
//    * bError / bListOk stay MEMBERS here (golden FTPClient.h:146 / :154).  KYECFTP/FTPClient_EventHandlers.h demoted
//      them (and tmpList / bTempList) to bare globals in ht9045_kyecftp, which wb_serve does not link (CMakeLists.txt
//      wb_serve link line).  F1 never lists the server, so nothing here reads those globals; tmpList is this TU's own
//      (internal linkage).  F2 must join the two when ht9045_kyecftp is linked (NMFTP1ListItem fills THAT tmpList).
//    * VCL semantics the golden lines rely on are modelled by small helpers (W906_SetText / W906_ComboSetItemIndex /
//      W906_RadioSetItemIndex ...): a TEdit fires OnChange only when Text really changes; a TRadioGroup fires OnClick when
//      ItemIndex is set to a new value; TComboBox / TListBox Clear() also clear the selection (and the combo text).
//    * VCL exceptions golden relies on (LoadFromFile / SaveToFile / TStringList index) are raised as W906_St02_VclError
//      at the same expression (vclcompat itself does not throw); the WS layer reports them the way VCL's default
//      handler would show them (a message, the handler stops there).
// ===========================================================================
#ifndef KYECFTP_FTPCLIENTFORM_ST02_H
#define KYECFTP_FTPCLIENTFORM_ST02_H

#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"

// golden TPageControl->ActivePage (vclcompat TPageControl only has ActivePageIndex)
struct TfFTPClientPageControl : public TPageControl {
    TTabSheet* ActivePage;
    TfFTPClientPageControl() : ActivePage(0) {}
};

// A VCL exception golden would raise at that expression (EFOpenError / EFCreateError / EStringListError).
struct W906_St02_VclError {
    std::string msg;
    explicit W906_St02_VclError(const std::string& m) : msg(m) {}
};

// What one WS operation reached (filled while an op runs; WebLotInfoFtp_St02.cpp reports it).
struct W906_St02_FtpReport {
    std::vector<std::string> gated;      // "golden :N -- why": S25 customer bodies and F2 network parts that were reached
    std::vector<std::string> notes;      // other port facts worth showing (e.g. the HD list fallback)
    std::string sessionJson;             // FileRW session of the recipe change (wb_serve only; see W906_St02_FtpHooks)
};
extern W906_St02_FtpReport* W906_St02_FtpCurrentReport;   // NULL = nobody is collecting
void W906_St02_FtpGated(const std::string& what);
void W906_St02_FtpNote(const std::string& what);

// Hooks the dialog needs from wb_serve-only code (WebLotInfoFtpInstall_St02.cpp sets them; a ctest sets fakes).
struct W906_St02_FtpHooks {
    int        (*ChangeSetUpFile)(AnsiString FileName);   // golden fMain->ChangeSetUpFile (golden 906_0625_Steven main.cpp:25019) = WebRecipeChange.cpp
                                                          //   W906_RC_ChangeSetUpFile; NULL -> plLoadClick reports not-installed (mode 1)
    void       (*cbSetupFileNameChange)();                // golden fMain->cbSetupFileNameChange (F2 plSLoadClick :1029); unused in F1
    bool       (*RecipeChainsReady)();                    // port-only guard (WebRecipeChange.cpp recipe.change: boot-read-chain-not-run)
    AnsiString (*LocalIp)();                              // golden :1919-1934 WSAStartup / gethostname / gethostbyname; NULL = that code
};
extern W906_St02_FtpHooks W906_St02_Ftp;

class TfFTPClient : public TObject
{
public:     // golden __published (FTPClient.h:17-96) -- the F1 subset
    TfFTPClientPageControl *PageControl1;
    TTabSheet  *TabSheet1;              // 'Network Setting' (never visible: ShowFTPModal hides it in every case)
    TTabSheet  *TabSheet2;              // 'Server'
    TTabSheet  *TabSheet3;              // 'HD'
    TTabSheet  *TabSheet4;              // 'Taster'
    TEdit      *edtServerWaferName;
    TListBox   *lstServerFile;
    TEdit      *edtHDWaferName;
    TListBox   *lstHDFile;
    TListBox   *ListBox1;               // dfm Visible=False
    TPanel     *plLoad;                 // 'Load from HD'
    TPanel     *plUnload;               // 'Upload to Server' (F2-3)
    TPanel     *plSLoad;                // 'Download to Handler' (F2)
    TPanel     *Panel15;                // 'Copy File Name' (Server tab, F2)
    TPanel     *Panel16;                // 'Clean File Name' (Server tab, F2)
    TPanel     *plUnloadALL;            // 'Upload All to Server' (dfm Visible=False; GIGAS, F2)
    TEdit      *edHandlerType;
    TEdit      *edHandlerID;
    TButton    *btSafeTasterName;       // 'Save'
    TButton    *Button3;                // 'Exit'
    TMemo      *memoFTP;
    TRadioGroup *rgInputMethod;         // 'By List' / 'Manually'
    TGroupBox  *grpTesterMap;
    TGroupBox  *grpTesterName;
    TComboBox  *cbTesterType;
    TComboBox  *cbTesterID;
    TComboBox  *cbTasterIp;             // dfm Enabled=False
    TEdit      *edTesterName;
    TLabel     *labN06_DownloadPath;
    TEdit      *FTP_DownPath;
    TEdit      *FTP_UpLdPath;
    TLabel     *labN06_UploadPath;
    TLabel     *labN06_DownloadPath1;
    TEdit      *FTP_DownPath1;
    TLabel     *labN06_UploadPath1;
    TEdit      *FTP_UpLdPath1;
    int         Left;                   // golden FormShow :1711-1712 (stored only; the page table places the window)
    int         Top;
    AnsiString  Caption;                // dfm 'FTP'

    // golden event handlers (FTPClient.h:98-139), F1 subset
    void plLoadClick(TObject *Sender);
    void plUnloadClick(TObject *Sender);   // golden 913 :1128-1229 (F2-3; KYECFTP/FTPClientUpload_St02.cpp, links ht9045_kyecftp)
    void FormClose(TObject *Sender);    // golden (TObject *Sender, TCloseAction &Action): Action is never touched
    void Button3Click(TObject *Sender);
    void FormShow(TObject *Sender);
    void cbTesterTypeChange(TObject *Sender);
    void btSafeTasterNameClick(TObject *Sender);
    void cbTesterIDChange(TObject *Sender);
    void memoFTPDblClick(TObject *Sender);
    void rgInputMethodClick(TObject *Sender);
    void FilterList(TObject *Sender);
    void edtHDWaferNameChange(TObject *Sender);
    void edtServerWaferNameChange(TObject *Sender);
    void lstServerFileDblClick(TObject *Sender);
    void lstHDFileDblClick(TObject *Sender);
    void edtServerWaferNameKeyPress(TObject *Sender, char &Key);
    void edtHDWaferNameKeyPress(TObject *Sender, char &Key);
    void FormCloseQuery(TObject *Sender, bool &CanClose);

private:    // golden FTPClient.h:140-149
    AnsiString sRootPath;
    bool enter;
    bool bListOk;
    int iHD;
    void GetTesterType();

public:     // golden FTPClient.h:150-159
    bool bShow;
    bool bControlBySECSGEM;
    int  iErrorBySECSGEM;
    bool bError;
    AnsiString aSetUpNameBySECSGEM;
    bool bControlByGPIB;
    int  iErrorByGPIB;
    AnsiString asSetUpNameByGPIB;

    TfFTPClient();                      // golden ctor :50-64 + the dfm initial values
    void ShowFTPModal(int HD);          // golden :1215-1573
    void Close();                       // golden TCustomForm::Close: FormCloseQuery -> FormClose (OnClose) -> the modal returns

    // ---- [PORT-ONLY] (W906) ----
    bool W906_bModalOpen;               // ShowModal "is running" = the web window is open
    int  W906_iOpenerTag;               // the Lot Info button that opened it (golden Ptr->Tag)
    void (*W906_ModalEnded)(int iOpenerTag);   // the caller's code after ShowModal returned (set before ShowFTPModal)
    std::string W906_LastVclError;      // last W906_St02_VclError (kept for the state reply)
    int  W906_iLastLoadMode;            // plLoadClick's ChangeSetUpFile result (0 changed, 1 refused, -1 not reached)
    int  W906_GetHD() const { return iHD; }  bool W906_Shown() const { return bShow; }   // W906_Shown: golden's own bShow, read on purpose (not W906_FormShowing: a web window can outlive the dialog, cf. TrayEditForm.cpp DoClose); FShow_Audit counts X->bShow reads only
    bool W906_GetEnter() const { return enter; }
    // VCL property semantics (see the file banner)
    void W906_SetText(TEdit* e, const AnsiString& v);
    static void W906_ListClear(TListBox* l);
    static void W906_ComboClear(TComboBox* c);
    static void W906_ComboSetItemIndex(TComboBox* c, int v);
    void W906_RadioSetItemIndex(TRadioGroup* r, int v);
    bool W906_BarcodeKeys() const;      // golden KeyPress :2193 / :2209: CosFunction.bFTPUseBarcodeReader && IniConfig.bN06_UseBarcode
};

extern TfFTPClient *fFTPClient;         // golden FTPClient.h:206 (FTPClient.cpp:41)

// golden file-scope helpers this dialog uses (FTPClient.cpp)
extern TStringList* W906_St02_FtpTmpList();   // golden `TStringList *tmpList;` (:42) -- this TU's copy in F1 (see the banner)
bool W906_St02_FtpCanExit();                  // golden `bool bCanExit=true;` (:48), read for the state reply

#endif // KYECFTP_FTPCLIENTFORM_ST02_H
