// =============================================================================
//  forms/fPassword.cpp  --  TfPassword bodies that fit the ht9045_forms link
//  diet (vclcompat + ht9045_globals + ht9045_core) -- i.e. every golden
//  method EXCEPT sbPasswordModifyOKClick, which needs ShowMyMessage
//  (ht9045_sm) and lives in the ROOT Password.cpp (cSetUp.cpp precedent).
//
//  AI(W906-FW-QWKEY1) 20260824: new file, translation wave FW-QWKEY1.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/Password.cpp
//  (398 lines, cp950, 0 U+FFFD, this wave).  Full banner -- scope table,
//  GATE REGISTER (P-B1..P-D1), GOLDEN NOTES (G-P1..G-P4), DEVIATIONS,
//  SECURITY DELTA, CMAKE STATUS -- lives in forms/fPassword.h; this file
//  carries per-statement pointer-back comments only.
//
//  CMAKE STATUS -- NOT WIRED by this wave.  Suggested: ht9045_forms source
//  list (next to forms/fSetup.cpp).  See forms/fPassword.h.
// =============================================================================
#include "forms/fPassword.h"
#include "BarcodeReader.h"   // AI(W906-FW-BARCODE3) 20260825: InputBarcodeNumber real since FW-BARCODE1 (e7b4bf8) -- P-B1..B4 opened, Clipboard lines narrowed

#include "forms/fQwertyKey.h"   // fQwertyKey global + ShowQwertyKey (landed this same wave)
#include "MachineType.h"        // CC_Greatek/CC_SCC/CC_KYEC_LEE (:330/:316/:292)
#include "cmydef.h"             // CUSTOMER_CODE/JCET_FOR_EVAN/USE_BARCODE_AS_KEYBOARD/
                                // bEnableEmployeeIDCheck/SystemStart/N_* flags
#include "Config.h"             // IniConfig (:1499)
#include "CosFunction.h"        // CosFunction (:488)

// golden :16-18 -- zero-initialized globals, NEVER constructed at static
// init (header NULL-GLOBAL note).
TfPassword *fPassword;
TfPassword *fPassword2;         //Steven 20150505 : 剛好在輸入時,Alarm會卡死
EvenLog_PASS_WORD EvenLogUSER;  //Steven 20181224 : For ASE-CL
//---------------------------------------------------------------------------
// AI(W906-FW-QWKEY1) 20260824: PORT-ONLY ctor -- fields only (defaults live
// as in-class member initializers in forms/fPassword.h).  Golden's ctor body
// is Init().
TfPassword::TfPassword()
{
    PageControl1->ActivePage=tsPassword;   // .dfm PageControl1 ActivePage=tsPassword
    cbUserName->Left=238;                  // .dfm cbUserName Left (FormShow later writes 216)
}
//---------------------------------------------------------------------------
// golden ctor body :20-26 -> explicit Init() (header DEVIATION (D-2)).  The
// ReadPasswordFile() call is ACTIVE; the password-book I/O gate sits INSIDE
// ReadPasswordFile itself (GATE (P-R1)).
void TfPassword::Init()
{
    bShow=false;
    bShowTab=0;                                                                 //Steven 20181224 : For ASE-CL
    ReadPasswordFile();                                                         //讀取密碼
}
//---------------------------------------------------------------------------
void TfPassword::ShowEventLogLogin()                                            //Steven 20181224 : For ASE-CL  -- golden :28-32
{
    bShowTab=1;
    fPassword->ShowModal();                                                     // offline no-op -- header NULL-GLOBAL note
}
//---------------------------------------------------------------------------
void TfPassword::FormShow()                                                     // golden :34-116 -- `TObject *Sender` dropped, never read
{
    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20200225 : 超豐的輸入帳號密碼位置改到右上，避免被他們的軟體視窗蓋掉。
    {
        Left=745;
        Top =0;
    }
    else
    {
        Left=(1024-Width)/2;
        Top =(768-Height)/2;
    }
    edPassword->Text="";
    edUserName->Visible=true;                                                   //Steven 20140626 : Fixed SetFocus問題
    edUserName->SetFocus();
    bShow=true;

    if(fQwertyKey->Showing)                                                     //jou 2013-04-16 必須強制關閉 fQwertyKey,不然密碼輸入錯誤偶發會hangup
    {
        fQwertyKey->Close();
    }

    //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    //{
    if(JCET_FOR_EVAN==1)                                                        //Steven 20221216 : 吳如春要改成從網路抓密碼本
    {
        edPassword->SetFocus();
        cbUserName->Visible=false;
        edUserName->Visible=false;
        lblUserName->Visible=false;
    }
    else
    {
        cbUserName->Visible=CosFunction.bTechComUseComboBox;
        edUserName->Visible=!CosFunction.bTechComUseComboBox;
    }

    if(bEnableEmployeeIDCheck==true)
    {
        cbUserName->Visible=false;
        edUserName->Visible=true;
        edUserName->Text="";        // name
        edPassword->Text="";        // password
    }
    cbUserName->Left=216;                                                       //Ifor 20190517 與Password切齊 200->216
    cbUserName->ItemIndex=0;

    if(CosFunction.bTechComUseComboBox)
        edUserName->Text=cbUserName->Text;
    //}

    btnDownload->Visible=IniConfig.bFtpPasswordDownload;                        //Steven 20221216 : 吳如春要改成從網路抓密碼本
    lblPWDownload->Visible=false;
    //Steven 20181224 : For ASE-CL
    //==>
    if(bShowTab==0)
    {
        tsPassword->TabVisible=true;
        tsEventlogLogin->TabVisible=false;
        tsEventLogLoginModify->TabVisible=false;
        PageControl1->ActivePage=tsPassword;
    }
    else if(bShowTab==0)                                                        // GOLDEN (G-P1): duplicate ==0 -- this branch is unreachable dead code in golden; translated verbatim
    {
        tsPassword->TabVisible=false;
        tsEventlogLogin->TabVisible=true;
        tsEventLogLoginModify->TabVisible=false;
        PageControl1->ActivePage=tsEventlogLogin;
    }
    //<==
    //Steven 20181224 : For ASE-CL
    if(CosFunction.bLoginASECL)                                             //JerryYang 20250120 : add
    {
        lblUserName->Visible=false;
        edUserName->Visible=false;
        edPassword->SetFocus();
    }
    else
    {
        lblUserName->Visible=true;
        edUserName->Visible=true;
    }
}
//---------------------------------------------------------------------------
void TfPassword::SpeedButton1Click()                                            // golden :118-121 -- DFM-ORPHANED (header G-P3); `TObject *Sender` dropped
{
    Close();
}
//---------------------------------------------------------------------------
void TfPassword::spbCancelClick()                                               // golden :123-128 -- `TObject *Sender` dropped, never read
{
    edPassword->Text="";
    edUserName->Text="";
    Close();
}
//---------------------------------------------------------------------------
void TfPassword::edUserNameKeyDown(unsigned short &Key)                         // golden :130-163 -- Sender/Shift dropped; WORD -> unsigned short
{
    #ifndef SOFT_SIMULTE
    if(IniConfig.bVTESTFunction==true)                                          //jou 20220912 : 增加VTEST不可以用鍵盤輸入
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE (P-B1) OPENED -- InputBarcodeNumber
        // real since FW-BARCODE1 (e7b4bf8); only the Clipboard() lines stay
        // gated (GATE (CLIP) in place).
        edUserName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        edUserName->Text=InputBarcodeNumber("Input User Name:", "UserName");

        edPassword->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        edPassword->Text=InputBarcodeNumber("Input Password:", "Password");
        if(edPassword->Text!="" && edUserName->Text!="")
            Close();
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            //Steven 20200302 : SCC楊恩民說只能用Barcode輸入
            CosFunction.bUseBarCoderAutoLogin)                                  //Sam 20221101 : 使用 BarCoder 自動登錄
    {
        edUserName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input User Name:", "UserName");
        edUserName->Text=sBarcodeID;
    }
    else
    #endif
    {
        if(Key==0x0d)                                                           //按下Enter
        {
            if(edPassword->Visible)                                             //JimmyChiu 20211122 fix get error bug without edpassword
                edPassword->SetFocus();
        }
    }
}
//---------------------------------------------------------------------------
void TfPassword::edPasswordKeyDown(unsigned short &Key)                         // golden :165-184 -- Sender/Shift dropped; WORD -> unsigned short
{
    #ifndef SOFT_SIMULTE
    if(CUSTOMER_CODE==CC_SCC || IniConfig.bVTESTFunction==true)                 //Steven 20200302 : SCC楊恩民說只能用Barcode輸入  //jou 20220912 : 增加VTEST不可以用鍵盤輸入
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE (P-B2) OPENED -- see P-B1 above.
        edPassword->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input Password:", "Password");
        edPassword->Text=sBarcodeID;
        if(edPassword->Text!="" && edUserName->Text!="")
            Close();
    }
    else
    #endif
    {
        if(Key==0x0d)                                                           //按下Enter
            Close();
    }
}
//---------------------------------------------------------------------------
void TfPassword::FormClose()                                                    // golden :186-207 -- `TObject *Sender, TCloseAction &Action` dropped, never read
{
    //Sam 20171019 (wei) : 超豐要求 BarCode 讀完 UserName 的字元要刪除 '$'
    //==>
    if(CUSTOMER_CODE==CC_Greatek)
    {
        if(edUserName->Text.Trim()!="" &&
           edUserName->Text.SubString(1, 1)=="$")
        {
            edUserName->Text=edUserName->Text.SubString(2, edUserName->Text.Length());
        }
    }
    //<==
    //Sam 20171019 (wei) : 超豐要求 BarCode 讀完 UserName 的字元要刪除 '$'
    edPasswordPassWord->Text="";    //Steven 20181224 : For ASE-CL
    edModifyPassword->Text="";      //Steven 20181224 : For ASE-CL
    edReModifyPassword->Text="";    //Steven 20181224 : For ASE-CL
    SystemStart=false;              //Ifor 20180928 :Add 避免密碼輸入完後機台直接動作  -- SAFE direction (header scope table)
    bShow=false;
    bShowTab=0;                     //Steven 20181224 : For ASE-CL
}
//---------------------------------------------------------------------------
void TfPassword::edPasswordMouseDown()                                          // golden :209-232 -- Sender/Button/Shift/X/Y dropped, never read
{
    #ifndef SOFT_SIMULTE
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        fQwertyKey->ShowQwertyKey(edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            //Steven 20200302 : SCC楊恩民說只能用Barcode輸入
       USE_BARCODE_AS_KEYBOARD!=0 || IniConfig.bVTESTFunction==true)            //jou 20220912 : 增加VTEST不可以用鍵盤輸入
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE (P-B3) OPENED -- see P-B1 above.
        edPassword->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input Password:", "Password");
        edPassword->Text=sBarcodeID;
        if(edPassword->Text!="" && edUserName->Text!="")
            Close();
    }
    else
    #endif
    {
        fQwertyKey->ShowQwertyKey(edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);    //Steven 20120904 : 改用QWERTY鍵盤
    }
}
//---------------------------------------------------------------------------
void TfPassword::edUserNameMouseDown()                                          // golden :234-263 -- Sender/Button/Shift/X/Y dropped, never read
{
    #ifndef SOFT_SIMULTE
    if(IniConfig.bVTESTFunction==true)                                          //jou 20220912 : 增加VTEST不可以用鍵盤輸入
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE (P-B4) OPENED -- see P-B1 above.
        edUserName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        edUserName->Text=InputBarcodeNumber("Input User Name:", "UserName");

        edPassword->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        edPassword->Text=InputBarcodeNumber("Input Password:", "Password");
        if(edPassword->Text!="" && edUserName->Text!="")
            Close();
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            //Steven 20200302 : SCC楊恩民說只能用Barcode輸入
            CosFunction.bUseBarCoderAutoLogin)                                  //Sam 20221101 : 使用 BarCoder 自動登錄
    {
        edUserName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input User Name:", "UserName");
        edUserName->Text=sBarcodeID;
    }
    else
    #endif
    {
        fQwertyKey->ShowQwertyKey(edUserName, N_NO_SYMBOL|N_NO_SPACE);          //Steven 20120904 : 改用QWERTY鍵盤
    }
}
//---------------------------------------------------------------------------
void TfPassword::cbUserNameChange()                                             // golden :265-269 -- `TObject *Sender` dropped, never read
{
    if(CosFunction.bTechComUseComboBox)                                         //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
        edUserName->Text=cbUserName->Text;
}
//Steven 20181224 : For ASE-CL
//==>
//---------------------------------------------------------------------------
void TfPassword::sbPasswordCancelClick()                                        // golden :273-276 -- `TObject *Sender` dropped, never read
{
    Close();
}
//---------------------------------------------------------------------------
void TfPassword::sbPasswordOKClick()                                            // golden :278-282 -- `TObject *Sender` dropped, never read
{
    CheckPassword();
    Close();
}
//---------------------------------------------------------------------------
// TfPassword::sbPasswordModifyOKClick (golden :284-297) lives in the ROOT
// Password.cpp -- ShowMyMessage (canary_support.h:80) is an ht9045_sm-layer
// symbol outside this file's ht9045_forms diet.  See forms/fPassword.h.
//---------------------------------------------------------------------------
void TfPassword::sbPasswordModifyClick()                                        // golden :299-312 -- `TObject *Sender` dropped, never read
{
    CheckPassword();
    if(bLoginSuccess)
    {
        cob_ModifyUseIDList->ItemIndex=cob_UseIDList->ItemIndex;
        PageControl1->ActivePage=tsEventLogLoginModify;
    }
    else
    {
        cob_UseIDList->ItemIndex=0;
        Close();
    }
}
//---------------------------------------------------------------------------
void TfPassword::SetUserLevel(int iLevel)                                       // golden :314-317
{
    iNowEvenLogUserLevel=iLevel;
}
//---------------------------------------------------------------------------
void TfPassword::CheckPassword()                                                // golden :319-336
{
    bLoginSuccess=false;
    int iLength=sizeof(EvenLogUSER.ID)/sizeof(EvenLogUSER.ID[0]);
    for(int i=0; i<iLength; i++)
    {
        if(EvenLogUSER.ID[i]==cob_UseIDList->Text ||
           JCET_FOR_EVAN==1)                                                    //Steven 20221216 : 吳如春要改成從網路抓密碼本  -- GOLDEN (G-P4)
        {
            if(EvenLogUSER.PassWord[i]==edPasswordPassWord->Text)
            {
                bLoginSuccess=true;
                iNowEvenLogUserLevel=i;                                         //目前的EvenLog Level
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
bool TfPassword::CheckLoginSuccess()                                            // golden :338-341
{
    return bLoginSuccess;
}
//---------------------------------------------------------------------------
int TfPassword::GetLoginLevel()                                                 // golden :343-346
{
    return iNowEvenLogUserLevel;
}
//---------------------------------------------------------------------------
void TfPassword::ReadPasswordFile()                                             // golden :348-369
{
    AnsiString sFileName=EVENLOG_LEVEL_INI;                                     // AI(W906-BA-GL6o) 20260911: was the hardcoded literal; see fPassword.h

    // AI(W906-FW-QWKEY1) 20260824: GATE (P-R1), all six PassWord reads --
    // CheckAndReadIniData WRITES a missing key back into the shared-machine
    // password book (would seed password "1" for every account).  ⚠ SECURITY
    // DELTA while gated: PassWord[] stays "" and CheckPassword() would accept
    // an empty password -- the wiring wave must resolve this gate BEFORE any
    // EventLog-login path is wired.  Full argument: forms/fPassword.h.
    EvenLogUSER.ID[OPERATOR]        ="OPERATOR";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[OPERATOR]  =CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[OPERATOR], AnsiString("1"));
#endif // GATE (P-R1)

    EvenLogUSER.ID[ENG_VENDOR]      ="ENG_VENDOR";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[ENG_VENDOR]=CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[ENG_VENDOR], AnsiString("1"));
#endif // GATE (P-R1)

    EvenLogUSER.ID[ENG_MI]          ="ENG_MI";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[ENG_MI]    =CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[ENG_MI] , AnsiString("1"));
#endif // GATE (P-R1)

    EvenLogUSER.ID[PM]              ="PM";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[PM]        =CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[PM], AnsiString("1"));
#endif // GATE (P-R1)

    EvenLogUSER.ID[AUDIT]           ="AUDIT";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[AUDIT]     =CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[AUDIT], AnsiString("1"));
#endif // GATE (P-R1)

    EvenLogUSER.ID[ADMIN]           ="ADMIN";
#if 0 // GATE (P-R1)
    EvenLogUSER.PassWord[ADMIN]     =CheckAndReadIniData(sFileName, "LEVEL", EvenLogUSER.ID[ADMIN], AnsiString("1"));
#endif // GATE (P-R1)
    (void)sFileName;    // PORT-ONLY: silences -Wunused-variable while (P-R1) is closed; delete when the gate opens
}
//---------------------------------------------------------------------------
void TfPassword::SavePasswordFile()                                             // golden :371-381
{
    AnsiString sFileName = EVENLOG_LEVEL_INI;                                   // AI(W906-BA-GL6o) 20260911: was the hardcoded literal; see fPassword.h

    // AI(W906-FW-QWKEY1) 20260824: GATE (P-S1) -- rewrites the on-disk
    // password book (密碼檔寫入, 一律 gate 不解).  See forms/fPassword.h.
#if 0 // GATE (P-S1)
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[OPERATOR],    EvenLogUSER.PassWord[OPERATOR]);       //未登入也使用OPERATOR
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[ENG_VENDOR],  EvenLogUSER.PassWord[ENG_VENDOR]);
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[ENG_MI],      EvenLogUSER.PassWord[ENG_MI]);
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[PM],          EvenLogUSER.PassWord[PM]);
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[AUDIT],       EvenLogUSER.PassWord[AUDIT]);
    WriteIniData(sFileName, "LEVEL", EvenLogUSER.ID[ADMIN],       EvenLogUSER.PassWord[ADMIN]);
#endif // GATE (P-S1)
    (void)sFileName;    // PORT-ONLY: silences -Wunused-variable while (P-S1) is closed; delete when the gate opens
}
//---------------------------------------------------------------------------
//<==
//Steven 20181224 : For ASE-CL
void TfPassword::btnOKClick()                                                   // golden :385-388 -- `TObject *Sender` dropped, never read
{
    Close();
}
//---------------------------------------------------------------------------
void TfPassword::btnDownloadClick()                                             // golden :390-397 -- `TObject *Sender` dropped, never read
{
    //Steven 20221216 : 吳如春要改成從網路抓密碼本
    // AI(W906-FW-QWKEY1) 20260824: GATE (P-D1) -- fFTPClient singleton was
    // DEMOTED by the KYECFTP wave (no `fFTPClient->` surface exists).
#if 0 // GATE (P-D1)
    bool bFlag=fFTPClient->DownloadPasswordFormServer();                        //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
    if(bFlag)
        lblPWDownload->Visible=true;
#endif // GATE (P-D1)
    btnDownload->Down=false;
}
//---------------------------------------------------------------------------
