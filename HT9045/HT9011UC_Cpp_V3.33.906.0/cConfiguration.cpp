// =============================================================================
//  cConfiguration.cpp  --  configuration-form display side: pure
//                           ShowQwertyKey one-liners + the Tray/HP StringGrid
//                           read/edit/reload cluster (batch-5 cConfiguration
//                           Wave A -- see forms/fConfiguration.h banner
//                           FIRST, it documents the full 28-method
//                           classification, GATE register, DEVIATION list,
//                           and TfConfigurationGrid design notes; not
//                           repeated in full here).
//
//  Faithful translation of golden cConfiguration.cpp (7,808 lines, BCB6,
//  cp950 -- decoded with `python3 -c "open(path,'rb').read()
//  .decode('cp950')"`, 0 U+FFFD, 20260820).
//  Translator: AI(W906-FW3-Config-WA) 20260820.
//  Translation wave: FW-3 cConfiguration Wave A (batch 5, display-only
//  mandate, per docs/RECON_cConfiguration_displayside.md section 6).
//  Facade: forms/fConfiguration.h (NEW this wave -- no prior facade existed
//  anywhere in this tree, verified by grep 20260820).
//
//  WAVE SCOPE / GATE REGISTER (WA-1) / DEVIATION / DESIGN NOTES -- all in
//  forms/fConfiguration.h. Summary: 28 of 129 golden TfConfiguration::
//  methods (see docs/RECON_cConfiguration_displayside.md section 1 for the
//  full 129-method inventory), 289 golden lines by this wave's own per-
//  method accounting (the recon's section 6.4 rollup states 354; see this
//  file's facade header for the reconciliation of that discrepancy). 17 of
//  the 28 touch GATE WA-1 (fQwertyKey; OPENED 20260824 by FW-QWKEY2 --
//  entity real since FW-QWKEY1); the remaining 11 are fully real.
//
//  fConfiguration global is NOT declared this wave -- see forms/
//  fConfiguration.h's own INTEGRATION STATUS note (same posture as
//  cTemperFrom.cpp/cSpeed.cpp: the ctor is (b) write-path and out of this
//  wave's scope, so there is nothing faithful to construct a live instance
//  from yet).
// =============================================================================
#include "forms/fConfiguration.h"

#include "cmydef.h"      // CUSTOMER_CODE/CC_SCC/CC_SCK/CC_AMD_M/CC_ETRENDTECH (EnableRMSFunc)
#include "Config.h"      // IniConfig (EnableRMSFunc: bShowLotInfo)
#include "CosFunction.h" // CosFunction (EnableRMSFunc: bEnableRMS)
#include "common.h"      // TrayTablePath/PlateTablePath (sbtReloadTrayClick/sbtReloadHPClick)

// AI(W906-FW-CFG-W1) 20260825: the registration table InitConfigEdtList_ItemA/
// B/C builds needs the edit-list layer. Both were already fully ported; this
// TU simply had no reason to see them until now.
#include "Public/HTEditList.h"  // HTEditList + the elConfig / elConfig_byRecipe / cbLastSet globals (:247/:252/:253) and the TEditContent enum (ECBool/ECInteger/ECText)
#include "LastSet.h"            // LAST_GENERAL_SET LastSet (:587) -- ItemA registers several fields against it
#include "mycylin.h"            // AI(W906-FW-CFG-W4b) 20260825: TMyCylinder Cylinder[] (:176) -- ChangeCBListProperty gates two checkbox groups on Cylinder[C_Shuttle_Knocker_1/2].Enable and Cylinder[C_HotplateVibration].Enable
#include "Motor/mymotor.h"      // AI(W906-FW-CFG-W5) 20260825: TTrayMotor MOT[] (:385) -- FormShow 用 MOT[i].Alias 當 soft-speed 標籤
#include "cMyDB.h"              // AI(W906-FW-CFG-W5) 20260825: MyDBIProcess -- FormShow 的 catch(...) 診斷沉澱點
// AI(W906-FW-CFG-W2) 20260825: TU-LOCAL FORWARD DECLARATION -- CheckFile.
// ItemD reads the Security_new.def input limit for D41IndexCheckOffset through
// it. The real declaration is cAuthority.h:83 and the real body
// cAuthority.cpp:503, but this TU CANNOT include cAuthority.h: that header
// pulls in language.h, whose `class TWinControl` (language.h:79) collides with
// the one Public/HTEdit.h:140 already defines, and HTEdit.h arrives here via
// the HTEditList.h include above. Measured, not assumed -- adding the include
// produced exactly that redefinition error and nothing else.
// A TU-local declaration is this tree's established answer to a header pair
// that cannot coexist; database.cpp:96 does the same thing for ShowMyMessage,
// and cObserver.cpp carries the identical pattern. Signature copied verbatim
// from cAuthority.h:83 so the two cannot disagree.
AnsiString CheckFile(AnsiString szDir, AnsiString str);

// ---------------------------------------------------------------------------
//  AI(W906-FW-CFG-W5) 20260825: TU-LOCAL FORWARD DECLARATIONS.
//  cAuthority.h cannot be included here -- it pulls in language.h, whose
//  `class TWinControl` collides with Public/HTEdit.h:140's, and HTEdit.h
//  arrives via HTEditList.h above. Measured in FW-CFG-W2, unchanged.
//  Signatures copied verbatim from cAuthority.h so the two cannot disagree.
//  NOTE on ChangeCompomentEnabled: its BODY is gated in cAuthority.cpp (that
//  header's own :32 note) because it walks a TWinControl control tree this
//  facade has none of -- so calling it is a documented no-op, not an effect.
// ---------------------------------------------------------------------------
extern bool authConfig[5];        // cAuthority.h:59
extern bool authConf[14];         // cAuthority.h:60  (JerryYang 20180511 12 -> 14)
void GetConfAuth();               // cAuthority.h:75
void ChangeCompomentEnabled(TWinControl *PCtrl, bool bEnable, bool bMustEnable=false);  // cAuthority.h:82
void ShowMyMessage(AnsiString S1, AnsiString S2="", AnsiString S3="",
                   bool Ok=false, bool bServoOff=false);   // canary_support.h:80-81

#ifndef HT9045_W906_FWCFG_TCOLOR_SHIM
#define HT9045_W906_FWCFG_TCOLOR_SHIM
// AI(W906-FW-CFG-W5) 20260825: TU-local standard-colour constants. Same
// convention cObserver.cpp:117-135 states: no shared umbrella colour header;
// each TU that needs a colour name not already visible declares its own.
static const TColor clRed = 0x000000FF;
#endif // HT9045_W906_FWCFG_TCOLOR_SHIM

// ---------------------------------------------------------------------------
//  AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:58-86 的檔案層資料。
//  `static`（內部連結）——golden cConfiguration.h 沒有把其中任何一個宣告成
//  extern，所以 TU 外沒有合法的引用者；static 只是少一個多餘的外部符號，
//  不改變任何可觀察行為。同 cObserver.cpp:151-156 的處理方式。
//
//  更正紀錄：bStopChange / bM01Enter / bSave 在 FW-CFG-W5 被誤加成
//  TfConfiguration 的成員。golden 把它們放在這裡（:58 / :63 / :86），
//  只有 fShow 才是類別成員（golden cConfiguration.h:2368）。W6 更正。
// ---------------------------------------------------------------------------
static bool bStopChange = false;                                                // golden :58
static bool bM01Enter = false;                                                  // golden :63  (ChungHung 20131009)
// JerryYang 20170922 (Steven) add for EP change log
static AnsiString aEP60OldData = "";                                            // golden :67  (Ifor 20170807)
static AnsiString aEP40OldData = "";                                            // golden :68
static AnsiString aEP30OldData = "";                                            // golden :69
static AnsiString aEP60OldData_NS = "";                                         // golden :70
static AnsiString aEP40OldData_NS = "";                                         // golden :71
static AnsiString aEP30OldData_NS = "";                                         // golden :72
static bool bEP60DataChange = false;                                            // golden :73
static bool bEP40DataChange = false;                                            // golden :74
static bool bEP30DataChange = false;                                            // golden :75
static bool bEP60DataChange_NS = false;                                         // golden :76  (Ifor 20170807)
static bool bEP40DataChange_NS = false;                                         // golden :77
static bool bEP30DataChange_NS = false;                                         // golden :78
static int  old60data = 0;                                                      // golden :79
static int  old40data = 0;                                                      // golden :80
static int  old30data = 0;                                                      // golden :81
static int  old60data_NS = 0;                                                   // golden :82
static int  old40data_NS = 0;                                                   // golden :83
static int  old30data_NS = 0;                                                   // golden :84
static double dTemp = 0.0;                                                      // golden :85
static bool bSave = false;                                                      // golden :86

#include <cstdlib>       // atoi/atof
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)

// -----------------------------------------------------------------------------
//  6.1 bucket -- pure ShowQwertyKey one-liners, GATE WA-1 (see forms/
//  fConfiguration.h banner; OPENED 20260824). The 13 one-liners below are
//  live, latent until HTEdit GATE (6) wires the keyboard instance.
// -----------------------------------------------------------------------------

// -- edSetTempClick (golden cConfiguration.cpp:5962-5965) -- WA-1 -----------
void TfConfiguration::edSetTempClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824) -- see forms/fConfiguration.h
    // GATE WA-1 OPENED 20260824 (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 (fc08e09).
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 300.0);
}

// -- edSetTempKeyDown (golden cConfiguration.cpp:5967-5971) -- WA-1 ---------
void TfConfiguration::edSetTempKeyDown(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 300.0);
}

// -- edE31_Fi1XClick (golden cConfiguration.cpp:5983-5986) -- WA-1 ----------
void TfConfiguration::edE31_Fi1XClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 6, true, 0.95, 1.05);
}

// -- edD25_60mmClick (golden cConfiguration.cpp:6440-6443) -- WA-1 ----------
void TfConfiguration::edD25_60mmClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.5, -0.5);   //2014-06-26 Dell for TSMC high-temp Load cell offset
}

// -- edD60_56mmClick (golden cConfiguration.cpp:6580-6583) -- WA-1 ----------
void TfConfiguration::edD60_56mmClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.5, -0.5);   //2014-06-26 Dell for TSMC high-temp Load cell offset
}

// -- edA32_1Click (golden cConfiguration.cpp:7611-7614) -- WA-1 -------------
void TfConfiguration::edA32_1Click(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL|N_NO_SPACE);
}

// -- edtC15Click (golden cConfiguration.cpp:7675-7678) -- WA-1 --------------
void TfConfiguration::edtC15Click(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    // GOLDEN ODDITY (recorded, not "fixed"): Min/Max args are reversed
    // (1440, 30) exactly as golden wrote them -- preserved verbatim.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 0, true, 1440, 30);
}

// -- edL09_Sh1LClick (golden cConfiguration.cpp:7680-7683) -- WA-1 ----------
void TfConfiguration::edL09_Sh1LClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    // GOLDEN ODDITY: Min/Max reversed (1000, -1000), preserved verbatim.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 1000, -1000);
}

// -- edN05_AmbTempClick (golden cConfiguration.cpp:7685-7688) -- WA-1 -------
void TfConfiguration::edN05_AmbTempClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 15.0, 50.0);
}

// -- edN06_PortClick (golden cConfiguration.cpp:7690-7693) -- WA-1 ----------
void TfConfiguration::edN06_PortClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_PORT);
}

// -- edN06_PasswordClick (golden cConfiguration.cpp:7695-7698) -- WA-1 ------
void TfConfiguration::edN06_PasswordClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_PASSWORD);
}

// -- edtSetIPSCQtyClick (golden cConfiguration.cpp:7700-7703) -- WA-1 -------
void TfConfiguration::edtSetIPSCQtyClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    // GOLDEN ODDITY: Min/Max reversed (60000, 0), preserved verbatim.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 60000, 0);
}

// -- edN04_ModelClick (golden cConfiguration.cpp:7705-7708) -- WA-1 ---------
void TfConfiguration::edN04_ModelClick(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SPACE);
}

// -- edA22_2Click (golden cConfiguration.cpp:6593-6601) -- WA-1 (partial) ---
// Frank 20161109 add magnetic scale
void TfConfiguration::edA22_2Click(TObject *Sender)
{
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824) -- see forms/fConfiguration.h
    // GATE WA-1 OPENED 20260824. The edA22_3 nudge below is REAL and unaffected.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.01, 10.0);

    if(atof(edA22_3->Text.c_str())<atof(edA22_2->Text.c_str()))
    {
        edA22_3->Text=atof(edA22_2->Text.c_str())+0.01;
    }
}

// -- edA22_3Click (golden cConfiguration.cpp:6603-6607) -- WA-1 (partial) ---
// Frank 20161109 add magnetic scale
void TfConfiguration::edA22_3Click(TObject *Sender)
{
    double dCount=atof(edA22_2->Text.c_str())+0.01;
    // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see edSetTempClick above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, dCount, 10.0);
    (void)dCount;   // AI(W906-FW-QWKEY2) 20260824: gate opened, dCount consumed again; harmless cast kept
}

// -- EnableRMSFunc (golden cConfiguration.cpp:7726-7735) -- REAL, no gate ---
bool TfConfiguration::EnableRMSFunc()
{
    bool bret=false;
    bret=CosFunction.bEnableRMS ||
         (IniConfig.bShowLotInfo && (CUSTOMER_CODE==CC_SCC    ||                //Steven 20101102 : RMS
                                     CUSTOMER_CODE==CC_SCK     ||
                                     CUSTOMER_CODE==CC_AMD_M   ||               //Ifor 20200812 add: TF-AMD USE RMS
                                     CUSTOMER_CODE==CC_ETRENDTECH));            //Steven 20230302 : add for Etrendtech
    return bret;
}

// -----------------------------------------------------------------------------
//  6.2 bucket -- Tray/HP StringGrid cluster (read/edit/reload only; the
//  paired Update buttons -- sbUpdateTrayClick/sbUpdateHPClick -- are (b)
//  write-path and NOT declared/defined here).
// -----------------------------------------------------------------------------

// -- strngrdTraySelectCell (golden cConfiguration.cpp:6839-6844) -- REAL ----
void TfConfiguration::strngrdTraySelectCell(int ACol, int ARow)
{
    iSelTrayRow=ARow;
    iSelTrayCol=ACol;
}

// -- strngrdTrayDblClick (golden cConfiguration.cpp:6846-6849) -- REAL ------
void TfConfiguration::strngrdTrayDblClick()
{
    // AI(W906-FW3-Config-WA) 20260820: DEVIATION, see forms/fConfiguration.h
    // banner -- TSpeedButton::Click() is a real, compiling, tree-wide
    // documented offline no-op (vclcompat/Controls.h TControl::Click()),
    // NOT a missing-symbol gate. Translated as-is.
    btnModifyTray->Click();
}

// -- btnModifyTrayClick (golden cConfiguration.cpp:6851-6876) -- WA-1 (partial) --
void TfConfiguration::btnModifyTrayClick()
{
    if(iSelTrayRow>0 && iSelTrayCol>=0)
    {
        edtTemp->Text=strngrdTray->Cells[iSelTrayCol][iSelTrayRow];
        if(strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Package Type"))!=0 ||
           strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Group"))!=0     ||
           strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Memo"))!=0)
        {
            // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see forms/fConfiguration.h.
            fQwertyKey->ShowQwertyKey(edtTemp, N_NO_SYMBOL);
        }
        else if(strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Columns (X)"))!=0 ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Rows (Y)"))!=0     ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("BlockNumberX"))!=0 ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("BlockNumberY"))!=0)
        {
            fQwertyKey->ShowQwertyKey(edtTemp, N_INTEGER, 0, true, 0, 1000);
        }
        else
        {
            fQwertyKey->ShowQwertyKey(edtTemp, N_DOUBLE, 2, true, 0.00, 1000.00);
        }
        strngrdTray->Cells[iSelTrayCol][iSelTrayRow]=edtTemp->Text;
    }
    btnModifyTray->Down=false;
}

// -- btnAddTrayClick (golden cConfiguration.cpp:6878-6888) -- REAL ----------
void TfConfiguration::btnAddTrayClick()
{
    int iRow=strngrdTray->RowCount;
    strngrdTray->RowCount=iRow+1;
    for(int j=0; j<strngrdTray->ColCount; j++)
    {
        strngrdTray->Cells[j][iRow]="";
    }
    btnAddTray->Down=false;
    strngrdTray->Row=strngrdTray->RowCount-1;
}

// -- btnDeleteTrayClick (golden cConfiguration.cpp:6890-6904) -- REAL -------
void TfConfiguration::btnDeleteTrayClick()
{
    if(iSelTrayRow<=0)
        return;
    Tag=atoi(strngrdTray->Cells[0][iSelTrayRow].c_str());
    for(int i=iSelTrayRow; i<strngrdTray->RowCount-1; i++)
    {
        for(int j=0; j<strngrdTray->ColCount; j++)
        {
            strngrdTray->Cells[j][i]=strngrdTray->Cells[j][i+1];
        }
    }
    strngrdTray->RowCount=strngrdTray->RowCount-1;
    btnDeleteTray->Down=false;
}

// -- sbtReloadTrayClick (golden cConfiguration.cpp:6931-6975) -- REAL -------
void TfConfiguration::sbtReloadTrayClick()
{
    TStringList *List=new TStringList();
    AnsiString S1, S2;
    int iPos=0;

    strngrdTray->ColCount=16;
    strngrdTray->Font->Size=10;
    strngrdTray->DefaultColWidth=80;
    strngrdTray->ColWidths[0]=200;

    for(int i=0; i<strngrdTray->RowCount; i++)
    {
        for(int j=0; j<16; j++)
            strngrdTray->Cells[j][i]="";
    }

    List->LoadFromFile(TrayTablePath);
    strngrdTray->RowCount=List->Count;

    for(int i=0; i<List->Count; i++)
    {
        int j=0;
        S1=List->Strings[i];
        S1=StringReplace(S1, "\"", "", TReplaceFlags()<<rfReplaceAll);
        do
        {
            iPos=S1.AnsiPos(",");
            if(iPos>0)
            {
                S2=S1.SubString(0, iPos-1);
                S1=S1.SubString(iPos+1, S1.Length());
                strngrdTray->Cells[j][i]=S2;
                j++;
            }
        }while(iPos>0);
    }

    if(strngrdTray->RowCount>1)
        strngrdTray->FixedRows=1;
    strngrdTray->FixedCols=0;
    List->Clear();
    delete List;
    sbtReloadTray->Down=false;
}

// -- strngrdHPSelectCell (golden cConfiguration.cpp:6977-6982) -- REAL ------
void TfConfiguration::strngrdHPSelectCell(int ACol, int ARow)
{
    iSelHPRow=ARow;
    iSelHPCol=ACol;
}

// -- strngrdHPDblClick (golden cConfiguration.cpp:6984-6987) -- REAL --------
void TfConfiguration::strngrdHPDblClick()
{
    // AI(W906-FW3-Config-WA) 20260820: DEVIATION, see strngrdTrayDblClick above.
    btnModifyHP->Click();
}

// -- btnModifyHPClick (golden cConfiguration.cpp:6989-7012) -- WA-1 (partial) --
void TfConfiguration::btnModifyHPClick()
{
    if(iSelHPRow>0 && iSelHPCol>=0)
    {
        edtTemp->Text=strngrdHP->Cells[iSelHPCol][iSelHPRow];
        if(strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Package Type"))!=0 ||
           strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Group"))!=0     ||
           strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Memo"))!=0)
        {
            // AI(W906-FW3-Config-WA) 20260820: GATE WA-1 (OPENED 20260824), see forms/fConfiguration.h.
            fQwertyKey->ShowQwertyKey(edtTemp, N_NO_SYMBOL);
        }
        else if(strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Columns (X)"))!=0 ||
                strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Rows (Y)"))!=0)
        {
            fQwertyKey->ShowQwertyKey(edtTemp, N_INTEGER, 0, true, 0, 1000);
        }
        else
        {
            fQwertyKey->ShowQwertyKey(edtTemp, N_DOUBLE, 2, true, 0.00, 1000.00);
        }
        strngrdHP->Cells[iSelHPCol][iSelHPRow]=edtTemp->Text;
    }
    btnModifyHP->Down=false;
}

// -- btnAddHPClick (golden cConfiguration.cpp:7014-7024) -- REAL -----------
void TfConfiguration::btnAddHPClick()
{
    int iRow=strngrdHP->RowCount;
    strngrdHP->RowCount=iRow+1;
    for(int j=0; j<strngrdHP->ColCount; j++)
    {
        strngrdHP->Cells[j][iRow]="";
    }
    btnAddHP->Down=false;
    strngrdHP->Row=strngrdHP->RowCount-1;
}

// -- btnDeleteHPClick (golden cConfiguration.cpp:7026-7040) -- REAL --------
void TfConfiguration::btnDeleteHPClick()
{
    if(iSelHPRow<=0)
        return;
    Tag=atoi(strngrdHP->Cells[0][iSelHPRow].c_str());
    for(int i=iSelHPRow; i<strngrdHP->RowCount-1; i++)
    {
        for(int j=0; j<strngrdHP->ColCount; j++)
        {
            strngrdHP->Cells[j][i]=strngrdHP->Cells[j][i+1];
        }
    }
    strngrdHP->RowCount=strngrdHP->RowCount-1;
    btnDeleteHP->Down=false;
}

// -- sbtReloadHPClick (golden cConfiguration.cpp:7042-7086) -- REAL --------
void TfConfiguration::sbtReloadHPClick()
{
    TStringList *List=new TStringList();
    AnsiString S1, S2;
    int iPos=0;

    strngrdHP->ColCount=16;
    strngrdHP->Font->Size=10;
    strngrdHP->DefaultColWidth=80;
    strngrdHP->ColWidths[0]=200;

    for(int i=0; i<strngrdHP->RowCount; i++)
    {
        for(int j=0; j<16; j++)
            strngrdHP->Cells[j][i]="";
    }

    List->LoadFromFile(PlateTablePath);
    strngrdHP->RowCount=List->Count;

    for(int i=0; i<List->Count; i++)
    {
        int j=0;
        S1=List->Strings[i];
        S1=StringReplace(S1, "\"", "", TReplaceFlags()<<rfReplaceAll);
        do
        {
            iPos=S1.AnsiPos(",");
            if(iPos>0)
            {
                S2=S1.SubString(0, iPos-1);
                S1=S1.SubString(iPos+1, S1.Length());
                strngrdHP->Cells[j][i]=S2;
                j++;
            }
        }while(iPos>0);
    }

    if(strngrdHP->RowCount>1)
        strngrdHP->FixedRows=1;
    strngrdHP->FixedCols=0;
    List->Clear();
    delete List;
    sbtReloadHP->Down=false;
}

// =============================================================================
// FW-CFG-W1 -- InitConfigEdtList_ItemA / ItemB / ItemC (merged 20260825)
// =============================================================================
// AI(W906-FW-CFG-W1) 20260825: golden cConfiguration.cpp:378-977, transcribed
// VERBATIM (cp950 -> UTF-8). This family is a pure registration table: every
// line is `elConfig->Add(widget, &IniConfig.field, type, section, key, ...)`,
// and the port's HTEditList::Add signature (Public/HTEditList.h:187-199) is
// argument-for-argument identical to golden's, so no adaptation is needed and
// none was invented. Deviations, if any, are marked inline.
void TfConfiguration::InitConfigEdtList_ItemA()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

    if(IniConfig.bVTESTFunction==true   ||
       CUSTOMER_CODE==CC_AMKOR_China    ||
       CUSTOMER_CODE==CC_ASE_KaohSiung  ||
       CUSTOMER_CODE==CC_KYEC_LEE       ||
       CUSTOMER_CODE==CC_QUALCOMM       ||
       IniConfig.bSPILFunction==true    ||
       CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20220215 : ASE-CL松諭要求鎖定A01
    {
        elConfig->Add(cbA01, &IniConfig.bA01AutoSwitchToOperatorMode,           ECBool,     "Function", "bAutoSwitchToOperatorMode",            bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbA01, &IniConfig.bA01AutoSwitchToOperatorMode,           ECBool,     "Function", "bAutoSwitchToOperatorMode",            bShow, bEnable, bReadFromFile, (int)LastSet.bAutoSwitchToOperatorMode);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150206 超過時間變成op模式
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 60);
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_ASE_CL)                                         //wei 20150903 鎖定A01
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 600);
    }
    else if(IniConfig.bSPILFunction==true)                                      //JerryYang 20200311 矽品國龍說固定180秒
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 180);
    }
    else if(CUSTOMER_CODE==CC_AMKOR_China ||
            CUSTOMER_CODE==CC_QUALCOMM)
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bEnable, bReadFromFile, 60,     false,  10,     180);
    }
    else if(IniConfig.bVTESTFunction==true)
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bFixedValue, 120);
    }
    else if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                   //Sam 20250220 : 北興俊堯要求時間要 Disable
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bDisable, bReadFromFile, 60,     false,  10,     1000);
    }
    else
    {
        elConfig->Add(edA01,                &IniConfig.iA01ChangeOpTime,        ECInteger,  "Function",  "iChangeOpTime",   bShow, bEnable, bReadFromFile, 60,     false,  10,     1000);
    }

    if(CUSTOMER_CODE==CC_JCET ||                                                //JerryYang 20170705 (Steven) add for JCET 按start要切為OP權限改為選項
       CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //RogerYang 20251224 張寧要求打開
        elConfig->Add(cbA01_1, &IniConfig.bA01PressStartAutoSwitchToOperatorMode, ECBool,   "Function",     "bPressStartAutoSwitchToOperatorMode",  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA01_1, &IniConfig.bA01PressStartAutoSwitchToOperatorMode, ECBool,   "Function",     "bPressStartAutoSwitchToOperatorMode",  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_SJ_Semiconductor)                                      //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
    {
        elConfig->Add(cbA01_2, &IniConfig.bA02DisableSaveParsWhenSwitchToOp,    ECBool,   "Function",     "b2DisableSaveParsWhenSwitchToOp",  bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbA01_2, &IniConfig.bA02DisableSaveParsWhenSwitchToOp,    ECBool,   "Function",     "b2DisableSaveParsWhenSwitchToOp",  bNoShow, bDisable, bFixedValue, 0);   //RogerYang 20260312 : Fix
    }

    if(IniConfig.bFTBin2RTBin)                                                  //ChungHung 20120912 add Bin Setting like Epson
    {
        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20220215 : ASE-CL松諭要求
        {
            elConfig->Add(cbA02,    &IniConfig.bA02BinModelPrime,               ECBool,     "Function", "BinModelPrime",                        bShow, bDisable, bFixedValue, 0);
        }
        else
        {
            elConfig->Add(cbA02,    &IniConfig.bA02BinModelPrime,               ECBool,     "Function", "BinModelPrime",                        bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(cbA02,    &IniConfig.bA02BinModelPrime,                   ECBool,     "Function", "BinModelPrime",                        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bLockA03ByFile)                                              //JerryYang 20260504 : for SPIL -- A03 Enable
    {
        elConfig->Add(cbA03,    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, IniConfig.bA03_Enable, bReadFromFile, 0);
    }
    else if(CUSTOMER_CODE==CC_SCS ||
       CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190926 : add ASEM強制開啟[A03]
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制開啟[A03]
        elConfig->Add(cbA03,    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbA03,    &IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin,ECBool,     "Function", "bA03UseAfterHomeCarryAndSuckIcToRBin", bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(cbA04,        &IniConfig.bA04LoaderTraySplitFailCanSkip,      ECBool,     "Function", "bA04LoaderTraySplitFailCanSkip",       bShow, bEnable, bReadFromFile, LastSet.bLoaderTraySplitFailCanSkip?"1":"0");
    elConfig->Add(cbA05,        &IniConfig.bA05UseAutoDocking,                  ECBool,     "Function", "bUseAutoDocking",                      bShow, bEnable, bReadFromFile, 0);                            //ChungHung 20120718 add UseAutoDocking Check Sensor
    if(IniConfig.bKoreaFunction)                                                //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
        elConfig->Add(cbA08,    &IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain, ECBool, "Function", "bLastLoaderAutoCleanOutAndCheckAgain", bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA08,    &IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain, ECBool, "Function", "bLastLoaderAutoCleanOutAndCheckAgain", bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //ChungHung 20130910 alter for SCK can close site by Index
    {
        elConfig->Add(cbA09,    &IniConfig.bA09_ByArmCloseSite,                 ECBool,     "Function", "bCloseSiteByIndexArm",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkA09_1, &IniConfig.bA09_1_AutoCloseArm,                 ECBool,     "Function", "bA09_1_AutoCloseArm",                  bNoShow, bDisable, bFixedValue, 0);     //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
    }
    else
    {
        elConfig->Add(cbA09,    &IniConfig.bA09_ByArmCloseSite,                 ECBool,     "Function", "bCloseSiteByIndexArm",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkA09_1, &IniConfig.bA09_1_AutoCloseArm,                 ECBool,     "Function", "bA09_1_AutoCloseArm",                  bShow, bEnable, bReadFromFile, 0);      //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20170119 TSMC 命名ART==>ATR
    {
        gbA10->Caption="[A10] Auto Tray retest parameter (ATR)";
        cbA10->Caption="[A10-1] Enable ATR";
    }
    else
    {
        gbA10->Caption="[A10] Auto retest parameter (ART)";
        cbA10->Caption="[A10-1] Enable ART";
    }

    // AI(W906-FW-CFG-W1) 20260825: GATE (CFG1-fMain). golden reads
    // `fMain->cbRunStartMode->Text`, the run-mode combo on the MAIN form.
    // forms/fMain.h's TfMain does not carry that member -- its only mention is
    // a comment.
    //   cmd: grep -n "cbRunStartMode" forms/fMain.h -> 1 hit, a comment (20260825)
    // ACTIVE ARM: golden's own `else`, `cbA10->Enabled=true`, is emitted
    // unconditionally below. That is not a made-up default -- it is the branch
    // golden itself takes on every machine except a CC_KYEC_LEE handler that is
    // ALREADY running in an ART start mode, where golden greys out the "Enable
    // ART" checkbox so the operator cannot switch it off mid-mode. Losing that
    // one interlock is stated plainly rather than hidden: on KYEC_LEE, the
    // checkbox stays clickable. It is a UI enable flag, not a machine action,
    // and nothing in this port reads cbA10->Enabled back.
#if 0 // GATE (CFG1-fMain) -- fMain->cbRunStartMode has no port (grep + date above)
    if(CUSTOMER_CODE==CC_KYEC_LEE &&
       (fMain->cbRunStartMode->Text.Pos("ART") ||
        fMain->cbRunStartMode->Text==StartModeName[rsmAutoRetest]))             //Ifor 20220506 Add ART 模式不可關閉ART
    {
        cbA10->Enabled=false;
    }
    else
#endif // GATE (CFG1-fMain)
    {
        cbA10->Enabled=true;
    }

    gbA10->Visible              =(USE_AUTO_RETEST==eartInstall &&               //ChungHung 20140317 add Auto Retest
                                  CUSTOMER_CODE!=CC_ASE_KaohSiung);
    if(USE_AUTO_RETEST==eartUninstall)                                          //ChungHung 20140317 add Auto Retest
        elConfig->Add(cbA10,    &IniConfig.bA10_AutoReTest,                     ECBool,     "Function", "bAutoReTest",                          bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbA10,    &IniConfig.bA10_AutoReTest,                     ECBool,     "Function", "bAutoReTest",                          bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_SCK)                                                   //Steven 20171211 (Wei) : Auto correction for SCK ART
    {
        elConfig->Add(cbA10_3,  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbA10_5,  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bShow, bEnable, bReadFromFile, 1);
    }
    else if(USE_AUTO_RETEST==eartInstall && CosFunction.bUseSCKART)
    {
        elConfig->Add(cbA10_3,  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbA10_5,  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA10_3,  &IniConfig.bA10TestModeForART,                  ECBool,     "Function", "EnableA10TestModeForART",              bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbA10_5,  &IniConfig.bA10_5SCKART_AutoCorrection,         ECBool,     "Function", "bA10_5SCKART_AutoCorrection",          bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_HANA_MICRON)                                           //JimmyChiu 20241023 HANA ART Function
    {
        elConfig->Add(cbA10_6,              &IniConfig.bA10_6_HANA_ART_TestMode_Enable,  ECBool,     "Function", "bA10_6_HANA_ART_TestMode_Enable",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbA10_6_HANA_ARTMode, &IniConfig.iA10_6_HANA_ART_TestMode,         ECInteger,  "Function", "iA10_6_HANA_ART_TestMode",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA10_6,              &IniConfig.bA10_6_HANA_ART_TestMode_Enable,  ECBool,     "Function", "bA10_6_HANA_ART_TestMode_Enable",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbA10_6_HANA_ARTMode, &IniConfig.iA10_6_HANA_ART_TestMode,         ECInteger,  "Function", "iA10_6_HANA_ART_TestMode",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                           //RogerYang 20251108 : 瑞薩FTCT
    {
        elConfig->Add(cbA10_7,              &IniConfig.bA10_7_Renesas_FTCT,     ECBool,     "Function", "bA10_7_Renesas_FTCT",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA10_7,              &IniConfig.bA10_7_Renesas_FTCT,     ECBool,     "Function", "bA10_7_Renesas_FTCT",   bNoShow, bDisable, bFixedValue, 0);
    }

    palA10_ART->Visible         =false;
    elConfig->Add(edA10_2,      &IniConfig.iAutoRetestLimit,                    ECInteger,  "Auto Retest Parameter", "iAutoRetestLimit",        bShow, bEnable, bReadFromFile, 3,      false,  1,      10);
    elConfig->Add(edA10_3,      &IniConfig.iFailYieldRate_ART,                  ECInteger,  "Auto Retest Parameter", "iFailYieldRate_ART",      bShow, bEnable, bReadFromFile, 10,     false,  1,      100);
    elConfig->Add(edA10_4,      &IniConfig.iARTTrayArmSpeed,                    ECInteger,  "Auto Retest Parameter", "A10_4_ARTTrayArmSpeed",   bShow, bEnable, bReadFromFile, 30,     false,  1,      100);
    if(USE_AUTO_RETEST==eartInstall && CosFunction.bUseSCKART)                  //Steven 20161208 : ART need to set to 32binGS for ATK
        elConfig->Add(cbA10_3_ARTTestMode,  &IniConfig.iA10TestModeForART,      ECInteger,  "Function",  "A10TestModeForART",                   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA10_3_ARTTestMode,  &IniConfig.iA10TestModeForART,      ECInteger,  "Function",  "A10TestModeForART",                   bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //2013-04-23    Dell     // 20140103 wei KYEC Barcode Reader
    {
        elConfig->Add(cbA11,    &IniConfig.bA11BarcodeTime,                     ECBool,     "Barcode Reader", "bBarcodeTime",                   bShow, bDisable, bFixedValue, 1);
        elConfig->Add(edA11,    &IniConfig.iA11BarcodeTime,                     ECInteger,  "Barcode Reader", "iBarcodeTime",                   bShow, bEnable, bReadFromFile, 60,     false,  10,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }
    else
    {
        elConfig->Add(cbA11,    &IniConfig.bA11BarcodeTime,                     ECBool,     "Barcode Reader", "bBarcodeTime",                   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edA11,    &IniConfig.iA11BarcodeTime,                     ECInteger,  "Barcode Reader", "iBarcodeTime",                   bNoShow, bDisable, bFixedValue, 60);
    }

    if(USE_AUTO_RETEST==eartInstall)                                            //ChungHung 20141001 add
    {
        pnlA12->Visible=true;                                                   //wei 20150826 拍拍Tray X軸 Offset
        elConfig->Add(cbA12,    &IniConfig.bA12ClearLoaderDevice,               ECBool,     "Function", "bClearLoaderDevice",                   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edA12,    &IniConfig.iClearLoaderCount,                   ECInteger,  "Function", "iClearLoaderCount",                    bShow, bEnable, bReadFromFile, 2,      false,  1,      10);            //wei 20150810 拍拍Tray次數設定
        elConfig->Add(edtA12_1, &IniConfig.iCleanLoaderOffset,                  ECInteger,  "Function", "iCleanLoaderOffset",                   bShow, bEnable, bReadFromFile, 0,      false,  0,      500);           //wei 20150826 拍拍Tray X軸 Offset
    }
    else
    {
        pnlA12->Visible=false;                                                  //wei 20150826 拍拍Tray X軸 Offset
        elConfig->Add(cbA12,    &IniConfig.bA12ClearLoaderDevice,               ECBool,     "Function", "bClearLoaderDevice",                   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bEnableBarcodeReader)                                        //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
        elConfig->Add(cbA14,    &IniConfig.bA14UseBarCodeSetWorkFile,           ECBool,     "Function", "bUseBarCodeSetWorkFile",               bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA14,    &IniConfig.bA14UseBarCodeSetWorkFile,           ECBool,     "Function", "bUseBarCodeSetWorkFile",               bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bStartESDAutoDecayFunction &&                                //Ifor 20150924 :Add Auto Decay Test
       USE_NOVX3360==true)                                                      //Ifor 20210302 add:有安裝SIMCO才顯示與使用
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)
        {
            elConfig->Add(cbA15,    &IniConfig.bA15AutoDecayTest,               ECBool,     "Function", "bAutoDecayTest",                       bShow, bDisable, bFixedValue, 1);    //Ifor 20220823 add: KYEC 樹榮要求[A15]強制開啟
        }
        else
        {
            elConfig->Add(cbA15,    &IniConfig.bA15AutoDecayTest,               ECBool,     "Function", "bAutoDecayTest",                       bShow, bEnable, bReadFromFile, 0);
        }
        elConfig->Add(edA15_ESDReportTime,      &IniConfig.dESDDataReportTime,  ECDouble,   "Index",    "dESDDataReportTime",                   bShow, bEnable, bReadFromFile, 10.0,   false,  0.01,   600.0);         //2~600秒 //Ifor 20160321 : edESDReportTime
        elConfig->Add(cbA15_1,    &IniConfig.bA15_1ESDGiveWayFunction,          ECBool,     "Function", "bA15_1ESDGiveWayFunction",             bShow, bEnable, bReadFromFile, 0);   //Eastsun 20260521 整合
    }
    else
    {
        elConfig->Add(cbA15,    &IniConfig.bA15AutoDecayTest,                   ECBool,     "Function", "bAutoDecayTest",                       bNoShow, bDisable, bFixedValue, 0);
        edA15_ESDReportTime->Visible=false;
        elConfig->Add(cbA15_1,    &IniConfig.bA15_1ESDGiveWayFunction,          ECBool,     "Function", "bA15_1ESDGiveWayFunction",             bNoShow, bDisable, bFixedValue, 0);  //Eastsun 20260521 整合
    }

    if(CUSTOMER_CODE==CC_KYEC_CHEN)                                             //wei 20150831
    {
        lblA16->Visible=true;
        elConfig->Add(chA16,    &IniConfig.bA16ContactTestDropContact,          ECBool,     "Function", "bContactTestDropContact",              bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        lblA16->Visible=false;
        elConfig->Add(chA16,    &IniConfig.bA16ContactTestDropContact,          ECBool,     "Function", "bContactTestDropContact",              bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190618 : add ASEM 強制 Disable RESET Button
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制 Disable RESET Button
    {
        elConfig->Add(cbA17_1,  &IniConfig.bA17RESETButtonDisable,              ECBool,     "Function", "bRESETButtonDisable",                  bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbA17_1,  &IniConfig.bA17RESETButtonDisable,              ECBool,     "Function", "bRESETButtonDisable",                  bShow, bEnable, bReadFromFile, 0);  //kevin 20151113 RESET 按鍵使用
    }

    if(CUSTOMER_CODE==CC_UMC)                                                   //JimmuyChiu 20211012   Add Reset->Clean out with no testing
        elConfig->Add(cbA17_2,  &IniConfig.bA17_1RESETCleanOutWithoutTest,      ECBool,     "Function", "bA17_1RESETCleanOutWithoutTest",       bShow, bDisable, bReadFromFile, 1);
    else
        elConfig->Add(cbA17_2,  &IniConfig.bA17_1RESETCleanOutWithoutTest,      ECBool,     "Function", "bA17_1RESETCleanOutWithoutTest",       bShow, bEnable, bReadFromFile, 0);

    if(CosFunction.bUsePMAlarmFunction)                                         //wei 20160225 PMAlarmFunction
        elConfig->Add(cbA19,    &IniConfig.bA19UsePMAlarmFunction,              ECBool,     "Function", "bUsePMAlarmFunction",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA19,    &IniConfig.bA19UsePMAlarmFunction,              ECBool,     "Function", "bUsePMAlarmFunction",                  bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbA20_1,      &IniConfig.bA20_1CheckRTCFunction,              ECBool,     "Function", "bA20_1CheckRTCFunction",               bNoShow, bEnable, bFixedValue, 0);  //wei 20171023 Disable Start比對
    elConfig->Add(cbA20_2,      &IniConfig.bA20_2CheckTrayIDFunction,           ECBool,     "Function", "bA20_2CheckTrayIDFunction",            bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(cbA20_3,      &IniConfig.bA20_3CheckAutocleanFunction,        ECBool,     "Function", "bA20_3CheckAutocleanFunction",         bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(cbA20_4,      &IniConfig.bA20_4CheckContsFailFunction,        ECBool,     "Function", "bA20_4CheckContsFailFunction",         bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(cbA20_5,      &IniConfig.bA20_5CheckOCRFunction,              ECBool,     "Function", "bA20_5CheckOCRFunction",               bNoShow, bEnable, bFixedValue, 0);

    if(USE_ROTATE_KIT==1 && iRotate_Type!=eCynRotate)                           //JerryYang 20160825 Rotate sensor偵測異常要先試著旋轉三次再跳alarm
        elConfig->Add(cbA21,    &IniConfig.bA21RotateDetectErrNeedShake,        ECBool,     "Function", "bRotateDetectErrNeedShake",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA21,    &IniConfig.bA21RotateDetectErrNeedShake,        ECBool,     "Function", "bRotateDetectErrNeedShake",            bNoShow, bEnable, bFixedValue, 0);

    if(USE_MAGNETIC_SCALE)
    {
        gbA22->Visible=true;
        elConfig->Add(cbA22_1,  &IniConfig.bA22MagneticScale,                   ECBool,     "Function", "bA22MagneticScale",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edA22_2,  &IniConfig.dA22MagneticScaleKeepRunRange,       ECDouble,   "Function", "dMagneticScaleKeepRunRange",           bShow, bEnable, bReadFromFile, 0.01,   true,   0.01,   10.0);
        elConfig->Add(edA22_3,  &IniConfig.dA22MagneticScaleStopRunRange,       ECDouble,   "Function", "dMagneticScaleStopRunRange",           bShow, bEnable, bReadFromFile, 0.01,   true,   0.01,   10.0);
    }
    else
    {
        gbA22->Visible=false;
        elConfig->Add(cbA22_1,  &IniConfig.bA22MagneticScale,                   ECBool,     "Function", "bA22MagneticScale",                    bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  //JerryYang 20170421 (Steven) JCET吳如春要求必須輸入lot no才能start   //JerryYang 20170706 (Steven) 重新啟用A23
        elConfig->Add(cbA23,    &IniConfig.bA23CheckLotNoInSLTReport,           ECBool,     "Function", "bA23CheckLotNoInSLTReport",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA23,    &IniConfig.bA23CheckLotNoInSLTReport,           ECBool,     "Function", "bA23CheckLotNoInSLTReport",            bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bUseAutoBackUpSetupFile)                                     //Ifor 20170508 (wei) add Auto BackUp Setup File ,& Last Data
        elConfig->Add(cbA24,    &IniConfig.bA24AutoBackupSetupFile,             ECBool,     "Function", "bA24AutoBackupSetupFile",              bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbA24,    &IniConfig.bA24AutoBackupSetupFile,             ECBool,     "Function", "bA24AutoBackupSetupFile",              bNoShow, bEnable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045
    {
        grpA25->Visible=true;
        elConfig->Add(edA25_1,  &IniConfig.asA25RunExecutFile,                  ECText,     "Function", "asA25RunExecutFile",                   bShow, bEnable, bReadFromFile, "C:\\GTK\\Switch.exe");
        elConfig->Add(edA25_2,  &IniConfig.asA25RunExecutButtonName,            ECText,     "Function", "asA25RunExecutButtonName",             bShow, bEnable, bReadFromFile, "SWITCH");
    }
    else
    {
        grpA25->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //KaiChen 20171225 (Steven)：Add Speed Display
        elConfig->Add(cbA26,    &IniConfig.bA26MotorSpeedSortDisplay,           ECBool,     "Function", "bA26MotorSpeedSortDisplay",            bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbA26,    &IniConfig.bA26MotorSpeedSortDisplay,           ECBool,     "Function", "bA26MotorSpeedSortDisplay",            bShow, bEnable, bReadFromFile, 0);

    spbA27->Visible             =(CosFunction.bConfigStandard);                 //KaiChen 20171113 ：超豐 將 config.ini 另外存成 config_Standard.ini      //JimmyChiu 20220114 CUSTOMER_CODE==CC_Greatek => CosFunction.bConfigStandard
    elConfig->Add(cbA27,        &IniConfig.bA27EnableLightScale,                ECBool,     "Function", "bA27EnableLightScale",                 bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbA27_1,      &IniConfig.bA27_1LogEnableLightScaleData,       ECBool,     "Function", "bA27_1LogEnableLightScaleData",        bShow, bEnable, bReadFromFile, 0);  //KaiChen 20171228 ：Log Light Scale Data
    if(CUSTOMER_CODE==CC_Greatek)
    {
        grpA28->Visible=true;                                                   //KaiChen 20171111 (Steven) ：超豐 開啟指定路徑的 HTML 檔案(PM SOP)
        elConfig->Add(edtA28_1, &IniConfig.asA28_1ShowPmSopReadFilePath,        ECText,     "Function", "asA26_1ShowPmSopReadFilePath",         bShow, bEnable, bReadFromFile, "");     //Sam 20220926 : 整理
    }
    else
    {
        grpA28->Visible=false;
        elConfig->Add(edtA28_1, &IniConfig.asA28_1ShowPmSopReadFilePath,        ECText,     "Function", "asA26_1ShowPmSopReadFilePath",         bNoShow, bEnable, bFixedValue, "");
    }

    elConfig->Add(cbA29,        &IniConfig.bA29EnableAutoCleanFunction,         ECBool,     "Function", "bA29EnableAutoCleanFunction",          bNoShow, bEnable, bFixedValue, 0);  //wei 20171023 Auto clean開關

    if(CosFunction.bSetupTeach)                                                 //JerryYang 20180921 Setup Teach功能
    {
        elConfig->Add(cbA30,    &IniConfig.bA30SetupTeachFunction,              ECBool,     "Function", "bA30SetupTeachFunction",               bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA30,    &IniConfig.bA30SetupTeachFunction,              ECBool,     "Function", "bA30SetupTeachFunction",               bNoShow, bEnable, bFixedValue, 0);
    }

    if(USE_AutoCleanIonFan==1 && CosFunction.bIOTriggerIonFanAutoClean)         //Isaac 20210609 : IO觸發IonFan清針
    {
        gbA31->Visible=true;
        elConfig->Add(cbA31,    &IniConfig.bA31EnableAutoCleanIonFanFunction,   ECBool, "Function", "bA33EnableAutoCleanIonFanFunction",          bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA31_1,  &IniConfig.bA31AutoCleanIonFanInitialStart,     ECBool, "Function", "bA33AutoCleanIonFanFunctionInitialStart",    bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        gbA31->Visible=false;
        elConfig->Add(cbA31,    &IniConfig.bA31EnableAutoCleanIonFanFunction,   ECBool, "Function", "bA33EnableAutoCleanIonFanFunction",          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbA31_1,  &IniConfig.bA31AutoCleanIonFanInitialStart,     ECBool, "Function", "bA33AutoCleanIonFanFunctionInitialStart",    bNoShow, bDisable, bFixedValue, 0);
    }

    grpA32_1->Visible=false;
    if(IniConfig.bSIGURDFunction)
    {
        grpA32->Visible=true;
        elConfig->Add(cbA32,    &IniConfig.bA32EnableFTPAutomation,             ECBool,     "Function", "bA32EnableFTPAutomation",              bShow, bEnable, bReadFromFile, 1);   //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(edA32_1,  &IniConfig.asA32_1_HandlerID,                   ECText,     "Function", "asA32_1_HandlerID",                    bShow, bEnable, bReadFromFile, "");    //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(cbA32_2,  &IniConfig.bA32_2ReturnHandlerID2OI,            ECBool,     "Function", "bA32_2ReturnHandlerID2OI",             bShow, bEnable, bReadFromFile, 1);   //KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI

        if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                  //Sam 20220620 : 中興廠新增 A32-3 功能 for 93K function
            elConfig->Add(cbA32_3, &IniConfig.bA32_3For93KFunction,             ECBool,     "Function", "bA32_3For93KFunction",                 bShow,  bEnable, bReadFromFile, 0);
        else
            elConfig->Add(cbA32_3, &IniConfig.bA32_3For93KFunction,             ECBool,     "Function", "bA32_3For93KFunction",                 bNoShow,bEnable, bFixedValue, 0);

        elConfig->Add(cbA32_01, &IniConfig.bA32EnableCheckList[eCL_Temperature],ECBool, "FTPAutomation", "CheckList_Enable_Temperature",                bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_02, &IniConfig.bA32EnableCheckList[eCL_Alarm],      ECBool, "FTPAutomation", "CheckList_Enable_Alarm",                      bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_03, &IniConfig.bA32EnableCheckList[eCL_FT_Yield],   ECBool, "FTPAutomation", "CheckList_Enable_FT_Yield",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_04, &IniConfig.bA32EnableCheckList[eCL_SiteMapping],ECBool, "FTPAutomation", "CheckList_Enable_SiteMapping",                bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_05, &IniConfig.bA32EnableCheckList[eCL_Speed],      ECBool, "FTPAutomation", "CheckList_Enable_Speed",                      bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_06, &IniConfig.bA32EnableCheckList[eCL_Contact],    ECBool, "FTPAutomation", "CheckList_Enable_Contact",                    bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_07, &IniConfig.bA32EnableCheckList[eCL_Category],   ECBool, "FTPAutomation", "CheckList_Enable_Category",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_08, &IniConfig.bA32EnableCheckList[eCL_BinSetting], ECBool, "FTPAutomation", "CheckList_Enable_BinSetting",                 bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_09, &IniConfig.bA32EnableCheckList[eCL_TrayForm],   ECBool, "FTPAutomation", "CheckList_Enable_TrayForm",                   bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(cbA32_10, &IniConfig.bA32EnableCheckList[eCL_HotPlate],   ECBool, "FTPAutomation", "CheckList_Enable_HotPlate",                   bShow, bEnable, bReadFromFile, 1);

        elConfig->Add(cbA33,    &IniConfig.bA33SetICToErrBinAfterOutShtLossIC,  ECBool, "Function", "bA33SetMachineIC2ErrorBinAfterOutShuttleLossIC",   bShow, bEnable, bReadFromFile, 0);  //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
    }
    else
    {
        grpA32->Visible=false;
        elConfig->Add(cbA32,    &IniConfig.bA32EnableFTPAutomation,             ECBool, "Function", "bA32EnableFTPAutomation",                          bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20190530 ：Sigurd FTP Automation
        elConfig->Add(cbA32_2,  &IniConfig.bA32_2ReturnHandlerID2OI,            ECBool, "Function", "bA32_2ReturnHandlerID2OI",                         bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI
        elConfig->Add(cbA33,    &IniConfig.bA33SetICToErrBinAfterOutShtLossIC,  ECBool, "Function", "bA33SetMachineIC2ErrorBinAfterOutShuttleLossIC",   bNoShow, bEnable, bFixedValue, 0);  //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
    {
        elConfig->Add(cbA35,    &IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr, ECBool, "Function", "bA35SetErrBinWhenOutShtLoseAndOutShtPickupErr", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA35,    &IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr, ECBool, "Function", "bA35SetErrBinWhenOutShtLoseAndOutShtPickupErr", bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_Microchip_Phil)                                        //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
    {
        elConfig->Add(cbA36,    &IniConfig.bA36OpenDoorSetErrBin,               ECBool, "Function", "bA36OpenDoorSetErrBin",        bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA36,    &IniConfig.bA36OpenDoorSetErrBin,               ECBool, "Function", "bA36OpenDoorSetErrBin",        bNoShow, bEnable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    {
        elConfig->Add(cbA37,    &IniConfig.bA37LotStartLotEnd,                  ECBool, "Function", "bA32LotStartLotEnd",           bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA37,    &IniConfig.bA37LotStartLotEnd,                  ECBool, "Function", "bA32LotStartLotEnd",           bNoShow, bEnable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : add for SLT lot summary
    {
        elConfig->Add(cbA38,    &IniConfig.bA38_SLT_Summary,                    ECBool, "Function", "bA35_SLT_Summary",             bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA38,    &IniConfig.bA38_SLT_Summary,                    ECBool, "Function", "bA35_SLT_Summary",             bNoShow, bEnable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_M)                                                 //Ifor 20230420 add:ASEM要求新增Run Status Log Function
    {
        elConfig->Add(cbA39,    &IniConfig.bA39RecordRunState,                  ECBool, "Function", "bA39RecordRunState",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edA39,    &IniConfig.iA39RecordTime,                      ECInteger, "Function", "iA39RecordTime",            bShow, bEnable, bReadFromFile, 5,     false,  5,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }
    else
    {
        elConfig->Add(cbA39,    &IniConfig.bA39RecordRunState,                  ECBool, "Function", "bA39RecordRunState",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edA39,    &IniConfig.iA39RecordTime,                      ECInteger, "Function", "iA39RecordTime",            bNoShow, bDisable, bReadFromFile, 60,     false,  5,     1000);          //20140310 wei : [A09]   Barcode Reader持續時間
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)               //Ifor 20201126 add:Contact 頁面開啟時不記錄2D 資料
        elConfig->Add(cbA40,    &IniConfig.bA40DoNotRecord2DDataWhenContactShow,ECBool, "Function", "bA40DoNotRecord2DDataWhenContactShow", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(cbA40,    &IniConfig.bA40DoNotRecord2DDataWhenContactShow,ECBool, "Function", "bA40DoNotRecord2DDataWhenContactShow", bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bNonCenterModeCanUseShtOffset)                               //Steven 20200715 : for Tinton
    {
        elConfig->Add(chkA50,        &IniConfig.bA50Enable1x4BiasYOffset,       ECBool, "Function", "bA32Enable1x4BiasYOffset",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(chkA50,        &IniConfig.bA50Enable1x4BiasYOffset,       ECBool, "Function", "bA32Enable1x4BiasYOffset",     bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bCanDisableQAMode)                                           //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
    {
        elConfig->Add(cbA51,        &IniConfig.bA51EnableEQCMode,               ECBool, "Function", "bA31EnableEQCMode",            bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA51,        &IniConfig.bA51EnableEQCMode,               ECBool, "Function", "bA31EnableEQCMode",            bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUsePMAlarmFunction && CosFunction.bOEEFunction)             //JimmyChiu 20230315 : PM Alarm Update From Server by FTP
    {
        grpA55->Visible=true;
        elConfig->Add(cbA55, &IniConfig.bA55EnablePMAlarmUpdateFromServerbyFTP, ECBool, "Function", "bA55EnablePMAlarmUpdateFromServerbyFTP", bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        grpA55->Visible=false;
        elConfig->Add(cbA55, &IniConfig.bA55EnablePMAlarmUpdateFromServerbyFTP, ECBool, "Function", "bA55EnablePMAlarmUpdateFromServerbyFTP", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bManualSteplAutoTeach)                                       //JimmyChiu 20211020 : Auto alignment mode
    {
        gbA56->Visible=true;
        elConfig->Add(cbA56_1,  &IniConfig.bA56EnableAutoTeachFunciton,                 ECBool,     "Function",    "bA56EnableAutoTeachFunciton",              bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(edA56_2,  &IniConfig.iA56ShuttlePickUpOffsetWhenAutoTeach,        ECInteger,  "Function",    "iA56ShuttlePickUpOffsetWhenAutoTeach",     bShow, bEnable, bReadFromFile, 1,   true,   1,   2000);
        elConfig->Add(edA56_3,  &IniConfig.iA56SocketPickUpOffsetWhenAutoTeachOnly,     ECInteger,  "Function",    "iA56SocketPickUpOffsetWhenAutoTeachOnly",  bShow, bEnable, bReadFromFile, 1,   true,   1,   2000);
    }
    else
    {
        gbA56->Visible=false;
        elConfig->Add(cbA56_1,  &IniConfig.bA56EnableAutoTeachFunciton,                 ECBool,     "Function",    "bA56EnableAutoTeachFunciton",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edA56_2,  &IniConfig.iA56ShuttlePickUpOffsetWhenAutoTeach,        ECInteger,  "Function",    "iA56ShuttlePickUpOffsetWhenAutoTeach",     bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(edA56_3,  &IniConfig.iA56SocketPickUpOffsetWhenAutoTeachOnly,     ECInteger,  "Function",    "iA56SocketPickUpOffsetWhenAutoTeachOnly",  bNoShow, bDisable, bFixedValue, 1);
    }

    gbA57->Visible=(CosFunction.bSaveArmSpeedByMachine ||
                    CosFunction.bSaveTemperatureByMachine);                     //JimmyChiu 20220618 : save by machine
    elConfig->Add(cbA57_1,  &IniConfig.bA57_1SaveArmSpeedByMachine,             ECBool, "Function", "bA57_1SaveArmSpeedByMachine",      (CosFunction.bSaveArmSpeedByMachine)?bShow:bNoShow, bEnable,    (CosFunction.bSaveArmSpeedByMachine)?bReadFromFile:bFixedValue,     0);
    elConfig->Add(cbA57_2,  &IniConfig.bA57_2SaveTemperatureByMachine,          ECBool, "Function", "bA57_2SaveTemperatureByMachine",   (CosFunction.bSaveTemperatureByMachine)?bShow:bNoShow, bEnable, (CosFunction.bSaveTemperatureByMachine)?bReadFromFile:bFixedValue,  0);
    elConfig->Add(cbA57_3,  &IniConfig.bA57_3SaveOffsetByMachine,               ECBool, "Function", "bA57_3SaveOffsetByMachine",        (CosFunction.bSaveOffsetByMachine)?bShow:bNoShow, bEnable,      (CosFunction.bSaveOffsetByMachine)?bReadFromFile:bFixedValue,       0);

    if(IniConfig.bSPILFunction==true)                                           //Jimmychiu 20230925 : Show Close Sites Alarm When Start
    {
        elConfig->Add(cbA58,    &IniConfig.bShowCloseSiteAlarmWhenStart,        ECBool,    "PrecautionRecord", "bShowCloseSiteAlarmWhenStart", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA58,    &IniConfig.bShowCloseSiteAlarmWhenStart,        ECBool,    "PrecautionRecord", "bShowCloseSiteAlarmWhenStart", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bEnableHandlerResultServer)                                  //Sam 20240304 : 新增 AMR 功能
    {
        grpA60->Visible=true;
        elConfig->Add(cbA60_1,  &IniConfig.bA60EnableAMR,                       ECBool,"Function", "bA60EnableAMR",             bShow, bEnable, bReadFromFile, 0);
        // AI(W906-FW-CFG-W1) 20260825: GATE (CFG1-Font). `->Font->Color` is a
        // pure rendering property. vclcompat::TLabel (Controls.h:274-280)
        // carries Caption and Color but no Font object, which is that facade's
        // stated scope call ("no rendering"), not an oversight -- vclcompat/
        // Controls.h:180 does define a TFont, but no widget owns one.
        //   cmd: grep -n "Font" vclcompat/Controls.h -> only the standalone
        //        class TFont at :180; no widget member anywhere (20260825)
        // ACTIVE ARM: the five labels still exist and still register nothing --
        // only their blue tint is lost. The five elConfig_byRecipe->Add calls
        // right below, which are the actual purpose of this branch, all run.
#if 0 // GATE (CFG1-Font) -- vclcompat::TLabel has no ->Font (grep + date above)
        lblA60_1->Font->Color=clBlue;
        lblA60_2->Font->Color=clBlue;
        lblA60_3->Font->Color=clBlue;
        lblA60_4->Font->Color=clBlue;
        lblA60_5->Font->Color=clBlue;
#endif // GATE (CFG1-Font)
        elConfig_byRecipe->Add(edA60_1,     &IniConfig.iA60NotifyQty[0],        ECInteger,  "Function",     "iA60NotifyQtyLoader",    bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(edA60_2,     &IniConfig.iA60NotifyQty[3],        ECInteger,  "Function",     "iA60NotifyQtyAuto1",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(edA60_3,     &IniConfig.iA60NotifyQty[4],        ECInteger,  "Function",     "iA60NotifyQtyAuto2",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(edA60_4,     &IniConfig.iA60NotifyQty[5],        ECInteger,  "Function",     "iA60NotifyQtyAuto3",     bShow, bEnable, bReadFromFile, 10     ,false, 1,       100);
        elConfig_byRecipe->Add(edA60_5,     &IniConfig.iA60LoaderQtyAtOneTime,  ECInteger,  "Function",     "iA60QtyAtOneTime",       bShow, bEnable, bReadFromFile, 9      ,false, 1,       100);
    }
    else
    {
        grpA60->Visible=false;
        elConfig->Add(cbA60_1,  &IniConfig.bA60EnableAMR,                       ECBool,"Function", "bA60EnableAMR",             bNoShow, bEnable, bFixedValue, 0);
    }

    if(USE_COVER_TRAYID!=tCIDNotUse)                                            //JerryYang 20250304 : A65有安裝硬體才Show出來
    {
        elConfig->Add(cbA65,    &IniConfig.bA65_BundleIDList,                   ECBool,    "Function",          "bA65_BundleIDList",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbA68,    &IniConfig.bA68_AutoLoadUnload,                 ECBool,    "Function",          "bA68_AutoLoadUnload",         bShow, bEnable, bReadFromFile, 0);   //JerryYang 20250220 : add
        elConfig->Add(cbA69,    &IniConfig.bA69_ScanBinLabelToBinCodeEdit,     ECBool,    "Function",          "bA69_ScanBinLabelToBinCodeEdit", bShow, bEnable, bReadFromFile, 0);   //JerryYang 20260409 : add
    }
    else
    {
        elConfig->Add(cbA65,    &IniConfig.bA65_BundleIDList,                   ECBool,    "Function",          "bA65_BundleIDList",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbA68,    &IniConfig.bA68_AutoLoadUnload,                 ECBool,    "Function",          "bA68_AutoLoadUnload",         bNoShow, bDisable, bFixedValue, 0);  //JerryYang 20250220 : add
        elConfig->Add(cbA69,    &IniConfig.bA69_ScanBinLabelToBinCodeEdit,     ECBool,    "Function",          "bA69_ScanBinLabelToBinCodeEdit", bNoShow, bDisable, bFixedValue, 0);  //JerryYang 20260409 : add
    }

    elConfig->Add(cbA66,        &IniConfig.bA66_2D_Sort,                        ECBool,    "Function",          "bA66_2D_Sort",                bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Jeff 20241001 add Continuous Mode disable  clean MUBA
       elConfig->Add(cbA61,    &IniConfig.bA61DisableCleanMUBA,                 ECBool, "Function", "bA61DisableCleanMUBA",            bShow, bEnable, bReadFromFile, 0);
    else
       elConfig->Add(cbA61,    &IniConfig.bA61DisableCleanMUBA,                 ECBool, "Function", "bA61DisableCleanMUBA",            bNoShow, bDisable, bReadFromFile, 0);
    elConfig->Add(cbA62,       &IniConfig.bA62bUseStopMachineArmHome,           ECBool, "Function", "bA62bUseStopMachineArmHome",      bShow, bEnable, bReadFromFile, 0);//Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20241028 : 矽品彰化要求 特定ALARM要觸發ONE CYCLE
    {
        elConfig->Add(cbA67,    &IniConfig.bA67TriggerOneCycleWhenAlarm,        ECBool,    "Function", "bA67TriggerOneCycleWhenAlarm", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA67,    &IniConfig.bA67TriggerOneCycleWhenAlarm,        ECBool,    "Function", "bA67TriggerOneCycleWhenAlarm", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_GIGAS)
    {
        elConfig->Add(cbA71,    &IniConfig.bA71UseBackupNowRecipe,              ECBool,    "Function",     "bA71UseBackupNowRecipe",            bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(edA71,    &IniConfig.sA71FolderPath,                      ECText,    "Function",     "sA71FolderPath",                    bShow, bEnable, bReadFromFile, "D:\\Run");
    }
    else
    {
        elConfig->Add(cbA71,    &IniConfig.bA71UseBackupNowRecipe,              ECBool,    "Function",     "bA71UseBackupNowRecipe",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edA71,    &IniConfig.sA71FolderPath,                      ECText,    "Function",     "sA71FolderPath",                    bNoShow, bDisable, bFixedValue, "D:\\Run");
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Eastsun 20260519 整合
    {
        elConfig->Add(cbA72,  &IniConfig.bA72_FixAOIButton,               ECBool, "Function", "bA72_FixAOIButton",        bShow, bEnable,      bReadFromFile,       1);
    }
    else
    {
        elConfig->Add(cbA72,  &IniConfig.bA72_FixAOIButton,               ECBool, "Function", "bA72_FixAOIButton",        bNoShow, bEnable,      bReadFromFile,       0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)   //EastSun 20260508 : A41 KYEC user manage FT/RT Bin Share (skip LotStart override)
    {
        elConfig->Add(cbA73,    &IniConfig.bA73UserOverrideFTRTBin,            ECBool, "Function", "bA73UserOverrideFTRTBin",         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbA73,    &IniConfig.bA73UserOverrideFTRTBin,            ECBool, "Function", "bA73UserOverrideFTRTBin",         bNoShow, bDisable, bReadFromFile, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)   //EastSun 20260508 : A41 KYEC user manage FT/RT Bin Share (skip LotStart override)
    {
        elConfig->Add(cbA74,    &IniConfig.bA74AOIFailCountLinkLotRunMode,            ECBool, "Function", "bA74AOIFailCountLinkLotRunMode",         bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbA74,    &IniConfig.bA74AOIFailCountLinkLotRunMode,            ECBool, "Function", "bA74AOIFailCountLinkLotRunMode",         bNoShow, bDisable, bReadFromFile, 0);
    }

    elConfig->Add(cbA75, &IniConfig.bA75DownloadItemByAccessLevel, ECBool, "Function", "bA75DownloadItemByAccessLevel", bShow, bEnable, bReadFromFile, 0);   //AI(safety-critical-change) 20260611 (RogerYang) : 下載複寫項目依權限控管(OP禁用)

    if(IniConfig.bSPILFunction==true)
    {
        elConfig->Add(cbA81WaitSECS, &IniConfig.bA81WaitSECS, ECBool, "SECS GEM", "Wait SECS",                         bShow, bEnable, bReadFromFile, 0);       //KevinCheng 20250922 Wait SECS
    }
    else
    {
        elConfig->Add(cbA81WaitSECS, &IniConfig.bA81WaitSECS, ECBool, "SECS GEM", "Wait SECS",                         bNoShow, bDisable, bFixedValue, 0);
    }
}

// AI(W906-FW-CFG-W1) 20260825: golden cConfiguration.cpp:979-1035, transcribed
// VERBATIM (cp950 -> UTF-8). This family is a pure registration table: every
// line is `elConfig->Add(widget, &IniConfig.field, type, section, key, ...)`,
// and the port's HTEditList::Add signature (Public/HTEditList.h:187-199) is
// argument-for-argument identical to golden's, so no adaptation is needed and
// none was invented. Deviations, if any, are marked inline.
void TfConfiguration::InitConfigEdtList_ItemB()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

//    tsb00->TabVisible=(CUSTOMER_CODE==CC_Greatek  ||
//                       CUSTOMER_CODE==CC_PTI);                                //Sam 20240804 : 新增 PTI
    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    {
        grpB01->Visible=true;
        grpB02->Visible=true;
        elConfig->Add(cbB01,    &IniConfig.bB01_UsePrecautionRecordFunction,            ECBool,     "PrecautionRecord",     "bB01_UsePrecautionRecordFunction",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbB02,    &IniConfig.bB02_HanderMajorMaintenanceRecordFunction,   ECBool,     "PrecautionRecord",     "bB02_HanderMajorMaintenanceRecordFunction",    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edB01,    &IniConfig.iB01_AutoWakeupPrecautionRecordFormTime,     ECInteger,  "PrecautionRecord",     "iB01_AutoWakeupPrecautionRecordFormTime",      bShow, bEnable, bReadFromFile, 10,   false,  0.01,   600.0);
        elConfig->Add(edB01_1,  &IniConfig.asB01_PrecautionRecordSavePath,              ECText,     "PrecautionRecord",     "asB01_PrecautionRecordSavePath",               bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edB02,    &IniConfig.asB02_HanderMajorMaintenanceRecordSavePath,  ECText,     "PrecautionRecord",     "asB02_HanderMajorMaintenanceRecordSavePath",   bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        grpB01->Visible=false;
        grpB02->Visible=false;
        elConfig->Add(cbB01,    &IniConfig.bB01_UsePrecautionRecordFunction,            ECBool,     "PrecautionRecord",     "bB01_UsePrecautionRecordFunction",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbB02,    &IniConfig.bB02_HanderMajorMaintenanceRecordFunction,   ECBool,     "PrecautionRecord",     "bB02_HanderMajorMaintenanceRecordFunction",    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20231115 : PTI 新增 Tester report
    {
        grpB03->Visible=true;
        elConfig         ->Add(cbB03,       &IniConfig.bB03_TesterReport,   ECBool,     "Report",   "bB03_TesterReport",    bShow, bEnable, bReadFromFile, 0);
        elConfig_byRecipe->Add(edB03_1,     &IniConfig.sB03_Customer,       ECText,     "Report",   "sB03_Customer",        bShow, bEnable, bReadFromFile, "Customer");     //Sam 20231115 : PTI 新增 Tester report
        elConfig_byRecipe->Add(edB03_2,     &IniConfig.sB03_DeviceID,       ECText,     "Report",   "sB03_DeviceID",        bShow, bEnable, bReadFromFile, "DeviceID");     //Sam 20231115 : PTI 新增 Tester report
    }
    else
    {
        grpB03->Visible=false;
        elConfig->Add(cbB03,    &IniConfig.bB03_TesterReport,               ECBool,     "Report",   "bB03_TesterReport",    bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbB05,        &IniConfig.bB05_OSReport,                   ECBool,     "Report",   "bB05_OSReport",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edtB05,       &IniConfig.sB05_OSReportPath,               ECText,     "Report",   "sB05_OSReportPath",    bShow, bEnable, bReadFromFile, "");             //Steven 20250513 : OS Report
    if(CUSTOMER_CODE==CC_PANTHER)
    {
        elConfig->Add(cbB11Enable,          &IniConfig.bB11UsePATServerFile,  ECBool,  "PrecautionRecord",     "bB11UsePATServerFile",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edB11PATServerPath,   &IniConfig.sB11PATServerPath,     ECText,  "PrecautionRecord",     "sB11PATServerPath",     bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(cbB12Enable,          &IniConfig.bB12UsePATSetup,       ECBool,  "PrecautionRecord",     "bB12UsePATSetup",       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edB12Path,            &IniConfig.sB12PATSetupPath,      ECText,  "PrecautionRecord",     "sB12PATSetupPath",      bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(edB13UploadPath,      &IniConfig.sB13PATJobUploadPath,  ECText,  "PrecautionRecord",     "sB13PATJobUploadPath",  bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(edB13DownloadPath,    &IniConfig.sB13PATJobDownloadPath,ECText,  "PrecautionRecord",     "sB13PATJobDownloadPath",bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
        elConfig->Add(edB14IntervalTime,    &IniConfig.iB14IntervalTime,      ECInteger,"PrecautionRecord",     "iB14IntervalTime",      bShow, bEnable, bReadFromFile, 20,   false,  0,   600);
        elConfig->Add(edB14ReportRealTime,  &IniConfig.sB14RealTimePath,      ECText,  "PrecautionRecord",     "sB14RealTimePath",      bShow, bEnable, bReadFromFile, "D:\\HT9045\\system\\PATFolder");
    }
    else
    {
        pal_B11->Visible=false;
    }
}

// AI(W906-FW-CFG-W1) 20260825: golden cConfiguration.cpp:1037-1242, transcribed
// VERBATIM (cp950 -> UTF-8). This family is a pure registration table: every
// line is `elConfig->Add(widget, &IniConfig.field, type, section, key, ...)`,
// and the port's HTEditList::Add signature (Public/HTEditList.h:187-199) is
// argument-for-argument identical to golden's, so no adaptation is needed and
// none was invented. Deviations, if any, are marked inline.
void TfConfiguration::InitConfigEdtList_ItemC()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_HONPREC_QC)                //wei 20160215大風扇方向
        elConfig->Add(cbC01,    &IniConfig.bC01_FanDirection,                   ECBool, "Function", "bFanDirection",                    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbC01,    &IniConfig.bC01_FanDirection,                   ECBool, "Function", "bFanDirection",                    bNoShow, bEnable, bFixedValue, 0);

    if(IniConfig.bEnableCCDUSETCPIP==true)
        cbLastSet->Add(cbC02,   &IniConfig.bC02InstallCCD,                      ECBool, "Index",    "bC02InstallCCD",                   bShow, bEnable, bReadFromFile, LastSet.bC02InstallCCD?"1":"0");
    else
        cbLastSet->Add(cbC02,   &IniConfig.bC02InstallCCD,                      ECBool, "Index",    "bC02InstallCCD",                   bNoShow, bDisable, bFixedValue, 0);

    if(USE_AUTO_RETEST==eartInstall)
    {
        if((SubMachineType==Type_HT9046AU))                                     //RogerYang 20250508 9046AU No Catch Cylinder
        {
            cbLastSet->Add(cbC03,   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bDisable, bFixedValue, 0);
        }
        else
        {
            cbLastSet->Add(cbC03,   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bDisable, bFixedValue, 1);
        }
    }
    else
        cbLastSet->Add(cbC03,   &IniConfig.bC03UseCatchTray,                    ECBool, "Tray",     "bC03UseCatchTray",                 bShow, bEnable, bReadFromFile, LastSet.bC03UseCatchTray?"1":"0");

    if(CUSTOMER_CODE==CC_HONPREC_QC)
        elConfig->Add(cbC04,    &IniConfig.bC04EnableTestTempIC,                ECBool, "Index",    "bC04EnableTestTempIC",             bShow, bEnable, bReadFromFile, LastSet.bEnableTestTempIC?"1":"0");
    else
        elConfig->Add(cbC04,    &IniConfig.bC04EnableTestTempIC,                ECBool, "Index",    "bC04EnableTestTempIC",             bNoShow, bEnable, bFixedValue, 0);

    if(IniConfig.bPowerSaveFunction)
    {
        gbC05->Visible=true;
        elConfig->Add(rgC05,        &IniConfig.iPowersaveMode,                  ECInteger,  "OutPowerSave", "PowersaveMode",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC05_Motor,  &IniConfig.bC05_PowerSaveMotor,             ECBool,     "OutPowerSave", "UsepowerSaveMotor",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC05_Temp,   &IniConfig.bC05_PowerSaveTemp,              ECBool,     "OutPowerSave", "UsepowerSaveTemp",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC05_Vacuum, &IniConfig.bC05_PowerSaveVacuum,            ECBool,     "OutPowerSave", "UsepowerSaveVacuum",       bShow, bEnable, bReadFromFile, 0);      //Steven 20221215 : Power saving for vacuum pump
        if(ATC_SYSTEM!=eNewATCSystem)
        {
            elConfig->Add(cbC05_ATC,&IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bNoShow, bDisable, bFixedValue, 0);      //Ifor 20240401 : Power saving for ATC System
        }
        else
        {
            elConfig->Add(cbC05_ATC,&IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20240401 : Power saving for ATC System
        }
        elConfig->Add(edC05_Motor,  &IniConfig.iHaltTime_Motor,                 ECInteger,  "OutPowerSave", "HaltTime_Motor",           bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //kevin 20110328設定省電模式時間
        elConfig->Add(edC05_Temp,   &IniConfig.iHaltTime_Temp,                  ECInteger,  "OutPowerSave", "HaltTime_Temp",            bShow, bEnable, bReadFromFile, 1,      false,  1,      200);
        elConfig->Add(edC05_Vacuum, &IniConfig.iC05HaltTime_Vacuum,             ECInteger,  "OutPowerSave", "HaltTime_Vacuum",          bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //Steven 20221215 : Power saving for vacuum pump
        elConfig->Add(edC05_ATC,    &IniConfig.iHaltTime_ATC,                   ECInteger,  "OutPowerSave", "HaltTime_ATC",             bShow, bEnable, bReadFromFile, 1,      false,  1,      200);            //Ifor 20240401 : Power saving for ATC System
    }
    else
    {
        gbC05->Visible=false;
        elConfig->Add(rgC05,        &IniConfig.iPowersaveMode,                  ECInteger,  "OutPowerSave", "PowersaveMode",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbC05_Motor,  &IniConfig.bC05_PowerSaveMotor,             ECBool,     "OutPowerSave", "UsepowerSaveMotor",        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbC05_Temp,   &IniConfig.bC05_PowerSaveTemp,              ECBool,     "OutPowerSave", "UsepowerSaveTemp",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbC05_Vacuum, &IniConfig.bC05_PowerSaveVacuum,            ECBool,     "OutPowerSave", "UsepowerSaveVacuum",       bNoShow, bDisable, bFixedValue, 0);         //Steven 20221215 : Power saving for vacuum pump
        elConfig->Add(cbC05_ATC,    &IniConfig.bC05_PowerSaveATC,               ECBool,     "OutPowerSave", "UsepowerSaveATC",          bNoShow, bDisable, bFixedValue, 0);         //Ifor 20240401 : Power saving for ATC System
        elConfig->Add(edC05_Motor,  &IniConfig.iHaltTime_Motor,                 ECInteger,  "OutPowerSave", "HaltTime_Motor",           bNoShow, bDisable, bFixedValue, 1);         //kevin 20110328設定省電模式時間
        elConfig->Add(edC05_Temp,   &IniConfig.iHaltTime_Temp,                  ECInteger,  "OutPowerSave", "HaltTime_Temp",            bNoShow, bDisable, bFixedValue, 1);
        elConfig->Add(edC05_Vacuum, &IniConfig.iC05HaltTime_Vacuum,             ECInteger,  "OutPowerSave", "HaltTime_Vacuum",          bNoShow, bDisable, bFixedValue, 1);         //Steven 20221215 : Power saving for vacuum pump
        elConfig->Add(edC05_ATC,    &IniConfig.iHaltTime_ATC,                   ECInteger,  "OutPowerSave", "HaltTime_ATC",             bNoShow, bDisable, bFixedValue, 1);         //Ifor 20240401 : Power saving for ATC System
    }

    if(IniConfig.bCanByPassIonFan)                                              //Steven 20111013
    {
        gbC06->Visible=true;
        elConfig->Add(cbC06_01, &IniConfig.bC06_ByPassIonFan[0],                ECBool,     "bCanByPassIonFan", "Fan01",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_02, &IniConfig.bC06_ByPassIonFan[1],                ECBool,     "bCanByPassIonFan", "Fan02",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_03, &IniConfig.bC06_ByPassIonFan[2],                ECBool,     "bCanByPassIonFan", "Fan03",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_04, &IniConfig.bC06_ByPassIonFan[3],                ECBool,     "bCanByPassIonFan", "Fan04",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_05, &IniConfig.bC06_ByPassIonFan[4],                ECBool,     "bCanByPassIonFan", "Fan05",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_06, &IniConfig.bC06_ByPassIonFan[5],                ECBool,     "bCanByPassIonFan", "Fan06",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_07, &IniConfig.bC06_ByPassIonFan[6],                ECBool,     "bCanByPassIonFan", "Fan07",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_08, &IniConfig.bC06_ByPassIonFan[7],                ECBool,     "bCanByPassIonFan", "Fan08",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_09, &IniConfig.bC06_ByPassIonFan[8],                ECBool,     "bCanByPassIonFan", "Fan09",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_10, &IniConfig.bC06_ByPassIonFan[9],                ECBool,     "bCanByPassIonFan", "Fan10",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_11, &IniConfig.bC06_ByPassIonFan[10],               ECBool,     "bCanByPassIonFan", "Fan11",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbC06_12, &IniConfig.bC06_ByPassIonFan[11],               ECBool,     "bCanByPassIonFan", "Fan12",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        gbC06->Visible=false;
        elConfig->Add(cbC06_01, &IniConfig.bC06_ByPassIonFan[0],                ECBool,     "bCanByPassIonFan", "Fan01",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_02, &IniConfig.bC06_ByPassIonFan[1],                ECBool,     "bCanByPassIonFan", "Fan02",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_03, &IniConfig.bC06_ByPassIonFan[2],                ECBool,     "bCanByPassIonFan", "Fan03",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_04, &IniConfig.bC06_ByPassIonFan[3],                ECBool,     "bCanByPassIonFan", "Fan04",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_05, &IniConfig.bC06_ByPassIonFan[4],                ECBool,     "bCanByPassIonFan", "Fan05",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_06, &IniConfig.bC06_ByPassIonFan[5],                ECBool,     "bCanByPassIonFan", "Fan06",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_07, &IniConfig.bC06_ByPassIonFan[6],                ECBool,     "bCanByPassIonFan", "Fan07",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_08, &IniConfig.bC06_ByPassIonFan[7],                ECBool,     "bCanByPassIonFan", "Fan08",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_09, &IniConfig.bC06_ByPassIonFan[8],                ECBool,     "bCanByPassIonFan", "Fan09",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_10, &IniConfig.bC06_ByPassIonFan[9],                ECBool,     "bCanByPassIonFan", "Fan10",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_11, &IniConfig.bC06_ByPassIonFan[10],               ECBool,     "bCanByPassIonFan", "Fan11",                bNoShow, bEnable, bFixedValue, 0);
        elConfig->Add(cbC06_12, &IniConfig.bC06_ByPassIonFan[11],               ECBool,     "bCanByPassIonFan", "Fan12",                bNoShow, bEnable, bFixedValue, 0);
    }

    cbC07->Visible=(INSTALL_OCR);
    #ifdef MTK_Version
    elConfig->Add(cbC08,        &IniConfig.bC08_SocketSensor,                   ECBool,     "Index", "bUseSocketSensor",                bShow, bDisable, bFixedValue, 1);
    elConfig->Add(cbC08_1,      &IniConfig.bC08_1_CheckSocketSensorDetectON,    ECBool,     "Index", "bCheckSocketSensorDetectON",      bShow, bDisable, bFixedValue, 1);       //Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能
    #else
    elConfig->Add(cbC08,        &IniConfig.bC08_SocketSensor,                   ECBool,     "Index", "bUseSocketSensor",                bShow, bEnable, bReadFromFile, 0);      //kevin 20130504 使用socket sensor detect 功能 piggy back
    elConfig->Add(cbC08_1,      &IniConfig.bC08_1_CheckSocketSensorDetectON,    ECBool,     "Index", "bCheckSocketSensorDetectON",      bShow, bEnable, bReadFromFile, 0);      //Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能
    #endif
    elConfig->Add(cbC09,        &IniConfig.bC09_CarRecord,                      ECBool,     "Car Record", "bCarRecord",                 bNoShow, bEnable, bFixedValue, 0);      //wei 2013-12-09
    elConfig->Add(edC09,        &IniConfig.iCarRecordDelayTime,                 ECInteger,  "Car Record", "iCarRecordDelayTime",        bShow, bEnable, bReadFromFile,  200,   false,  1,      200);            //wei 2013-12-09

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20170125 add   //Ifor 20150724 :Enable ESD COM ERR Report Function
    {
        if(ESD_Monitor==false && USE_NOVX3360==false)
            elConfig->Add(cbC10,&IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bNoShow, bEnable, bFixedValue, 0);
        else
            elConfig->Add(cbC10,&IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbC10,    &IniConfig.bEnable_ESD_COMERR_Report,           ECBool,     "Index", "bEnable_ESD_COMERR_Report",       bShow, bEnable, bReadFromFile, 0);
    }

    #ifdef SOFT_SIMULTE
    IniConfig.bC11UseMonitorView=false;
    #else
    if(CosFunction.bUseMonitorView)                                             //JerryYang 20160621 錄影監視功能
        elConfig->Add(cbC11,    &IniConfig.bC11UseMonitorView,                  ECBool,     "Function", "bC11UseMonitorView",           bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbC11,    &IniConfig.bC11UseMonitorView,                  ECBool,     "Function", "bC11UseMonitorView",           bNoShow, bEnable, bFixedValue, 0);
    #endif

    if(CosFunction.bUsePEModelFunction)                                         //Ifor 20170125 (Steven) add PE Mode Switch
        elConfig->Add(cbC12,    &IniConfig.bC12UsePEMode,                       ECBool,     "Function", "bC12UsePEMode",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbC12,    &IniConfig.bC12UsePEMode,                       ECBool,     "Function", "bC12UsePEMode",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bNeedToRestartGroundWhenInitialStart)                        //Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan
        elConfig->Add(cbC13,    &IniConfig.bC13NeedToRestartGroundWhenInitialStart, ECBool, "Function", "bC13NeedToRestartGroundWhenInitialStart", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(cbC13,    &IniConfig.bC13NeedToRestartGroundWhenInitialStart, ECBool, "Function", "bC13NeedToRestartGroundWhenInitialStart", bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(chkC14,       &IniConfig.bC14SaveBinDisplayLog,               ECBool,     "Function", "bC14SaveBinDisplayLog",        bShow, bEnable, bReadFromFile, 0);  //Steven 20220309 : BinDisplay Log

    if(CosFunction.bUseBarCoderChangeSetupFile)                                 //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
        elConfig->Add(cbC16,    &IniConfig.bC16UseBarCoderChangeSetupFile,      ECBool,     "Function", "bC16UseBarCoderChangeSetupFile", bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(cbC16,    &IniConfig.bC16UseBarCoderChangeSetupFile,      ECBool,     "Function", "bC16UseBarCoderChangeSetupFile", bNoShow, bEnable, bFixedValue, 0);

    if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader)                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
    {
        elConfig->Add(cbC17,    &IniConfig.bC17UseLoaderColorSensor,            ECBool,     "Function", "bC20UseLoaderColorSensor", bShow,   bEnable, bReadFromFile, 1);
        elConfig->Add(edtC17,   &IniConfig.iC17DelayTimes,                      ECInteger,  "Function", "iC20DelayTimes",           bShow,   bEnable, bReadFromFile, 3,      false,  1,      10);
        elConfig->Add(cbC17_1,  &IniConfig.bC17_1_SkipAlarm,                    ECBool,     "Function", "bC20_1_SkipAlarm",         bNoShow, bEnable, bReadFromFile, 0);
        grpC17->Visible=true;
    }
    else
    {
        elConfig->Add(cbC17,    &IniConfig.bC17UseLoaderColorSensor,            ECBool,     "Function", "bC20UseLoaderColorSensor", bNoShow, bEnable, bFixedValue, 0);
        grpC17->Visible=false;
    }

    if(USE_VibrationCommunication)                                              //JerryYang 20230814 : add震動馬達通訊調速版本
    {
        grpC21->Visible=true;
        elConfig->Add(edtC21_HP1,       &IniConfig.iC21MotSp_HP1[0],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1",         bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_HP2,       &IniConfig.iC21MotSp_HP2[0],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2",         bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT1,      &IniConfig.iC21MotSp_SHT1[0],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1",        bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT2,      &IniConfig.iC21MotSp_SHT2[0],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2",        bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto1,     &IniConfig.iC21MotSp_AUTO1[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto2,     &IniConfig.iC21MotSp_AUTO2[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto3,     &IniConfig.iC21MotSp_AUTO3[0],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);

        elConfig->Add(edtC21_HP1_m,     &IniConfig.iC21MotSp_HP1[1],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1_M",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_HP2_m,     &IniConfig.iC21MotSp_HP2[1],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2_M",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT1_m,    &IniConfig.iC21MotSp_SHT1[1],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1_M",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT2_m,    &IniConfig.iC21MotSp_SHT2[1],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2_M",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto1_m,   &IniConfig.iC21MotSp_AUTO1[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto2_m,   &IniConfig.iC21MotSp_AUTO2[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto3_m,   &IniConfig.iC21MotSp_AUTO3[1],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3_M",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);

        elConfig->Add(edtC21_HP1_b,     &IniConfig.iC21MotSp_HP1[2],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP1_B",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_HP2_b,     &IniConfig.iC21MotSp_HP2[2],            ECInteger,  "C16Vibrate",       "C16VibrateMotSp_HP2_B",       bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT1_b,    &IniConfig.iC21MotSp_SHT1[2],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT1_B",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_SHT2_b,    &IniConfig.iC21MotSp_SHT2[2],           ECInteger,  "C16Vibrate",       "C16VibrateMotSp_SHT2_B",      bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto1_b,   &IniConfig.iC21MotSp_AUTO1[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO1_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto2_b,   &IniConfig.iC21MotSp_AUTO2[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO2_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
        elConfig->Add(edtC21_Auto3_b,   &IniConfig.iC21MotSp_AUTO3[2],          ECInteger,  "C16Vibrate",       "C16VibrateMotSp_AUTO3_B",     bShow, bEnable, bReadFromFile, 50,      false,  1,      60);
    }
    else
    {
        grpC21->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)            //JerryYang 20250120 : add
    {
        elConfig->Add(cbC24,    &IniConfig.bC24InitialStartCheckCylinder,           ECBool,     "Function", "bC24InitialStartCheckCylinder",       bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbC24,    &IniConfig.bC24InitialStartCheckCylinder,           ECBool,     "Function", "bC24InitialStartCheckCylinder",       bNoShow, bEnable, bFixedValue, 0);
    }
}

// =============================================================================
// FW-CFG-W2 -- InitConfigEdtList_ItemD / InitConfigEdtList_ItemE / InitConfigEdtList_ItemF / InitConfigEdtList_ItemG / InitConfigEdtList_ItemL / InitConfigEdtList_ItemM / InitConfigEdtList_ItemO (merged 20260825)
// =============================================================================
// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:1244-1759, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemD()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(USE_IO_CHANGE_TOQUE==true)                                               //jou 2013-07-12 兩個funtion不能同時開
        elConfig->Add(cbD01,        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bDisable, bFixedValue, 0);
    else if(CUSTOMER_CODE==CC_KYEC_XILINX &&
            IniConfig.bChangeKitNoHardStop==true &&
            IniConfig.bRemeberAutoHeight==true)                                 //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
        elConfig->Add(cbD01,        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbD01,        &IniConfig.bD01EnableReadTorque,            ECBool,     "Index", "bD01EnableReadTorque",            bShow, bEnable, bReadFromFile, LastSet.bEnableReadTorque?"1":"0");

    #ifndef ASE_KaohSiung                                                       //KaiHuang 20201221 Add
        elConfig->Add(edD01,        &IniConfig.iD01ReadTorqueTimeCount,         ECInteger,  "Index", "iD01ReadTorqueTimeCount",         bShow, bEnable, bReadFromFile,  200,   false,  0,      1000);           //0~1000次
        elConfig->Add(cbD01_1,      &IniConfig.bD01_1EnableReadAndCheckTorque,  ECBool,     "Index", "bD01_1EnableReadAndCheckTorque",  bShow  , bEnable , bReadFromFile, 0);
        elConfig->Add(edD01_Xilinx, &IniConfig.dD01ReadTorque,                  ECDouble,   "Index", "dD01ReadTorque",                  bNoShow, bDisable, bFixedValue, 10.0,  false,  10.0,   100.0);          //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
        elConfig->Add(edD01DelayTime_Xilinx,&IniConfig.dD01ReadTorqueDelayTime, ECDouble,   "Index", "dD01ReadTorqueDelayTime",         bNoShow, bDisable, bFixedValue, 1.0,   false,  0.01,   5.0);            //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    #else
        //借來用
        elConfig->Add(edD01,        &IniConfig.iD01ReadTorqueTimeCount,         ECInteger,  "Index", "iD01ReadTorqueTimeCount",         bNoShow, bDisable, bReadFromFile,  200,   false,  0,      1000);
        elConfig->Add(cbD01_1,      &IniConfig.bD01_1EnableReadAndCheckTorque,  ECBool,     "Index", "bD01_1EnableReadAndCheckTorque",  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edD01_Xilinx, &IniConfig.dD01ReadTorque,                  ECDouble,   "Index", "dD01ReadTorque",                  bShow,   bEnable,  bReadFromFile,  3.0,   false,  0.1,    100.0);
        elConfig->Add(edD01DelayTime_Xilinx,&IniConfig.dD01ReadTorqueDelayTime, ECDouble,   "Index", "dD01ReadTorqueDelayTime",         bShow,   bEnable,  bReadFromFile,  0.1,   false,  0.1,   10.0);
    #endif
    elConfig->Add(cbD02,        &IniConfig.bD02OffReadTorqueDuringTest,         ECBool,     "Index", "bD02OffReadTorqueDuringTest",     bShow, bEnable, bReadFromFile,  LastSet.bOffReadTorqueDuringTest?"1":"0");
    elConfig->Add(cbD04,        &IniConfig.bD04MinForceByFile,                  ECBool,     "Contact Force", "bD04MinForceByFile",      bShow, bEnable, bReadFromFile,  0);                                     //Steven 20190314 : Min force is read from file
    elConfig->Add(edtD04,       &IniConfig.dD04MinForceByFile,                  ECDouble,   "Contact Force", "dD04MinForceByFile",      bShow, bEnable, bReadFromFile,  1.0,   false,  1.0,    120.0);          //Steven 20190314 : Min force is read from file
    elConfig->Add(edtD04_20mm,  &IniConfig.dD04MinForceByFile_20mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_20mm", bShow, bEnable, bReadFromFile,  0.5,   false,  1.0,    120.0);          //Steven 20220822 : Min force for different SLK
    elConfig->Add(edtD04_30mm,  &IniConfig.dD04MinForceByFile_30mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_30mm", bShow, bEnable, bReadFromFile,  1.5,   false,  1.0,    120.0);
    elConfig->Add(edtD04_40mm,  &IniConfig.dD04MinForceByFile_40mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_40mm", bShow, bEnable, bReadFromFile,  4.0,   false,  1.0,    120.0);
    elConfig->Add(edtD04_60mm,  &IniConfig.dD04MinForceByFile_60mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_60mm", bShow, bEnable, bReadFromFile,  8.0,   false,  1.0,    120.0);
    elConfig->Add(edtD04_80mm,  &IniConfig.dD04MinForceByFile_80mm,             ECDouble,   "Contact Force", "dD04MinForceByFile_80mm", bShow, bEnable, bReadFromFile,  15.0,  false,  1.0,    180.0);          //Ifor 20240620 : add缸徑 80
    elConfig->Add(cbD05,        &IniConfig.bD05ContactCountAlarm,               ECBool,     "Index", "bD05ContactCountAlarm",           bNoShow, bEnable, bFixedValue,  0);                                     //wei 20170327
    elConfig->Add(edD05,        &IniConfig.iD05_ContactCountAlarm,              ECInteger,  "Contact Force", "iD05_ContactCountAlarm",  bNoShow, bDisable, bFixedValue, 1,     false,  0,      5000);           //wei 20170327 add
    elConfig->Add(cbD05_1,      &IniConfig.bD05_1SaveSocketCntByHandler,        ECBool,     "Index", "bD05_1SaveSocketCntByHandler",    bShow, bEnable, bReadFromFile,  0);                                     //Steven 20250807 : By handler save contact count
    elConfig->Add(cbD06,        &IniConfig.bD06ContactOffsetDefaultValue,       ECBool,     "Index", "bD06ContactOffsetDefaultValue",   bShow, bEnable, bReadFromFile,  0);                                     //JimmyChiu 20220120 add ContactOffsetDefaultValue
    elConfig->Add(edD06,        &IniConfig.dD06_ContactOffsetDefaultValue,      ECDouble,   "Contact Force", "dD06_ContactOffsetDefaultValue",   bShow, bEnable, bReadFromFile,  3,     false,  0,      10);    //JimmyChiu 20220120 add ContactOffsetDefaultValue
    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        elConfig->Add(cbD10,    &IniConfig.bD10ManualHeightComptibleWithNS,     ECBool,     "Index", "bD10ManualHeightComptibleWithNS", bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbD10,    &IniConfig.bD10ManualHeightComptibleWithNS,     ECBool,     "Index", "bD10ManualHeightComptibleWithNS", bShow, bEnable, bReadFromFile, LastSet.bManualHeightComptibleWithNS?"1":"0");

    elConfig->Add(cbD11,        &IniConfig.bD11NoIcSkipAutoHeight,              ECBool,     "Index", "bNoIcSkipAutoHeight",             bShow, bEnable, bReadFromFile, 0);      //Steven 20110726
//    if(IniConfig.bSPILFunction==true)                                         //JerryYang 20241011 : Mark掉 //Steven 20140220 : 用生產ep去做蝦頭auto high
//        elConfig->Add(cbD12,    &IniConfig.bD12UseDeviceFormPressDoShtHeight,   ECBool,     "Index", "bUseDeviceFormPressDoShtHeight",  bShow, bDisable, bFixedValue, 1);
//    else
        elConfig->Add(cbD12,    &IniConfig.bD12UseDeviceFormPressDoShtHeight,   ECBool,     "Index", "bUseDeviceFormPressDoShtHeight",  bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(cdD13,        &IniConfig.bD13CheckIndexHomeSensor,            ECBool,     "Index", "bCheckIndexHomeSensor",           bShow, bEnable, bReadFromFile, 0);      //Steven 20140828 : 歸零後檢查Index位置
    elConfig->Add(cbD14,        &IniConfig.bD14_AutoHeightUseSetTorque,         ECBool,     "Index", "bD14_AutoHeightUseSetTorque",     bShow, bEnable, bReadFromFile, 0);      //Steven 20141105 : 使用指定的扭力進行Auto Height
    elConfig->Add(edD14,        &IniConfig.iD14_AutoHeightUseSetTorque,         ECInteger,  "Index", "iD14_AutoHeightUseSetTorque",     bShow, bEnable, bReadFromFile,  15,    false,  5,      30);             //kevin 20210730 10->5
    elConfig->Add(cbD15,        &IniConfig.bD15_AutoContactTest,                ECBool,     "Index", "bD15_AutoContactTest",            bShow, bEnable, bReadFromFile, 0);      //Steven 20150224 : Auto Contact Test
    elConfig->Add(cbD16,        &IniConfig.bD16_StepContactTest,                ECBool,     "Index", "bD16_StepContactTest",            bShow, bEnable, bReadFromFile, 0);      //Steven 20150811 : Step by Step Contact Test
    elConfig->Add(rgD17,        &IniConfig.iD17_UseHardwareHeightToContact,     ECInteger,  "Index", "bD17_UseHardwareHeightToContact", bShow, bEnable, bReadFromFile, 0);      //Steven 20170411 (wei) : SCK的SIP怕刮傷所以Contact Height使用硬體高度
    elConfig->Add(edtD17_3,     &IniConfig.dD17_3CheckEPLeakage,                ECDouble,   "Index", "dD17_3CheckEPLeakage",            bShow, bEnable, bReadFromFile, 1.0, false,  1.0,    3.0);    //Steven 20231025 : 充氣跟不充氣都做一次auto height, 然後檢查有沒有漏氣
    elConfig->Add(chkD18,       &IniConfig.bD18_AutoHeightWhenChangeRecipe,     ECBool,     "Index", "bD18_AutoHeightWhenChangeRecipe", bShow, bEnable, bReadFromFile, 0);      //Steven 20221013 : ATK希望更換工作檔時, 需要提示有沒有做Auto Height.

    elConfig->Add(cbD21,        &IniConfig.bD21EnableFinishTestUpWait,          ECBool,     "Tester", "bD21EnableFinishTestUpWait",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edD21_mm,     &IniConfig.iD21FinishTestUpWaitHeight,          ECInteger,  "Tester", "iD21FinishTestUpWaitHeight",     bShow, bEnable, bReadFromFile, LastSet.iFinishTestUpWaitHeight,       false,  0,      10, EUuMToMM);  //0~10mm
    elConfig->Add(edD21_Sec,    &IniConfig.iD21FinishTestUpWaitTime,            ECInteger,  "Tester", "iD21FinishTestUpWaitTime",       bShow, bEnable, bReadFromFile, (int)LastSet.iD21FinishTestUpWaitTime, false,  0.1, 100.0, EUMSToSec);  //0.1~100Sec

    elConfig->Add(cbD22_1,      &IniConfig.bD22SupportMultiDoubleContact,       ECBool,     "Index", "bD22SupportMultiDoubleContact",   bShow, bEnable, bReadFromFile, LastSet.bSupportMultiDoubleContact?"1":"0");
    elConfig->Add(cbD22_2,      &IniConfig.bD22DoubleContactNoNeedReContact,    ECBool,     "Index", "bDoubleContactNoNeedReContact",   bShow, bEnable, bReadFromFile, 0);      //Steven 20131202 : Double Contact不需要Index Arm上下動
    elConfig->Add(coD22,        &IniConfig.iD22DoubleContactCount,              ECInteger,  "Index", "iD22DoubleContactCount",          bShow, bEnable, bReadFromFile, LastSet.iDoubleContactCount);

    if(CUSTOMER_CODE==CC_AnalogDevice_Phil)                                     //JerryYang 20230721 : Analog double contact鎖定送0X42
    {
        elConfig->Add(cbD22_3_, &IniConfig.bD22DoubleContactUseDiffSRQ,         ECBool,     "Index", "bD22DoubleContactUseDiffSRQ",     bShow, bDisable, bFixedValue, 1);       //Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
        elConfig->Add(coD22_3,  &IniConfig.iD23_DoubleContactSRQ,               ECInteger,  "Index", "iD23_DoubleContactSRQ",           bShow, bDisable, bFixedValue, 2);
    }
    else
    {
        elConfig->Add(cbD22_3_, &IniConfig.bD22DoubleContactUseDiffSRQ,         ECBool,     "Index", "bD22DoubleContactUseDiffSRQ",     bShow, bEnable, bReadFromFile, 0);       //Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
        elConfig->Add(coD22_3,  &IniConfig.iD23_DoubleContactSRQ,               ECInteger,  "Index", "iD23_DoubleContactSRQ",           bShow, bEnable, bReadFromFile, 0);
    }

    /*  //Sam 20231117 : 整合到 QA 模式 Mark
    if(CosFunction.bVerifyMode)                                                 //Sam 20221012 : 新增 VerifyMode 功能
        elConfig->Add(cbD22_3,  &IniConfig.bD22VerifyMode,                      ECBool,     "Index", "bD22VerifyMode",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD22_3,  &IniConfig.bD22VerifyMode,                      ECBool,     "Index", "bD22VerifyMode",                  bNoShow, bDisable, bFixedValue, 0);
    */
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20230909 : pass bin也可以設定Double contact
    {
        elConfig->Add(cbD22_4, &IniConfig.bD22_4_PassBinCanDoubleContact,       ECBool,     "Index", "bD22_4_PassBinCanDoubleContact",  bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbD22_4, &IniConfig.bD22_4_PassBinCanDoubleContact,       ECBool,     "Index", "bD22_4_PassBinCanDoubleContact",  bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbD23,        &IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting, ECBool, "Index", "bEveryDeviceDoubleContactFirstNoTestting", bShow, bEnable, bReadFromFile, 0); //ChungHung 20140709 add for SPIL
    elConfig->Add(edD23,        &IniConfig.iD23_MultiContactCount,              ECInteger,  "Index", "iMultiContactCount",              bShow, bEnable, bReadFromFile,  2,     false,  2,      30);             //Steven 20151001 : Add for TSMC

    if(IniConfig.bIndexEveryTimeCheckEP==true)
        elConfig->Add(cbD24,    &IniConfig.bD24EnableEPCheckFuntion,            ECBool,     "EP Check", "EnableEPCheckFuntion",         bShow, bEnable, bReadFromFile, 0);
    else                                                                        ECBool,
        elConfig->Add(cbD24,    &IniConfig.bD24EnableEPCheckFuntion,            ECBool,     "EP Check", "EnableEPCheckFuntion",         bNoShow, bEnable, bFixedValue, 0);

    if(EP_Install==3 || EP_Install==5)
    {
        elConfig->Add(cbD26,    &IniConfig.bD26EnableEPEncoderRange,            ECBool,     "Index", "bEnableEPEncoderRange",           bShow, bEnable, bReadFromFile, 0);      //ChungHung 20111217
        elConfig->Add(cbD26_1,  &IniConfig.bD26EnableEPLog,                     ECBool,     "Index", "bEnableEPLog",                    bShow, bEnable, bReadFromFile, 0);      //Ifor 20150706 :新增EP LOG 開關
        elConfig->Add(cbD26_2,  &IniConfig.bD26EnableEncodeShow,                ECBool,     "Index", "bEnableEncodeShow",               bShow, bEnable, bReadFromFile, 0);      //Ifor 20160304 :新增EP encoder value Show 開關
        if(CosFunction.bHiSiliconFunction && CUSTOMER_CODE==CC_ASE_KaohSiung)   //kevin 20200313 add eP range 5
            elConfig->Add(edD26,    &IniConfig.iD26EPEncoderRange,              ECInteger,  "Index", "iEPEncoderRange",                 bShow, bDisable, bFixedValue,  5);     //kevin 20200313 add eP range 5
        else
            elConfig->Add(edD26,    &IniConfig.iD26EPEncoderRange,              ECInteger,  "Index", "iEPEncoderRange",                 bShow, bEnable, bReadFromFile,  100,   false,  10,     100);            //ChungHung 20111217
    }
    else
    {
        elConfig->Add(cbD26,    &IniConfig.bD26EnableEPEncoderRange,            ECBool,     "Index", "bEnableEPEncoderRange",           bNoShow, bEnable, bFixedValue, 0);      //ChungHung 20111217
        elConfig->Add(cbD26_1,  &IniConfig.bD26EnableEPLog,                     ECBool,     "Index", "bEnableEPLog",                    bNoShow, bEnable, bFixedValue, 0);      //Ifor 20150706 :新增EP LOG 開關
        elConfig->Add(cbD26_2,  &IniConfig.bD26EnableEncodeShow,                ECBool,     "Index", "bEnableEncodeShow",               bNoShow, bEnable, bFixedValue, 0);      //Ifor 20160304 :新增EP encoder value Show 開關
        elConfig->Add(edD26,    &IniConfig.iD26EPEncoderRange,                  ECInteger,  "Index", "iEPEncoderRange",                 bNoShow, bDisable, bFixedValue, 100);
    }

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                            //AI(ht9045-v899) 20260504: V899 wrote ==true (==1) which dropped Multi EP path; restored V874 behavior.
    {
        elConfig->Add(cbD26_3,  &IniConfig.bD26_3EnableDualEPEncoderRange,      ECBool,     "Index", "bD26_3EnableDualEPEncoderRange",  bShow, bEnable, bReadFromFile, 0);
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)
        {
            cbD26_3->Caption="[D26_3] Enable Dual EP encoder range +,-                   Kg";
        }
        elConfig->Add(edD26_3,  &IniConfig.iD26_3DualEPEncoderRange,            ECInteger,  "Index",  "iD26_3DualEPEncoderRange",       bShow, bEnable, bReadFromFile,  100,   false,  1,      100);
    }
    else
    {
        elConfig->Add(cbD26_3,  &IniConfig.bD26_3EnableDualEPEncoderRange,      ECBool,      "Index", "bD26_3EnableDualEPEncoderRange", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edD26_3,  &IniConfig.iD26_3DualEPEncoderRange,            ECInteger,   "Index", "iD26_3DualEPEncoderRange",       bNoShow, bDisable, bFixedValue, 100);
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  //Richard 20230426 : JECT ADD 固定值改為百分比
        elConfig->Add(coD26,    &IniConfig.iD26_3FixValueOrPercentage,          ECInteger,  "Index", "iD26_3FixValueOrPercentage",      bShow, bEnable, bReadFromFile, 0); //Richard 20230428 : EP固定值或百分比
    else
        elConfig->Add(coD26,    &IniConfig.iD26_3FixValueOrPercentage,          ECInteger,  "Index", "iD26_3FixValueOrPercentage",      bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ATMEL || CUSTOMER_CODE==CC_QUALCOMM)                   //Steven 20131203 : bD27UseSingleSite85kg ATK強制開啟
        elConfig->Add(cbD27,    &IniConfig.bD27UseSingleSite85kg,               ECBool,     "Index", "UseSingleSite85kg",               bShow, bDisable, bFixedValue, 1);        //2012-01-03    Dell 在1X2模式下關Site能達85kg
    else
        elConfig->Add(cbD27,    &IniConfig.bD27UseSingleSite85kg,               ECBool,     "Index", "UseSingleSite85kg",               bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20251017 Add:ASE SG 要求強制關閉[D28]
        elConfig->Add(cbD28,        &IniConfig.bD28MaxForceLimitByDiameter,         ECBool,     "Index", "bD28MaxForceLimitByDiameter",     bShow, bDisable, bFixedValue, 0);      //Steven 20200813 : 用缸徑計算最大壓力
    else
        elConfig->Add(cbD28,        &IniConfig.bD28MaxForceLimitByDiameter,         ECBool,     "Index", "bD28MaxForceLimitByDiameter",     bShow, bDisable, bFixedValue, 1);      //Steven 20200813 : 用缸徑計算最大壓力

    if(CosFunction.bHaveIndexContactDelay)                                      //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
        elConfig->Add(cbD29,    &IniConfig.bD29EnableIndexContactDelay,         ECBool,      "Index", "bEnableIndexContactDelay",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD29,    &IniConfig.bD29EnableIndexContactDelay,         ECBool,      "Index", "bEnableIndexContactDelay",       bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(cbD30,        &IniConfig.bD30EnableSiteModeSelect,            ECBool,     "Index", "bD30EnableSiteModeSelect",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbD31,    &IniConfig.bD31RTCChangeRecipeNeedreCreateModel,    ECBool,     "Configuration", "bRTCChangeRecipeNeedreCreateModel", bShow, bEnable, bReadFromFile, 0);//jou 2012-03-01 [D31] RTC Change Recipe Need reCreate RTC Model
    elConfig->Add(cbD32,    &IniConfig.bD32_35TrayPitchIndexDOnMoreThen500MS,   ECBool,     "Index", "b35TrayPitchIndexDOnMoreThen500MSec", bShow, bEnable, bReadFromFile, 0);      //Steven 20120727 : Tray Pitch 大於35mm的話 Index吹氣至少要0.5秒
    elConfig->Add(cbD33,        &IniConfig.bD33RTCInitStartVerify,              ECBool,     "Configuration", "bRTCInitStartVerify",     bShow, bEnable, bReadFromFile, 0);      //Handler Use Model Verify
    elConfig->Add(cbD34,        &IniConfig.bD34GailDMCProtection,               ECBool,     "Index", "bGailDMCProtection",              bShow, bEnable, bReadFromFile, 0);      //ChungHung 20131230 add for ATK    //Steven 20140315 : 改成可以選擇[D34]

    if(IniConfig.bHaveRTCCheckSiteMap)                                          //Steven 20140513
        elConfig->Add(cbD35,    &IniConfig.bD35RTCCheckSiteMap,                 ECBool,     "Index", "bRTCCheckSiteMap",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD35,    &IniConfig.bD35RTCCheckSiteMap,                 ECBool,     "Index", "bRTCCheckSiteMap",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bRTCAutoModelVerify)
    {
        elConfig->Add(cbD36,    &IniConfig.bD36EnableRTCAutoModelVerify,        ECBool,     "Index", "bEnableRTCAutoModelVerify",           bShow, bEnable, bReadFromFile, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(cbD36_1,  &IniConfig.bD36_1EnableRTCAutoModelVerifyLive,  ECBool,     "Index", "bD36_1EnableRTCAutoModelVerifyLive",  bShow, bEnable, bReadFromFile, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(cbD36_2,  &IniConfig.bD36_2AfterOneCycleNeedAutoVerify,   ECBool,     "Index", "bD36_2AfterOneCycleNeedAutoVerify",   bShow, bEnable, bReadFromFile, 0);      //JerryYang 20201116 : One cycle後要執行RTC auto verify
    }
    else
    {
        elConfig->Add(cbD36,    &IniConfig.bD36EnableRTCAutoModelVerify,        ECBool,     "Index", "bEnableRTCAutoModelVerify",           bNoShow, bEnable, bFixedValue, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(cbD36_1,  &IniConfig.bD36_1EnableRTCAutoModelVerifyLive,  ECBool,     "Index", "bD36_1EnableRTCAutoModelVerifyLive",  bNoShow, bEnable, bFixedValue, 0);      //jou 2014-06-24 RTC 自動進行Model驗證
        elConfig->Add(cbD36_2,  &IniConfig.bD36_2AfterOneCycleNeedAutoVerify,   ECBool,     "Index", "bD36_2AfterOneCycleNeedAutoVerify",   bNoShow, bEnable, bFixedValue, 0);      //JerryYang 20201116 : One cycle後要執行RTC auto verify
    }

    if(CosFunction.bManualProcess)                                              //ChungHung 20150526 add for QualComm US
        elConfig->Add(cbD37,    &IniConfig.bD37EnableManualProcess,             ECBool,     "Index", "bEnableManualProcess",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD37,    &IniConfig.bD37EnableManualProcess,             ECBool,     "Index", "bEnableManualProcess",                bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(cbD38,        &IniConfig.bD38IndexPutICToShtNoWaitMotion,     ECBool,     "Index", "bD38IndexPutICToShtNoWaitMotion",     bShow, bEnable, bReadFromFile, 0);      //Steven 20181228 : Add Index Action   //ChungHung 20171116 modify for Index Action

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
       CUSTOMER_CODE==CC_AMKOR_China || IniConfig.bKoreaFunction ||
       CUSTOMER_CODE==CC_KYEC_LEE    ||                                         //wei 20150624 KYEC強制打開
       IniConfig.bMaximFunction ||                                              //JerryYang 20190522 Maxim統一軟體功能
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 (Steven) add QUALCOMM
    {
        elConfig->Add(cbD40,    &IniConfig.bD40IndexICFallDownMustPressFMotorDown, ECBool,  "Index", "bD40IndexICFallDownMustPressFMotorDown", bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbD40,    &IniConfig.bD40IndexICFallDownMustPressFMotorDown, ECBool,  "Index", "bD40IndexICFallDownMustPressFMotorDown", bShow, bEnable, bReadFromFile, LastSet.bIndexICFallDownMustPressFMotorDown?"1":"0");
    }

    if(IniConfig.bTestIcCheckInContact==true)                                   //ChungHung 20140327 add by Customer
    {
        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                                   //Jeff 20241001 add for enable and disable checkvacum in contact
           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        {
           pnlD41->Visible=true;

           labD41_1->Visible=false;
           labD41_3->Visible=false;
           labD41_2->Visible=false;
           labD41->Visible=false;
        }
        else
        {
           pnlD41->Visible=false;
        }
        elConfig->Add(edD41,    &IniConfig.dD41SocketInitialCheckOffset,        ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bNoShow, bDisable, bFixedValue, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 10.0);        //Steven 20100818 : [D41]
        elConfig->Add(edD41_2,  &IniConfig.fIndexCheckOffset,                   ECDouble,   "Index", "fIndexCheckOffset",                       bNoShow, bDisable, bFixedValue, 0.0, false, 0.0, 10.0);       //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
        elConfig->Add(coD41,    &IniConfig.iD41SocketInitialICCheckPosition,    ECInteger,  "Index", "iD41SocketInitialICCheckPosition",        bNoShow, bDisable, bFixedValue, LastSet.iD41SocketInitialICCheckPosition);
    }
    else
    {
        pnlD41->Visible=true;
        if(CosFunction.bIndexZDownToAboveSocket)
        {
            if(IniConfig.bIndexArm2SupplyLight)
                elConfig->Add(edD41,&IniConfig.dD41SocketInitialCheckOffset,    ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bShow, bEnable, bReadFromFile, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 20.0);        //Steven 20100818 : [D41]
            else
                elConfig->Add(edD41,&IniConfig.dD41SocketInitialCheckOffset,    ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bShow, bEnable, bReadFromFile, LastSet.dD41SocketInitialCheckOffset, false, 0.0, 10.0);        //Steven 20100818 : [D41]

            elConfig->Add(edD41_2,  &IniConfig.fIndexCheckOffset,               ECDouble,   "Index", "fIndexCheckOffset",                       bNoShow, bDisable, bFixedValue, 0.0, false);
        }
        else
        {
            double dMaxOffset=10.0;
            AnsiString szDirSecurity=CheckFile(AuthPath, "Security_new.def");   //Jimmychiu 20231128 : Edit Max index check offset by file
            dMaxOffset=CheckRange(CheckAndReadIniData(szDirSecurity, "Input Limit", "D41IndexCheckOffset", 10.0), 5.0, 100.0);
            elConfig->Add(edD41,&IniConfig.dD41SocketInitialCheckOffset,        ECDouble,   "Index", "fD41SocketInitialICCheckPositionOffset",  bNoShow, bDisable, bFixedValue, LastSet.dD41SocketInitialCheckOffset);
            elConfig->Add(edD41_2,  &IniConfig.fIndexCheckOffset,               ECDouble,   "Index", "fIndexCheckOffset",                       bShow, bEnable, bReadFromFile, 0.0, false, 0.0, dMaxOffset);       //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
        }

        elConfig->Add(coD41,    &IniConfig.iD41SocketInitialICCheckPosition,    ECInteger,  "Index", "iD41SocketInitialICCheckPosition",        bShow, bEnable, bReadFromFile, LastSet.iD41SocketInitialICCheckPosition);

        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                 //kevin 20200716 add ASE_KH 卡輸入
        {
            elConfig->Add(cbD41,&IniConfig.bD41CheckbySetup,                    ECBool,     "Index", "bD41CheckbySetup",                    bShow, bEnable, bReadFromFile, 0); //kevin 20200801 by setup 功能只能用一次
        }
        else
        {
            elConfig->Add(cbD41,&IniConfig.bD41CheckbySetup,                    ECBool,     "Index", "bD41CheckbySetup",                    bNoShow, bDisable, bFixedValue, 0);                         //kevin 20200801 by setup 功能只能用一次
        }
    }

    elConfig->Add(cbD42,        &IniConfig.bD42IndexPickICShuttlePause,         ECBool,     "Index", "bD42IndexPickICShuttlePause",         bShow, bEnable, bReadFromFile, LastSet.bIndexPickICShuttlePause?"1":"0");
    elConfig->Add(cbD43,        &IniConfig.bD43IndexDropErrorCanRetryandSkip,   ECBool,     "Index", "bIndexDropErrorCanRetryandSkip",      bShow, bEnable, bReadFromFile, 0);  //ChungHung 20120717 add Index Drop Error Can Retry and Start
    elConfig->Add(cbD43_1,      &IniConfig.bD43AutoRetryWhenIndexPickErr,       ECBool,     "Index", "bD43AutoRetryWhenIndexPickErr",       bShow, bEnable, bReadFromFile, 0);  //Steven 20170105 : Index吸取異常要退出來用Shuttle Sensor檢查後 再進去吸一次
    elConfig->Add(cbD43_2,      &IniConfig.bD43IndexPickErrCheckSocket,         ECBool,     "Index", "bD43IndexPickErrCheckSocket",         bShow, bEnable, bReadFromFile, 0);  //Steven 20190115 : SCC要求吸取異常要檢查Socket

    if(CUSTOMER_CODE==CC_ASE_CL         ||                                      //JerryYang 20210129 : ASE-CL松諭要求強制開啟
       CUSTOMER_CODE==CC_SIGURD_HUKOU   ||                                      //Sam 20220527 : 矽格湖口 Jonas 要求強制開啟
       CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Sam 20240606 : 北興廠也要強制開啟
    {
        elConfig->Add(cbD44,    &IniConfig.bD44CheckIndexICDestroy,             ECBool,     "Index", "bD44CheckIndexICDestroy",             bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbD44,    &IniConfig.bD44CheckIndexICDestroy,             ECBool,     "Index", "bD44CheckIndexICDestroy",             bShow, bEnable, bReadFromFile, LastSet.bCheckIndexICDestroy?"1":"0");
    }

    if(INDEX_SUCKER_TYPE==1)                                                    //kevin 20121023 設定範圍避免過久壓降掉料
        elConfig->Add(edD44,    &IniConfig.iD44TestHeadCheckVacuumTime,         ECInteger,  "Index", "iD44TestHeadCheckVacuumTime",         bShow, bEnable, bReadFromFile, (int)LastSet.iTestHeadCheckVacuumTime, false,  0.1, 0.15, EUMSToSec);  //0.1~100Sec
    else
        elConfig->Add(edD44,    &IniConfig.iD44TestHeadCheckVacuumTime,         ECInteger,  "Index", "iD44TestHeadCheckVacuumTime",         bShow, bEnable, bReadFromFile, (int)LastSet.iTestHeadCheckVacuumTime, false,  0.1, 100.0, EUMSToSec);  //0.1~100Sec

//    elConfig->Add(edD44_Height, &IniConfig.iD44TestHeadCheckVacuumHeight,       ECInteger,  "Index", "TestHeadCheckVacuumHeight",           bShow, bEnable, bReadFromFile,  0,     false,  0,      2000);        //wei 20150609 延遲破壞高度
    elConfig->Add(cbD45,        &IniConfig.bD45UseOutArmCheckIndex,             ECBool,     "Index", "bD45UseOutArmCheckIndex",             bShow, bEnable, bReadFromFile, "1");

    elConfig->Add(edD46, &IniConfig.iD46WaitIndexDestroyTime,                   ECInteger,  "Index", "iD46WaitIndexDestroyTime",            bShow, bEnable, bReadFromFile,  (int)LastSet.iWaitIndexDestroyTime,     false,  5,      15);

    if(CUSTOMER_CODE==CC_ASE_CL)
        elConfig->Add(cbD47,    &IniConfig.bD47EnableSocketPurgeFunction,       ECBool,     "Index", "bD47EnableSocketPurgeFunction",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD47,    &IniConfig.bD47EnableSocketPurgeFunction,       ECBool,     "Index", "bD47EnableSocketPurgeFunction",       bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbbD47,       &IniConfig.iD47SocketPurgeType,                 ECInteger,  "Index", "iD47SocketPurgeType",                 bShow, bEnable, bReadFromFile, 0);    //Steven 20190703 : Socket Purge include shuttle
    elConfig->Add(edD47_Count,  &IniConfig.iD47SocketPurgeCount,                ECInteger,  "Index", "iD47SocketPurgeCount",                bShow, bEnable, bReadFromFile,  (int)LastSet.iWaitIndexDestroyTime, false, 0,    10);
    elConfig->Add(edD47_Time,   &IniConfig.iD47SocketPurgeTime,                 ECInteger,  "Index", "iD47SocketPurgeTime",                 bShow, bEnable, bReadFromFile,  (int)LastSet.iSocketPurgeTime,      false, 0,    10);
    cbLastSet->Add(edD47_3,     &LastSet.iD47SocketTestedCount,                 ECInteger,  "Index", "iD47SocketTestedCount",               bShow, bEnable, bReadFromFile,  10,    false,  0   , 1000000);
    elConfig->Add(edtD47_5,     &IniConfig.dD47_5_ContactOffset,                ECDouble,   "Index", "dD47_5_ContactOffset",                bShow, bEnable, bReadFromFile,  0.0,   false,  5.0, -5.0);
    elConfig->Add(edtD47_6,     &IniConfig.dD47_6_ShuttleOffset,                ECDouble,   "Index", "dD47_6_ShuttleOffset",                bShow, bEnable, bReadFromFile,  0.0,   false,  5.0, -5.0);

    elConfig->Add(cbD48,        &IniConfig.bD48PowerOffEmgCanNotUseZ1Z2,        ECBool, "Index", "bD48DisablePowerOffEmgStopZ1Z2KeyButton", bShow, bEnable, bReadFromFile, LastSet.bD48PowerOffEmgCanNotUseZ1Z2?"1":"0");
    if(CosFunction.bRTCAlarmSetIndexToErrBin)                                   //JerryYang 20160712 for 力成發生RTC Alarm時把Index上所有IC設為Errorbin
        elConfig->Add(cbD49,    &IniConfig.bD49RTCAlarmSetIndexToErrBin,        ECBool,     "Index", "bD49RTCAlarmSetIndexToErrBin",        bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD49,    &IniConfig.bD49RTCAlarmSetIndexToErrBin,        ECBool,     "Index", "bD49RTCAlarmSetIndexToErrBin",        bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexPickErrSkipNeedCheckVac)                               //jou 20171031 (Steven) : 增加開關JSCC要求index pick up error 需再慢速下降吸一次
        elConfig->Add(cbD50,    &IniConfig.bD50IndexPickErrSkipNeedCheckVac,    ECBool,     "Index", "bD50IndexPickErrSkipNeedCheckVac",    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD50,    &IniConfig.bD50IndexPickErrSkipNeedCheckVac,    ECBool,     "Index", "bD50IndexPickErrSkipNeedCheckVac",    bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(cbD51, &IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear, ECBool,     "Index", "bD51UseOnecycleCleanOutFinishTestArmAtRear", bShow, bEnable, bReadFromFile, LastSet.bUseOnecycleCleanOutFinishTestArmAtRear?"1":"0");
    elConfig->Add(cbD52,        &IniConfig.bD52InterFaceErrHeadNeedUp,          ECBool,     "Index", "bD52InterFaceErrHeadNeedUp",          bShow, bEnable, bReadFromFile, LastSet.bInterFaceErrHeadNeedUp?"1":"0");
    elConfig->Add(cbD53,        &IniConfig.bD53CCDLightOn,                      ECBool,     "Index", "bD53CCDLightOn",                      bShow, bEnable, bReadFromFile, LastSet.bCCDLightOn?"1":"0");

    elConfig->Add(edD53,        &IniConfig.iD53LightOnMin,                      ECInteger,  "Index", "iD53LightOnMin",                      bShow, bEnable, bReadFromFile,  (int)LastSet.iLightOnMin,    false,  1,      10000);
    elConfig->Add(cbD54,        &IniConfig.bD54SlowDown,                        ECBool,     "Index", "bSlowDown",                           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edD54,        &IniConfig.iD54SlowDownScale,                   ECInteger,  "Index", "iSlowDownScale",                      bShow, bEnable, bReadFromFile,  50,    false,  1,      100);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(cbD55,    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bNoShow, bDisable, bFixedValue, 0);
    }
    // AI(W906-FW-CFG-W2) 20260825: GATE (CFG2-COM2) -- NARROWEST FORM. golden's
    // condition is `REAL_TIME_CCD==false || (COM2!=NULL && COM2->bCCDDummyRum)`.
    // The first disjunct is real and stays LIVE; only the second is gated.
    // COM2 exists in this tree as TCOM2Shim (atester_shims.h:405), but that
    // shim deliberately carries a scoped member list and `bCCDDummyRum` is NOT
    // one of its members -- it appears there only inside comments.
    //   cmd: grep -n "bCCDDummyRum" atester_shims.h atester_shims.cpp
    //        -> 3 hits, all comment text, no member (20260825)
    // Gating the WHOLE arm would have been wrong: `REAL_TIME_CCD==false` is the
    // ordinary case on every machine without real-time CCD, and golden shows
    // that arm's checkbox as visible-but-disabled. Keeping it live preserves
    // that. What is lost is narrow and stated: on a machine with RTC ON that is
    // ALSO running the CCD in dummy mode, golden takes this arm and this port
    // falls through to the next one, so cbD55 ends up enabled instead of
    // disabled. A UI enable flag; no machine action depends on it.
    else if(REAL_TIME_CCD==false
#if 0 // GATE (CFG2-COM2) -- COM2->bCCDDummyRum has no port (grep + date above)
            || (COM2!=NULL && COM2->bCCDDummyRum)
#endif // GATE (CFG2-COM2)
           )                                                                    //RTC 沒有開不能Disable IndexCheck
    {
        elConfig->Add(cbD55,    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bDisable, bFixedValue, 0);
    }
    else if(IniConfig.bM0109RTCOffCheckYieldPiggyBack)                          //Steven 20160727 : Fixed for SCK
    {
        elConfig->Add(cbD55,    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bDisable, bFixedValue, 1);
    }
    else                                                                        //ChungHung 20120710 add SCK Disable IndexCheck
    {
        elConfig->Add(cbD55,    &IniConfig.bD55DisableIndexCheck,               ECBool,     "Index", "bDisableIndexCheck",                  bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(cbD56,        &IniConfig.bD56YieldPiggyBackEnable,            ECBool,     "Index", "bYieldPiggyBackEnable",               bShow, bEnable, bReadFromFile, 0);  //kevin 20131009 強致 Enable Yield 裡面piggyback 功能

    if(IniConfig.bKoreaFunction || CUSTOMER_CODE==CC_ASE_KaohSiung)             //kevin 20141203 關site顯示 site號碼
        elConfig->Add(cbD57,    &IniConfig.bD57SiteMapCloseDisplay,             ECBool,     "Index", "bSiteMapCloseDisplay",                bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbD57,    &IniConfig.bD57SiteMapCloseDisplay,             ECBool,     "Index", "bSiteMapCloseDisplay",                bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(cbD58,        &IniConfig.bD58UseArm1PickPlaceArm2Test,        ECBool,     "Index", "bUseArm1PickPlaceArm2Test",           bShow, bEnable, bReadFromFile, 0);  //kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試
    if(MachineTypeChoice==Type_HT9046_LS)                                       //Steven 20150910 : 32Site 雙Arm一起吸放
        elConfig->Add(cbD59,    &IniConfig.bD59_32SitePnpTogether,              ECBool,     "Index", "bD59_32SitePnpTogether",              bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD59,    &IniConfig.bD59_32SitePnpTogether,              ECBool,     "Index", "bD59_32SitePnpTogether",              bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bIndexArmVacOffErrNeedPiggyBack)                             //JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back
        elConfig->Add(cbD61,    &IniConfig.bD61IndexArmVacOffErrNeedPiggyBack,  ECBool,     "Index", "bD61IndexArmVacOffErrNeedPiggyBack",  bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbD61,    &IniConfig.bD61IndexArmVacOffErrNeedPiggyBack,  ECBool,     "Index", "bD61IndexArmVacOffErrNeedPiggyBack",  bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(cbD62,        &IniConfig.bD62PickUpErrorNeedPurge,            ECBool,     "Index", "bD62PickUpErrorNeedPurge",            bShow, bEnable, bReadFromFile, 0);  //Steveb 20161024 : 吸取異常需要吹氣一次

    if(CosFunction.bIndexPickErrSkipBlowAirTime)                                //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
    {
        elConfig->Add(edD62,    &IniConfig.iD62IndexBlowAirTime,                ECInteger,  "Index", "iD62IndexBlowAirTime",                bShow, bEnable, bReadFromFile,  0,     false,  1,      10);
        labD62->Visible=true;
    }
    else
    {
        elConfig->Add(edD62,    &IniConfig.iD62IndexBlowAirTime,                ECInteger,  "Index", "iD62IndexBlowAirTime",                bNoShow, bDisable, bFixedValue, 0);
        labD62->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        pnlD63->Visible=true;
        if(CosFunction.bHiSiliconFunction==true)
        {
            elConfig->Add(cbD63,&IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 1);   //kevin 20170515 (wei) Zphase home
            elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 1);
        }
        else
        {
            elConfig->Add(cbD63,&IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 0);  //kevin 20170515 (wei) Zphase home
            elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bDisable, bFixedValue, 0);
        }
        elConfig->Add(cbD64,    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CosFunction.bHiSiliconFunction==true)                               //Ifor 20170809 (wei) add 海思版本強制開啟 Z Phase 功能
    {
        pnlD63->Visible=true;
        elConfig->Add(cbD63,    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 1);      //kevin 20170515 (wei) Zphase home
        elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable,  bReadFromFile, 0);
        elConfig->Add(cbD64,    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bDisable, bFixedValue, 0);      //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                    //kevin 20180727 move
    {
        pnlD63->Visible=true;
        elConfig->Add(cbD63,    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);     //kevin 20170515 (wei) Zphase home
        elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbD64,    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }
    else if(CUSTOMER_CODE==CC_GIGAS || CUSTOMER_CODE==CC_HONPREC_QC)            //Isaac 20201110 : Index Y find motor phase
    {
        pnlD63->Visible=true;
        elConfig->Add(cbD63,    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);     //Isaac 20201110 : Index Y find motor phase
    }
    else if(IniConfig.bSPILFunction)                                            //Isaac 20201110 : Index Y find motor phase
    {
        pnlD63->Visible=true;
        elConfig->Add(cbD63,    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);     //Isaac 20201110 : Index Y find motor phase
        elConfig->Add(cbD64,    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bDisable, bFixedValue, 0);      //kevin 20171103 (wei) index pick up error only skip
    }
    else
    {
        pnlD63->Visible=false;
        elConfig->Add(cbD63,    &IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange, ECBool,"Index", "bD63CheckIndexZHomeToZPhaseDistanceRange", bShow, bDisable, bFixedValue, 0);      //kevin 20170515 (wei) Zphase home
        elConfig->Add(cbD63_1,  &IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess,   ECBool,"Index", "bD63_1FindMotorPhaseEveryGoHomeProcess"  , bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbD64,    &IniConfig.bD64IndexPickErrOnlySKIP,                 ECBool,"Index", "bD64IndexPickErrOnlySKIP",                 bShow, bEnable, bReadFromFile, 0);     //kevin 20171103 (wei) index pick up error only skip
    }

    if(IniConfig.bSPILFunction==true && CosFunction.bHiSiliconFunction==true)   //JerryYang 20170921 (Steven) SPIL守志要求range要鎖定不得修改
        elConfig->Add(edD63,    &IniConfig.iD63IndexZHomeToZPhaseRange,         ECInteger,  "Index", "iD63IndexZHomeToZPhaseRange",             bShow, bDisable, bFixedValue,   10,    false,  1,      10);       //Isaac 20201124 : default:50->10,Range:1~1000->1~10
    else
        elConfig->Add(edD63,    &IniConfig.iD63IndexZHomeToZPhaseRange,         ECInteger,  "Index", "iD63IndexZHomeToZPhaseRange",             bShow, bEnable, bReadFromFile,  10,    false,  1,      10);           //kevin 20170515 (wei) add Z phase range   //Isaac 20201124 : default:100->10,Range:1~1000->1~10

    if(CUSTOMER_CODE==CC_JCET)                                                  //Ifor 20171123 add
        elConfig->Add(cbD65,    &IniConfig.bD65EnableCheckSocketsensorFunction, ECBool,     "Index", "bD65EnableCheckSocketsensorFunction",     bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD65,    &IniConfig.bD65EnableCheckSocketsensorFunction, ECBool,     "Index", "bD65EnableCheckSocketsensorFunction",     bNoShow, bEnable, bFixedValue, 0);
    elConfig->Add(cbD66,        &IniConfig.bD66Initialstartautoheight,          ECBool,     "Index", "bD66Initialstartautoheight",              bNoShow, bEnable, bFixedValue, 0);          //wei 20180227 Initial start auto height

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20190907 Arm 測區次數道量測 功能;
        elConfig->Add(cbD67,    &IniConfig.bD67LoadCellMeasure,                 ECBool,     "Index", "bD67LoadCellMeasure",                     bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD67,    &IniConfig.bD67LoadCellMeasure,                 ECBool,     "Index", "bD67LoadCellMeasure",                     bNoShow, bEnable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20190925 : add KYEC 要求contact Hight 高度變更超過設定需報警
    {
        labD68_1->Visible=true;
        elConfig->Add(edD68,    &IniConfig.dD68DistanceRange,                   ECDouble,   "Index", "dD68DistanceRange",                       bShow, bEnable, bReadFromFile,  10,   false,  0.01,      10.0);        //Ifor 20190925 : add KYEC 要求contact Hight 高度變更超過設定需報警
    }
    else
    {
        labD68_1->Visible=false;
        elConfig->Add(edD68,    &IniConfig.dD68DistanceRange,                   ECDouble,   "Index", "dD68DistanceRange",                       bNoShow, bDisable, bFixedValue,  10,   false,  0.01,      10.0);
    }

    if(CosFunction.bAfterAutoCleanNoIndexCheck)                                 //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
        elConfig->Add(rgD69,    &IniConfig.iD69IndexCheckModeForAutoClean,      ECInteger,  "Index", "iD69IndexCheckModeForAutoClean",          bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(rgD69,    &IniConfig.iD69IndexCheckModeForAutoClean,      ECInteger,  "Index", "iD69IndexCheckModeForAutoClean",          bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexCycleTimeRecord)                                       //Sam 20200916 : Add Index Cycle Time Record
        elConfig->Add(cbD70,    &IniConfig.bD70IndexCycleTimeRecord,            ECBool,     "Index", "bD70IndexCycleTimeRecord",                bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbD70,    &IniConfig.bD70IndexCycleTimeRecord,            ECBool,     "Index", "bD70IndexCycleTimeRecord",                bNoShow, bEnable, bFixedValue, 0);

    if(CosFunction.bIndexCheckCanTurnOff)                                       //Isaac 20211019 : 可選擇做index check的時機
    {
        elConfig->Add(rgD71,    &IniConfig.iD71IndexCheckOnOffMode,             ECInteger,  "Index", "iD71IndexCheckOnOffMode",                 bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(rgD71,    &IniConfig.iD71IndexCheckOnOffMode,             ECInteger,  "Index", "iD71IndexCheckOnOffMode",                 bNoShow, bEnable, bFixedValue, 0);
    }

    elConfig->Add(cbD72,        &IniConfig.bD72NNModeMoveShtAfterContact,       ECBool,     "Index", "bD72NNModeMoveShtAfterContact",           bShow, bEnable, bReadFromFile, 0);                  //Steven 20220531 : index下壓之後才能移動shuttle
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(cbD73,    &IniConfig.bD73ContactModeFast,                 ECBool,     "Index", "bD73ContactModeFast",                     bShow, bDisable, bFixedValue, 1);                   //kevin 20220827 : 高雄 Contact mode fast
    else
        elConfig->Add(cbD73,    &IniConfig.bD73ContactModeFast,                 ECBool,     "Index", "bD73ContactModeFast",                     bShow, bEnable, bReadFromFile, 0);                  //kevin 20220817 : Conttact mode 加速

    if(REAL_TIME_CCD)                         //Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden
    {
        elConfig->Add(cbD74,        &IniConfig.bD74RTCAutoTuning, ECBool,  "Index", "bD74RTCAutoTuning", bShow, bEnable, bReadFromFile, 0);      //Sam 20240711 : RTC Auto Tuning
        if(CUSTOMER_CODE==CC_SIGURD_HUKOU)     //Sam 20240709 : 矽格湖口家葳要求要強制鎖定開啟
            elConfig->Add(cbD75,    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool,"Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(cbD75,    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool,"Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbD74,    &IniConfig.bD74RTCAutoTuning                       , ECBool, "Index", "bD74RTCAutoTuning",                        bNoShow, bEnable, bFixedValue, 0);   //Sam 20240711 : RTC Auto Tuning
        elConfig->Add(cbD75,    &IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden, ECBool, "Index", "bD75OneCycleFinishedAlawayLearnRTCGolden", bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUseIndexCheckHasICNeedPurge==true)                          //Ifor 20200622 add:Index Check Has IC Need Purge
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)//Ifor 20230619 add:KYEC要求強制開啟D68功能
        {
            elConfig->Add(cbD78,&IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(cbD78,&IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(cbD78,    &IniConfig.bD78EnableIndexCheckHasICNeedPurge,  ECBool,     "Index", "bD78EnableIndexCheckHasICNeedPurge",  bNoShow, bEnable, bFixedValue, 0);
    }

    if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true)                      //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)          //Ifor 20230619 add:KYEC要求強制開啟D69功能
        {
            elConfig->Add(cbD79,&IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(cbD79,&IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bShow, bEnable, bReadFromFile, 0);
        }
    }
    else
    {
        elConfig->Add(cbD79,    &IniConfig.bD79EnableIndexPickShuttleErrNeedPurge,ECBool,   "Index", "bD79EnableIndexPickShuttleErrNeedPurge", bNoShow, bEnable, bFixedValue, 0);
    }

    elConfig->Add(cbD80,        &IniConfig.bD80AfterOutSHToRightSiteIndexCheck, ECBool,     "Index", "bD80EnableIndexPickShuttleErrNeedPurge", bShow, bEnable, bReadFromFile, 1);

    elConfig->Add(cbD81,        &IniConfig.bD81IndexCheckVacuumOnShuttle,       ECBool,     "Index", "bD81IndexCheckVacuumOnShuttle",          bShow, bEnable, bReadFromFile, 0);
    if(CosFunction.bSocketSensorCheckICAtArmDown)                               //Jimmychiu 20250826 : 每次下壓確認有IC在socket
    {
        elConfig->Add(cbD82,    &IniConfig.bD82CheckIndexHasIC,                 ECBool,     "Index", "bD82CheckIndexHasIC",                    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbD82,    &IniConfig.bD82CheckIndexHasIC,                 ECBool,     "Index", "bD82CheckIndexHasIC",                    bNoShow, bDisable, bFixedValue, 0);
    }
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:1761-2111, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemE()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

//    cbTest->Add(edE31_Au1Y,   &IniConfig.dTrayYScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[0], true, 0.95, 1.05);

    cbLastSet->Add(cbE30,       &IniConfig.bE30InArmUseDifferentScale,          ECBool,     "In/Out Arm", "bE30InArmUseDifferentScale",         bShow, bEnable, bReadFromFile, LastSet.bE30InArmUseDifferentScale?"1":"0");         //Steven 20090710

    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(cbE30_1,&IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,     "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bEnable, bReadFromFile, 0);      //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
        cbLastSet->Add(cbE30_2,&IniConfig.bE30_2InArmUseDifferentScale_Cold,    ECBool,     "In/Out Arm", "bE30_2InArmUseDifferentScale_Cold",    bShow, bEnable, bReadFromFile, 0);
    }

    cbLastSet->Add(cbE31,       &IniConfig.bE31OutArmUseDifferentScale,         ECBool,     "In/Out Arm", "bE31OutArmUseDifferentScale",        bShow, bEnable, bReadFromFile, LastSet.bE31OutArmUseDifferentScale?"1":"0");        //Steven 20090710
    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(cbE31_1,        &IniConfig.bE31_1OutArmUseDifferentScale_Hot,     ECBool,     "In/Out Arm", "bE31_1OutArmUseDifferentScale_Hot",    bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(edE31_1_Au1X,   &IniConfig.dTrayXScale_Hot[eAuto1],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto1_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[0], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au1Y,   &IniConfig.dTrayYScale_Hot[eAuto1],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto1_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[0], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Au2X,   &IniConfig.dTrayXScale_Hot[eAuto2],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto2_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[1], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au2Y,   &IniConfig.dTrayYScale_Hot[eAuto2],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto2_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[1], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Au3X,   &IniConfig.dTrayXScale_Hot[eAuto3],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto3_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[2], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au3Y,   &IniConfig.dTrayYScale_Hot[eAuto3],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto3_Hot",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[2], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Au4X,   &IniConfig.dTrayXScale_Hot[eAuto4],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto4_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au4Y,   &IniConfig.dTrayYScale_Hot[eAuto4],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto4_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Au5X,   &IniConfig.dTrayXScale_Hot[eAuto5],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto5_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au5Y,   &IniConfig.dTrayYScale_Hot[eAuto5],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto5_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Au6X,   &IniConfig.dTrayXScale_Hot[eAuto6],      ECDouble,   "In/Out Arm", "dTrayXScaleAuto6_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Au6Y,   &IniConfig.dTrayYScale_Hot[eAuto6],      ECDouble,   "In/Out Arm", "dTrayYScaleAuto6_Hot",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi1X,   &IniConfig.dTrayXScale_Hot[eFix1],       ECDouble,   "In/Out Arm", "dTrayXScaleFix1_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[3], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi1Y,   &IniConfig.dTrayYScale_Hot[eFix1],       ECDouble,   "In/Out Arm", "dTrayYScaleFix1_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[3], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi2X,   &IniConfig.dTrayXScale_Hot[eFix2],       ECDouble,   "In/Out Arm", "dTrayXScaleFix2_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[4], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi2Y,   &IniConfig.dTrayYScale_Hot[eFix2],       ECDouble,   "In/Out Arm", "dTrayYScaleFix2_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[4], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi3X,   &IniConfig.dTrayXScale_Hot[eFix3],       ECDouble,   "In/Out Arm", "dTrayXScaleFix3_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Hot[5], false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi3Y,   &IniConfig.dTrayYScale_Hot[eFix3],       ECDouble,   "In/Out Arm", "dTrayYScaleFix3_Hot",                bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Hot[5], false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi4X,   &IniConfig.dTrayXScale_Hot[eFix4],       ECDouble,   "In/Out Arm", "dTrayXScaleFix4_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi4Y,   &IniConfig.dTrayYScale_Hot[eFix4],       ECDouble,   "In/Out Arm", "dTrayYScaleFix4_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi5X,   &IniConfig.dTrayXScale_Hot[eFix5],       ECDouble,   "In/Out Arm", "dTrayXScaleFix5_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi5Y,   &IniConfig.dTrayYScale_Hot[eFix5],       ECDouble,   "In/Out Arm", "dTrayYScaleFix5_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_1_Fi6X,   &IniConfig.dTrayXScale_Hot[eFix6],       ECDouble,   "In/Out Arm", "dTrayXScaleFix6_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_1_Fi6Y,   &IniConfig.dTrayYScale_Hot[eFix6],       ECDouble,   "In/Out Arm", "dTrayYScaleFix6_Hot",                bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(cbE31_2,        &IniConfig.bE31_2OutArmUseDifferentScale_Cold,   ECBool,     "In/Out Arm", "bE31_2OutArmUseDifferentScale_Cold",   bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(edE31_2_Au1X,   &IniConfig.dTrayXScale_Cold[eAuto1],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto1_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[0], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au1Y,   &IniConfig.dTrayYScale_Cold[eAuto1],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto1_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[0], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Au2X,   &IniConfig.dTrayXScale_Cold[eAuto2],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto2_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[1], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au2Y,   &IniConfig.dTrayYScale_Cold[eAuto2],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto2_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[1], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Au3X,   &IniConfig.dTrayXScale_Cold[eAuto3],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto3_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[2], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au3Y,   &IniConfig.dTrayYScale_Cold[eAuto3],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto3_Cold",              bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[2], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Au4X,   &IniConfig.dTrayXScale_Cold[eAuto4],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto4_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au4Y,   &IniConfig.dTrayYScale_Cold[eAuto4],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto4_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Au5X,   &IniConfig.dTrayXScale_Cold[eAuto5],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto5_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au5Y,   &IniConfig.dTrayYScale_Cold[eAuto5],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto5_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Au6X,   &IniConfig.dTrayXScale_Cold[eAuto6],     ECDouble,   "In/Out Arm", "dTrayXScaleAuto6_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Au6Y,   &IniConfig.dTrayYScale_Cold[eAuto6],     ECDouble,   "In/Out Arm", "dTrayYScaleAuto6_Cold",              bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi1X,   &IniConfig.dTrayXScale_Cold[eFix1],      ECDouble,   "In/Out Arm", "dTrayXScaleFix1_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[3], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi1Y,   &IniConfig.dTrayYScale_Cold[eFix1],      ECDouble,   "In/Out Arm", "dTrayYScaleFix1_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[3], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi2X,   &IniConfig.dTrayXScale_Cold[eFix2],      ECDouble,   "In/Out Arm", "dTrayXScaleFix2_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[4], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi2Y,   &IniConfig.dTrayYScale_Cold[eFix2],      ECDouble,   "In/Out Arm", "dTrayYScaleFix2_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[4], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi3X,   &IniConfig.dTrayXScale_Cold[eFix3],      ECDouble,   "In/Out Arm", "dTrayXScaleFix3_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayXScale_Cold[5], false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi3Y,   &IniConfig.dTrayYScale_Cold[eFix3],      ECDouble,   "In/Out Arm", "dTrayYScaleFix3_Cold",               bShow, bEnable, bReadFromFile, LastSet.fTrayYScale_Cold[5], false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi4X,   &IniConfig.dTrayXScale_Cold[eFix4],      ECDouble,   "In/Out Arm", "dTrayXScaleFix4_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi4Y,   &IniConfig.dTrayYScale_Cold[eFix4],      ECDouble,   "In/Out Arm", "dTrayYScaleFix4_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi5X,   &IniConfig.dTrayXScale_Cold[eFix5],      ECDouble,   "In/Out Arm", "dTrayXScaleFix5_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi5Y,   &IniConfig.dTrayYScale_Cold[eFix5],      ECDouble,   "In/Out Arm", "dTrayYScaleFix5_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);

        cbLastSet->Add(edE31_2_Fi6X,   &IniConfig.dTrayXScale_Cold[eFix6],      ECDouble,   "In/Out Arm", "dTrayXScaleFix6_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
        cbLastSet->Add(edE31_2_Fi6Y,   &IniConfig.dTrayYScale_Cold[eFix6],      ECDouble,   "In/Out Arm", "dTrayYScaleFix6_Cold",               bShow, bEnable, bReadFromFile, 1.0, false, 0.95, 1.05);
    }
    cbLastSet->Add(cbE32,       &IniConfig.bE32ShuttleUseDifferentScale,        ECBool,     "In/Out Arm", "bE32ShuttleUseDifferentScale",       bShow, bEnable, bReadFromFile, LastSet.bE32ShuttleUseDifferentScale?"1":"0");       //Steven 20090717

    if(Tri_Temp_Machine==1)
    {
        cbLastSet->Add(cbE32_1,&IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,     "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(cbE32_2,&IniConfig.bE32_2ShuttleUseDifferentScale_Cold,  ECBool,     "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Cold",  bShow, bEnable, bReadFromFile, 0);      //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
    }

    cbLastSet->Add(edE31_Au1X,   &IniConfig.dTrayXScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[0], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au1Y,   &IniConfig.dTrayYScale[eAuto1],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto1",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[0], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Au2X,   &IniConfig.dTrayXScale[eAuto2],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto2",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[1], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au2Y,   &IniConfig.dTrayYScale[eAuto2],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto2",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[1], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Au3X,   &IniConfig.dTrayXScale[eAuto3],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto3",                   bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[2], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au3Y,   &IniConfig.dTrayYScale[eAuto3],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto3",                   bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[2], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Au4X,   &IniConfig.dTrayXScale[eAuto4],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto4",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au4Y,   &IniConfig.dTrayYScale[eAuto4],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto4",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(edE31_Au5X,   &IniConfig.dTrayXScale[eAuto5],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto5",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au5Y,   &IniConfig.dTrayYScale[eAuto5],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto5",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(edE31_Au6X,   &IniConfig.dTrayXScale[eAuto6],                ECDouble,   "In/Out Arm", "dTrayXScaleAuto6",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Au6Y,   &IniConfig.dTrayYScale[eAuto6],                ECDouble,   "In/Out Arm", "dTrayYScaleAuto6",                   bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi1X,   &IniConfig.dTrayXScale[eFix1],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix1",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[3], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi1Y,   &IniConfig.dTrayYScale[eFix1],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix1",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[3], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi2X,   &IniConfig.dTrayXScale[eFix2],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix2",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[4], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi2Y,   &IniConfig.dTrayYScale[eFix2],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix2",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[4], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi3X,   &IniConfig.dTrayXScale[eFix3],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix3",                    bShow, bEnable, bReadFromFile, LastSet.fTrayXScale[5], true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi3Y,   &IniConfig.dTrayYScale[eFix3],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix3",                    bShow, bEnable, bReadFromFile, LastSet.fTrayYScale[5], true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi4X,   &IniConfig.dTrayXScale[eFix4],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix4",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi4Y,   &IniConfig.dTrayYScale[eFix4],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix4",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi5X,   &IniConfig.dTrayXScale[eFix5],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix5",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi5Y,   &IniConfig.dTrayYScale[eFix5],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix5",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    cbLastSet->Add(edE31_Fi6X,   &IniConfig.dTrayXScale[eFix6],                 ECDouble,   "In/Out Arm", "dTrayXScaleFix6",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);
    cbLastSet->Add(edE31_Fi6Y,   &IniConfig.dTrayYScale[eFix6],                 ECDouble,   "In/Out Arm", "dTrayYScaleFix6",                    bShow, bEnable, bReadFromFile, 1.0, true, 0.95, 1.05);

    if(CosFunction.bHotModeUseDiffScale)            //JerryYang 20250120 : add
    {
        cbLastSet->Add(cbE30_1,  &IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,   "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bEnable, bReadFromFile, 0);
        cbLastSet->Add(cbE32_1,  &IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,   "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        cbLastSet->Add(cbE30_1,  &IniConfig.bE30_1InArmUseDifferentScale_Hot,     ECBool,   "In/Out Arm", "bE30_1InArmUseDifferentScale_Hot",     bShow, bDisable, bFixedValue, 0);
        cbLastSet->Add(cbE32_1,  &IniConfig.bE32_1ShuttleUseDifferentScale_Hot,   ECBool,   "In/Out Arm", "bE32_1ShuttleUseDifferentScale_Hot",   bShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbE33,        &IniConfig.bE33InOutArmZOffsetSameOne,          ECBool,     "In/Out Arm", "bE33InOutArmZOffsetSameOne",         bShow, bEnable, bReadFromFile, LastSet.bInOutArmZOffsetSameOne?"1":"0");            //jou 2010-06-21
    if(CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(cbE34,    &IniConfig.bE34InOutArmPitchZOffsetSameOne,     ECBool,     "In/Out Arm", "bE34InOutArmPitchZOffsetSameOne",    bShow, bDisable, bFixedValue, 0);         //Ifor 20160920 KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E34   功能(pick/release offset 要獨立)
    else
        elConfig->Add(cbE34,    &IniConfig.bE34InOutArmPitchZOffsetSameOne,     ECBool,     "In/Out Arm", "bE34InOutArmPitchZOffsetSameOne",    bShow, bEnable, bReadFromFile, LastSet.bInOutArmPitchZOffsetSameOne?"1":"0"); //jou 2010-12-20 Pitch ,& Z 縮減為一個

    if(IniConfig.bEnableInOutArmPlaceSkipSuckDetect)
        elConfig->Add(cbE35,    &IniConfig.bInOutArmPlaceSkipSuckDetect,        ECBool,     "In/Out Arm", "bInOutArmPlaceSkipSuckDetect",       bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbE35,    &IniConfig.bInOutArmPlaceSkipSuckDetect,        ECBool,     "In/Out Arm", "bInOutArmPlaceSkipSuckDetect",       bNoShow, bEnable, bFixedValue, 0);

    elConfig->Add(edE36,        &IniConfig.iInArm60mmOffset,                    ECInteger,  "In/Out Arm", "iInArm60mmOffset",                   bShow, bEnable, bReadFromFile,  0,     false,  100,    -100);           //jou 2011-08-05 In Arm Y Pitch 60mm Offset
    elConfig->Add(edE37,        &IniConfig.iOutArm60mmOffset,                   ECInteger,  "In/Out Arm", "iOutArm60mmOffset",                  bShow, bEnable, bReadFromFile,  3,     false,  100,    -100);           //jou 2011-08-05 Out Arm Y Pitch 60mm Offset

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(cbE38,    &IniConfig.bE38CheckHotPlateWhileInitialStart,  ECBool,         "In/Out Arm", "CheckHotPlateWhileInitialStart", bShow, bDisable, bFixedValue, 0);                       //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E38   功能(Initial Start Check HotPlate)
        elConfig->Add(cbE39,&IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed, ECBool, "In/Out Arm", "CheckHotPlateAfterCleanOutAndBeforeTrayFeed", bShow, bDisable, bFixedValue, 0);          //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E39   功能(Clean Out Tray Feed之前Check HotPlate)
        elConfig->Add(cbE39_1,  &IniConfig.bE39_1PutTheDevicesToErrorBin,       ECBool,         "In/Out Arm", "PutTheDevicesToErrorBin",        bShow, bDisable, bFixedValue, 0);                       //Ifor 20170417 (wei) KYEC 客戶 喬智要求反灰不可修改，強制設定不使用E39_1 功能(HotPlate Check 放至 Err bin)
    }
    else
    {
        elConfig->Add(cbE38,    &IniConfig.bE38CheckHotPlateWhileInitialStart,  ECBool,         "In/Out Arm", "CheckHotPlateWhileInitialStart", bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120206 Hotplate check
        elConfig->Add(cbE39,&IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed, ECBool, "In/Out Arm", "CheckHotPlateAfterCleanOutAndBeforeTrayFeed", bShow, bEnable, bReadFromFile, 0);         //ChungHung 20120206 Hotplate check
        elConfig->Add(cbE39_1,  &IniConfig.bE39_1PutTheDevicesToErrorBin,       ECBool,         "In/Out Arm", "PutTheDevicesToErrorBin",        bShow, bEnable, bReadFromFile, 0);
    }

    elConfig->Add(cbE40,        &IniConfig.bE40ClearAllHotICThenPickLoadIC,     ECBool,     "In/Out Arm", "bClearAllHotICThenPickLoadIC",       bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120514 Clear HotIC then Pick Load IC
    elConfig->Add(cbE41,        &IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent, ECBool,"In/Out Arm", "b35TrayPitchInOutSpeedSmall80Percent", bShow, bEnable, bReadFromFile, 0);                    //Steven 20120727 : Tray Pitch 大於35mm的話 In Out Arm需要小於80%
    if(CUSTOMER_CODE==CC_SCC)
        elConfig->Add(cbE42,    &IniConfig.bE42OutShuttleAlarmInArmServoOff,    ECBool,     "In/Out Arm", "bOutShuttleAlarmInArmServoOff",      bShow, bDisable, bFixedValue, 1);                       //ChungHung 20120723 Output Shuttle Alarm InArm Servo off
    else
        elConfig->Add(cbE42,    &IniConfig.bE42OutShuttleAlarmInArmServoOff,    ECBool,     "In/Out Arm", "bOutShuttleAlarmInArmServoOff",      bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120723 Output Shuttle Alarm InArm Servo off

    if(CosFunction.bAutoCleanUseHPSetByRecipe)                                  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    {
        cbE43->Visible=false;
    }
    else if(IniConfig.bEnableAutoCleanFunction)                                 //Steven 20231121 : 有Auto Clean就可以選擇使用加熱盤
    {
        elConfig->Add(cbE43,    &IniConfig.bE43AutoCleanUseHotplate,            ECBool,     "In/Out Arm", "bAutoCleanUseHotplate",              bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20131120 AutoClean use Hotplate1cbE38,&IniConfig.bE38CheckHotPlateWhileInitialStart}       //ChungHung 20120206 Hotplate check
    }
    else
    {
        elConfig->Add(cbE43,    &IniConfig.bE43AutoCleanUseHotplate,            ECBool,     "In/Out Arm", "bAutoCleanUseHotplate",              bNoShow, bDisable, bFixedValue, 0);                     //ChungHung 20140408 add only for SCK
    }

    if(CUSTOMER_CODE==CC_SJ_Semiconductor)
        elConfig->Add(chkE43_1, &IniConfig.bE43_1_AutoCleanCountSaveFolder,     ECBool,     "In/Out Arm", "bE43_1_AutoCleanCountSaveFolder",    bShow, bEnable, bReadFromFile, 1);                      //Steven 20250527 : Save auto clean count to folder
    else
        elConfig->Add(chkE43_1, &IniConfig.bE43_1_AutoCleanCountSaveFolder,     ECBool,     "In/Out Arm", "bE43_1_AutoCleanCountSaveFolder",    bShow, bEnable, bReadFromFile, 0);                      //Steven 20250527 : Save auto clean count to folder

    if(CosFunction.bLoseDeviceOutShuttleServoOff==false)                        //ChungHung 20140522 add OutShuttle lose devices can servo off
        elConfig->Add(cbE44, &IniConfig.bE44EnableLoseDeviceOutShuttleServoOff, ECBool,     "In/Out Arm", "bEnableLoseDeviceOutShuttleServoOff", bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbE44, &IniConfig.bE44EnableLoseDeviceOutShuttleServoOff, ECBool,     "In/Out Arm", "bEnableLoseDeviceOutShuttleServoOff", bShow, bEnable, bReadFromFile, 0);

    if(Tri_Temp_Machine==1)                                                     //Wenqi 20240516 Modify offset by workfile
        elConfig->Add(cbE45,    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bEnable, bReadFromFile, 0);
    else if(CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(cbE45,    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bDisable, bFixedValue, 1);                       //wei 20151104 複製機台的offset避免offset讀不到資料  //Ifor 20191001 : KYEC [E45] false => true
    else if(CUSTOMER_CODE==CC_ASE_CL)   //JerryYang 20250120 : ASECL SONG要求強制關閉
        elConfig->Add(cbE45,    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bDisable, bFixedValue, 0);                       //wei 20151104 複製機台的offset避免offset讀不到資料  //Ifor 20191001 : KYEC [E45] false => true
    else
        elConfig->Add(cbE45,    &IniConfig.bE45_AllSetupFileUseOneFile,         ECBool,     "In/Out Arm", "bE45_AllSetupFileUseOneFile",        bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : 所有工作檔共用同一個Offset檔案

    elConfig->Add(cbE46,        &IniConfig.bE46_LoaderUse2Offset,               ECBool,     "In/Out Arm", "bE46_LoaderUse2Offset",              bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : Loader可以分別調前後排的Offset
    elConfig->Add(cbE47,        &IniConfig.bE47_ShuttleUse4Offset,              ECBool,     "In/Out Arm", "bE47_ShuttleUse4Offset",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20140827 : Shuttle可以分別調前後排的Offset
    elConfig->Add(cbE48,        &IniConfig.bE48_ShuttleUse4Offset_Autoclean,    ECBool,     "In/Out Arm", "bE48_ShuttleUse4Offset_Autoclean",   bShow, bEnable, bReadFromFile, 0);                      //20140923 wei :  Auto clean Shuttle可以分別調前後排的Offset
    elConfig->Add(cbE49,        &IniConfig.bE49_LoaderOnlyRetryAndCleanOut,     ECBool,     "In/Out Arm", "bE49_LoaderOnlyRetryAndCleanOut",    bShow, bEnable, bReadFromFile, 0);                      //Steven 20141105 : Loader吸取異常只能Retry與CleanOut

    if(CosFunction.bOutArmPickUpErrBySetup==true)                               //JerryYang 20220923 : E50改成選項
    {
        rgE50->Visible=false;
    }
    else
    {
        elConfig->Add(rgE50,    &IniConfig.iE50_OutArmPickUpErrorOption,        ECInteger,  "InOutArm", "iE50_OutArmPickUpErrorOption",         bShow, bEnable, bReadFromFile, 0);                      //Steven 20181226 : 測試Time out可以按的按鈕
    }

    elConfig->Add(cbE51,        &IniConfig.bE51_EnableInArmZADC,                ECBool,     "In/Out Arm", "bE51_EnableInArmZADC",               bShow, bEnable, bReadFromFile, 0);                      //Steven 20141212 : 使用固定的ADC
    elConfig->Add(edE51,        &IniConfig.iE51_EnableInArmZADC,                ECInteger,  "In/Out Arm", "iE51_EnableInArmZADC",               bShow, bEnable, bReadFromFile,  100,   false,  1,      100);            //Steven 20141212 : 使用固定的ADC
    elConfig->Add(cdE52,        &IniConfig.bE52_EnableOutArmZADC,               ECBool,     "In/Out Arm", "bE52_EnableOutArmZADC",              bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edE52,        &IniConfig.iE52_EnableOutArmZADC,               ECInteger,  "In/Out Arm", "iE52_EnableOutArmZADC",              bShow, bEnable, bReadFromFile,  100,   false,  1,      100);            //Steven 20141212 : 使用固定的ADC

    if(CosFunction.bLowYieldAutoSiteOff)
        elConfig->Add(cbE53,    &IniConfig.bE53LowYieldAutoClean,               ECBool,     "In/Out Arm", "bE53LowYieldAutoClean",              bNoShow, bDisable, bFixedValue, 0);                     //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    else
        elConfig->Add(cbE53,    &IniConfig.bE53LowYieldAutoClean,               ECBool,     "In/Out Arm", "bE53LowYieldAutoClean",              bShow, bEnable, bReadFromFile, 0);                      //kevin 20160802 : CosFunction.bLowYieldAutoClean --> IniConfig.bE53LowYieldAutoClean

    elConfig->Add(cbE54,        &IniConfig.bE54CheckCloseSiteNoIC,              ECBool,     "In/Out Arm", "bE54CheckCloseSiteNoIC",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20160922 : 因為OneCycle永遠先跑蝦頭1 檢查加熱盤錯誤功能與ByArmCloseSite衝突

    if(CosFunction.bUseFix3FullTray && FIX3_FULL_PLACE==Fix3K_Uninstall)        //Ifor 20161209 如果有安裝 Fix3 Full Place 強制關閉 Fix3 Full Tray 功能
        elConfig->Add(cbE55,    &IniConfig.bE55UseFix3FullTray,                 ECBool,     "In/Out Arm", "bE55UseFix3FullTray",                bShow, bEnable, bReadFromFile, 0);                      //Ifor 20161121 : add Use Fix3 Full Tray Function
    else
        elConfig->Add(cbE55,    &IniConfig.bE55UseFix3FullTray,                 ECBool,     "In/Out Arm", "bE55UseFix3FullTray",                bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bAutoSkipNoDropError)
    {
        elConfig->Add(cbE56,    &IniConfig.bE56LoaderRetryAtSamePosition,       ECBool,     "In/Out Arm", "bE56LoaderRetryAtSamePosition",      bShow, bDisable, bFixedValue, 1);                       //Steven 20170828 (wei) : Loader吸取異常時要在同一個位置作Retry
    }
    else
    {
        elConfig->Add(cbE56,    &IniConfig.bE56LoaderRetryAtSamePosition,       ECBool,     "In/Out Arm", "bE56LoaderRetryAtSamePosition",      bShow, bEnable, bReadFromFile, 0);                      //Steven 20170828 (wei) : Loader吸取異常時要在同一個位置作Retry
    }

    elConfig->Add(cbE57,        &IniConfig.bE57HPCanUseAnotherVacuumDelay,      ECBool,     "In/Out Arm", "bE57HPCanUseAnotherVacuumDelay",     bShow, bEnable, bReadFromFile, 0);                      //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    elConfig->Add(cbE58,        &IniConfig.bE57YPitchHome,                      ECBool,     "In/Out Arm", "bE57YPitchHome",                     bShow, bEnable, bReadFromFile, 0);                      //kevin 20180822 (Steven) : in out arm Y pitch 放完IC 歸 Y PITCH HOME
    elConfig->Add(cbE59,        &IniConfig.bE59GroupOffsetFile,                 ECBool,     "In/Out Arm", "bE59GroupOffsetFile",                bShow, bEnable, bReadFromFile, 0);                      //Steven 20190327 : Offset file使用中括號做群組
    if(CosFunction.bAutoSkipNoDropError)
    {
        elConfig->Add(cbE60,    &IniConfig.bE60PickLoaderDropAutoSkip,          ECBool,     "In/Out Arm", "bE60PickLoaderDropAutoSkip",         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbE60,    &IniConfig.bE60PickLoaderDropAutoSkip,          ECBool,     "In/Out Arm", "bE60PickLoaderDropAutoSkip",         bNoShow, bDisable, bFixedValue, 0);
    }
    elConfig->Add(cbE61,        &IniConfig.bE61InArmStandbyPosOnLoader,         ECBool,     "In/Out Arm", "bE61InArmStandbyPosOnLoader",        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbE62,        &IniConfig.bE62TryPickLastRow,                  ECBool,     "In/Out Arm", "bE62TryPickLastRow",                 bShow, bEnable, bReadFromFile, 0);                      //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
    elConfig->Add(cbE63,        &IniConfig.bE63RetryPickLoader,                 ECBool,     "In/Out Arm", "bE63RetryPickLoader",                bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20210324 add:KYEC 暫不使用等待後續討論結束
    {
        elConfig->Add(cbE64,    &IniConfig.bE64_50TrayPitchInOutSpeedSmall50Percent, ECBool, "In/Out Arm", "bE64_50TrayPitchInOutSpeedSmall50Percent", bShow, bDisable, bFixedValue, 1);     //Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50%
        elConfig->Add(chkE65,   &IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr,   ECBool, "In/Out Arm", "bE65_ClearTrayDataWhenOutArmDestoryErr", bShow, bDisable, bFixedValue, 0);                  //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
    }
    else
    {
        elConfig->Add(cbE64,    &IniConfig.bE64_50TrayPitchInOutSpeedSmall50Percent,    ECBool, "In/Out Arm",   "bE64_50TrayPitchInOutSpeedSmall50Percent", bShow, bEnable, bReadFromFile, 0);     //Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50%
        elConfig->Add(chkE65,   &IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr,      ECBool, "In/Out Arm",   "bE65_ClearTrayDataWhenOutArmDestoryErr", bShow, bEnable, bReadFromFile, 0);                  //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
    }

    elConfig->Add(chkE66,       &IniConfig.bE66_LogHotPlateAction,              ECBool,     "In/Out Arm", "bE66_LogHotPlateAction",             bShow, bEnable, bReadFromFile, 0);                      //Steven 20211104 : 紀錄加熱盤的動作
    elConfig->Add(chkE67,       &IniConfig.bE67_LoadPickerrorMoveWaitpos,       ECBool,     "In/Out Arm", "bE67_LoadPickerrorMoveWaitpos",      bShow, bEnable, bReadFromFile, 0);                      //kevin 20220723  : Load Pick up error Move wait pos
    elConfig->Add(cbE68,        &IniConfig.bE68RecheckInOutArmICFallDown,       ECBool,     "In/Out Arm", "bE68RecheckInOutArmICFallDown",      bShow, bEnable, bReadFromFile, 0);                      //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    elConfig->Add(edE68,        &IniConfig.iE68RecheckInOutArmICFallDown,       ECInteger,  "In/Out Arm", "iE68RecheckInOutArmICFallDown",      bShow, bEnable, bReadFromFile,  3,     false,  1,      100);              //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    elConfig->Add(chkE69,       &IniConfig.bE69_PickupErrorPlacement,           ECBool,     "In/Out Arm", "bE69_PickupErrorPlacement",          (USE_InPlacement==eartUninstall)?bNoShow:bShow, (USE_InPlacement==eartUninstall)?bDisable:bEnable, bReadFromFile, 0);  //JimmyChiu 20220908 add Pickup Error Placement
    if(CosFunction.bUseTrayThickAdjustZHeight==true)                            //Ifor 20221214 add: 使用Tray 厚度自動校正吸嘴高度
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE  && bEnable_KLT_Function==false)          //Ifor 20221220 add:KYEC要求強制開啟並反灰
        {
            elConfig->Add(chkE70,&IniConfig.bE70_UseTrayThickAdjustZHeight,     ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bShow, bDisable, bFixedValue, 1);       //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
        }
        else
        {
            elConfig->Add(chkE70,&IniConfig.bE70_UseTrayThickAdjustZHeight,     ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
        }
    }
    else
    {
        elConfig->Add(chkE70,&IniConfig.bE70_UseTrayThickAdjustZHeight,         ECBool,     "Tray", "bE70_UseTrayThickAdjustZHeight" ,          bNoShow, bDisable, bFixedValue, 0);     //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)               //Ifor 20221220 add:KYEC要求強制開啟並反灰
    {
        elConfig->Add(chkE71,   &IniConfig.bE71_10TrayPitchLockTrayAssign,      ECBool,     "Tray",  "bE71_10TrayPitchLockTrayAssign",          bShow, bDisable, bFixedValue, 1);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    }
    else
    {
        elConfig->Add(chkE71,   &IniConfig.bE71_10TrayPitchLockTrayAssign,      ECBool,     "Tray",  "bE71_10TrayPitchLockTrayAssign",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    }
    elConfig->Add(chkE72,       &IniConfig.bE72_InarmPickICNeedWaitSH,          ECBool,     "In/Out Arm", "bE72_InarmPickICNeedWaitSH",         bShow, bEnable, bReadFromFile, 0);      //KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態

    elConfig->Add(chkE73,       &IniConfig.bE73_InOutZStepMotorLossCheck,       ECBool,     "Tray",  "bE73_InOutZStepMotorLossCheck",           bShow, bEnable, bReadFromFile, 0);      //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    elConfig->Add(edE73,        &IniConfig.iE73StepMotorCheckCnt,               ECInteger,  "In/Out Arm",           "iE73StepMotorCheckCnt",            bShow, bEnable, bReadFromFile,  5,   false,  3,      10);               //JerryYang 20231113 : add

    #ifdef SOFT_SIMULTE
    elConfig->Add(chkE74,       &IniConfig.bE74_InspectArmPosition,             ECBool,     "Tray",  "bE74_InspectArmPosition",                 bShow, bEnable, bReadFromFile, 0);      //Jimmychiu 20240408 : debug for inarm position
    #else
    elConfig->Add(chkE74,       &IniConfig.bE74_InspectArmPosition,             ECBool,     "Tray",  "bE74_InspectArmPosition",                 bNoShow, bDisable, bFixedValue, 0);      //Jimmychiu 20240408 : debug for inarm position
    #endif
    if(CosFunction.bOneByOneWhenPickErrAtLoader==true)
    {
        elConfig->Add(cbkE78,       &IniConfig.bE78OneByOneWhenPickErrAtLoader,     ECBool,     "Tray",  "bE78OneByOneWhenPickErrAtLoader",         bShow, bEnable, bReadFromFile, 1);  //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
    }

    if(CosFunction.bFillTheTrayAfterOutArmPlace==true)                          //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    {
        elConfig->Add(cbE85_Enable,     &IniConfig.bE85_FillTray_Enable,        ECBool,     "Tray",  "bE85_FillTray_Enable",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbE85_Auto1,      &IniConfig.bE85_FillTray_Auto1,         ECBool,     "Tray",  "bE85_FillTray_Auto1",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbE85_Auto3,      &IniConfig.bE85_FillTray_Auto3,         ECBool,     "Tray",  "bE85_FillTray_Auto3",                     bShow, bEnable, bReadFromFile, 0);
    }
    gbE85->Visible=CosFunction.bFillTheTrayAfterOutArmPlace;
    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(cbE86,       &IniConfig.bE86_InArmPickErrOnLoaderOnlyCanSKIP, ECBool,   "In/Out Arm",  "bE72_InArmPickErrOnLoaderOnlyCanSKIP", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbE87,       &IniConfig.bE87PickupErrorAtLoaderNeedOpenDoor,  ECBool,   "In/Out Arm",  "bE73PickupErrorAtLoaderNeedOpenDoor",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbE88,       &IniConfig.bE88_InArmHeightFollow7000,           ECBool,   "In/Out Arm",  "bE75_InArmHeightFollow7000",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbE89,       &IniConfig.bE89_InArmHotPlatePitchUseScale,      ECBool,   "In/Out Arm",  "bE76_InArmHotPlatePitchUseScale",      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbE86,       &IniConfig.bE86_InArmPickErrOnLoaderOnlyCanSKIP, ECBool,   "In/Out Arm",  "bE72_InArmPickErrOnLoaderOnlyCanSKIP", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbE87,       &IniConfig.bE87PickupErrorAtLoaderNeedOpenDoor,  ECBool,   "In/Out Arm",   "bE73PickupErrorAtLoaderNeedOpenDoor", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbE88,       &IniConfig.bE88_InArmHeightFollow7000,           ECBool,   "In/Out Arm",  "bE75_InArmHeightFollow7000",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbE89,       &IniConfig.bE89_InArmHotPlatePitchUseScale,      ECBool,   "In/Out Arm",  "bE76_InArmHotPlatePitchUseScale",      bNoShow, bDisable, bFixedValue, 0);
    }
    //Steven 20260428 [E90]: Fix Tray Full retreat OutArm Y by -15000 for back-door tray pickup; ASE_SG forced ON
    if(CUSTOMER_CODE==CC_ASE_SG)
        elConfig->Add(cbE90,   &IniConfig.bE90_OutArmFixFullExtraY, ECBool, "In/Out Arm", "bE90_OutArmFixFullExtraY", bNoShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbE90,   &IniConfig.bE90_OutArmFixFullExtraY, ECBool, "In/Out Arm", "bE90_OutArmFixFullExtraY", bShow,   bEnable,  bReadFromFile, 0);
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:2113-2328, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemF()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(cbF01,        &IniConfig.bF01ShakeShuttleWhenJam,             ECBool,     "Shuttle", "bF01ShakeShuttleWhenJam",               bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edF01,        &IniConfig.iF01ShuttleShakeSpeed,               ECInteger,  "Shuttle", "iF01ShuttleShakeSpeed",                 bShow, bEnable, bReadFromFile, LastSet.iShuttleShakeSpeed,    false,  1,      100);//Steven 20120802 : Shuttle搖搖速度   //Steven 20160531 (wei) : 50 --> 1
    elConfig->Add(cbF03,        &IniConfig.bF03OutputShuttleSkipICMiss,         ECBool,     "Shuttle", "bF03OutputShuttleSkipICMiss",           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edF05,        &IniConfig.iF05ShuttlePurgeCount,               ECInteger,  "Shuttle", "iF05ShuttlePurgeCount",                 bShow, bEnable, bReadFromFile, LastSet.iShuttlePurgeCount,    false,  1,      100);
    elConfig->Add(cbF05,        &IniConfig.bF05EnableShtPurgeFunction,          ECBool,     "Shuttle", "bF05EnableShtPurgeFunction",            bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbF06,        &IniConfig.bF06InitialICCheck,                  ECBool,     "Shuttle", "bF06InitialICCheck",                    bShow, bEnable, bReadFromFile, 0);

//    if(ENABLE_OUT_SHUTTLEY_LATCH==false)                                        //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
//        elConfig->Add(rgF07,    &IniConfig.iF07OutShuttleSensorMode,            ECInteger,  "Shuttle", "iF07OutShuttleSensorMode",              bShow, bDisable, bFixedValue, 0);
//    else
        elConfig->Add(rgF07,    &IniConfig.iF07OutShuttleSensorMode,            ECInteger,  "Shuttle", "iF07OutShuttleSensorMode",              bShow, bEnable, bReadFromFile, LastSet.iOutShuttleSensorMode);
    elConfig->Add(cbF09,        &IniConfig.bF09CheckICWhichFirstTimeLoad,       ECBool,     "Shuttle", "bF09CheckICWhichFirstTimeLoad",         bShow, bEnable, bReadFromFile, 0);                  //Steven 20090907 [F09]
    if(ENABLE_OUT_SHUTTLE_SENEOR==false)
        elConfig->Add(cbF11,    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bNoShow, bDisable, bFixedValue, 0);                 //JerryYang 20190401 out shuttle前後對照，硬體選項沒有勾就不要顯示
    else if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)
        elConfig->Add(cbF11,    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bShow, bDisable, bFixedValue, 1);                   //jou 2013-04-09 KYEC 邱凱偉要求強制打開
    else
        elConfig->Add(cbF11,    &IniConfig.bF11OutShtUseFrontRearSensor,        ECBool,     "Shuttle", "bF11OutShtUseFrontRearSensor",          bShow, bEnable, bReadFromFile, 0);                  //jou 2010-09-09 [F11] Out Shuttle 使用前後對照Sensor檢查殘留IC

    if(IniConfig.bHaveRotateShuttle && CUSTOMER_CODE!=CC_ASE_KaohSiung)
    {
        labF12->Visible=true;
        elConfig->Add(cbF12,    &IniConfig.bRotateShNeedCheck,                  ECBool,     "Rotate Shuttle", "bRotateShNeedCheck",             bShow, bEnable, bReadFromFile, 0);                  //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭
        elConfig->Add(EdF12,    &IniConfig.dRoShCheckDelayTime,                 ECDouble,   "Rotate Shuttle", "dRoShCheckDelayTime",            bShow, bEnable, bReadFromFile,  0.01,  false, 1.0,     0.01);            //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭的延遲時間
    }
    else
    {
        labF12->Visible=false;
        elConfig->Add(cbF12,    &IniConfig.bRotateShNeedCheck,                  ECBool,     "Rotate Shuttle", "bRotateShNeedCheck",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(EdF12,    &IniConfig.dRoShCheckDelayTime,                 ECDouble,   "Rotate Shuttle", "dRoShCheckDelayTime",            bNoShow, bDisable, bFixedValue, 0.01,  false, 1.0,     0.01);
    }

    if(IniConfig.bHaveRotateShuttle)                                            //Steven 20110819--------
    {
        gbF13_RotateShtSpeed->Visible=(CUSTOMER_CODE!=CC_ASE_KaohSiung);
        elConfig->Add(edF13_ADC,&IniConfig.iRotateADC,                          ECInteger,  "Configuration", "Rotate ADC",                      bShow, bEnable, bReadFromFile,  10,    false, 1,       100);            //Steven 20101018 : 轉轉蝦頭的加減速
        elConfig->Add(edF13_Ini,&IniConfig.iInitSpeed,                          ECInteger,  "Configuration", "RotateInitSpeed",                 bShow, bEnable, bReadFromFile,  50,    false, 1,       100);            //kevin 20110531 旋轉SHUTTLE 鎖最高速度
        elConfig->Add(edF13_Jog,&IniConfig.iPJogHighSpeed,                      ECInteger,  "Configuration", "RotateJogHighSpeed",              bShow, bEnable, bReadFromFile,  1000,  false, 1,       5000);           //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    }
    else
    {
        gbF13_RotateShtSpeed->Visible=false;
        elConfig->Add(edF13_ADC,&IniConfig.iRotateADC,                          ECInteger,  "Configuration", "Rotate ADC",                      bNoShow, bDisable, bFixedValue, 10,    false, 1,       100);            //Steven 20101018 : 轉轉蝦頭的加減速
        elConfig->Add(edF13_Ini,&IniConfig.iInitSpeed,                          ECInteger,  "Configuration", "RotateInitSpeed",                 bNoShow, bDisable, bFixedValue, 50,    false, 1,       100);            //kevin 20110531 旋轉SHUTTLE 鎖最高速度
        elConfig->Add(edF13_Jog,&IniConfig.iPJogHighSpeed,                      ECInteger,  "Configuration", "RotateJogHighSpeed",              bNoShow, bDisable, bFixedValue, 1000,  false, 1,       5000);           //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    }

    elConfig->Add(cbF14,        &IniConfig.bF14KnockShuttle,                    ECBool,     "Shuttle", "bKnockShuttle",                         bShow, bEnable, bReadFromFile,  0);                                     //Steven 20120801 : Shuttle敲敲
    elConfig->Add(cbF14_1,      &IniConfig.bF14_1KnockShuttleFirst,             ECBool,     "Shuttle", "bKnockShuttleFirst",                    bShow, bEnable, bReadFromFile, 0);                                      //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    elConfig->Add(edF14,        &IniConfig.dF14KnockShuttleInterval,            ECDouble,   "Shuttle", "dKnockShuttleInterval",                 bShow, bEnable, bReadFromFile,  0.1,   false, 1.0,     0.01);           //Steven 20120801 : Shuttle敲敲
    elConfig->Add(edF14_No,     &IniConfig.iF14KnockShuttleNo,                  ECInteger,  "Shuttle", "iKnockShuttleNo",                       bShow, bEnable, bReadFromFile,  3,     false, 3,       100);            //wei 20121206
    elConfig->Add(edF14_1,      &IniConfig.dF14KnockShuttleIntervalFirst,       ECDouble,   "Shuttle", "dF14KnockShuttleIntervalFirst",         bShow, bEnable, bReadFromFile,  0.1,   false, 1.0,     0.01);           //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    elConfig->Add(edF14_1_No,   &IniConfig.iF14KnockShuttleNoFirst,             ECInteger,  "Shuttle", "iF14KnockShuttleNoFirst",               bShow, bEnable, bReadFromFile,  3,     false, 1,       100);            //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊

    elConfig->Add(cbF15,        &IniConfig.bF15OutShuttleLoseICNeedPWD,         ECBool,     "Shuttle", "OutShuttleLoseICNeedPWD",               bShow, bEnable, bReadFromFile, 0);                  //ChungHung 20120912 Amkor 需求Shuttle lose ic need password
    elConfig->Add(cbF16,        &IniConfig.bF16CheckShuttleSensorBroken,        ECBool,     "Shuttle", "CheckShuttleSensorBroken",              bShow, bEnable, bReadFromFile, 0);                  //2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線

    elConfig->Add(cbF17,        &IniConfig.bF17Sht1First,                       ECBool,     "Shuttle", "bF17Sht1First",                         bShow, bEnable, bReadFromFile, 0);                  //Steven 20160926 : One cycle後要先跑蝦頭一

    if(CosFunction.bF18F22InshuttleDetectSaveByRecipe)                          //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
        elConfig_byRecipe->Add(cbF18,    &IniConfig.bF18InshuttleDetect,        ECBool,     "Shuttle", "bInshuttleDetect",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic
    else if(IniConfig.bSPILFunction==true)
        elConfig->Add(cbF18,    &IniConfig.bF18InshuttleDetect,                 ECBool,     "Shuttle", "bInshuttleDetect",                      bNoShow, bDisable, bFixedValue, 0);                 //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic
    else
        elConfig->Add(cbF18,    &IniConfig.bF18InshuttleDetect,                 ECBool,     "Shuttle", "bInshuttleDetect",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic

    if(CosFunction.bLockF19AlwaysOn)                                            //Steven 20150709 : Out shuttle lose IC要做Piggyback
        elConfig->Add(cbF19,    &IniConfig.bF19OutShuttleLoseICNeedPiggyback,   ECBool,     "Shuttle", "bOutShuttleLoseICNeedPiggyback",        bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbF19,    &IniConfig.bF19OutShuttleLoseICNeedPiggyback,   ECBool,     "Shuttle", "bOutShuttleLoseICNeedPiggyback",        bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_PTI ||                                                 //Alick 20160815 add for 力成 Input Shuttle 檢測IC是否放到外圍，避免壓壞
       CUSTOMER_CODE==CC_SCC)                                                   //JerryYang 20210107 : Add SCC
    {
        cbF20Label2->Visible=true;
        elConfig->Add(cbF20,    &IniConfig.bF20InShuttleProminentDetect,        ECBool,     "Shuttle", "bF20InShuttleProminentDetect",          bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        cbF20Label2->Visible=false;
        elConfig->Add(cbF20,    &IniConfig.bF20InShuttleProminentDetect,        ECBool,     "Shuttle", "bF20InShuttleProminentDetect",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX)
        elConfig->Add(cbF21,    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bNoShow, bDisable, bFixedValue, 0);                 //kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor  //kevin 20180525 Ifor 20180228 (Steven) KYEC暫時關閉避免16 Site 放料異常(蝦頭2 Kit1 吸嘴上下跳動)
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //Eastsun 20260515 F022: D6 Z home sensor fix
        elConfig->Add(cbF21,    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bShow, bDisable, bFixedValue, 1);                 //kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor  //kevin 20180525 Ifor 20180228 (Steven) KYEC暫時關閉避免16 Site 放料異常(蝦頭2 Kit1 吸嘴上下跳動)
    else
        elConfig->Add(cbF21,    &IniConfig.bF21InOutArmZMotorPrivate,           ECBool,     "Shuttle", "bF21InOutArmZMotorPrivate",             bShow, bEnable, bReadFromFile, 0);

    if(CosFunction.bF18F22InshuttleDetectSaveByRecipe)                          //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
    {
        elConfig_byRecipe->Add(cbF22, &IniConfig.bF22InShuttleDetectOutNoIC,    ECBool,     "Shuttle",  "bF22InshuttleDetectOutNoIC",           bShow, bEnable, bReadFromFile, 0);                  //kevin 20161108 shuttle 出來撿測有無IC殘留
    }
    else
    {
        elConfig->Add(cbF22,    &IniConfig.bF22InShuttleDetectOutNoIC,          ECBool,     "Shuttle",  "bF22InshuttleDetectOutNoIC",           bShow, bEnable, bReadFromFile, 0);                  //kevin 20161108 shuttle 出來撿測有無IC殘留
    }

    if(CosFunction.bKnockerSetBySetupFile==false)                               //JerryYang 20200504 修正開啟程式時振動功能失效問題
    {
        if(ShuttleVibration_INSTALL)                                            //JerryYang 20171006 (wei) Shuttle 震動馬達
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : SPIL強制啟用shuttle震動馬達功能
            {
                elConfig->Add(cbF23,    &IniConfig.bF23ShuttleVibration,        ECBool,     "Shuttle", "bF23ShuttleVibration",                  bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(cbF23,    &IniConfig.bF23ShuttleVibration,        ECBool,     "Shuttle", "bF23ShuttleVibration",                  bShow, bEnable, bReadFromFile, 0);
            }

            if(CUSTOMER_CODE==CC_TERAPOWER ||                                   //Sam 20210520 : TPW 修改震動時間 Range
               CUSTOMER_CODE==CC_AMKOR_Korea)
                elConfig->Add(edF23,    &IniConfig.iF23ShuttleVibrationTime,    ECInteger,  "Shuttle", "iF23ShuttleVibrationTime",              bShow, bEnable, bReadFromFile,  20,   false, 1,      100);
            else
                elConfig->Add(edF23,    &IniConfig.iF23ShuttleVibrationTime,    ECInteger,  "Shuttle", "iF23ShuttleVibrationTime",              bShow, bEnable, bReadFromFile,  20,   false, 20,     100);
            elConfig->Add(edtF23_1,     &IniConfig.iF23ShuttleVibrationCount,   ECInteger,  "Shuttle", "iF23ShuttleVibrationCount",             bShow, bEnable, bReadFromFile,  1,    false, 1,      3);    //JerryYang 20191113 shuttle振動馬達可設定次數
        }
        else
        {
            elConfig->Add(cbF23,    &IniConfig.bF23ShuttleVibration,            ECBool,     "Shuttle", "bF23ShuttleVibration",                  bNoShow, bDisable, bFixedValue, 0);
            edF23->Visible=false;
            edtF23_1->Visible=false;
        }
    }
    else
    {
        cbF23->Visible=false;
        edF23->Visible=false;
        edtF23_1->Visible=false;
    }

    elConfig->Add(cbF24,        &IniConfig.bF24OutShuttleLoseIcOpenIndexDoor,   ECBool,     "Shuttle", "OutShuttleLoseIcOpenIndexDoor",         bShow, bEnable, bReadFromFile, 0);                              //kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door
    elConfig->Add(cbF25,        &IniConfig.bF25VibrateForOutShuttle,            ECBool,     "Shuttle", "bF25VibrateForOutShuttle",              bShow, bEnable, bReadFromFile, 0);                              //kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door
    elConfig->Add(edF25,        &IniConfig.iF25VibrateTime,                     ECInteger,  "Shuttle", "iF25VibrateTime",                       bShow, bEnable, bReadFromFile,  20,    false, 20,      100);    //JerryYang 20171205 (Steven) shuttle震動馬達功能
    elConfig->Add(rgF26,        &IniConfig.iF26OutShuttleJamSelectSkipOrRetry,  ECInteger,  "Shuttle", "iF25OutShuttleJamSelectSkipOrRetry",    bShow, bEnable, bReadFromFile,  0,    false, 2,      0);        //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry;

    if(CUSTOMER_CODE==CC_JCET)
        elConfig->Add(chkF27,   &IniConfig.bF27OutShtLoseICNeedToPick,          ECBool,     "Shuttle", "bF27OutShtLoseICNeedToPick",            bShow, bEnable, bReadFromFile, 0);                  //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
    else
        elConfig->Add(chkF27,   &IniConfig.bF27OutShtLoseICNeedToPick,          ECBool,     "Shuttle", "bF27OutShtLoseICNeedToPick",            bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbF28,        &IniConfig.bF28IndexCheckShuttlePos,            ECBool,     "Shuttle", "bF26IndexCheckShuttlePos",              bShow, bEnable, bReadFromFile, 0);                  //kevin 20220512 add  Index check shuttle pos for Sensor

    if(ShuttleVibration_INSTALL)                                                //Sam 20210602 : 每次都要強制震動
        elConfig->Add(cbF29,    &IniConfig.bF29AlwaysVibrateOnShuttle,          ECBool,     "Shuttle", "bF26AlwaysVibrateOnShuttle",            bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbF29,    &IniConfig.bF29AlwaysVibrateOnShuttle,          ECBool,     "Shuttle", "bF26AlwaysVibrateOnShuttle",            bNoShow, bDisable, bFixedValue, 0);

    if(MachineTypeChoice==Type_HT9046_LS && SubMachineType==Type_HT9016C)       //Jimmychiu 20230628 Add TempPosShift for HT9016C
        elConfig->Add(cbF30,    &IniConfig.bF30InShuttleSensorFollow16site,     ECBool,     "Shuttle", "bF26InShuttleSensorFollow16site",       bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(cbF30,    &IniConfig.bF30InShuttleSensorFollow16site,     ECBool,     "Shuttle", "bF26InShuttleSensorFollow16site",       bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20210426 : 2x4, 2x6 follow 2x8 shuttle sensor位置

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function ==>
    elConfig->Add(cbF31,    &IniConfig.bF31_CheckShtMoveCnt,    ECBool,         "F_Shuttle", "bF31_CheckShtMoveCnt",                  bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edF31_1,  &IniConfig.iShtMoveCntSet[0],       ECInteger,      "F_Shuttle", "F31_1_iShuttleMotorMoveCountSet",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(edF31_2,  &IniConfig.iShtMoveCntSet[1],       ECInteger,      "F_Shuttle", "F31_2_iShuttleMotorMoveCountSet",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(edF31_3,  &IniConfig.iShtMoveCntNow[0],       ECInteger,      "F_Shuttle", "F31_3_iShuttleMotorMoveCountNow",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(edF31_4,  &IniConfig.iShtMoveCntNow[1],       ECInteger,      "F_Shuttle", "F31_4_iShuttleMotorMoveCountNow",       bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(edF31_5,  &IniConfig.iShtMoveCntHis[0],       ECInteger,      "F_Shuttle", "F31_5_iShuttleMotorMoveCountHistroy",   bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    elConfig->Add(edF31_6,  &IniConfig.iShtMoveCntHis[1],       ECInteger,      "F_Shuttle", "F31_6_iShuttleMotorMoveCountHistroy",   bShow, bEnable, bReadFromFile,  20,    false, 0,      1000000);
    //Ztex 2023.04.19 Add HT-1032 TriTemp Function <==

    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
      (SHUTTLE_SENSOR_TYPE==eSensorCanBus || SHUTTLE_SENSOR_TYPE==eSensorCanBus))   //KenHsieh 20230829 : add Check In Shuttle Sensor By pass
    {
        elConfig->Add(cbF32,    &IniConfig.bF32CheckInSHSenBypass,              ECBool,     "Shuttle",  "bF30CheckInSHSenBypass",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edF32,    &IniConfig.iF32InSHSenBypassValue,              ECInteger,  "Shuttle",  "iF30InSHSenBypassValue",               bShow, bEnable, bReadFromFile,  10,    false, 10,      4095);
    }
    else
    {
        elConfig->Add(cbF32,    &IniConfig.bF32CheckInSHSenBypass,              ECBool,     "Shuttle",  "bF30CheckInSHSenBypass",               bNoShow, bDisable, bFixedValue, 0);
        edF32->Visible=false;
    }

    if(IniConfig.bSPILFunction==true &&
       (BAR_CODE_INSTALL==ebctUseCCDMode ||                                     //JerryYang 20250220 : 2DID硬體順序檢查功能
       BAR_CODE_INSTALL==ebctInShtIntel ||
       BAR_CODE_INSTALL==ebctEtherNetCCD))
    {
        elConfig->Add(cbF33_Check2DHardware,    &IniConfig.bF33_Check2DHardware,ECBool,     "Shuttle",  "bF33_Check2DHardware",                 bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbF33_Check2DHardware,    &IniConfig.bF33_Check2DHardware,ECBool,     "Shuttle",  "bF33_Check2DHardware",                 bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bMaximFunction || CUSTOMER_CODE==CC_AnalogDevice_Phil)
    {
        elConfig->Add(cbF34,        &IniConfig.bF34OutShtPickErrSetErrBin,           ECBool,     "Shuttle",  "bF34OutShtPickErrSetErrBin",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbF35,        &IniConfig.bF35OutShtLoseNeedSetErrBin,          ECBool,     "Shuttle",  "bF35OutShtLoseNeedSetErrBin",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbF34,        &IniConfig.bF34OutShtPickErrSetErrBin,           ECBool,     "Shuttle",  "bF34OutShtPickErrSetErrBin",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbF35,        &IniConfig.bF35OutShtLoseNeedSetErrBin,          ECBool,     "Shuttle",  "bF35OutShtLoseNeedSetErrBin",     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(cbF36,   &IniConfig.bF36OutShtLoseICResetSetAllToErr,          ECBool,     "Shuttle",     "bF36OutShtLoseICResetSetAllToErr",    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbF36,   &IniConfig.bF36OutShtLoseICResetSetAllToErr,          ECBool,      "Shuttle",     "bF36OutShtLoseICResetSetAllToErr",    bNoShow, bDisable, bFixedValue, 0);
    }
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:2330-2419, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemG()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(cbG01,        &IniConfig.bG01Show_TestRate,                   ECBool,     "Visible", "bG01Show_TestRate",                     bShow, bEnable, bReadFromFile, LastSet.bShow_TestRate?"1":"0"); //Steven 20090715
    elConfig->Add(cbG04,        &IniConfig.bG04ShowFailAlarmCount,              ECBool,     "Visible", "bG04ShowFailAlarmCount",                bShow, bEnable, bReadFromFile, LastSet.bShowFailAlarmCount?"1":"0");            //jou 980717 add from ht9040 show alarm enabled
    elConfig->Add(cbG05,        &IniConfig.bG05ShowSpeedMessage,                ECBool,     "Visible", "bG05ShowSpeedMessage",                  bShow, bEnable, bReadFromFile, LastSet.bShowSpeedMessage?"1":"0");              //Steven 20090720 show speed at uShowMessage
    elConfig->Add(cbG06,        &IniConfig.bG06HomeinitialCheckZ1,              ECBool,     "Visible", "bHomeinitialCheckZ1",                   bShow, bEnable, bReadFromFile, 0);                  //kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認
    elConfig->Add(cbG07,        &IniConfig.bG07MultiColorForFailBin,            ECBool,     "Visible", "bSupportMultiColorForFailBin",          bShow, bEnable, bReadFromFile, 0);                  //Steven 20160310 : 改成有顏色的fail bin

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160413 顯示Are you sure
        elConfig->Add(cbG08,    &IniConfig.bG08VisibleAreyousure,               ECBool,     "Visible", "bVisibleAreyousure",                    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbG08,    &IniConfig.bG08VisibleAreyousure,               ECBool,     "Visible", "bVisibleAreyousure",                    bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bG09NeedPasswordWhenEditSiteMap)                             //JerryYang 20160425 修改Site map需要密碼
        elConfig->Add(cbG09,    &IniConfig.bG09NeedPasswordWhenEditSiteMap,     ECBool,     "Function", "bNeedPasswordWhenEditSiteMap",         bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20160425 修改Site map需要密碼
    else
        elConfig->Add(cbG09,    &IniConfig.bG09NeedPasswordWhenEditSiteMap,     ECBool,     "Function", "bNeedPasswordWhenEditSiteMap",         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_CL)        //JerryYang 20250120 : add
    {
        elConfig->Add(cbG10,        &IniConfig.bG10ShowImmediateUPH,                ECBool,     "Visible", "bG10ShowImmediateUPH",                  bShow, bDisable, bFixedValue, 1);                  //Steven 20160727 : Show immediate UPH
    }
    else
    {
        elConfig->Add(cbG10,        &IniConfig.bG10ShowImmediateUPH,                ECBool,     "Visible", "bG10ShowImmediateUPH",                  bShow, bEnable, bReadFromFile, 0);                  //Steven 20160727 : Show immediate UPH
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(cbG11,    &IniConfig.bG11ASEReport,                       ECBool,     "Visible", "bG11ASEReport",                         bShow, bEnable, bReadFromFile, 0);                  //kevin 20170306
    else
        elConfig->Add(cbG11,    &IniConfig.bG11ASEReport,                       ECBool,     "Visible", "bG11ASEReport",                         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_OSE || CUSTOMER_CODE==CC_ASE_KaohSiung)                //kevin 20180221 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
        elConfig->Add(cbG12,    &IniConfig.bG12ContractModeManualMessage,       ECBool,     "Visible", "bG12ContractModeManualMessage",         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbG12,    &IniConfig.bG12ContractModeManualMessage,       ECBool,     "Visible", "bG12ContractModeManualMessage",         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbG13,        &IniConfig.bG13ShowTempOffsetOnContact,         ECBool,     "Visible", "bG13ShowTempOffsetOnContact",           bShow, bEnable, bReadFromFile, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //Steven 20231018 : Fixed for G14
    {
        elConfig->Add(cbG14,    &IniConfig.bG14UseStartSoundAlarm,              ECBool,     "Visible", "bG14UseStartSoundAlarm",                bShow, bEnable, bReadFromFile, 0);                  //kevin 20201116  Start 發出聲音 不動 5sec
        elConfig->Add(edG14,    &IniConfig.iG14StartWarrTime,                   ECInteger,  "Visible", "iStartWarrTime",                        bShow, bEnable, bReadFromFile,  3,     false, 1,       10);     //kevin 20201124 run Start alarm
    }
    else
    {
        elConfig->Add(cbG14,    &IniConfig.bG14UseStartSoundAlarm,              ECBool,     "Visible", "bG14UseStartSoundAlarm",                bNoShow, bDisable, bFixedValue, 0);                 //kevin 20201116  Start 發出聲音 不動 5sec
        elConfig->Add(edG14,    &IniConfig.iG14StartWarrTime,                   ECInteger,  "Visible", "iStartWarrTime",                        bNoShow, bDisable, bFixedValue, 3);                 //kevin 20201124 run Start alarm
    }

    elConfig->Add(cbG15,        &IniConfig.bG15LoadInputCount,                  ECBool,     "Visible", "bG15LoadInputCount",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20211106 輸入顆數達成就 Clean out        //JimmyChiu 20220124 fix name
    elConfig->Add(chkG16,       &IniConfig.bG16BinDispNeedAlarm,                ECBool,     "Visible", "bG16BinDispNeedAlarm",                  bShow, bEnable, bReadFromFile, 0);                  //Steven 20211130 : JSCC要求Bin顯示器異常要alarm

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(cbG17,    &IniConfig.b17bUseLoadCCDTrayMap,               ECBool,     "Visible", "b17bUseLoadCCDTrayMap",                 bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610  Load 到定位通知eKeep 拍照，ekeep 自己按Start
        elConfig->Add(cbG17_1,  &IniConfig.b17bUseLoadCCDTrayMapTrayend,        ECBool,     "Visible", "b17bUseLoadCCDTrayMapTrayend",          bShow, bEnable, bReadFromFile, 0);                  //kevin 20230530 trayend 拍照
    }
    else
    {
        elConfig->Add(cbG17,    &IniConfig.b17bUseLoadCCDTrayMap,               ECBool,     "Visible", "b17bUseLoadCCDTrayMap",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbG17_1,  &IniConfig.b17bUseLoadCCDTrayMapTrayend,        ECBool,     "Visible", "b17bUseLoadCCDTrayMapTrayend",          bNoShow, bDisable, bFixedValue, 0);
    }
    elConfig->Add(cbG18_1,      &IniConfig.b18bUseAutoTrayMap,                  ECBool,     "Visible", "b18bUseAutoTrayMap",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610 add Auto 123 Tray Map
    elConfig->Add(cbG18_2,      &IniConfig.b18bDoubleUnloadTray,                ECBool,     "Visible", "b18bDoubleUnloadTray",                  bShow, bEnable, bReadFromFile, 0);                  //kevin 20220610 add Unload Tray 2  倍 的設定 放空盤
    elConfig->Add(edG18Auto1Count, &IniConfig.iUnloaderTrayCount[0],            ECInteger,  "Visible", "iAuto1Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);
    elConfig->Add(edG18Auto2Count, &IniConfig.iUnloaderTrayCount[1],            ECInteger,  "Visible", "iAuto2Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);
    elConfig->Add(edG18Auto3Count, &IniConfig.iUnloaderTrayCount[2],            ECInteger,  "Visible", "iAuto3Count",                           bShow, bEnable, bReadFromFile,  3,     false, 2,       40);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(cbG19,    &IniConfig.b19InSHAutoCleanPA,                  ECBool,     "Visible", "b19InSHAutoCleanPA",                    bShow, bEnable, bReadFromFile, 0);                  //kevin 20230530 add Autoclean shuttle define new 參數
        elConfig->Add(cbG20,    &IniConfig.b20FixfullWaitpos,                   ECBool,     "Visible", "b20FixfullWaitpos",                     bShow, bEnable, bReadFromFile, 0);                  //kevin 20230816 out arm move Wait
        elConfig->Add(cbG21,    &IniConfig.b21colorfulltray,                    ECBool,     "Visible", "b21colorfulltray",                      bShow, bEnable, bReadFromFile, 0);                  //kevin 20230918 colortray no Alarm
    }
    else
    {
        elConfig->Add(cbG19,    &IniConfig.b19InSHAutoCleanPA,                  ECBool,     "Visible", "b19InSHAutoCleanPA",                    bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230530 add Autoclean shutt
        elConfig->Add(cbG20,    &IniConfig.b20FixfullWaitpos,                   ECBool,     "Visible", "b20FixfullWaitpos",                     bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230816 out arm move Wait
        elConfig->Add(cbG21,    &IniConfig.b21colorfulltray,                    ECBool,     "Visible", "b21colorfulltray",                      bNoShow, bDisable, bFixedValue, 0);                 //kevin 20230918 colortray no Alarm
    }
    elConfig->Add(cbG22,        &IniConfig.bG22NoticeTakeoutTray,               ECBool,     "Visible", "bG22NoticeTakeoutTray",                 bShow, bEnable, bReadFromFile, 0);                  //JerryYang 20231218 : G22提醒人員取tray功能
    elConfig->Add(cbG23,        &IniConfig.bG23DiasbleFuncStatusView,           ECBool,     "Visible", "bG23DiasbleFuncStatusView",             bShow, bEnable, bReadFromFile, IniConfig.bVTESTFunction==true?"1":"0");    //RogerYang 20250728 Vtest設開關決定要不要顯示View
    elConfig->Add(cbG24,        &IniConfig.bG24DisableSECSGEMStatus,            ECBool,     "Visible", "bG24DisableSECSGEMStatus",              bShow, bEnable, bReadFromFile, 0);                  //RogerYang 20251222 : 丁曉東要求新增選項(G24)是否顯示
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:2873-3118, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemL()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    elConfig->Add(cbL03,        &IniConfig.bL03SocketAirCoolingCT,              ECBool,     "Tempture", "bL03SocketAirCoolingCT",           bShow, bEnable, bReadFromFile, 0);      //jou 2016-04-28 Socket Air Cooling contact count trun on
    elConfig->Add(edL03,        &IniConfig.iL03SocketAirCoolingCT,              ECInteger,  "Tempture", "iL03SocketAirCoolingCT",           bShow, bEnable, bReadFromFile,  2,     false, 0,       50);     //jou 2016-04-28 Socket Air Cooling contact count trun on

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(edL04,    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  3);     //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CosFunction.bHiSiliconFunction==true)                               //Ifor 20160406 海思 ATC 相關參數寫死
        elConfig->Add(edL04,    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bDisable, bReadFromFile, 2,     false, 1,       10);
    else if(CUSTOMER_CODE==CC_ASE_CL)                                           //JerryYang 20200127 : 松諭要求把溫度range下限改為正負1度
        elConfig->Add(edL04,    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  2,     false, 1,       10);
    else if(IniConfig.bVTESTFunction==true)
        elConfig_byRecipe->Add(edL04,       &IniConfig.iL04TemptureRange,       ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  3,     false, 2,       10);
    else
        elConfig->Add(edL04,    &IniConfig.iL04TemptureRange,                   ECInteger,  "Tempture", "iL04TemptureRange",                bShow, bEnable, bReadFromFile,  2,     false, 2,       10);

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(edL05,    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bEnable, bReadFromFile,   3); //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CosFunction.bHiSiliconFunction==true)
        elConfig->Add(edL05,    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bDisable, bReadFromFile, 2,     false, 2,       30);
    else
        elConfig->Add(edL05,    &IniConfig.iL05ChamberTemptureRange,            ECInteger,  "Tempture", "iL05ChamberTemptureRange",         bShow, bEnable,  bReadFromFile, 2,     false, 2,       30);

    if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_KYEC_LEE)
        elConfig->Add(edtL06,   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bEnable, bReadFromFile,   3); //Ifor 20160311 客戶要求L04~L06強制寫入3    //Ifor 20220419 add:連軍要求不卡控 L04~L06
    else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                   //Steven 20230410 : 南茂鍾永生要求修改
        elConfig->Add(edtL06,   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bDisable, bReadFromFile, 2,     false, 0,       5);
    else if(CosFunction.bHiSiliconFunction==true)
        elConfig->Add(edtL06,   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bDisable, bReadFromFile, 2,     false, 0,       10);
    else
        elConfig->Add(edtL06,   &IniConfig.iAmbTemperatureRange,                ECInteger,  "Temperature", "iAmbTemperatureRange",          bShow, bEnable,  bReadFromFile, 2,     false, 0,       10);         //jou 2013-04-11 Ambient Temperture Range

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        elConfig->Add(cbL07,    &IniConfig.bL07UseSingleTenmpertureLimit,       ECBool,     "Tempture", "bL07UseSingleTenmpertureLimit",    bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbL07,    &IniConfig.bL07UseSingleTenmpertureLimit,       ECBool,     "Tempture", "bL07UseSingleTenmpertureLimit",    bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(edL08_Over,   &IniConfig.iSocketTemptureRangeOver,            ECInteger,  "Tempture", "iSocketTemptureRange",             bShow, bEnable,  bReadFromFile, 3,     false, 2,       30);         //Steven 20140308 : DUT溫度限制改成上下限分開
    elConfig->Add(edL08_Under,  &IniConfig.iSocketTemptureRangeUnder,           ECInteger,  "Tempture", "iSocketTemptureRangeUnder",        bShow, bEnable,  bReadFromFile, 3,     false, 2,       30);         //Steven 20140308 : DUT溫度限制改成上下限分開

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(cbL09,    &IniConfig.bL09HotTempShuttleNoAddPos,          ECBool,     "Tempture", "bL23HotTempShuttlenoAddPos",       bShow, bEnable, bFixedValue, 1);        //kevin 20200812 : ASEKH 高溫shuttle不補位置
    else
        elConfig->Add(cbL09,    &IniConfig.bL09HotTempShuttleNoAddPos,          ECBool,     "Tempture", "bL23HotTempShuttlenoAddPos",       bShow, bEnable, bReadFromFile, 0);      //kevin 20200718 : ASEKH 高溫shuttle不補位置

    if(CUSTOMER_CODE==CC_ASE_KaohSiung && CosFunction.bHiSiliconFunction==true)
        elConfig->Add(cbL10,    &IniConfig.bL10IndexTestlogTemp,                ECBool,     "Tempture", "bL10IndexTestlogTemp",             bShow, bEnable, bFixedValue, 1);        //kevin 20200227 add 下壓溫度記 錄
    else
        elConfig->Add(cbL10,    &IniConfig.bL10IndexTestlogTemp,                ECBool,     "Tempture", "bL10IndexTestlogTemp",             bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(rgL10,  &IniConfig.iL10TempRecordInterval,                    ECInteger,  "Index", "iL10TempRecordInterval",                bShow, bEnable,  bReadFromFile, LastSet.iTempRecordInterval);   //Steven 20100225 //Steven 20231017 : fixed

    if(CosFunction.bHiSiliconFunction)                                          //Ifor 20160406 海思 ATC 相關參數寫死
    {
        elConfig->Add(edL11_1,  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bDisable, bFixedValue,   3);                                 //Ifor 20160406 L11-1 強制設定3度
    }
    else
    {
        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(edL11_1,  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bEnable,  bReadFromFile, 3,     false, 1,       3);          //jou 20240426 : VTEST 要求 ATC 溫度限制設定1~3度
        else
            elConfig->Add(edL11_1,  &IniConfig.iATCTemperatureRange,                ECInteger,  "Index", "iATCTemperatureRange",                bShow, bEnable,  bReadFromFile, 10,     false, 1,       30);        //Eliot 2015_0105 2~50度
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bHiSiliconFunction==true)      //Ifor 20160310  取消水冷機自動關閉功能
        elConfig->Add(cbL11_2,  &IniConfig.bL11_2ATCChillerProtectedFunction,   ECBool,     "Index", "bATCChillerProtectedFunction",        bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbL11_2,  &IniConfig.bL11_2ATCChillerProtectedFunction,   ECBool,     "Index", "bATCChillerProtectedFunction",        bShow, bEnable, bReadFromFile, 0);      // 2012.05.07  Joye  Chiller

    elConfig->Add(edL11_1_2,    &IniConfig.dATCTemperatureCheckTime,            ECDouble,   "Index", "dATCTemperatureCheckTime",            bShow, bEnable,  bReadFromFile, 10.0,   false, 0.01,    180.0);     //Steven 20121222 : ATC Temperature Over Check Time
    elConfig->Add(edL11_2,      &IniConfig.iATCChillerCheckTime,                ECInteger,  "Index", "iATCChillerCheckTime",                bShow, bEnable,  bReadFromFile, 5,      false, 5,       300);
    elConfig->Add(edL11_4,      &IniConfig.iATCTemperatureOverLimit,            ECInteger,  "Index", "iATCTemperatureOverLimit",            bShow, bEnable,  bReadFromFile, 130,    false, 2,       130);       //Steven 20140916 : [L11-4] ATC的最高上限溫度

    if(ATC_SYSTEM==eATCUninstall || ATC_SYSTEM==eNonChamber)
    {
        elConfig->Add(cbL11_1,  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbL11_6,  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbL11_7,  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbL11_8,  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bNoShow, bDisable, bFixedValue, 0);
    }
    else if(CosFunction.bHiSiliconFunction==true)
    {
        elConfig->Add(cbL11_1,  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bDisable, bFixedValue, 1);
        elConfig->Add(cbL11_6,  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bShow, bDisable, bFixedValue, 1);
        elConfig->Add(edL11_6_Outside,    &IniConfig.iATCTemperatureOutside,    ECInteger,  "Tempture", "iATCTemperatureOutside",           bShow, bDisable, bFixedValue, 3);   //Ifor 20160406 海思版本ATC功能[L11-6]強制設定 3度
        elConfig->Add(edL11_6_Continuous, &IniConfig.iATCTemperatureContinuous, ECInteger,  "Tempture", "iATCTemperatureContinuous",        bShow, bDisable, bFixedValue, 3);   //Ifor 20160406 海思版本ATC功能[L11-6]強制設定 3秒
        elConfig->Add(cbL11_7,  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bShow, bDisable, bFixedValue, 1);
        elConfig->Add(edL11_7_MaxSurge,   &IniConfig.iATCMaxSurgeAlarm,         ECInteger,  "Tempture", "iATCMaxSurgeAlarm",                bShow, bDisable, bFixedValue, 7);   //Ifor 20160406 海思版本ATC功能[L11-7]強制設定 7度
        elConfig->Add(cbL11_8,  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(cbL11_1,  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bDisable, bFixedValue, 1);      // 2012.05.11  Joye  ATC Temperature Over Check
        else
            elConfig->Add(cbL11_1,  &IniConfig.bL11_1ATCTemperatureOverAlarm,       ECBool,     "Index", "bATCTemperatureOverAlarm",            bShow, bEnable, bReadFromFile, 0);      // 2012.05.11  Joye  ATC Temperature Over Check
        elConfig->Add(cbL11_6,  &IniConfig.bL11_6ATCUseTemperatureOutsideAlarm, ECBool,     "Tempture", "bATCUseTemperatureOutsideAlarm",   bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-6] ATC突波於設定範圍內且持續發生超過設定時間後發出Alarm
        elConfig->Add(edL11_6_Outside,    &IniConfig.iATCTemperatureOutside,    ECInteger,  "Tempture", "iATCTemperatureOutside",           bShow, bEnable,  bReadFromFile, 2,      false, 1,       30);    //Ifor 20150911 : [L11-6] ATC 突波超出設定溫度
        elConfig->Add(edL11_6_Continuous, &IniConfig.iATCTemperatureContinuous, ECInteger,  "Tempture", "iATCTemperatureContinuous",        bShow, bEnable,  bReadFromFile, 3,      false, 1,       30);    //Ifor 20150911 : [L11-6] ATC 允許突波持續發生時間(s)
        elConfig->Add(cbL11_7,  &IniConfig.bL11_7ATCUseMaxSurgeAlarm,           ECBool,     "Tempture", "bATCUseMaxSurgeAlarm",             bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-7] ATC突波超出最大設定值發出Alarmed
        elConfig->Add(edL11_7_MaxSurge,   &IniConfig.iATCMaxSurgeAlarm,         ECInteger,  "Tempture", "iATCMaxSurgeAlarm",                bShow, bEnable,  bReadFromFile, 7,      false, 1,       30);    //Ifor 20150911 : [L11-7] ATC 最高突波上限
        elConfig->Add(cbL11_8,  &IniConfig.bL11_8ATCUseTemperatureCompare,      ECBool,     "Tempture", "bATCUseTemperatureCompare",        bShow, bEnable, bReadFromFile, 0);      //Ifor 20150911 : [L11-8] ATC第1組sensor跟第2組sensor溫度差值超過設定值發出警報
    }

    elConfig->Add(cbL12,        &IniConfig.bL12TempErrNoCloseHeater,            ECBool,     "Tempture", "TempErrNoCloseHeater",                     bShow, bEnable, bReadFromFile, 0);                      //ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱
    elConfig->Add(cbL13,     &IniConfig.bL13HotPlateAndShuttleUseOneTempOffset, ECBool,     "Tempture", "bHotPlateAndShuttleUseOneTempOffset",      bShow, bEnable, bReadFromFile, 0);        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    if(CosFunction.bChamberModeEvenBlowNeedWaitTime)                            //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
        elConfig->Add(cbL15, &IniConfig.bL15EnableChamberModeEvenBlowNeedWaitTime, ECBool,  "Tempture", "bEnableChamberModeEvenBlowNeedWaitTime",   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbL15, &IniConfig.bL15EnableChamberModeEvenBlowNeedWaitTime, ECBool,  "Tempture", "bEnableChamberModeEvenBlowNeedWaitTime",   bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbL17,        &IniConfig.bL17HeadHeaterOnWhenCloseSite,       ECBool,     "Tempture", "bHeadHeaterOnWhenCloseSite",       bShow, bEnable, bReadFromFile, 0);      //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20160107 No FullSite Add Offset
        elConfig->Add(cbL18,    &IniConfig.bL18NofullsiteaddTemperatureoffset,  ECBool,     "Tempture", "NofullsiteaddTemperatureoffset",   bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbL18,    &IniConfig.bL18NofullsiteaddTemperatureoffset,  ECBool,     "Tempture", "NofullsiteaddTemperatureoffset",   bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_JCET)
        elConfig->Add(cbL19,    &IniConfig.bL19OpenHeatDoorgiveupchamberhot,    ECBool,     "Tempture", "bL19OpenHeatDoorgiveupchamberhot", bShow, bEnable, bReadFromFile, 0);      //kevin 20170520 (wei) 開chamber門只有不加熱chamber
    else
        elConfig->Add(cbL19,    &IniConfig.bL19OpenHeatDoorgiveupchamberhot,    ECBool,     "Tempture", "bL19OpenHeatDoorgiveupchamberhot", bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbL20,        &IniConfig.bL20AbientGuardBand,                 ECBool,     "Tempture", "bL20AbientGuardBand",              bShow, bEnable, bReadFromFile, 0);      //kevin 20180115 (Steven) add Amient Guard Band
    elConfig->Add(cbL21,        &IniConfig.bL21PowerOffTemperature,             ECBool,     "Tempture", "bL21PowerOffTemperature",          bShow, bEnable, bReadFromFile, 0);      //kevin 20181112 (Steven) : power off 開 Chamber door 斷所有加熱電

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(cbL22,    &IniConfig.bL22Enable3SigmaTempMonitor,         ECBool,     "Tempture", "bL22Enable3SigmaTempMonitor",      bShow, bEnable, bReadFromFile, 0);      //kevin 20200521 : ASEKH 3SIGMA  溫度統計
    else
        elConfig->Add(cbL22,    &IniConfig.bL22Enable3SigmaTempMonitor,         ECBool,     "Tempture", "bL22Enable3SigmaTempMonitor",      bNoShow, bDisable, bFixedValue, 0);      //kevin 20200521 : ASEKH 3SIGMA  溫度統計

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20210122 : ASE-CL新增待溫功能
    {
        elConfig->Add(cbL24,    &IniConfig.bL24HeaterStableTime,                ECBool,     "Tempture", "bL24HeaterStableTime",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edL24,    &IniConfig.iL24HeaterStableTime,                ECInteger,  "Index", "iL24HeaterStableTime",                bShow, bEnable, bReadFromFile,  30, false, 10, 1800);
    }
    else
    {
        elConfig->Add(cbL24,    &IniConfig.bL24HeaterStableTime,                ECBool,     "Tempture", "bL24HeaterStableTime",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edL24,    &IniConfig.iL24HeaterStableTime,                ECInteger,  "Index", "iL24HeaterStableTime",                bNoShow, bDisable, bFixedValue,  0);
    }

    if(CosFunction.bHiSiliconFunction==true ||
       TestIF_File.i2DIDFormat==eAMD ||
       CosFunction.bATCUseHandlerRecipeName==true)
    {
        elConfig->Add(cbL25,    &IniConfig.bL25_1ATCFileNameWithTemp,           ECBool,     "Tempture", "bL25_1ATCFileNameWithTemp",        bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbL25,    &IniConfig.bL25_1ATCFileNameWithTemp,           ECBool,     "Tempture", "bL25_1ATCFileNameWithTemp",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                               //Sam 20231214 : Temp offset use ready temp range
        elConfig->Add(cbL28,    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bShow, bDisable, bFixedValue, 1);
    else if(CUSTOMER_CODE==CC_ASE_SG)                                           //Ifor 20251105 add: ASE SG 客戶要求[L28]強制關閉不顯示
        elConfig->Add(cbL28,    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bNoShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbL28,    &IniConfig.bL28TempOfsUseReadyTempRange,        ECBool,     "Tempture", "bL28TempOfsUseReadyTempRange",     bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(cbL29,        &IniConfig.bL29AmbientNotShowTemp,              ECBool,     "Tempture", "bL29AmbientNotShowTemp",           bShow, bEnable, bReadFromFile, 0);      //Sam 20221101 : 常溫模式不顯示溫度

    if(CosFunction.bUse1CableLayoutKitByConfig)                                 //Sam 20210715 : Use 1CableLayoutKit By Config
    {
        gbL30->Visible=true;
        elConfig->Add(cbL30,    &IniConfig.bL30Use1CableLayoutKitByConfig,      ECBool,     "Tempture", "bL30Use1CableLayoutKitByConfig",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbL30_1,  &IniConfig.bL30Use1CableLayoutKit,              ECBool,     "Tempture", "bL30Use1CableLayoutKit",           bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        gbL30->Visible=false;
        elConfig->Add(cbL30,    &IniConfig.bL30Use1CableLayoutKitByConfig,      ECBool,     "Tempture", "bL30Use1CableLayoutKitByConfig",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbL30_1,  &IniConfig.bL30Use1CableLayoutKit,              ECBool,     "Tempture", "bL30Use1CableLayoutKit",           bNoShow, bDisable, bFixedValue, 0);
    }

    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    if(Tri_Temp_Machine==1)
    {
        elConfig->Add(edtL09_1,   &IniConfig.iL09_1HightTemp_Sht_Shift,         ECInteger,  "Tempture", "iL09_1HightTemp_Sht_Shift",            bShow, bEnable, bReadFromFile,  15,  false, -100, 100.0);//Ztex 2024.02.21 Add Change Temp When Shuttle Auto Shift Pos
        elConfig->Add(edtL09_2,   &IniConfig.iL09_2LowTemp_Sht_Shift,           ECInteger,  "Tempture", "iL09_2LowTemp_Sht_Shift",              bShow, bEnable, bReadFromFile,  -40, false, -100, 100.0);//Ztex 2024.02.21 Add Change Temp When Shuttle Auto Shift Pos

        elConfig->Add(cbL32_1,    &IniConfig.bL32_1ManuDefrost,                 ECBool,     "Tempture", "bL32_1ManualDefrost",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbL32_2,    &IniConfig.bL32_2AutoDefrostFunction,         ECBool,     "Tempture", "bL32_2AutoDefrostFunction",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edL32_4,    &IniConfig.iL32_4SetDefrostTemp,              ECInteger,  "Tempture", "iL32_4SetDefrostTempature",            bShow, bEnable, bReadFromFile,  10, false, 80, 100.0);
        elConfig->Add(edL32_5,    &IniConfig.iL32_5SetDefrostTime,              ECInteger,  "Tempture", "iL32_5SetDefrostTime",                 bShow, bEnable, bReadFromFile,  10, false, 10, 60.0);
        elConfig->Add(edL32_6,    &IniConfig.iL32_6LowTempRunAlarmDegree,       ECInteger,  "Tempture", "iL32_6LowTempRunAlarmDegree",          bShow, bEnable, bReadFromFile,  30, false, 10, 40);
        elConfig->Add(edL32_7_1,  &IniConfig.iL32_7LowTempRunAlarmHour,         ECInteger,  "Tempture", "iL32_7LowTempRunAlarmHour",            bShow, bEnable, bReadFromFile,  2, false, 2, 72);
        elConfig->Add(edL32_7_2,  &IniConfig.iL32_7LowTempRunAlarmMin,          ECInteger,  "Tempture", "iL32_7LowTempRunAlarmMin",             bShow, bEnable, bReadFromFile,  30, false, 0, 59);
        elConfig->Add(edL32_8,    &IniConfig.iL32_8SetAirStreamTemp,            ECInteger,  "Tempture", "iL32_8SetAirStreamTemp",               bShow, bEnable, bReadFromFile,  40, false, 40, 180);

        elConfig->Add(cbL33_1,    &IniConfig.bL33_1CheckDoorOpenForTriTemp,     ECBool,     "Tempture", "bL33_1CheckDoorOpenForTriTemp",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbL33_2,    &IniConfig.bL33_2DoorOpenRunDefrost,          ECBool,     "Tempture", "bL33_2DoorOpenRunDefrost",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edL33_3,    &IniConfig.iL33_3BDoorOpenTimeForLowTemp,     ECInteger,  "Tempture", "iL33_3BDoorOpenTimeForLowTemp",        bShow, bEnable, bReadFromFile,  10, false, 10, 200.0);
        elConfig->Add(edL33_4,    &IniConfig.iL33_4SDoorOpenTimeForLowTemp,     ECInteger,  "Tempture", "iL33_4SDoorOpenTimeForLowTemp",        bShow, bEnable, bReadFromFile,  10, false, 10, 200.0);
        elConfig->Add(edL33_5,    &IniConfig.iL33_5DoorOpenTempForLowTemp,      ECInteger,  "Tempture", "iL33_5DoorOpenTempForLowTemp",         bShow, bEnable, bReadFromFile,  10, false, -55, 40.0);
        elConfig->Add(edL33_6,    &IniConfig.iL33_6DoorOpenTempForHotTemp,      ECInteger,  "Tempture", "iL33_6DoorOpenTempForHotTemp",         bShow, bEnable, bReadFromFile,  10, false, 40, 175.0);

        elConfig->Add(cbL34_1,    &IniConfig.bL34_1DelayOfFixDoorOpen,          ECBool,     "Tempture", "bL34_1DelayOfFixDoorOpen",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edL34_2,    &IniConfig.iL34_2DelaySecOfFixDoorOpen,       ECInteger,  "Tempture", "iL34_2DelaySecOfFixDoorOpen",          bShow, bEnable, bReadFromFile,  10, false, 10, 100.0);
        elConfig->Add(edL34_3,    &IniConfig.dL34_3DewPointOfFixDoorOpen,       ECDouble,   "Tempture", "dL34_3DewPointOfFixDoorOpen",          bShow, bEnable,  bReadFromFile, 10.0,   false, 0.00,    -60.0);     //Steven 20121222 : ATC Temperature Over Check Time
        elConfig->Add(edL34_4,    &IniConfig.iL34_4OpenAuto3TrackGateSec,       ECInteger,  "Tempture", "iL34_4OpenAuto3TrackGateSec",          bShow, bEnable, bReadFromFile,  30, false, 10, 1800);
        elConfig->Add(cbL34_5,    &IniConfig.bL34_5FixTrayDoorCynAutoOpen,      ECBool,     "Tempture", "bL34_5FixTrayDoorCynAutoOpen",         bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(cbL35_1,    &IniConfig.bL35_1OverSetTempOpenFan,          ECBool,     "Tempture", "bL35_1OverSetTempOpenFan",             bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edL35_2,    &IniConfig.iL35_2OpenFanTemp,                 ECInteger,  "Tempture", "iL35_2OpenFanTemp",                    bShow, bEnable, bReadFromFile,  30, false, 40, 175);

        elConfig->Add(edtL36_1,   &IniConfig.iL36_1Tri_Temp_Rang_ATC,           ECInteger,  "Tempture", "iL36_1Tri_Temp_Rang_ATC",                            bShow, bEnable, bReadFromFile, 3, false, 1,    10.0);
        elConfig->Add(edtL36_2,   &IniConfig.iL36_2Tri_Temp_Rang_Heater,        ECInteger,  "Tempture", "iL36_2Tri_Temp_Rang_Heater",                         bShow, bEnable, bReadFromFile, 3, false, 1,    20.0);

        elConfig->Add(cbL37,      &IniConfig.bL37UnDockTurnOffAir,              ECBool,     "Tempture", "bL37UnDockTurnOffAir",               bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(cbL39_1,    &IniConfig.bL39_1AutoRunWhenTempOk,           ECBool,     "Tempture", "bL39_1AutoRunWhenTempOk",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbL39_2,    &IniConfig.bL39_2WaitTempstabilize,           ECBool,     "Tempture", "bL39_2WaitTempstabilize",                bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtL39_2,   &IniConfig.iL39_2WaitTempstabilize,           ECInteger,  "Tempture", "iL39_2WaitTempstabilize",                bShow, bEnable, bReadFromFile,  3, false, 1, 180);

        elConfig->Add(edL40,      &IniConfig.iL40ImmediateTempExceedsAlarm,     ECInteger,  "Tempture", "iL40ImmediateTempExceedsAlarm",     bShow, bEnable, bReadFromFile,  7, false, 1, 10);

        elConfig->Add(edL41,      &IniConfig.iL41TemperatureAlarmSecond,        ECInteger,  "Tempture", "iL41TemperatureAlarmSecond",     bShow, bEnable, bReadFromFile,  3, false, 1, 10);

        elConfig->Add(cbL42,      &IniConfig.bL42_UseOutShuttleDesoakTime,      ECBool,     "Tempture", "bL42_UseOutShuttleDesoakTime",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtL42,     &IniConfig.iL42_UseOutShuttleDesoakTime,      ECInteger,  "Tempture", "iL42_UseOutShuttleDesoakTime",         bShow, bEnable, bReadFromFile,  30, false, 0, 120);

        elConfig->Add(cbL44,      &IniConfig.bL44_SetColdAirSwitchTemp,         ECBool,     "Tempture", "bL44_SetColdAirSwitchTemp",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtL44,     &IniConfig.iL44_SetColdAirSwitchTemp,         ECInteger,  "Tempture", "iL44_SetColdAirSwitchTemp",         bShow, bEnable, bReadFromFile,  30, false, 0, 25);

        elConfig->Add(cbL45,      &IniConfig.bL45_SetDewPointOffset,            ECBool,     "Tempture", "bL45_SetDewPointOffset",         bShow, bEnable, bReadFromFile, 0);                //Ztex 2025.04.01 Add Set Dew Point Offset
        elConfig->Add(edtL45,     &IniConfig.iL45_SetDewPointOffset,            ECInteger,  "Tempture", "iL45_SetDewPointOffset",         bShow, bEnable, bReadFromFile,  30, false, -25, 25);//Ztex 2025.04.01 Add Set Dew Point Offset
        elConfig->Add(cbL46,      &IniConfig.bL46_AStreamErrorCompressOnecycle, ECBool,     "Tempture", "bL43_AStreamErrorCompressOnecycle",    bShow, bEnable, bReadFromFile, 0);//Ztex 2024.10.01 Add AStream Error Compress Onecycle
        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    }
    else
    {
        elConfig->Add(cbL42,     &IniConfig.bL42_UseOutShuttleDesoakTime,       ECBool,     "Tempture", "bL42_UseOutShuttleDesoakTime",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edtL42,    &IniConfig.iL42_UseOutShuttleDesoakTime,       ECInteger,  "Tempture", "iL42_UseOutShuttleDesoakTime",         bNoShow, bDisable, bFixedValue, 0);
    }

    if(ATC_SYSTEM==eNewATCSystem)                                               //KenHsieh 20240216 : add ATC Power Follow Function
        elConfig->Add(cbL43,    &IniConfig.bL43EnableATCPowerFollow,            ECBool,     "Tempture", "bL43EnableATCPowerFollow",             bShow,   bEnable,  bReadFromFile, 0);
    else
        elConfig->Add(cbL43,    &IniConfig.bL43EnableATCPowerFollow,            ECBool,     "Tempture", "bL43EnableATCPowerFollow",             bNoShow, bDisable, bFixedValue, 0);
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:3120-3161, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemM()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;
    tsM00->TabVisible=(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL);                                  //ChungHung 20131009 add for SCK
    if(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL)
    {
        elConfig->Add(cbM01,    &IniConfig.bM01EnableMonitorFunction,           ECBool,     "Monitor", "bEnableMonitorFunction",                        bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_01, &IniConfig.bM0101ContactModeUseDifferentSpeed,  ECBool,     "Monitor", "bMonitorContactModeMustSelectDifferentSpeed",   bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_02, &IniConfig.bM0102SiteYieldDifferentMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldDifferentMustOn",              bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_03, &IniConfig.bM0103ContinueFailBySocketMustOn,    ECBool,     "Monitor", "bMonitorSiteYieldContinueFailBySocketMustOn",   bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_04, &IniConfig.bM0104ContinueFailByHeadMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldContinueFailByHeadMustOn",     bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_05, &IniConfig.bM0105InOutArmDeviceCheckMustOn,     ECBool,     "Monitor", "bMonitorInOutArmDeviceCheckMustOn",             bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_06, &IniConfig.bM0106IndexDeviceCheckDestoryMustOn, ECBool,     "Monitor", "bMonitorIndexArmDeviceCheckDestoryMustOn",      bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_07, &IniConfig.bM0107AutoSpeedMustOn,               ECBool,     "Monitor", "bMonitorAutoSpeedMustOn",                       bShow, bEnable, bReadFromFile, 0);  //ChungHung 20131009 add for SCK
        elConfig->Add(cbM01_08, &IniConfig.bM0108EveryFirstDeviceMustOn,        ECBool,     "Monitor", "bMonitorEveryFirstDeviceMustEnabled",           bShow, bEnable, bReadFromFile, 0);  //ChungHung 20141210 add for SCK want to add Monitor every first device have delay time
        elConfig->Add(cbM01_09, &IniConfig.bM0109RTCOffCheckYieldPiggyBack,     ECBool,     "Monitor", "bM0109RTCOffCheckYieldPiggyBack",               bShow, bEnable, bReadFromFile, 0);  //ChungHung 20150613 add for SCK want to check and show message
        elConfig->Add(cbM01_10, &IniConfig.bM1010Disable_I12,                   ECBool,     "Monitor", "bM1010Disable_I12",                             bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(cbM01_12, &IniConfig.bM1012EnableAutoClean,               ECBool,     "Monitor", "bM1012EnableAutoClean",                         bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(cbM01_13, &IniConfig.bM1013Enable2DID,                    ECBool,     "Monitor", "bM1013Enable2DID",                              bShow, bEnable, bReadFromFile, 0);  //Steven 20160727 : For SCK
        elConfig->Add(cbM01_14, &IniConfig.bM1014EnableATC,                     ECBool,     "Monitor", "bM1014EnableATC",                               bShow, bEnable, bReadFromFile, 0);  //Steven 20191129 : For SCK
    }
    else
    {
        elConfig->Add(cbM01,    &IniConfig.bM01EnableMonitorFunction,           ECBool,     "Monitor", "bEnableMonitorFunction",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_01, &IniConfig.bM0101ContactModeUseDifferentSpeed,  ECBool,     "Monitor", "bMonitorContactModeMustSelectDifferentSpeed",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_02, &IniConfig.bM0102SiteYieldDifferentMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldDifferentMustOn",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_03, &IniConfig.bM0103ContinueFailBySocketMustOn,    ECBool,     "Monitor", "bMonitorSiteYieldContinueFailBySocketMustOn",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_04, &IniConfig.bM0104ContinueFailByHeadMustOn,      ECBool,     "Monitor", "bMonitorSiteYieldContinueFailByHeadMustOn",     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_05, &IniConfig.bM0105InOutArmDeviceCheckMustOn,     ECBool,     "Monitor", "bMonitorInOutArmDeviceCheckMustOn",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_06, &IniConfig.bM0106IndexDeviceCheckDestoryMustOn, ECBool,     "Monitor", "bMonitorIndexArmDeviceCheckDestoryMustOn",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_07, &IniConfig.bM0107AutoSpeedMustOn,               ECBool,     "Monitor", "bMonitorAutoSpeedMustOn",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_08, &IniConfig.bM0108EveryFirstDeviceMustOn,        ECBool,     "Monitor", "bMonitorEveryFirstDeviceMustEnabled",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_09, &IniConfig.bM0109RTCOffCheckYieldPiggyBack,     ECBool,     "Monitor", "bM0109RTCOffCheckYieldPiggyBack",               bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_10, &IniConfig.bM1010Disable_I12,                   ECBool,     "Monitor", "bM1010Disable_I12",                             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_12, &IniConfig.bM1012EnableAutoClean,               ECBool,     "Monitor", "bM1012EnableAutoClean",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_13, &IniConfig.bM1013Enable2DID,                    ECBool,     "Monitor", "bM1013Enable2DID",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbM01_14, &IniConfig.bM1014EnableATC,                     ECBool,     "Monitor", "bM1014EnableATC",                               bNoShow, bDisable, bFixedValue, 0); //Steven 20191129 : For SCK
    }
}

// AI(W906-FW-CFG-W2) 20260825: golden cConfiguration.cpp:3921-4095, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemO()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;
    bool bDefaultO10=false;

    bDefaultO10=IniConfig.bSPILFunction;                    //JerryYang 20251013 : fix O10預設值錯誤

    elConfig->Add(cbO01,        &IniConfig.bO01_ResetNeedClearAndCheckHP,       ECBool,     "Count", "bResetNeedClearAndCheckHotPlate",                 bShow, bEnable, bReadFromFile, 0);
    if(IniConfig.bSPILFunction || IniConfig.bResetClearArmIC==true)
    {
        elConfig->Add(cbO02,    &IniConfig.bO02ResetNotClearPlate,              ECBool,     "Count", "bO02ResetNotClearPlate",                          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbO05,    &IniConfig.bO05ResetNeedRemoveAllTray,          ECBool,     "Count", "bO05ResetNeedRemoveAllTray",                      bShow, bEnable, bReadFromFile, 0);      //Eliot 2010_03_05
    }
    else
    {
        elConfig->Add(cbO02,    &IniConfig.bO02ResetNotClearPlate,              ECBool,     "Count", "bO02ResetNotClearPlate",                          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbO05,    &IniConfig.bO05ResetNeedRemoveAllTray,          ECBool,     "Count", "bO05ResetNeedRemoveAllTray",                      bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bEventLogAutoSaveFunction)                                     //Steven 20110221 Start : EventLogAutoSave
    {
        elConfig->Add(cbO06,                &IniConfig.bO06_EventLogAutoSave,           ECBool, "Event Log", "EnableAutoSaveEventLog",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkO06UseNetDrive,    &IniConfig.bAlarmStatistAutoSaveNetDrive,   ECBool, "Event Log", "bAlarmStatistAutoSaveNetDrive",           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkO06TimePeriod,     &IniConfig.bO06SaveLogTimePeriod,           ECBool, "Event Log", "EnanleTimePeriodSaveLog",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkO06AlarmHistroy,   &IniConfig.bEnableAlarmHistroyAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmHistroy",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkO06AlarmStatist,   &IniConfig.bEnableAlarmStatistAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmStatist",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkO06Production,     &IniConfig.bEnableProductionAutoSave,       ECBool, "Event Log", "EnableAutoSaveProductiont",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edO06_FilePath,       &IniConfig.asEventLogAutoSavePath,          ECText, "Event Log", "AutoSaveEventLogPath",                    bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(edtO06AlarmHistroy,   &IniConfig.asAlarmHistroyAutoSavePath,      ECText, "Event Log", "AutoSaveAlarmHistroyPath",                bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(edtO06AlarmStatist,   &IniConfig.asAlarmStatistAutoSavePath,      ECText, "Event Log", "AutoSaveAlarmStatistPath",                bShow, bEnable, bReadFromFile, "D:\\RMS");
        elConfig->Add(edtO06Production,     &IniConfig.asProductionAutoSavePath,        ECText, "Event Log", "AutoSaveProductionPath",                  bShow, bEnable, bReadFromFile, "D:\\RMS");          //Steven 20140816 : Production Data
        elConfig->Add(edtO06_Remote,        &IniConfig.asAlarmRemoteDirectory,          ECText, "Event Log", "asAlarmLocalDirectory",                   bShow, bEnable, bReadFromFile, "Z:");               //Steven 20140306 : 使用網路硬碟
        elConfig->Add(edtO06_Local,         &IniConfig.asAlarmLocalDirectory,           ECText, "Event Log", "asAlarmRemoteDirectory",                  bShow, bEnable, bReadFromFile, "\\\\NET_DRVE");     //Steven 20140306 : 使用網路硬碟
        elConfig->Add(cbO06TimePeriod,      &IniConfig.iO06SaveLogTimePeriod,           ECInteger, "Event Log", "TimePeriodSaveLog",                    bShow, bEnable, bReadFromFile, 0); //JerryYang 20151026 : ProductionData週期時間(0->10mins,1->30mins)
    }
    else
    {
        elConfig->Add(cbO06,                &IniConfig.bO06_EventLogAutoSave,           ECBool, "Event Log", "EnableAutoSaveEventLog",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkO06UseNetDrive,    &IniConfig.bAlarmStatistAutoSaveNetDrive,   ECBool, "Event Log", "bAlarmStatistAutoSaveNetDrive",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkO06TimePeriod,     &IniConfig.bO06SaveLogTimePeriod,           ECBool, "Event Log", "EnanleTimePeriodSaveLog",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkO06AlarmHistroy,   &IniConfig.bEnableAlarmHistroyAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmHistroy",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkO06AlarmStatist,   &IniConfig.bEnableAlarmStatistAutoSave,     ECBool, "Event Log", "EnableAutoSaveAlarmStatist",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkO06Production,     &IniConfig.bEnableProductionAutoSave,       ECBool, "Event Log", "EnableAutoSaveProductiont",               bNoShow, bDisable, bFixedValue, 0);
    }

//    if(IniConfig.bSPILFunction==true)     //JerryYang 20250305 : mark掉, 暫時先用Event log Saver                                           //Steven 20240604 : SPIL格式的event log
//    {
//        elConfig->Add(cbO10_EventLogSaver,  &IniConfig.bO10UseEventLogSaver,            ECBool, "Event Log", "bO10UseEventLogSaver",                    bNoShow, bDisable, bFixedValue, 0);      //Steven 20140902 : 使用外掛小程式
//    }
//    else
    {
        if(IniConfig.bEventLogAutoSaveFunction ||
           CosFunction.bUseMDB)
            elConfig->Add(cbO10_EventLogSaver,  &IniConfig.bO10UseEventLogSaver,        ECBool, "Event Log", "bO10UseEventLogSaver",                    bShow, bEnable, bReadFromFile, bDefaultO10?"1":"0");      //JerryYang 20251013 : fix O10預設值錯誤  //Steven 20140902 : 使用外掛小程式
    }

    elConfig->Add(cbO07,                    &IniConfig.bFTContinueON,           ECBool,     "Count", "bFTContinueON",                                   bShow, bEnable, bReadFromFile, 0);      //kevin FT MODE 不能關連續FAIL

    int iMaxFTContinue=100;
    if(CosFunction.bOEEFunction==true)                                          //Jimmychiu 20231026 : no limited max consecutive faulure count in Greatek
    {
        iMaxFTContinue=99999;
    }

    elConfig->Add(edO07,                &IniConfig.iFTMAXValue,                 ECInteger,  "Count", "iFTMAXValue",                                     bShow, bEnable, bReadFromFile, 5,  false,  2,      iMaxFTContinue);            //kevin 20121008 FT MODE 連續FAIL 最大數量
    elConfig->Add(cbO09,                &IniConfig.bInitialStartNeedAsk,        ECBool,     "Count", "bInitialStartNeedAsk",                            bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問
    elConfig->Add(cbO11,                &IniConfig.bRecordJamRateByTime,        ECBool,     "Count", "Record Jam Rate By Time",                         bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edtO11,               &IniConfig.iRecordJamRateIntervalTime,  ECInteger,  "Count", "Record Jam Rate Interval Time",                   bShow, bEnable, bReadFromFile, 1,  false,  1,      999999);     // 2015.11.11 , Joye , Add Jam Rate Record

    if(CosFunction.bUseHeadContactCount==true && CUSTOMER_CODE!=CC_SCC)         //Ifor 20191128 : Fix 銦片 Life Time 功能無顯示
    {
        if((CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bHiSiliconFunction==true) || //Ifor 20180919 (Steven) : KYEC 要求海思版第一個Life Time Count [O12]功能強制開啟並反灰
           CUSTOMER_CODE==CC_AMD_M)
            elConfig->Add(cbO12_LifeTimeCount,  &IniConfig.bLifeTimeCount[0],   ECBool,     "O_Count", "bLifeTimeCount 1",                              bShow, bDisable, bFixedValue, 1);        //wei 20160509 Life Time Count
        else if(IniConfig.bVTESTFunction)                                       //AI(ht9045-config) 20260508 (RogerYang) : VTEST O12跟隨Recipe(configByRecipe.ini)
            elConfig_byRecipe->Add(cbO12_LifeTimeCount, &IniConfig.bLifeTimeCount[0], ECBool, "O_Count", "bLifeTimeCount 1",                            bShow, bEnable, bReadFromFile, 0);
        else
            elConfig->Add(cbO12_LifeTimeCount,  &IniConfig.bLifeTimeCount[0],   ECBool,     "O_Count", "bLifeTimeCount 1",                              bShow, bEnable, bReadFromFile, 0);

        if(IniConfig.bVTESTFunction)                                            //AI(ht9045-config) 20260508 (RogerYang) : VTEST只用O12，隱藏O13/O14及HeadCondition2/3
        {
            elConfig->Add(cbO13_LifeTimeCount,  &IniConfig.bLifeTimeCount[1],   ECBool,     "O_Count", "bLifeTimeCount 2",                              bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(cbO14_LifeTimeCount,  &IniConfig.bLifeTimeCount[2],   ECBool,     "O_Count", "bLifeTimeCount 3",                              bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(edHeadCondition1,     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bShow, bEnable, bReadFromFile, "HeadCondition1");
            elConfig->Add(edHeadCondition2,     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bNoShow, bDisable, bReadFromFile, "HeadCondition2");
            elConfig->Add(edHeadCondition3,     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bNoShow, bDisable, bReadFromFile, "HeadCondition3");
        }
        else
        {
            elConfig->Add(cbO13_LifeTimeCount,      &IniConfig.bLifeTimeCount[1],   ECBool,     "O_Count", "bLifeTimeCount 2",                              bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(cbO14_LifeTimeCount,      &IniConfig.bLifeTimeCount[2],   ECBool,     "O_Count", "bLifeTimeCount 3",                              bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(edHeadCondition1,     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bShow, bEnable, bReadFromFile, "HeadCondition1");
            elConfig->Add(edHeadCondition2,     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bShow, bEnable, bReadFromFile, "HeadCondition2");
            elConfig->Add(edHeadCondition3,     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bShow, bEnable, bReadFromFile, "HeadCondition3");
        }
    }
    else
    {
        elConfig->Add(cbO12_LifeTimeCount,  &IniConfig.bLifeTimeCount[0],       ECBool,     "O_Count", "bLifeTimeCount 1",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbO13_LifeTimeCount,  &IniConfig.bLifeTimeCount[1],       ECBool,     "O_Count", "bLifeTimeCount 2",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbO14_LifeTimeCount,  &IniConfig.bLifeTimeCount[2],       ECBool,     "O_Count", "bLifeTimeCount 3",                              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edHeadCondition1,     &IniConfig.ContactConditionName[0], ECText,     "O_Count", "O14_ContactConditionName1",                     bNoShow, bDisable, bFixedValue, "HeadCondition1");
        elConfig->Add(edHeadCondition2,     &IniConfig.ContactConditionName[1], ECText,     "O_Count", "O15_ContactConditionName2",                     bNoShow, bDisable, bFixedValue, "HeadCondition2");
        elConfig->Add(edHeadCondition3,     &IniConfig.ContactConditionName[2], ECText,     "O_Count", "O16_ContactConditionName3",                     bNoShow, bDisable, bFixedValue, "HeadCondition3");
    }

    elConfig->Add(cbbO15_1,     &IniConfig.iO15_SaveFilePeriod,                 ECInteger,  "Event Log", "iO15_SaveFilePeriod",                         bShow, bEnable, bReadFromFile,  6);     //Steven 20170829 (wei) : Event Log檔案存檔設定
    elConfig->Add(chkO15_2,     &IniConfig.bO15_EventLogFileNameWithMachineID,  ECBool,     "Event Log", "bO15_EventLogFileNameWithMachineID",          bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(chkO15_3,     &IniConfig.bO15_EventLogSaveSameFolder,         ECBool,     "Event Log", "bO15_EventLogSaveSameFolder",                 bShow, bEnable, bReadFromFile, 0);       //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾

    if(CosFunction.bConAlarmNeedKeyInPassword)
    {
        elConfig->Add(chkO16,   &IniConfig.bO16ConAlarmNeedKeyInPasswordCT,     ECBool,     "Count", "bO16ConAlarmNeedKeyInPasswordCT",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtO16,   &IniConfig.iO16ConAlarmNeedKeyInPasswordCT,     ECInteger,  "Count", "Con Alarm Need KeyIn Password CT",                bShow, bEnable, bReadFromFile,  3,     false,  1,      100);
    }
    else
    {
        elConfig->Add(chkO16,   &IniConfig.bO16ConAlarmNeedKeyInPasswordCT,     ECBool,     "Count", "bO16ConAlarmNeedKeyInPasswordCT",                 bNoShow, bDisable, bFixedValue, 0);
        edtO16->Visible=false;
    }

    if(CosFunction.bConAlarmInTimeLevelUp)                                      //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    {
        elConfig->Add(chkO17,       &IniConfig.bO17EnableLevelUpWhenContiAlarm, ECBool,     "Count", "bO17EnableLevelUpWhenContiAlarm",                 bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtO17_Count, &IniConfig.iO17LevelUpWhenContiAlarmCount,  ECInteger,  "Count", "iO17LevelUpWhenContiAlarmCount",                  bShow, bEnable, bReadFromFile,  3,     false,  1,      100);            //Steven 20210127 : 連續alarm次數
        elConfig->Add(edtO17_Time,  &IniConfig.iO17LevelUpWhenContiAlarmTime,   ECInteger,  "Count", "iO17LevelUpWhenContiAlarmTime",                   bShow, bEnable, bReadFromFile,  30,    false,  1,      100000);         //Steven 20210127 : 連續alarm時間
    }
    else
    {
        elConfig->Add(chkO17,       &IniConfig.bO17EnableLevelUpWhenContiAlarm, ECBool,     "Count", "bO17EnableLevelUpWhenContiAlarm",                 bNoShow, bDisable, bFixedValue, 0);
        edtO17_Count->Visible=false;
        edtO17_Time->Visible=false;
        lblO17->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        elConfig->Add(cbO18,        &IniConfig.bO18SafeDoorOnOffDurationDetect, ECBool,     "Count", "bO18SafeDoorOnOffDurationDetect",                 bShow, bEnable, bReadFromFile, 1);
        elConfig->Add(edtO18,       &IniConfig.iO18SafeDoorOnOffDurationHour,   ECInteger,  "Count", "iO18SafeDoorOnOffDurationHour",                   bShow, bEnable, bReadFromFile,  168,     false,  1,      168);           //JerryYang 20210112 : 安全門檢查機制,長時間未開啟要跳出alarm
    }
    else
    {
        elConfig->Add(cbO18,        &IniConfig.bO18SafeDoorOnOffDurationDetect, ECBool,     "Count", "bO18SafeDoorOnOffDurationDetect",                 bNoShow, bDisable, bFixedValue, 0);
        edtO18->Visible=false;
    }

    elConfig->Add(cbO19_1,      &IniConfig.bO19_AutoRecordReportByEveryDay,     ECBool,     "Event Log", "bO19_AutoRecordReportByEveryDay",             !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week
    elConfig->Add(cbO19_2,      &IniConfig.bO19_AutoRecordReportByEveryWeek,    ECBool,     "Event Log", "bO19_AutoRecordReportByEveryWeek",            bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(coO19_3,      &IniConfig.iO19_WeekPeriod,                     ECInteger,  "Event Log", "iO19_WeekPeriod",                             bShow, bEnable, bReadFromFile, 0);      //Sam 20210107 : Summary Report fuction
    elConfig->Add(coO19_5,      &IniConfig.iO19_MonthPeriod,                    ECInteger,  "Event Log", "iO19_MonthPeriod",                            !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week //Sam 20210107 : Summary Report fuction
    elConfig->Add(cbO19_4,      &IniConfig.bO19_AutoRecordReportByEveryMonth,   ECBool,     "Event Log", "bO19_AutoRecordReportByEveryMonth",           !IniConfig.bVTESTFunction, bEnable, bReadFromFile, 0);      //RogerYang 20251104 : 偉測MTBF文件生成，只設定week
    elConfig->Add(edtO19_6,     &IniConfig.asO19_SavePath,                      ECText,     "Event Log", "asO19_SavePath",                              bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\Product_Loader");       //Sam 20210107 : Summary Report fuction
    elConfig->Add(cbO20,        &IniConfig.bO20InOutArmPickerLifeTimeCount,     ECBool,     "O_Count", "bO20InOutArmPickerLifeTimeCount",               bShow, bEnable, bReadFromFile, 0);      //wei 20160509 Life Time Count

    elConfig->Add(cbO20_1,      &IniConfig.bO20_1ClearLifeTimeWhenInitialStart, ECBool,     "O_Count", "bO20_1ClearLifeTimeWhenInitialStart",           bShow, bEnable, bReadFromFile, 0);      //Steven 20240531 : initial start的時候要清除資料

    elConfig->Add(cbO21,        &IniConfig.bO21FTAfterTrayEndClearFailBinCount, ECBool,     "O_Count", "bO21FTAfterTrayEndClearFailBinCount",           bShow, bEnable, bReadFromFile, 0);      //Frank 20241114 : Add
    elConfig->Add(chkO22,       &IniConfig.bO22_ClearSortCntByDoubleClick,      ECBool,     "O_Count", "bO22_ClearSortCntByDoubleClick",                bShow, bEnable, bReadFromFile, 1);      //Steven 20241206 : 點兩下可以清除數量

    if(CUSTOMER_CODE==CC_JCET ||
       CUSTOMER_CODE==CC_SCC)                                                   //Steven 20250428 : add SCC
        elConfig->Add(chkO23,   &IniConfig.bO23_InputLotIDByBarcode,            ECBool,     "O_Count", "bO23_InputLotIDByBarcode",                      bShow, bEnable, bReadFromFile, 1);      //Steven 20241224 : LotID只能用Barcode
    else
        elConfig->Add(chkO23,   &IniConfig.bO23_InputLotIDByBarcode,            ECBool,     "O_Count", "bO23_InputLotIDByBarcode",                      bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_JCET ||                                                //Steven 20250519 : Production Log By Lot
       CUSTOMER_CODE==CC_SCC ||
       CUSTOMER_CODE==CC_JSI_HAOXING)                                           //Frank 20241114 : Add
        elConfig->Add(chkO24,   &IniConfig.bO24_ProductionLogByLot,             ECBool,     "O_Count", "bO24_ProductionLogByLot",                       bShow, bEnable, bReadFromFile, 1);
    else
        elConfig->Add(chkO24,   &IniConfig.bO24_ProductionLogByLot,             ECBool,     "O_Count", "bO24_ProductionLogByLot",                       bShow, bEnable, bReadFromFile, 0);
}

// =============================================================================
// FW-CFG-W3 -- InitConfigEdtList_ItemI / InitConfigEdtList_ItemN / InitConfigEdtList_ItemP (merged 20260825)
// =============================================================================
// AI(W906-FW-CFG-W3) 20260825: golden cConfiguration.cpp:2421-2871, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemI()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    if(CUSTOMER_CODE==CC_AMKOR_Philippines || CUSTOMER_CODE==CC_ASE_KaohSiung)  //JerryYang 20160228 for Amkor Philippines,Output Shuttle lose IC會自動回Home,所以強制開啟此功能
    {
        elConfig->Add(cbI01,    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bDisable, bFixedValue, 1);
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                         //Ifor 20160922 KYEC 喬智 要求 I01 強制關閉不可修改
    {
        elConfig->Add(cbI01,    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bDisable, bFixedValue, 0);
    }
    else
    {
        elConfig->Add(cbI01,    &IniConfig.bI01TesterFinishThenHome,            ECBool,     "Tester", "bI01TesterFinishThenHome",               bShow, bEnable, bReadFromFile, 0);
    }

    if(CosFunction.bLockI02ByFile)                                              //JerryYang 20260504 : for SPIL -- I02 Enable
    {
        elConfig->Add(cbI02,    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, IniConfig.bI02_Enable, bReadFromFile, 0);                   //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
    }
    else if(CUSTOMER_CODE==CC_ASE_M ||                                               //Ifor 20190618 : add ASEM 強制 關閉[I02]功能 回HOME INDEX ARM 全部丟ERROR Bin
       CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20200915 add: ASE SG 強制 關閉[I02]功能 回HOME INDEX ARM 全部丟ERROR Bin
    {
        elConfig->Add(cbI02,    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bDisable, bFixedValue, 0);
    }
    else if(CUSTOMER_CODE==CC_ASE_CL ||                                         //JerryYang 20210129 : ASE-CL松諭要求強制開啟
           (CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true))
    {
        elConfig->Add(cbI02,    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbI02,    &IniConfig.bI02HomeSetSocketICToErrBin,         ECBool,     "Tester", "bI02_bHomeSetSocketICToErrBin",          bShow, bEnable, bReadFromFile, 0);                   //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
    }

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20210219 : 晶兆成需要開關 TempControl 模式
        elConfig->Add(cbI03,    &IniConfig.bI03AmbientTempControl,              ECBool,     "Tester", "bI03AmbientTempControl",                 bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbI03,    &IniConfig.bI03AmbientTempControl,              ECBool,     "Tester", "bI03AmbientTempControl",                 bShow, bDisable, bFixedValue, 1);

    elConfig->Add(cbI04,        &IniConfig.bI04EnableChangeBinDuringTesting,    ECBool,     "Tester", "bI04EnableChangeBinDuringTesting",       bShow, bEnable, bReadFromFile, LastSet.bEnableChangeBinDuringTesting?"1":"0");

    if(CUSTOMER_CODE==CC_JCET)                                                  //JerryYang 20190709 JCET黃剛要求low yield不要強制one cycle
    {
        elConfig->Add(cbI05,    &IniConfig.bI05LowYieldForcedOneCycle,          ECBool,     "Tester", "bI05LowYieldForcedOneCycle",             bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbI05,    &IniConfig.bI05LowYieldForcedOneCycle,          ECBool,     "Tester", "bI05LowYieldForcedOneCycle",             bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbI06,        &IniConfig.bI06TurnOnI01AfterHome,              ECBool,     "Tester", "bI06TurnOnI01AfterHome",                 bShow, bEnable, bReadFromFile, 0);       //Sam 20220216 : Home 完成後強制開啟 I01 Function。

    if(CUSTOMER_CODE==CC_KYEC_CHEN)                                             //jou 2015-05-27 KYEC BU6 要求 I07 ResetGPIBAfterOneCycleCleanOut 強制關閉
        elConfig->Add(cbI07,    &IniConfig.bI07ResetGPIBAfterOneCycleCleanOut,  ECBool,     "Tester", "bI07ResetGPIBAfterOneCycleCleanOut",     bShow, bDisable, bFixedValue, 0);
    else
        elConfig->Add(cbI07,    &IniConfig.bI07ResetGPIBAfterOneCycleCleanOut,  ECBool,     "Tester", "bI07ResetGPIBAfterOneCycleCleanOut",     bShow, bEnable, bReadFromFile, 0);

    if(BAR_CODE_INSTALL!=ebctUninstall)                                         //Steven 20190412 : Initial Start時檢查有沒有開啟2DID
        elConfig->Add(cbI08,    &IniConfig.bI08Check2DIDEnableWhenInitialStart, ECBool,     "Tester", "bI08Check2DIDEnableWhenInitialStart",    bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbI08,    &IniConfig.bI08Check2DIDEnableWhenInitialStart, ECBool,     "Tester", "bI08Check2DIDEnableWhenInitialStart",    bShow, bDisable, bFixedValue, 0);

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
    {
        elConfig->Add(cbI09,    &IniConfig.bI09LowYieldOneCycleDontCleanShuttle,ECBool,     "Tester", "bI09LowYieldOneCycleDontCleanShuttle",   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbI09,    &IniConfig.bI09LowYieldOneCycleDontCleanShuttle,ECBool,     "Tester", "bI09LowYieldOneCycleDontCleanShuttle",   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_SCC ||                                                 //jou 2012-06-27 We meet summary issue in SCC line, so please help fix two function (no one can change the setting)
       CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20150903 鎖定I12
        elConfig->Add(cbI12,    &IniConfig.bI12TesterTimerOutNotNeedReTest,     ECBool,     "Tester", "bI12TesterTimerOutNotNeedReTest",        bShow, bDisable, bFixedValue, 1);
    else
        elConfig->Add(cbI12,    &IniConfig.bI12TesterTimerOutNotNeedReTest,     ECBool,     "Tester", "bI12TesterTimerOutNotNeedReTest",        bShow, bEnable, bReadFromFile, LastSet.bTesterTimerOutNotNeedReTest?"1":"0");

    elConfig->Add(cbI13,        &IniConfig.bI13InitStartDelayHasFTandRT,        ECBool,     "Tester", "bI13InitStartDelayHasFTandRT",           bShow, bEnable, bReadFromFile, 0);      //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    elConfig->Add(cbI16,        &IniConfig.bI16TTLSaveInSetupFile,              ECBool,     "Tester", "bI16TTLSaveInSetupFile",                 bShow, bEnable, bReadFromFile, 0);      //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    elConfig->Add(cbI18,        &IniConfig.bI18CanReceiveEchoStop,              ECBool,     "Tester", "bCanReceiveEchoStop",                    bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130326 add for ASE_KR
    elConfig->Add(cbI19,        &IniConfig.bI19AuToSitMapPauseWaitBin,          ECBool,     "Tester", "bAuToSitMapPauseWaitBin",                bShow, bEnable, bReadFromFile, 0);      //kevin 20150122 AutoSitMap 使用gpib模擬器讓機台暫停 設定 bin別
    elConfig->Add(coI20,        &IniConfig.iI20ErrorBinAlphabet,                ECInteger,  "Tester", "iI20ErrorBinAlphabet",                   bShow, bEnable, bReadFromFile, LastSet.iErrorBinAlphabet);   //Steven 20100413

    if(IniConfig.bUseAutoSiteMapping)                                           //Steven 20110421 : Auto Site Mapping
    {
        lbI21_6->Visible=false;
        edI21_6->Visible=false;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
           edI21->Visible=false;
        }
        else if(IniConfig.bVTESTFunction==true)                                 //jou 20200707 : VTEST auto site mapping
        {
            lbI21_6->Visible=true;
            edI21_6->Visible=true;
        }

        gbI21->Visible=true;
        edI21_9->Visible=(CosFunction.bAutoSiteMappingUseFailBinSetting &&      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
                          !CosFunction.bAutoSiteMappingSetOpenBIN);             //Steven 20230213 : [I21-9]的OS Bin跟著工作檔

        if(CosFunction.bI21EnableASMByRecipe==false)                            //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
        {
            if(IniConfig.bVTESTFunction==true)                                  //RogerYang 20250624 修正無法By工作檔問題
                elConfig_byRecipe->Add(cbI21,   &IniConfig.bI21EnableASM,           ECBool,     "Auto Site Mapping", "Enable Auto Site Mapping",    bShow, bEnable, bReadFromFile, 1);
            else
                elConfig->Add(cbI21,            &IniConfig.bI21EnableASM,           ECBool,     "Auto Site Mapping", "Enable Auto Site Mapping",    bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            cbI21->Visible=false;
        }

        if(CUSTOMER_CODE==CC_LEADYO)                                            //KenHsieh 20251003 : LEADYO 需要設定fail次數報Alarm
        {
            elConfig->Add(edI21_1,              &IniConfig.iI21FailRetryCount,      ECInteger, "Auto Site Mapping", "iI21FailRetryCount",           bShow,  bEnable,   bReadFromFile, 0, false, 0, 20);
        }
        else
        {
            elConfig->Add(edI21_1,              &IniConfig.iI21FailRetryCount,      ECInteger, "Auto Site Mapping", "iI21FailRetryCount",           bNoShow, bDisable, bFixedValue,   0);
        }

        #ifdef ASE_KaohSiung                                                    //kevin 20150115
        elConfig->Add(edI21,                &IniConfig.fI21UseSameSoakTime,           ECDouble, "Auto Site Mapping", "Use Same Soak Time Sec",              bNoShow, bDisable, bFixedValue, 0.0);
        elConfig->Add(cbI21_SkipSoakTime,   &IniConfig.bI21SkipSoakTime,                ECBool, "Auto Site Mapping", "Skip Soak Time",                      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_SameSoakTime,   &IniConfig.bI21UseSameSoakTime,             ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_CheckOpen,      &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_5,              &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_6,              &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_7,              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_8,              &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_9,          &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        #else
        elConfig->Add(edI21,                &IniConfig.fI21UseSameSoakTime,           ECDouble, "Auto Site Mapping", "Use Same Soak Time Sec",              bShow, bEnable, bReadFromFile,  10.0, false, 0.0,     360.0);
        elConfig->Add(cbI21_SkipSoakTime,   &IniConfig.bI21SkipSoakTime,                ECBool, "Auto Site Mapping", "Skip Soak Time",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI21_SameSoakTime,   &IniConfig.bI21UseSameSoakTime,             ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bShow, bEnable, bReadFromFile, 0);
        if(CUSTOMER_CODE==CC_SCS)
            elConfig->Add(cbI21_CheckOpen,  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bShow, bDisable, bFixedValue, 1);        //Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open
        else if(CosFunction.bUSEJCETSiteMapMode)                                //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(cbI21_CheckOpen,  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(cbI21_CheckOpen,  &IniConfig.bI21ASMNeedCheckEachSiteOpen,    ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bShow, bEnable, bReadFromFile, 0);

        if(CosFunction.bUSEJCETSiteMapMode)                                     //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(cbI21_5,          &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);     //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
        else
            elConfig->Add(cbI21_5,          &IniConfig.bI21ASMRemoveLTrayManually,      ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bShow, bEnable, bReadFromFile, 0);      //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray

        if(CUSTOMER_CODE==CC_ASE_M)                                             //Ifor 20180420 : add ASEM 要求強制開啟不可修改
            elConfig->Add(cbI21_6,          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bDisable, bFixedValue, 1);        //Steven 20140729 : AutoSiteMapping, 邊生產邊做
        else if(IniConfig.bVTESTFunction==true)
            elConfig->Add(cbI21_6,          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bDisable, bFixedValue, 0);        //Steven 20140729 : AutoSiteMapping, 邊生產邊做
        else if(CosFunction.bUSEJCETSiteMapMode)                                //jou 2016-11-03 JCET 要求 Auto Site Mapping 需按照他們的要求撰寫
            elConfig->Add(cbI21_6,          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        else
            elConfig->Add(cbI21_6,          &IniConfig.bI21ASMRunTimeCHeck,             ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bShow, bEnable, bReadFromFile, 0);

        if(IniConfig.bVTESTFunction==true)
            elConfig->Add(cbI21_7,              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);      //kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2
        else
            elConfig->Add(cbI21_7,              &IniConfig.bASMAutoSiteMapBinComBine,       ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bShow, bEnable, bReadFromFile, 0);      //kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2

        if(CosFunction.bAutoSiteMappingUseHotPlate==true)
        {
            if(CosFunction.bUSEJCETSiteMapMode ||
               CUSTOMER_CODE==CC_ASE_M)                                         //Ifor 20190624 : add ASEM 要求強制開啟不可修改
            {
                elConfig->Add(cbI21_8,      &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(cbI21_8,      &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bShow, bEnable, bReadFromFile, 0);      //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
            }
        }
        else
        {
            elConfig->Add(cbI21_8,          &IniConfig.bI21AutoSiteMappingUseHotplate,  ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        }

        if(CosFunction.bAutoSiteMappingUseFailBinSetting==true)                 //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
        {
            elConfig->Add(cbI21_9,      &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bShow, bEnable, bReadFromFile, 0);      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
        }
        else
        {
            elConfig->Add(cbI21_9,      &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        }

        if(IniConfig.bVTESTFunction==true)                                      //jou 20200707 : VTEST auto site mapping
            elConfig->Add(edI21_6,          &IniConfig.iI21AutoSiteMappingErrCT,ECInteger,  "Auto Site Mapping", "iI21AutoSiteMappingErrCT",                bShow, bEnable, bReadFromFile,  3, false, 1,     5);
        else
            elConfig->Add(edI21_6,          &IniConfig.iI21AutoSiteMappingErrCT,ECInteger,  "Auto Site Mapping", "iI21AutoSiteMappingErrCT",                bNoShow, bDisable, bFixedValue,  0);

        if(CosFunction.bAutoSiteMappingUseFailBinSetting &&                     //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
           CosFunction.bAutoSiteMappingSetOpenBIN==false)                       //Steven 20230213 : [I21-9]的OS Bin跟著工作檔
            elConfig->Add(edI21_9,          &IniConfig.iI21UseFailBinSetting,   ECInteger,  "Auto Site Mapping", "iI21UseFailBinSetting",                   bShow, bEnable, bReadFromFile,  15, false, 2, 15);
        else
            elConfig->Add(edI21_9,          &IniConfig.iI21UseFailBinSetting,   ECInteger,  "Auto Site Mapping", "iI21UseFailBinSetting",                   bNoShow, bDisable, bFixedValue, 0);

        if(CUSTOMER_CODE==CC_JCET)                                              //Richard 20230427 : RT mode不跑sitemapping
        {
            elConfig->Add(cbI21_10,    &IniConfig.bI21RTmodeDonotRunSiteMapping,ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",  bShow, bEnable, bReadFromFile, 0);      //Ifor 20171128 (Steven) : add Auto Site Mapping Fail Bin Setting
        }
        else
        {
            elConfig->Add(cbI21_10,    &IniConfig.bI21RTmodeDonotRunSiteMapping,ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",  bNoShow, bDisable, bFixedValue, 0);
        }
        #endif
    }
    else
    {
        gbI21->Visible=false;
        elConfig->Add(cbI21,                &IniConfig.bI21EnableASM,                       ECBool, "Auto Site Mapping", "Enable Auto Site Mapping",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_SkipSoakTime,   &IniConfig.bI21SkipSoakTime,                    ECBool, "Auto Site Mapping", "Skip Soak Time",                      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_SameSoakTime,   &IniConfig.bI21UseSameSoakTime,                 ECBool, "Auto Site Mapping", "Use Same Soak Time",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_CheckOpen,      &IniConfig.bI21ASMNeedCheckEachSiteOpen,        ECBool, "Auto Site Mapping", "bASMNeedCheckEachSiteOpen",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_5,              &IniConfig.bI21ASMRemoveLTrayManually,          ECBool, "Auto Site Mapping", "bASMRemoveLTrayManually",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_6,              &IniConfig.bI21ASMRunTimeCHeck,                 ECBool, "Auto Site Mapping", "bASMRunTimeCHeck",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_7,              &IniConfig.bASMAutoSiteMapBinComBine,           ECBool, "Auto Site Mapping", "bASMAutoSiteMapBinComBine",           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_8,              &IniConfig.bI21AutoSiteMappingUseHotplate,      ECBool, "Auto Site Mapping", "bI21AutoSiteMappingUseHotplate",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_9,              &IniConfig.bI21AutoSiteMappingFailBinSetting,   ECBool, "Auto Site Mapping", "bI21AutoSiteMappingFailBinSetting",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI21_10,             &IniConfig.bI21RTmodeDonotRunSiteMapping,       ECBool, "Auto Site Mapping", "bI21RTmodeDonotRunSiteMapping",       bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bTestTimeOutOnlyShowSkip ||                                  //QQ 20230303
       CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       (CosFunction.bIndexAreaOnlyCanUseSkip &&
        CosFunction.bTestTimeOutShowSkipAndHome==false))                        //Steven 20141105 : Index內的所有異常都只能用Skip
    {
        elConfig->Add(cbI22,    &IniConfig.bI22TimeOutCanSkip,                  ECBool,     "Tester", "TestTimeOutCanSkip",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(rgI22,    &IniConfig.iI22TestTimeOutOption,               ECInteger,  "Tester", "iI22TestTimeOutOption",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edI22_1,  &IniConfig.fI22HomeDelay,                       ECDouble,   "Tester", "fI22HomeDelay",                          bNoShow, bDisable, bFixedValue, 0.0);
    }
    else
    {
        elConfig->Add(cbI22,    &IniConfig.bI22TimeOutCanSkip,                  ECBool,     "Tester", "TestTimeOutCanSkip",                     bShow, bEnable, bReadFromFile, 0);      //Steven 20111220 : 測試TimeOut可以Skip
        elConfig->Add(rgI22,    &IniConfig.iI22TestTimeOutOption,               ECInteger,  "Tester", "iI22TestTimeOutOption",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edI22_1,  &IniConfig.fI22HomeDelay,                       ECDouble,   "Tester", "fI22HomeDelay",                          bShow, bEnable, bReadFromFile, 30.0);
    }

    elConfig->Add(cbI23,        &IniConfig.bI23HotTestWaitingMode,              ECBool,     "Tester", "HotTestWaitingMode",                     bShow, bEnable, bReadFromFile, 0);      //ChungHung 20111230 Hot Test Waiting Mode

    if(IniConfig.bEnableTestingNeedStopAllMotor)
        elConfig->Add(cbI24,    &IniConfig.bI24TestingNeedStopAllMotor,         ECBool,     "Tester", "bTestingNeedStopAllMotor",               bShow, bEnable, bReadFromFile, 0);      //jou 2013-09-25 Testing Need Stop All Motor
    else
        elConfig->Add(cbI24,    &IniConfig.bI24TestingNeedStopAllMotor,         ECBool,     "Tester", "bTestingNeedStopAllMotor",               bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bHiSiliconFunction==true &&                                  //Steven 20160301 : For ATC     //kevin 20130705 台積電通訊規格  0:HT    1:NS   //Steven 20160301 : 改為海思格式
       CUSTOMER_CODE!=CC_SCC)                                                   //Steven 20241029 : SCC的KL版本通訊不一樣
    {
        elConfig->Add(coI25,    &IniConfig.iI25UseGPIBFormat,                   ECInteger,  "Temperature", "iUseGPIBFormat",                    bShow, bDisable, bFixedValue, 1);       //Ifor 20160407 海思專用版本 GPIB 格式強制設定為1
    }
    else
    {
        elConfig->Add(coI25,    &IniConfig.iI25UseGPIBFormat,                   ECInteger,  "Temperature", "iUseGPIBFormat",                    bShow, bEnable, bReadFromFile, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        elConfig->Add(cbI26,    &IniConfig.bI26TestCloseSiteHaveBin,            ECBool,     "Tester", "bTestCloseSiteHaveBin",                  bShow, bEnable, bReadFromFile, 0);      //kevin 20150202 測試時沒有 ic 出現bin資料或bin別沒設定需取出ic
    else
        elConfig->Add(cbI26,    &IniConfig.bI26TestCloseSiteHaveBin,            ECBool,     "Tester", "bTestCloseSiteHaveBin",                  bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bManualSortMode)
        elConfig->Add(cbI27,    &IniConfig.bI27_ManualSortMode,                 ECBool,     "Tester", "bI27_ManualSortMode",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20150915 : For TSMC 手動整盤功能
    else
        elConfig->Add(cbI27,    &IniConfig.bI27_ManualSortMode,                 ECBool,     "Tester", "bI27_ManualSortMode",                    bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbI28,        &IniConfig.bI28_OnOffSiteOnTheFly,              ECBool,     "Tester", "bI28_OnOffSiteOnTheFly",                 bShow, bEnable, bReadFromFile, 0);      //Steven 20150924 : 隨時開關Site功能

    elConfig->Add(cbI29,        &IniConfig.bI29EnableYieldRecord,               ECBool,     "Function", "bEnableYieldRecord",                   bShow, bEnable, bReadFromFile, 0);      //Ifor 20151221 :新增 Yield Record Function
    elConfig->Add(chkI29_1,     &IniConfig.bI29_1SaveYieldBySocketByBin,        ECBool,     "Function", "bI29_1SaveYieldBySocketByBin",         bShow, bEnable, bReadFromFile, 0);      //Steven 20171108 (wei) : By Socket By Bin存檔
    elConfig->Add(cbI29_3,      &IniConfig.bI29YieldRecordIntervalIC,           ECBool,     "Function", "bI29YieldRecordIntervalIC",            bShow, bEnable, bReadFromFile, 0);      //Sam 20231106 : 紀錄 Total yield
    elConfig->Add(edI29_3,      &IniConfig.iI29YieldRecordIntervalIC,           ECInteger,  "Function", "iI29YieldRecordIntervalIC",            bShow, bEnable, bReadFromFile, 100,   false, 1,       3000);          //Sam 20231106 : 紀錄 Total yield
    elConfig->Add(edI29,        &IniConfig.fI29YieldRecordInterval,             ECDouble,   "Function", "fYieldRecordInterval",                 bShow, bEnable, bReadFromFile, 600.0, false, 1.0,     3000.0);         //Ifor 20151221 :新增 Yield Record Interval Time
    elConfig->Add(cbI30,        &IniConfig.bI30ContFailBin,                     ECBool,     "Tester", "I30_bContFailBin",                       bShow, bEnable, bReadFromFile, 0);      //kevin 20160407 使用Count bin  Count number 設定bin fail 數量到達就ALARM 清除計數

    if(CUSTOMER_CODE==CC_TSMC_TAINAN ||
       CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        grpI31->Visible=true;
        elConfig->Add(cbI31_1,  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI31_2,  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bShow, bEnable, bReadFromFile, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(cbI31_3,  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bShow, bEnable, bReadFromFile, 0);
    }
    else if(CosFunction.bGPIBLotEnd==true)
    {
        grpI31->Visible=true;
        elConfig->Add(cbI31_1,  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI31_2,  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bNoShow, bDisable, bFixedValue, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(cbI31_3,  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bNoShow, bDisable, bFixedValue, 0);     //wei 20170918 Reset Command
    }
    else
    {
        grpI31->Visible=false;
        elConfig->Add(cbI31_1,  &IniConfig.bI31_1GPIBLotEnd,                    ECBool,     "Specific", "I31_GPIBLotEnd",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbI31_2,  &IniConfig.bI31_2GPIBLotStart,                  ECBool,     "Specific", "bI31_2GPIBLotStart",                   bNoShow, bDisable, bFixedValue, 0);     //wei 20170607 GPIB Lot Start Command
        elConfig->Add(cbI31_3,  &IniConfig.bI31_3GPIBReset,                     ECBool,     "Specific", "bI31_3GPIBReset",                      bNoShow, bDisable, bFixedValue, 0);     //wei 20170918 Reset Command
    }
    elConfig->Add(cbI32,        &IniConfig.bI32CanCelErrorBin,                  ECBool,     "Tester", "bI32CanCelErrorBin",                     bNoShow, bDisable, bFixedValue, 0);     //kevin 20160802 取消error bin 要手動在 outshuttle 取出

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20220902
        elConfig->Add(cbI33,    &IniConfig.bI33ErrorBinBox,                     ECBool,     "Tester", "bI33ErrorBinBox",                        bShow, bDisable, bFixedValue, 1);       //kevin 20160819 error bin 要放到 Bin Box
    else
        elConfig->Add(cbI33,    &IniConfig.bI33ErrorBinBox,                     ECBool,     "Tester", "bI33ErrorBinBox",                        bShow, bEnable, bReadFromFile, 0);      //kevin 20160819 error bin 要放到 Bin Box

    if(IniConfig.bSPILFunction==true)                                           //JerryYang20170329 (wei) 矽品中科,順信要求強制開啟I34功能
    {
        elConfig->Add(cbI34,    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bShow, bDisable, bFixedValue, 1);       //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    }
    else if(CosFunction.bAllSiteSameFailBinShowAlarm)                           //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    {
        elConfig->Add(cbI34,    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbI34,    &IniConfig.bI34AllSiteAreSameFailBinShowAlarm,  ECBool,     "Tester", "bI34AllSiteAreSameFailBinShowAlarm",     bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbI35,        &IniConfig.bI35UseThirdSiteControlByEngineer,   ECBool,     "Tester", "bI35UseThirdSiteControlByEngineer",      bShow, bEnable, bReadFromFile, 0);      //Alick 20160926 add for 第三組工程師用開關SITE
    elConfig->Add(cbI36,        &IniConfig.bI36TestTimeOut,                     ECBool,     "Tester", "bI36TestTimeOut",                        bShow, bEnable, bReadFromFile, 0);      //kevin 20161105 沒有收到測試資料需手動取出ic

    if(CosFunction.bHaveFIFOMode)                                               //Steven 20170302 (wei) : FIFO MODE
    {
        grpI37->Visible=true;
        elConfig->Add(cbI37_1,  &IniConfig.bI37_EnableFIFOMode,                 ECBool,     "Tester", "bI37_EnableFIFOMode",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(cbI37_2,  &IniConfig.bI37_EnableFIFOSiteOrder,            ECBool,     "Tester", "bI37_EnableFIFOSiteOrder",               bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(cbI37_3,  &IniConfig.bI37_LockLoaderDirection,            ECBool,     "Tester", "bI37_LockLoaderDirection",               bShow, bEnable, bReadFromFile, 0);      //Steven 20170302 (wei) : FIFO MODE
    }
    else
    {
        grpI37->Visible=false;
        elConfig->Add(cbI37_1,  &IniConfig.bI37_EnableFIFOMode,                 ECBool,     "Tester", "bI37_EnableFIFOMode",                    bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(cbI37_2,  &IniConfig.bI37_EnableFIFOSiteOrder,            ECBool,     "Tester", "bI37_EnableFIFOSiteOrder",               bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
        elConfig->Add(cbI37_3,  &IniConfig.bI37_LockLoaderDirection,            ECBool,     "Tester", "bI37_LockLoaderDirection",               bNoShow, bDisable, bFixedValue, 0);     //Steven 20170302 (wei) : FIFO MODE
    }

    elConfig->Add(rgI38,        &IniConfig.iI38SETTEMPRespondSetTemp,           ECInteger,  "Tester", "bI38SETTEMPRespondSetTemp",              bShow, bEnable, bReadFromFile, 0);      //kevin 20180308 Settemp? 回傳需要 Settemp +25.0.
    elConfig->Add(cbI39,        &IniConfig.bI39SpiroxTesterLotEnd,              ECBool,     "Tester", "bI39SpiroxTesterLotEnd",                 bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170515 (wei) JCET通知tester lot end command
    elConfig->Add(cbI40,        &IniConfig.bI40_bStartProductOnLine,            ECBool,     "Tester", "bI40_bStartProductOnLine",               bShow, bEnable, bReadFromFile, 0);      //kevin 20180517 on line for operator

    //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    //==>
    elConfig->Add(cbI41,        &IniConfig.bI41EnableEmptySocketCheck,          ECBool,     "Tester", "bI41EnableEmptySocketCheck",         bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbbI41,       &IniConfig.iI41_BinOfESC,                       ECInteger,  "Tester", "iI44_BinOfESC",                      bShow, bEnable, bReadFromFile, 0);  //Steven 20220817 : Bin of ESC function
    elConfig->Add(cbI41_1,      &IniConfig.bI41_1_StartOfLot,                   ECBool,     "Tester", "bI41_1_StartOfLot",                  bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbI41_2,      &IniConfig.bI41_2_OpenChamberDoor,              ECBool,     "Tester", "bI41_2_OpenChamberDoor",             bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbI41_3,      &IniConfig.bI41_3_AfterContactorTeminated,      ECBool,     "Tester", "bI41_3_AfterContactorTeminated",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbI41_4,      &IniConfig.bI41_4_AfterContactorJam,            ECBool,     "Tester", "bI41_4_AfterContactorJam",           bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbI41_5,      &IniConfig.bI41_5_RegularExecutionCycle,        ECBool,     "Tester", "bI41_5_RegularExecutionCycle",       bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(edtI41_5,     &IniConfig.iI41_5_RegularExecutionCycleCount,   ECInteger,  "Tester", "iI41_5_RegularExecutionCycleCount",  bShow, bEnable, bReadFromFile,  0,     false, 0,       100000);         //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    elConfig->Add(cbI41_6,      &IniConfig.bI41_6_Manual,                       ECBool,     "Tester", "bI41_6_Manual",                      bShow, bEnable, bReadFromFile, 0);
    //<==
    //Steven 20201022 : For RFMD Empty Socket Check Funstion.

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(cbI42,    &IniConfig.bI42_bEnableBarcodeFlowErr,          ECBool,  "Tester", "bI42_bEnableBarcodeFlowErr",         bShow, bEnable, bReadFromFile, 0);          //Ifor 20211116 add: Use Barcode Flow Err Check
    }
    else
    {
        elConfig->Add(cbI42,    &IniConfig.bI42_bEnableBarcodeFlowErr,          ECBool,  "Tester", "bI42_bEnableBarcodeFlowErr",         bShow, bEnable, bReadFromFile, 1);          //Ifor 20211116 add: Use Barcode Flow Err Check
    }

    elConfig->Add(cbI43,        &IniConfig.bI43ResetGPIBAfterTrayFeedFinish,    ECBool,     "Tester", "bI06ResetGPIBAfterTrayFeedFinish",   bShow, bEnable, bReadFromFile, 0);          //Sam 20211107 : 新增 Tray Feed Finish Reset GPIB

    if(CosFunction.bLowYieldAlarmIntervalTimeBySetting)
    {
        elConfig->Add(chkI44,   &IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting, ECBool, "Tester", "bI44_LowYieldAlarmIntervalTimeBySetting", bShow, bEnable, bReadFromFile, 0);     //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
        elConfig->Add(edI44,    &IniConfig.iI44_LowYieldAlarmIntervalTime,      ECInteger,  "Tester", "iI44_LowYieldAlarmIntervalTime",     bShow, bEnable, bReadFromFile,  60,     false,  1,      300);            //Steven 20141212 : 使用固定的ADC
    }
    else
    {
        elConfig->Add(chkI44,   &IniConfig.bI44_LowYieldAlarmIntervalTimeBySetting, ECBool, "Tester", "bI44_LowYieldAlarmIntervalTimeBySetting", bNoShow, bDisable, bFixedValue, 0);    //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
        elConfig->Add(edI44,    &IniConfig.iI44_LowYieldAlarmIntervalTime,      ECInteger,  "Tester", "iI44_LowYieldAlarmIntervalTime",     bNoShow, bDisable, bFixedValue, 60,     false,  1,      300);            //Steven 20141212 : 使用固定的ADC
    }

    if(BAR_CODE_INSTALL!=ebctUninstall && CUSTOMER_CODE==CC_ASE_KaohSiung)      //KenHsieh 20230607 : ASEKH add 2D SORT
        elConfig->Add(cbI45,    &IniConfig.bI45_Use2DIDSort,                    ECBool,     "Tester",  "bI45_Use2DIDSort",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbI45,    &IniConfig.bI45_Use2DIDSort,                    ECBool,     "Tester",  "bI45_Use2DIDSort",                  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20241018 : 錢寶龍說要強制開啟
        elConfig->Add(rgI46,    &IniConfig.iI46_ActionWhenGpibFlowErr,          ECInteger,  "Tester", "iI46_ActionWhenGpibFlowErr",         bShow, bDisable, bFixedValue,  1);
    else
        elConfig->Add(rgI46,    &IniConfig.iI46_ActionWhenGpibFlowErr,          ECInteger,  "Tester", "iI46_ActionWhenGpibFlowErr",         bShow, bEnable, bReadFromFile,  1);         //Steven 20231017 : GPIB flow error need alarm

    elConfig->Add(cbI49,        &IniConfig.bI49_TesterTimeOutResetAllIC,        ECBool,     "Tester", "bI49_TesterTimeOutResetAllIC",       bShow, bEnable, bReadFromFile, 0);                          //Sam 20240215 : Tester time out show reset all ic
    elConfig->Add(edI49,        &IniConfig.fI49_ChangeAboveSocket,              ECDouble,   "Tester", "fI49_ChangeAboveSocket",             bShow, bEnable, bReadFromFile,  0.0, false, 0.0,     50.0);

    if(IniConfig.bUseAutoSiteMapping)                                           //Jimmychiu 20230707 : Auto Site Mapping Trigger Function
    {
        gbI50->Visible=true;
        elConfig->Add(cbI50_EnableASM_Trigger,&IniConfig.bI50_EnableAutoSiteMappingTrigger, ECBool, "Tester", "bI50_EnableAutoSiteMappingTrigger",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_StartLot,       &IniConfig.bI50_StartLot,                       ECBool, "Tester", "bI50_StartLot",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_InitialStart,   &IniConfig.bI50_InitialStart,                   ECBool, "Tester", "bI50_InitialStart",  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_OnyCycle,       &IniConfig.bI50_OnyCycle,                       ECBool, "Tester", "bI50_OnyCycle",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_Pause,          &IniConfig.bI50_Pause,                          ECBool, "Tester", "bI50_Pause",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_RT,             &IniConfig.bI50_RT,                             ECBool, "Tester", "bI50_RT",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI50_TrayFeed,       &IniConfig.bI50_TrayFeed,                       ECBool, "Tester", "bI50_TrayFeed",      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbI50_EnableASM_Trigger, &IniConfig.bI50_EnableAutoSiteMappingTrigger, ECBool, "Tester", "bI50_EnableAutoSiteMappingTrigger",      bNoShow, bDisable, bFixedValue, 0);
        gbI50->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if(USE_STM_Function==true)
        {
            elConfig->Add(cbI51,&IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",          bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            elConfig->Add(cbI51,&IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",         bNoShow, bDisable, bFixedValue, 0);      //kevin 20180517 on line for operator
        }
        elConfig->Add(cbI52,    &IniConfig. bI52_bAQLSortMode,                  ECBool,  "Tester", "bI52_bAQLSortMode",                  bShow, bDisable, bFixedValue, 1);
    }
    else
    {
        elConfig->Add(cbI51,    &IniConfig.bI51_bNotSetErrBinForInput,          ECBool,  "Tester", "bI51_bNotSetErrBinForInput",         bNoShow, bDisable, bFixedValue, 0);      //kevin 20180517 on line for operator
        elConfig->Add(cbI52,    &IniConfig. bI52_bAQLSortMode,                  ECBool,  "Tester", "bI52_bAQLSortMode",                  bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true && CosFunction.bHiSiliconFunction==true)    //Ifor 20230210 add:新增 KLT 要求Onecycle Finish未超過設定時間不執行KL initial delay
    {
        elConfig->Add(cbI53,        &IniConfig. bI53_bKLTInitial,               ECBool,  "Tester", "bI53_bKLTInitial",      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbI53,        &IniConfig. bI53_bKLTInitial,               ECBool,  "Tester", "bI53_bKLTInitial",      bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bCheckTempDuringIndexArmTesting)                             //Jimmychiu 20240916 : Check the temperature during index arm testing
    {
        elConfig->Add(cbI54_Enable,         &IniConfig.bI54_Enable,             ECBool,  "Tester", "bI54_Enable",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI54_1,              &IniConfig.bI54_1_AllICErr,         ECBool,  "Tester", "bI54_1_AllICErr",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbI54_2,              &IniConfig.bI54_2_AbnormalICErr,    ECBool,  "Tester", "bI54_2_AbnormalICErr", bShow, bEnable, bReadFromFile, 0);
    }
    else                                                                        //Steven 20241108 : 加上保護
    {
        elConfig->Add(cbI54_Enable,         &IniConfig.bI54_Enable,             ECBool,  "Tester", "bI54_Enable",          bNoShow, bDisable, bFixedValue, 0);
        gbI54->Visible=false;
    }
}

// AI(W906-FW-CFG-W3) 20260825: golden cConfiguration.cpp:3163-3919, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemN()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    str=(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)?"RMS":"Server";
    #ifdef SOFT_SIMULTE
        elConfig->Add(cbN05_SCKWebService,  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bNoShow, bDisable, bFixedValue, 0);
    #else
    if(CUSTOMER_CODE==CC_SCK)
        elConfig->Add(cbN05_SCKWebService,  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bShow, bEnable, bReadFromFile, 0);             //Steven 20161201 : For SCK Web Service
    else
        elConfig->Add(cbN05_SCKWebService,  &IniConfig.bN05SCKWebService,       ECBool,     str, "EnableWebService",                                    bNoShow, bDisable, bFixedValue, 0);
    #endif
    elConfig->Add(rgUpDlMethod,             &IniConfig.iN05_UpDLMethod,         ECInteger,  str, "iN05_UpDLMethod",                                     bShow, bEnable, bReadFromFile,  eByNetwork);   //Jimmychiu 20250707 : add RMS connect method

    if(CosFunction.bFTPUseBarcodeReader==true)
    {
      #ifndef SOFT_SIMULTE
        if(CUSTOMER_CODE==CC_CYUEAN)                                            //Sam 20230608 : 楊建軍說不要開關功能
            elConfig->Add(cbN06_UseBarcode, &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bNoShow, bEnable, bFixedValue, 1);
        else
      #endif
            elConfig->Add(cbN06_UseBarcode, &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbN06_UseBarcode,     &IniConfig.bN06_UseBarcode,         ECBool,     "FTP", "Use Barcode Reader",                                bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(chkN06_Tester,            &IniConfig.bN06_CopyTesterFile,     ECBool,     "FTP", "bN06_CopyTesterFile",                               bShow, bEnable, bReadFromFile, 0);   //Steven 20250327 : OS測試機的工作檔也要上傳
    elConfig->Add(edN06_FileName,           &IniConfig.asN06_FileName,          ECText,     "FTP", "FTP File Name",                                     bShow, bEnable, bReadFromFile, "UnKnown");
    if(CosFunction.bEnable_SECS_GEM)                                            //Steven 20141006 : SECS GEM使用Remote Start功能
    {
        #ifndef SOFT_SIMULTE
        if(IniConfig.bSPILFunction==true || CUSTOMER_CODE==CC_JCET)             //JerryYang20170329 (wei) 矽品世明要求強制開啟SECS GEM  //JerryYang 20210107 : Add JCET
            elConfig->Add(cbN07_EnableSecs, &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bShow, bDisable, bFixedValue, 1);
        else if(CUSTOMER_CODE==CC_ASE_KaohSiung && (MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050))      //kevin 20181225 add HT9046 no use SECS GEM for ASE-KH //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
            elConfig->Add(cbN07_EnableSecs, &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bNoShow, bDisable, bFixedValue, 0);
        else
        #endif
            elConfig->Add(cbN07_EnableSecs, &IniConfig.bEnable_SECS_GEM,        ECBool,     "SECS GEM", "Enable SECS GEM",                              bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(edN07_2,              &IniConfig.iN07RunCheckAlarmTime,   ECInteger,  "Function", "iRunCheckAlarmTime",                           bShow, bEnable, bReadFromFile,  LastSet.iRunCheckAlarmTime, false, 10, 300);    //wei 20150512  Run Check Alarm Time

        if(CosFunction.bRCMDStart)
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung && (MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050)) //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
                elConfig->Add(cbN07_EnableHostStart, &IniConfig.bRCMDStart,     ECBool,     "SECS GEM", "Enable RCMD START",                            bNoShow, bDisable, bFixedValue, 0);
            else
                elConfig->Add(cbN07_EnableHostStart, &IniConfig.bRCMDStart,     ECBool,     "SECS GEM", "Enable RCMD START",                            bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            elConfig->Add(cbN07_EnableHostStart,    &IniConfig.bRCMDStart,      ECBool,     "SECS GEM", "Enable RCMD START",                            bNoShow, bDisable, bFixedValue, 0);
        }

        if(CosFunction.bSECS_GEM_OneCycle)                                      //wei 20150824 Secs_Gem 斷線Onecycle
            elConfig->Add(cbN07_EnableSecsOneCycle, &IniConfig.bSECS_GEM_OneCycle, ECBool,  "SECS GEM", "SECS GEM OneCycle",                            bShow, bEnable, bReadFromFile, 0);
        else
            elConfig->Add(cbN07_EnableSecsOneCycle, &IniConfig.bSECS_GEM_OneCycle, ECBool,  "SECS GEM", "SECS GEM OneCycle",                            bNoShow, bDisable, bFixedValue, 0);

        if(CosFunction.bUseN07_5==true)                                         //Steven 20200309 : [N07-5]改成by客戶開啟
            elConfig->Add(cbN07_EnableEmployeeCheak,&IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bShow, bEnable, bReadFromFile, 0);    //Ifor 20180911 (Steven) : Add 啟動 Employee ID Check
        else
            elConfig->Add(cbN07_EnableEmployeeCheak,&IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bNoShow, bDisable, bFixedValue, 0);

        if(CUSTOMER_CODE==CC_TSMC_TAINAN || CUSTOMER_CODE==CC_ASE_KaohSiung)
            elConfig->Add(cbN07_EnableSecsLotCheck, &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bShow, bEnable, bReadFromFile, 0);       //wei 20160727 Secs Gem Lot Check
        else
            elConfig->Add(cbN07_EnableSecsLotCheck, &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bNoShow, bDisable, bFixedValue, 0);

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
            elConfig->Add(edN07_5, &IniConfig.iN07_EmployeeIdCheakTime,         ECInteger,  "SECS GEM", "iEmployeeIdCheckTime",                         bShow, bEnable, bReadFromFile,  5, false, 10, 300);

        elConfig->Add(chkN07_6, &IniConfig.bN07_6EnableUploadOSRecipe,          ECBool, "Specific", "bN07_6EnableUploadOSRecipe",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : OS測試機的工作檔也要上傳
        elConfig->Add(lbledtN07_6, &IniConfig.sN07_6OSRecipePath,               ECFileName, "SECS GEM", "sN07_6OSRecipePath",                           bShow, bEnable, bReadFromFile, "Z:\\");
        elConfig->Add(cbN07_6CompressedFile, &IniConfig.bN07_6CompressedFile,   ECBool, "Specific", "bN07_6CompressedFile",                             bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : OS測試機的工作檔也要上傳
        elConfig->Add(chkN07_7, &IniConfig.bN07_7SendRecipeAsBinary,            ECBool, "Specific", "bN07_7SendRecipeAsBinary",                         bShow, bEnable, bReadFromFile, 0);      //Steven 20230710 : 工作檔使用二進制上傳下載
        elConfig->Add(edtN07_7, &IniConfig.dN07_7_DelayTime,                    ECDouble,  "SECS GEM", "iN07_7_DelayTime",                              bShow, bEnable, bReadFromFile,  1.0, false, 0.1, 10.0);
    }
    else
    {
        elConfig->Add(cbN07_EnableSecs,             &IniConfig.bEnable_SECS_GEM,        ECBool, "SECS GEM", "Enable SECS GEM",                          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN07_EnableHostStart,        &IniConfig.bRCMDStart,              ECBool, "SECS GEM", "Enable RCMD START",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN07_EnableSecsOneCycle,     &IniConfig.bSECS_GEM_OneCycle,      ECBool, "SECS GEM", "SECS GEM OneCycle",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN07_EnableEmployeeCheak,    &IniConfig.bN07_EnableEmployeeIdCheak, ECBool, "SECS GEM", "Enable Employee ID Cheak",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN07_EnableSecsLotCheck,     &IniConfig.bN07_EnableSecsLotCheck, ECBool, "Specific", "N07_SecsLotCheck",                         bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bOLPFunction)                                                //Steven 20141229 : OLP的Log要存檔
    {
        tsN08->TabVisible=true;
        elConfig->Add(cbN08_1,  &IniConfig.bN08_1SaveOLPLog,                    ECBool,     "Automation", "Save Log",                                   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN08_2,  &IniConfig.sN08OlpIP,                           ECText,     "OLP",        "OLP_IP",                                     bShow, bEnable, bReadFromFile, "192.168.120.143");
        elConfig->Add(edN08_3,  &IniConfig.sN08OlpPort,                         ECText,     "OLP",        "OLP_Port",                                   bShow, bEnable, bReadFromFile, "6670");
    }
    else
    {
        tsN08->TabVisible=false;
        elConfig->Add(cbN08_1,  &IniConfig.bN08_1SaveOLPLog,                    ECBool,     "Automation", "Save Log",                                   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseTSVFunction)                                             //Steven 20240904 : for ATK的TSV功能
    {                                                                           //Steven 20190521 : ATK lot count
        tsN09->TabVisible=true;
        elConfig->Add(chkN09,           &IniConfig.bN09_LotCountAutoFunc,       ECBool,     "Automation", "bN09_LotCountAutoFunc",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN09_TSV,       &IniConfig.iN09_TSV_Port,               ECPort,     "Automation", "iN09_TSV_Port",                              bShow, bEnable, bReadFromFile, 4000,   false);
        elConfig->Add(chkN09_2,         &IniConfig.bN09_Enable_TSV,             ECBool,     "Automation", "bN09_Enable_TSV",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20231017 : add for ATK
        elConfig->Add(edtN09_SearchTime,&IniConfig.dN09_SearchTime,             ECDouble,   "Automation", "dN09_SearchTime",                            bShow, bEnable, bReadFromFile, 0.0,    false,  1,      3000);
        elConfig->Add(edtN09_Handler,   &IniConfig.sN09_HandlerFolder,          ECText,     "Automation", "sN09_HandlerFolder",                         bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\TestSummary");
        elConfig->Add(rgN09_4,          &IniConfig.iN09_4_UploadMethod,         ECInteger,  "Automation", "iN09_4_UploadMethod",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN09_5User,     &IniConfig.sN09_5_User,                 ECText,     "Automation", "sN19_5_User",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN09_5Password, &IniConfig.sN09_5_Password,             ECPassword, "Automation", "sN19_5_Password",                            bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN09_5Host,     &IniConfig.sN09_5_Host,                 ECText,     "Automation", "sN19_5_Host",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN09_5Path,     &IniConfig.sN09_5_Path,                 ECText,     "Automation", "sN19_5_Path",                                bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN09_7,         &IniConfig.sN09_7_SkipIP,               ECText,     "Automation", "sN19_7_SkipIP",                              bShow, bEnable, bReadFromFile, "172,192");
    }
    else
    {
        tsN09->TabVisible=false;
        elConfig->Add(chkN09,           &IniConfig.bN09_LotCountAutoFunc,       ECBool,     "Automation", "bN09_LotCountAutoFunc",                      bNoShow, bDisable, bFixedValue, 0);
    }

    tsN10->TabVisible=(CosFunction.bHiSiliconFunction ||                        //JerryYang 20180508 (jou) : fix N10 顯示問題
                       CosFunction.bUseLogUploadToFTPFunction ||
                       CosFunction.bHisiLogUploadNetwork ||
                       CUSTOMER_CODE==CC_HANA_MICRON);                          //JimmyChiu 20211008 R211005-Hana-H9-01

    if(CosFunction.bHiSiliconFunction ||
       CosFunction.bUseLogUploadToFTPFunction ||
       CosFunction.bHisiLogUploadNetwork)
    {
        if(IniConfig.bVTESTFunction==true)
        {
            elConfig->Add(cbN10_1,      &IniConfig.bN10Enable_FTPUpLoadLog,     ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bNoShow, bDisable, bFixedValue, 0);
            elConfig->Add(cbN10_2,      &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
            elConfig->Add(cbN10_3,      &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bNoShow, bDisable, bFixedValue, 1);
            elConfig->Add(rgN10_3_1,    &IniConfig.iN10UploadProductMethod,     ECInteger,  "FTPUpLoad", "iN10UploadProductMethod",                     bNoShow, bDisable, bFixedValue, 1);     //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
            elConfig->Add(rgN10_4,      &IniConfig.iN10UploadMethod,            ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bNoShow, bDisable, bFixedValue, 1);

            rgN10_3_1->Visible=false;
            rgN10_4->Visible=false;

            grpN10_FTP->Visible=false;
            lblN10_6->Visible=false;
            edtN10_6->Visible=false;

            cbL11_1->Caption="[L11-1] ATC temperature range(1..3)";             //jou 20240426 : VTEST 要求 ATC 溫度限制設定1~3度
            // ------------------------------------------------------------
            // GOLDEN ODDITY (CFG-L11tag), golden cConfiguration.cpp:3314 vs
            // :2932/:2937/:2939. `edL11_1` is the ONLY widget in this file whose
            // Tag has two different owners writing two different meanings:
            //   ItemL registers it three times with elConfig->Add(), and Add
            //   mirrors the widget's LIST INDEX onto ->Tag (golden
            //   Public/HTEditList.cpp:149 and its four siblings);
            //   then ItemN, which the dispatcher runs AFTER ItemL (golden
            //   :4505 then :4507), overwrites it with the literal 3.
            // So once InitConfigEdtList() finishes, edL11_1->Tag is 3, not its
            // index -- the later write simply wins. Reproduced exactly, because
            // this port now performs both writes in golden's own order.
            // Measured before assuming it was harmless: a tree-wide scan for
            // widgets that are BOTH Add()-registered and directly Tag-assigned
            // returned exactly one name, this one, and nothing anywhere looks
            // edL11_1 up through elConfig->FEditList->Items[...->Tag]
            // (ChangeCBListProperty's thirteen lookups use cbA09/cbD42/cbD44/
            // cbF06/cbF11/cbF14/cbF14_1/cbF17/rgF26/cbI04/cbI06/cbP16/cbP24).
            // So the collision is latent in golden too -- an ODDITY, not a BUG.
            // ------------------------------------------------------------
            edL11_1->Tag=3;
        }
        else
        {
            if(CosFunction.bHiSiliconFunction==true ||                          //JerryYang 20170925 (Steven) Hisi版本強制開啟N10
               (CUSTOMER_CODE==CC_KYEC_LEE &&
                bEnable_KLT_Function==false))
            {
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20200110 add 高雄不需要
                    elConfig->Add(cbN10_1,  &IniConfig.bN10Enable_FTPUpLoadLog, ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bEnable, bReadFromFile, 0);       //Ifor 20160302 : Add FTP Upload Log To Host
                else
                    elConfig->Add(cbN10_1,  &IniConfig.bN10Enable_FTPUpLoadLog, ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bDisable, bFixedValue, 1);
            }
            else
            {
                elConfig->Add(cbN10_1,  &IniConfig.bN10Enable_FTPUpLoadLog,     ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bShow, bEnable, bReadFromFile, 0);       //Ifor 20160302 : Add FTP Upload Log To Host
            }

            if(CUSTOMER_CODE==CC_KYEC_LEE)
            {
                elConfig->Add(cbN10_2,  &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bNoShow, bDisable, bFixedValue, 0);
                elConfig->Add(cbN10_3,  &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bShow, bDisable, bFixedValue, 1);       //Steven 20180514 : JCET吳如春要求每日上傳Event Log Jam統計表 MTBF MUBF資料
            }
            else
            {
                elConfig->Add(cbN10_2,  &IniConfig.bN10_UploadSummaryToFTP,     ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
                elConfig->Add(cbN10_3,  &IniConfig.bN10_DailyUploadProdData,    ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bShow, bEnable, bReadFromFile, 0);      //Steven 20180514 : JCET吳如春要求每日上傳Event Log Jam統計表 MTBF MUBF資料
            }

            elConfig->Add(rgN10_3_1,    &IniConfig.iN10UploadProductMethod,     ECInteger,  "FTPUpLoad", "iN10UploadProductMethod",                     bShow, bEnable, bReadFromFile, 0);      //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
            if(CosFunction.bHisiLogUploadNetwork==true)                         //Steven 20190119 : Log上傳方式改為可選擇的
                elConfig->Add(rgN10_4,      &IniConfig.iN10UploadMethod,        ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bShow, bEnable, bReadFromFile, 1);
            else
                elConfig->Add(rgN10_4,      &IniConfig.iN10UploadMethod,        ECInteger,  "FTPUpLoad", "iN10UploadMethod",                            bShow, bEnable, bReadFromFile, 0);
        }

        if(CUSTOMER_CODE==CC_SIGURD_ChungXing ||                                //Sam 20170823 (wei) : 矽格中興 FTP Log 上傳增加時間格式選擇
           CUSTOMER_CODE==CC_SIGURD_PeiXing ||
           CUSTOMER_CODE==CC_SIGURD_HUKOU)                                      //Ifor 20180213 : add 矽格北興使用FTP Log 上傳增加時間格式選擇
        {
            elConfig->Add(coN10DataType,&IniConfig.iN10DataType,                ECInteger,  "FTPUpLoad", "iN10DataType",                                bShow, bEnable, bReadFromFile, 0);
        }
        else
        {
            coN10DataType->Visible=false;
            lblN10DataType->Visible=false;
        }
    }
    else
    {
        elConfig->Add(cbN10_1,      &IniConfig.bN10Enable_FTPUpLoadLog,         ECBool,     "FTPUpLoad", "bEnable_FTPUpLoadLog",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN10_2,      &IniConfig.bN10_UploadSummaryToFTP,         ECBool,     "FTPUpLoad", "bN10_UploadSummaryToFTP",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN10_3,      &IniConfig.bN10_DailyUploadProdData,        ECBool,     "FTPUpLoad", "bN10_DailyUploadProdData",                    bNoShow, bDisable, bFixedValue, 0);
        coN10DataType->Visible=false;
        lblN10DataType->Visible=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(edN10UserName,    &IniConfig.cN10FtpUserName,             ECText,     "FTPUpLoad",       "cN10FtpUserName",                       bShow, bEnable, bReadFromFile, "eap_handler_log");
        elConfig->Add(chkN10_Passive,   &IniConfig.bN10FtpPassive,              ECBool,     "FTPUpLoad",       "bN10FtpPassive",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN10_Port,      &IniConfig.iN10FtpPort,                 ECPort,     "FTPUpLoad",       "iN10FtpPort",                           bShow, bEnable, bReadFromFile, 21,   false);
        if(bEnable_KLT_Function==true)                                          //Ifor 20180813 (Steven) : Add 京隆 FTP 路徑
        {
            elConfig->Add(edN10Password,&IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "abc1234!");
            elConfig->Add(edN10Host,    &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "sz1ftp03.sz.com.cn");
        }
        else
        {
            elConfig->Add(edN10Password,&IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "!qaz2wsx");
            elConfig->Add(edN10Host,    &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "ch1fabfs05-ftp.kyec.com.tw");
        }
    }
    else
    {
        elConfig->Add(edN10UserName,    &IniConfig.cN10FtpUserName,             ECText,     "FTPUpLoad",       "cN10FtpUserName",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(chkN10_Passive,   &IniConfig.bN10FtpPassive,              ECBool,     "FTPUpLoad",       "bN10FtpPassive",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN10Password,    &IniConfig.cN10FtpPassword,             ECPassword, "FTPUpLoad",       "cN10FtpPassword",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN10Host,        &IniConfig.cN10FtpHost,                 ECText,     "FTPUpLoad",       "cN10FtpHost",                           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN10_Port,      &IniConfig.iN10FtpPort,                 ECPort,     "FTPUpLoad",       "iN10FtpPort",                           bShow, bEnable, bReadFromFile, 21,   false);
    }

    if(IniConfig.bVTESTFunction==true && CosFunction.bHiSiliconFunction==false)
        ;
    else
        elConfig->Add(edtN10_6,         &IniConfig.iN10UploadToHostIntervalTime,ECInteger,  "FTPUpLoad",       "iUploadToHostIntervalTime",             bShow, bEnable, bReadFromFile, 5, false,  5,      3600);

    elConfig->Add(edtN10_8,             &IniConfig.sN10UploadDrivePath,         ECText,     "FTPUpLoad",       "sN10UploadDrivePath",                   bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);

    if(CosFunction.bSaveProductionLogByUnloaderTray==true)                      //Steven 20200409 : production log by unloader tray存檔並上傳FTP
        elConfig->Add(chkN10_9,         &IniConfig.bN10_9_UploadUnloadTrayToFTP,ECBool,     "FTPUpLoad",        "bN10_9_UploadUnloadTrayToFTP",         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(chkN10_9,         &IniConfig.bN10_9_UploadUnloadTrayToFTP,ECBool,     "FTPUpLoad",        "bN10_9_UploadUnloadTrayToFTP",         bNoShow, bDisable, bFixedValue, 0);
    elConfig->Add(edN10UploadPath,      &IniConfig.cN10FtpUplaodPath,           ECText,     "FTPUpLoad",       "cN10FtpUplaodPath",                     bShow, bEnable, bReadFromFile, "\\");

    elConfig->Add(cbN10_11,             &IniConfig.bN10_11_Enable_UploadFTPEventLog,ECBool, "FTPUpLoad",        "bN10_11_Enable_UploadFTPEventLog",     bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(cbN10_12,             &IniConfig.bN10_12_Enable_UploadFTPGPIBLog, ECBool, "FTPUpLoad",        "bN10_12_Enable_UploadFTPGPIBLog",      bShow, bEnable, bReadFromFile, 0);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20160802 Clean Out Low Yield Close Site
    {
        elConfig->Add(cbN11_1,      &IniConfig.bN11_1CleanOutCloseSite,         ECBool,     "NETWORK", "bN11_1CleanOutCloseSite",                       bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        tsN11->TabVisible=false;                                                //Steven 20160804 : modify [N11] for 高雄
        elConfig->Add(cbN11_1,      &IniConfig.bN11_1CleanOutCloseSite,         ECBool,     "NETWORK", "bN11_1CleanOutCloseSite",                       bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20170525 (wei) 力成 add Socket ID Product Data Upload To FTP
    {
        tsN12->TabVisible=true;
        elConfig->Add(chkN12,  &IniConfig.bN12_EnableSocketIdProductDataFTP,    ECBool,     "FTP",     "Second Enable FTP",                             bShow, bEnable, bReadFromFile, 0);      //Sam 20170525 (wei) 力成 add Socket ID Product Data Upload To FTP
        elConfig->Add(edN12_UserName,   &IniConfig.asN12_FtpUserName,           ECText,     "Automation",       "Second FTP User Name",                 bShow, bEnable, bReadFromFile, "UnKnown");
        elConfig->Add(edN12_Password,   &IniConfig.asN12_FtpPassword,           ECPassword, "Automation",       "Second FTP Password",                  bShow, bEnable, bReadFromFile, "1234");
        elConfig->Add(edN12_HostName,   &IniConfig.asN12_FtpHost,               ECText,     "Automation",       "Second FTP Host",                      bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(edN12_UpLdPath,   &IniConfig.asN12_FtpUplaodPath,         ECText,     "Automation",       "Second FTP Upload Path",               bShow, bEnable, bReadFromFile, "/");
    }
    else
    {
        tsN12->TabVisible=false;
        elConfig->Add(chkN12,  &IniConfig.bN12_EnableSocketIdProductDataFTP,    ECBool,     "FTP",     "Second Enable FTP",                             bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseARMSFunction)                                            //Ifor 20170621 (wei) add ARMS Function
    {
        tsN13->TabVisible=true;
        elConfig->Add(cbN13_EnableARMSFunction, &IniConfig.bN13_EnableARMSFunction, ECBool, "ARMS", "bN13_EnableARMSFunction",                          bShow, bEnable, bReadFromFile, 0);      //Ifor 20170621 (wei) add ARMS Function
    }
    else
    {
        tsN13->TabVisible=false;
        elConfig->Add(cbN13_EnableARMSFunction, &IniConfig.bN13_EnableARMSFunction, ECBool, "ARMS", "bN13_EnableARMSFunction",                          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        tsN14->TabVisible=true;
        elConfig->Add(cbN14_1,          &IniConfig.bN14_1_EnableOEEFunction,    ECBool,     "Handler_OEE", "N14_HandlerOEEUseFunction",                 bShow, bEnable, bReadFromFile, 0);      //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(edtN14_1, &IniConfig.iN14_1_OEERecordCycleTime,   ECInteger,  "Handler_OEE", "N14_HandlerOEERecordCycleTime",             bShow, bEnable, bReadFromFile, 1800,    false, 60,       3000);  //Sam 20220922 : 改用 elConfig 方式
        elConfig->Add(cbN14_2,  &IniConfig.bN14_2_OEEUseSaveProdData,   ECBool,     "Handler_OEE", "N14_HandlerOEEUseSaveProductionDataToPath", bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(edtN14_2, &IniConfig.asN14_2_OEESaveProdPath,     ECText,     "Handler_OEE", "N14_HandlerOEESaveProductionDataToPath",    bShow,bEnable, bReadFromFile, "D:\\ProductionInfoBackup");
        elConfig->Add(cbN14_3,  &IniConfig.bN14_3_OEEFTPUpload,         ECBool,     "Handler_OEE", "N14_HandlerOEEFTPUpload",                   bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(edtN14_3UserName, &IniConfig.asN14_3_OEEFTPUserName,      ECText,     "Handler_OEE", "N14_HandlerOEEUserName",                    bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(edtN14_3Password, &IniConfig.asN14_3_OEEFTPPassword,      ECPassword, "Handler_OEE", "N14_HandlerOEEPassword",                    bShow, bEnable, bReadFromFile, " ");
        elConfig->Add(edtN14_3Host,     &IniConfig.asN14_3_OEEFTPHost,          ECText,     "Handler_OEE", "N14_HandlerOEEHost",                        bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(edtN14_3Path,     &IniConfig.asN14_3_OEEFTPUploadPath,    ECText,     "Handler_OEE", "N14_HandlerOEEUploadPath",                  bShow, bEnable, bReadFromFile, "\\OEEE\\");
        elConfig->Add(cbN14_4,          &IniConfig.bN14_4_OEEAutoLoadMOFile,    ECBool,     "Handler_OEE", "N14_HandlerOEEAutoLoadMOFile",              bShow, bEnable, bReadFromFile, 0);      //Sam 20190129 : Bug Fix //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(edtN14_4,         &IniConfig.asN14_4_MODownloadPath,      ECText,     "Handler_OEE", "N14_HandlerMODownloadPath",                 bShow, bEnable, bReadFromFile, "\\SETUP_FILE\\");
        elConfig->Add(edtN14_5,         &IniConfig.iN14_5_PauseIntervalTime,    ECInteger,  "Handler_OEE", "N14_PauseIntervalTimeSec",                  bShow, bEnable, bReadFromFile, 600,     false, 60,      3000);
        elConfig->Add(edtN14_6_1,       &IniConfig.iN14_6_BySiteContactCnt,     ECInteger,  "Handler_OEE", "iN14_6_BySiteContactCnt",                   bShow, bEnable, bReadFromFile, 1800,    false, 1,       3000);
        elConfig->Add(edtN14_6_2,       &IniConfig.dN14_6_BySiteLowYieldRate,   ECDouble,   "Handler_OEE", "dN14_6_BySiteLowYieldRate",                 bShow, bEnable, bReadFromFile, 100.0,   false, 1.0,     100.0);
        elConfig->Add(edtN14_6_3,       &IniConfig.dN14_6_BySiteCmpYield,       ECDouble,   "Handler_OEE", "dN14_6_BySiteCmpYield",                     bShow, bEnable, bReadFromFile, 100.0,   false, 1.0,     100.0);
        elConfig->Add(edtN14_6_4,       &IniConfig.iN14_6_BySiteAlarmYieldRate, ECInteger,  "Handler_OEE", "iN14_6_BySiteAlarmYieldRate",               bShow, bEnable, bReadFromFile, 100,     false, 1,       100);
        elConfig->Add(cbN14_7,          &IniConfig.bN14_7_AutoMotive,           ECBool,     "Handler_OEE", "bN14_7_AutoMotive",                         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN14_7,         &IniConfig.asN14_7_AutoMotivePath,      ECText,     "Handler_OEE", "asN14_7_AutoMotivePath",                    bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN14_8,          &IniConfig.bN14_8_ULSetup,              ECBool,     "Handler_OEE", "bN14_8_ULSetup",                            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN14_8,         &IniConfig.asN14_8_ULSetupPath,         ECText,     "Handler_OEE", "asN14_8_ULSetupPath",                       bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN14_9,          &IniConfig.bN14_9_ULQtyReport,          ECBool,     "Handler_OEE", "bN14_9_ULQtyReport",                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN14_9,         &IniConfig.asN14_9_ULQtyReportPath,     ECText,     "Handler_OEE", "asN14_9_ULQtyReportPath",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN14_10,         &IniConfig.bN14_10_DownFileByMO,        ECBool,     "Handler_OEE", "N14_AutoDownloadSetupFileByMO",             bShow, bEnable, bReadFromFile, 0);      //Sam 20190227 : Try Catch Mo Download SetupFile
        elConfig->Add(cbN14_11,         &IniConfig.bN14_11_CheckSiteMapByMO,    ECBool,     "Handler_OEE", "bN14_11_CheckSiteMapByMO",                  bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbN14_12,         &IniConfig.bN14_12_ULTempLogToFTP,      ECBool,     "Handler_OEE", "bN14_12_ULTempLogToFTP",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN14_12,        &IniConfig.iN14_12_ULTempLogInterval,   ECInteger,  "Handler_OEE", "iN14_12_ULTempLogInterval",                 bShow, bEnable, bReadFromFile, 100,     false, 1,       3000);
        elConfig->Add(edtN14_12_Path,   &IniConfig.asN14_12_ULTempLogPath,      ECText,     "Handler_OEE", "asN14_12_ULTempLogPath",                    bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN14_13,         &IniConfig.bN14_13_ULBinQtyToFTP,       ECBool,     "Handler_OEE", "bN14_13_ULBinQtyToFTP",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN14_13,        &IniConfig.iN14_13_ULBinQtyInterval,    ECInteger,  "Handler_OEE", "iN14_13_ULBinQtyInterval",                  bShow, bEnable, bReadFromFile, 100,     false, 1,       3000);
        elConfig->Add(edtN14_13_Path,   &IniConfig.asN14_13_ULBinQtyPath,       ECText,     "Handler_OEE", "asN14_13_ULBinQtyPath",                     bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN14_14,         &IniConfig.bN14_14_AlarmCtrlMachine,    ECBool,     "Handler_OEE", "bN14_14_AlarmCtrlMachine",                  bShow, bEnable, bReadFromFile, 0);
        // ------------------------------------------------------------------
        // GOLDEN BUG (CFG-N14esc), golden cConfiguration.cpp:3481, :3485, :3488.
        // The three DefValue strings below each lose their directory separator:
        // golden writes "C:\\GTK\EMG.exe" -- the FIRST backslash is doubled, the
        // SECOND is not, so `\E` / `\L` / `\I` are escape sequences, not path
        // separators. Every neighbouring path in this same block is written
        // correctly ("C:\\GTK_Control\\Message\\Alarm\\"), so this is three
        // typos, not a convention.
        //   cmd: scanned all string literals in golden cConfiguration.cpp for a
        //        single backslash followed by a non-escape character
        //        -> exactly 3 hits, these three (20260825)
        // WHAT IT COSTS: these are the DEFAULTS written when the ini key is
        // absent, and they are EXECUTABLE PATHS the GTK integrations launch
        // (N14_14 alarm control / N14_15 socket lifetime / N14_16 IPSC). A fresh
        // machine gets "C:\GTKEMG.exe", which does not exist, and the feature
        // silently never starts.
        // PORT DIVERGENCE, stated because it is real and cannot be avoided
        // without editing golden's text: `\L` and `\I` are unknown escapes in
        // both compilers and both yield the bare letter, so those two are
        // byte-identical to BCB6. `\E` is different -- GCC implements it as a
        // GNU extension meaning ESC (0x1B), while BCB6 yields plain 'E'. So the
        // first default differs from BCB6 by one byte. BOTH are wrong relative
        // to the path that was intended; neither reaches a real file.
        // Translated verbatim regardless. Repairing golden's typo is a
        // behaviour change and belongs to the user, not to a translation wave.
        // ------------------------------------------------------------------
        elConfig->Add(edtN14_14_1,      &IniConfig.asN14_14_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_14_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\EMG.exe");
        elConfig->Add(edtN14_14_2,      &IniConfig.asN14_14_MessageFilePath,    ECText,     "Handler_OEE", "asN14_14_MessageFilePath",                  bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Message\\Alarm\\");
        elConfig->Add(edtN14_14_3,      &IniConfig.asN14_14_FlagFilePath,       ECText,     "Handler_OEE", "asN14_14_FlagFilePath",                     bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Flag\\");
        elConfig->Add(cbN14_15,         &IniConfig.bN14_15_SocketLifeTime,      ECBool,     "Handler_OEE", "bN14_15_SocketLifeTime",                    bShow, bEnable, bReadFromFile, 0);
        // GOLDEN BUG (CFG-N14esc) -- see the full note at the N14_14 site above; same missing separator.
        elConfig->Add(edtN14_15_1,      &IniConfig.asN14_15_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_15_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\Lifetime_count.exe");
        elConfig->Add(edtN14_15_2,      &IniConfig.asN14_15_MessageFilePath,    ECText,     "Handler_OEE", "asN14_15_MessageFilePath",                  bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Message\\count\\");
        elConfig->Add(cbN14_16,         &IniConfig.bN14_16_EnableIPSC,          ECBool,     "Handler_OEE", "bN14_16_EnableIPSC",                        bShow, bEnable, bReadFromFile, 0);
        // GOLDEN BUG (CFG-N14esc) -- see the full note at the N14_14 site above; same missing separator.
        elConfig->Add(edtN14_16_1,      &IniConfig.asN14_16_ExecutFilePath,     ECText,     "Handler_OEE", "asN14_16_ExecutFilePath",                   bShow, bEnable, bReadFromFile, "C:\\GTK\IPSC.exe");
        elConfig->Add(edtN14_16_2,      &IniConfig.asN14_16_FlagFilePath,       ECText,     "Handler_OEE", "asN14_16_FlagFilePath",                     bShow, bEnable, bReadFromFile, "C:\\GTK_Control\\Flag\\IPSC.ini");
        elConfig->Add(edtN14_16_3,      &IniConfig.asN14_16_ProductionFilePath, ECText,     "Handler_OEE", "asN14_16_ProductionFilePath",               bShow, bEnable, bReadFromFile, "C:\\GTK_CONTROL\\MESSAGE\\");
        elConfig->Add(edtN14_16_4,      &IniConfig.iN14_16_IPSCInterval,        ECInteger,  "Handler_OEE", "iN14_16_IPSCInterval",                      bShow, bEnable, bReadFromFile, 2,     false, 1,       100);
        elConfig->Add(edtN14_17,        &IniConfig.iN14_17_AmbientULTemp,       ECInteger,  "Handler_OEE", "iN14_17_AmbientULTemp",                     bShow, bEnable, bReadFromFile, 39,     false, 1,       100);
        elConfig->Add(cbN14_18,         &IniConfig.bN14_18_EnableTempOffset,    ECBool,     "Handler_OEE", "bN14_18_EnableTempOffset",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20200806 : 溫度 By Servo
        elConfig->Add(edtN14_18,        &IniConfig.asN14_18_TempOffsetPath,     ECText,     "Handler_OEE", "asN14_18_TempOffsetPath",                   bShow, bEnable, bReadFromFile, "\\Handler_data\\Taj_offset_Tool");
        elConfig->Add(cbN14_19,         &IniConfig.bN14_19_TrayMappingToFTP,    ECBool,     "Handler_OEE", "bN14_19_TrayMappingToFTP",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20201209 : 增加資料上傳
        elConfig->Add(edtN14_19,        &IniConfig.asN14_19_TrayMappingPath,    ECText,     "Handler_OEE", "asN14_19_TrayMappingPath",                  bShow, bEnable, bReadFromFile, "\\TrayMapping\\");
        elConfig->Add(cbN14_20_1,&IniConfig.bN14_20_DefaultRecipeChangeLogCycleRecord,ECBool,"Handler_OEE", "bN14_20_DefaultRecipeChangeLogCycleRecord",bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(cbN14_20,  &IniConfig.bN14_20_DefaultRecipeChangeLog,     ECBool,     "Handler_OEE", "bN14_20_DefaultRecipeChangeLog",            bShow, bEnable, bReadFromFile, 0);  //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(rgN14_20_1,       &IniConfig.iN14_20_CycleTime,           ECInteger,  "Handler_OEE", "iN14_20_CycleTime",                         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN14_20,         &IniConfig.asN14_20_ChangeLogPath,      ECText,     "Handler_OEE", "asN14_20_DefaultRecipeChangeLogPath",       bShow, bEnable, bReadFromFile, "\\RecipeChangeLog\\");
        elConfig->Add(cbN14_21,         &IniConfig.bN14_21_SetUpConfiguration,  ECBool,     "Handler_OEE", "bN14_21_SetUpConfiguration",                bShow, bEnable, bReadFromFile, 0);  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(edN14_21,         &IniConfig.asN14_21_SetUpConfiguration, ECText,     "Handler_OEE", "asN14_21_SetUpConfiguration",               bShow, bDisable, bFixedValue, "//SetupFile_List//SiteMap//HT9046//");   //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(edN14_21_BC,      &IniConfig.asN14_21_BinCategory,        ECText,     "Handler_OEE", "asN14_21_BinCategory",                      bShow, bDisable, bFixedValue, "//SetupFile_List//BinCategory//HT9046//");  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(edN14_21_HP,      &IniConfig.asN14_21_HP_SetUpConfig,     ECText,     "Handler_OEE", "asN14_21_HP_SetUpConfiguration",            bShow, bDisable, bFixedValue, "//SetupFile_List//Other//HT90XX-HotPlateForm//");  //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(edN14_21_TF,      &IniConfig.asN14_21_TF_SetUpConfig,     ECText,     "Handler_OEE", "asN14_21_TF_SetUpConfiguration",            bShow, bDisable, bFixedValue, "//SetupFile_List//TrayForm//");               //JimmyChiu 20220303 : Add class SetUpConfiguration
        elConfig->Add(cbN14_22,         &IniConfig.bN14_21_ConfigUpdateFromServerExport,    ECBool,   "Handler_OEE", "bN14_21_ConfigUpdateFromServerExport",   bShow, bEnable, bReadFromFile, 0);  //JimmyChiu 20230410 : Config update from server
        elConfig->Add(edtN14_22Exp,     &IniConfig.asN14_22_ConfigUpdateFromServerExport,   ECText,   "Handler_OEE", "asN14_22_ConfigUpdateFromServerExport",  bShow, bEnable, bReadFromFile, "//handler_data//Config//export//");             //JimmyChiu 20230410 : Config update from server
        elConfig->Add(edtN14_22Imp,     &IniConfig.asN14_22_ConfigUpdateFromServerImport,   ECText,   "Handler_OEE", "asN14_22_ConfigUpdateFromServerImport",  bShow, bEnable, bReadFromFile, "//handler_datas//Config//import//");             //JimmyChiu 20230410 : Config update from server
        elConfig->Add(cb14_23,          &IniConfig.bN14_23_ReadTextFileforPassword,         ECBool,   "Handler_OEE",             "bN14_23_ReadTextFileforPassword",           bShow, bEnable, bReadFromFile, 0);                                              //JimmyChiu 20240115 : Read Text File for Password
        elConfig->Add(cb14_24,          &IniConfig.bN14_24_DynaMultiContinuPassSocket,      ECBool,   "Handler_OEE",             "bN14_24_DynaMultiContinuPassSocket",        bShow, bEnable, bReadFromFile, 0);                                              //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
        elConfig->Add(ed14_24,          &IniConfig.iN14_24_DyMultiPassPower,                ECInteger,"Handler_OEE",             "iN14_24_DyMultiPassPower",                  bShow, bEnable, bReadFromFile, 5);                                              //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
    }
    else
    {
        tsN14->TabVisible=false;
        elConfig->Add(cbN14_1,  &IniConfig.bN14_1_EnableOEEFunction,            ECBool,     "Handler_OEE", "N14_HandlerOEEUseFunction",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_2,  &IniConfig.bN14_2_OEEUseSaveProdData,           ECBool,     "Handler_OEE", "N14_HandlerOEEUseSaveProductionDataToPath", bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_3,  &IniConfig.bN14_3_OEEFTPUpload,                 ECBool,     "Handler_OEE", "N14_HandlerOEEFTPUpload",                   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_4,  &IniConfig.bN14_4_OEEAutoLoadMOFile,            ECBool,     "Handler_OEE", "N14_HandlerOEEAutoLoadMOFile",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_7,  &IniConfig.bN14_7_AutoMotive,                   ECBool,     "Handler_OEE", "bN14_7_AutoMotive",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_8,  &IniConfig.bN14_8_ULSetup,                      ECBool,     "Handler_OEE", "bN14_8_ULSetup",                            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_9,  &IniConfig.bN14_9_ULQtyReport,                  ECBool,     "Handler_OEE", "bN14_9_ULQtyReport",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_10, &IniConfig.bN14_10_DownFileByMO,                ECBool,     "Handler_OEE", "N14_AutoDownloadSetupFileByMO",             bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_11, &IniConfig.bN14_11_CheckSiteMapByMO,            ECBool,     "Handler_OEE", "bN14_11_CheckSiteMapByMO",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_12, &IniConfig.bN14_12_ULTempLogToFTP,              ECBool,     "Handler_OEE", "bN14_12_ULTempLogToFTP",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_13, &IniConfig.bN14_13_ULBinQtyToFTP,               ECBool,     "Handler_OEE", "bN14_13_ULBinQtyToFTP",                     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_14, &IniConfig.bN14_14_AlarmCtrlMachine,            ECBool,     "Handler_OEE", "bN14_14_AlarmCtrlMachine",                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_15, &IniConfig.bN14_15_SocketLifeTime,              ECBool,     "Handler_OEE", "bN14_15_SocketLifeTime",                    bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_16, &IniConfig.bN14_16_EnableIPSC,                  ECBool,     "Handler_OEE", "bN14_16_EnableIPSC",                        bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN14_18, &IniConfig.bN14_18_EnableTempOffset,            ECBool,     "Handler_OEE", "bN14_18_EnableTempOffset",                  bNoShow, bDisable, bFixedValue, 0); //Sam 20200806 : 溫度 By Servo
        elConfig->Add(cbN14_19, &IniConfig.bN14_19_TrayMappingToFTP,            ECBool,     "Handler_OEE", "bN14_19_TrayMappingToFTP",                  bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : 增加資料上傳
        elConfig->Add(cbN14_20, &IniConfig.bN14_20_DefaultRecipeChangeLog,      ECBool,     "Handler_OEE", "bN14_20_DefaultRecipeChangeLog",            bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(cbN14_20_1,&IniConfig.bN14_20_DefaultRecipeChangeLogCycleRecord,ECBool,"Handler_OEE", "bN14_20_DefaultRecipeChangeLogCycleRecord",bNoShow, bDisable, bFixedValue, 0); //Sam 20201209 : Default Recipe ChangeLog
        elConfig->Add(cbN14_21, &IniConfig.bN14_21_SetUpConfiguration,          ECBool,     "Handler_OEE", "bN14_21_SetUpConfiguration",                bNoShow, bDisable, bFixedValue, 0); //JimmyChiu 20220303 : Add class SetUpConfiguration
    }

    if(CUSTOMER_CODE==CC_Greatek)                                               //Sam 20220922 : 改用 elConfig 方式
    {
        tsN15->TabVisible=true;
        elConfig->Add(cbN15_1,          &IniConfig.bN15UserLevelByTxt,          ECBool,     "ESD_Control", "N15_ESDControlUserLevelByTxt",              bShow, bEnable, bReadFromFile, 0);  //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(cbN15_2,          &IniConfig.bN15UseESDControlMachine,    ECBool,     "ESD_Control", "N15_ESDControlUseMachine",                  bShow, bEnable, bReadFromFile, 0);  //Sam 20171124 (Steven) : 功能開關統一方式
        elConfig->Add(edtN15_1,         &IniConfig.asN15UserLevelByTxtReadFilePath,         ECText,   "ESD_Control",        "N15_ESDControlUserLevelByTxtReadFilePath", bShow, bEnable, bReadFromFile, "\\PASSWORD\\");
        elConfig->Add(edtN15_2,         &IniConfig.asN15UseESDControlMachineReadFilePath,   ECText,   "ESD_Control",        "N15_ESDControlUseMachineReadFilePath",     bShow, bEnable, bReadFromFile, "\\FT_ESD\\");
        elConfig->Add(edN15_UserName,   &IniConfig.asN15ESDFTP_UserName,                    ECText,   "ESD_Control",        "N15_ESDControlFTP_UserName",               bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(edN15_Password,   &IniConfig.asN15ESDFTP_Password,                    ECPassword,"ESD_Control",       "N15_ESDControlFTP_Password",               bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN15_Host,       &IniConfig.asN15ESDFTP_Host,                        ECText,   "ESD_Control",        "N15_ESDControlFTP_Host",                   bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(edN15_3,          &IniConfig.asN15ESDControlMachineSaveRecordFilePath,ECText,   "ESD_Control",        "N15_ESDControlMachineSaveRecordFilePath",  bShow, bEnable, bReadFromFile, "D:\\ESDControlLog");
        elConfig->Add(edtN15_4,         &IniConfig.asN15HandlerAUTOMOTIVEDownloadPath,      ECText,   "ESD_Control",        "N15_HandlerAUTOMOTIVEDownloadPath",        bShow, bEnable, bReadFromFile, "\\AUTOMOTIVE_FILE\\");
    }
    else
    {
        tsN15->TabVisible=false;
        elConfig->Add(cbN15_1,  &IniConfig.bN15UserLevelByTxt,                  ECBool,     "ESD_Control", "N15_ESDControlUserLevelByTxt",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN15_2,  &IniConfig.bN15UseESDControlMachine,            ECBool,     "ESD_Control", "N15_ESDControlUseMachine",                  bNoShow, bDisable, bFixedValue, 0);          //JimmyChiu 20220303 : Add class SetUpConfiguration
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20170821 offset ftp
    {
        tsN16->TabVisible=true;
        elConfig->Add(chkN16,  &IniConfig.bEnableOffsetFTP,                     ECBool,     "N16", "bEnableOffsetFTP",                                  bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edN16_UserName,   &IniConfig.cN16FtpUserName,             ECText,     "N16", "cN16FtpUserName",                                   bShow, bEnable, bReadFromFile, "HONPREC");
        elConfig->Add(edN16_Password,   &IniConfig.cN16FtpPassword,             ECPassword, "N16", "cN16FtpPassword",                                   bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN16_HostName,   &IniConfig.cN16FtpHost,                 ECText,     "N16", "cN16FtpHost",                                       bShow, bEnable, bReadFromFile, "127.0.0.1");
        elConfig->Add(edN16_DownPath,   &IniConfig.cN16FtpDownloadPath,         ECText,     "N16", "cN16FtpDownloadPath",                               bShow, bEnable, bReadFromFile, "D:\\ESDControlLog");
        elConfig->Add(edN16_UpLdPath,   &IniConfig.cN16FtpUplaodPath,           ECText,     "N16", "cN16FtpUplaodPath",                                 bShow, bEnable, bReadFromFile, "\\AUTOMOTIVE_FILE\\");
    }
    else
    {
        tsN16->TabVisible=false;
        elConfig->Add(chkN16,  &IniConfig.bEnableOffsetFTP,                     ECBool,     "N16", "bEnableOffsetFTP",                                  bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bSPILFunction==true ||                                         //JerryYang 20220923 : [N17] upload lot summary
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20230131 : add Qualcomm
    {
        tsN17->TabVisible=true;
        elConfig->Add(cbN17_1,          &IniConfig.bN17UploadLotSummary,        ECBool,     "Lot_Summary",      "bN17UploadLotSummary",                 bShow, bEnable, bReadFromFile, 0);  //JerryYang 20220923 : [N17] upload lot summary
        elConfig->Add(edN17_2,          &IniConfig.asN17LotSummaryPath,         ECText,     "Lot_Summary",      "asN17LotSummaryPath",                  bShow, bEnable, bReadFromFile, "\\\\handler-hs\\HANDLER_TEMP\\RAWDATA\\JOBEND_LOG\\");  //JerryYang 20220923 : [N17] upload lot summary
        elConfig->Add(cbN17_3,          &IniConfig.bN17UploadProdLog,           ECBool,     "Production_Log",   "bN17UploadProdLog",                    bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN17_4,          &IniConfig.asN17ProductionLogPath,      ECText,     "Production_Log",   "asN17ProductionLogPath",               bShow, bEnable, bReadFromFile, "D:\\RMS\\");
    }
    else
    {
        tsN17->TabVisible=false;
        elConfig->Add(cbN17_1,          &IniConfig.bN17UploadLotSummary,        ECBool,     "Lot_Summary",      "bN17UploadLotSummary",                 bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN17_3,          &IniConfig.bN17UploadProdLog,           ECBool,     "Production_Log",   "bN17UploadProdLog",                    bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(chkN20,               &IniConfig.bN20_CheckMD5,               ECBool,     "NETWORK", "bN20_CheckMD5",                                 bShow, bEnable, bReadFromFile, 0);      //Steven 20170927 (wei) : 比對工作檔的檢查碼是否正確

    if(CosFunction.bHandlerStateChangeUploadServer)                             //Sam 20230511 : 機台資料變更後須上傳 FTP
    {
        tsN21->TabVisible=true;
        elConfig->Add(chkN21_1, &IniConfig.bN21_HandlerChangeStateUploadServer, ECBool,     "bHandlerStateChangeUploadServer", "bN21_HandlerChangeStateUploadServer",bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN21_1,  &IniConfig.sN21_FTPUserName,                    ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPUserName",     bShow, bEnable, bReadFromFile, "SGPHANDLER");
        elConfig->Add(edN21_2,  &IniConfig.sN21_FTPPassword,                    ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPPassword",     bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(edN21_3,  &IniConfig.sN21_FTPHost,                        ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPHost",         bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(edN21_4,  &IniConfig.sN21_FTPUploadPath,                  ECText,     "bHandlerStateChangeUploadServer", "sN21_FTPUploadPath", bShow, bEnable, bReadFromFile, "\\Handler\\HandlerStateChange\\");
    }
    else
    {
        tsN21->TabVisible=false;
        elConfig->Add(chkN21_1, &IniConfig.bN21_HandlerChangeStateUploadServer, ECBool,     "bHandlerStateChangeUploadServer", "bN21_HandlerChangeStateUploadServer",bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //Steven 20181224 : For ASE-CL
    {
        tsN22->TabVisible=true;
        elConfig->Add(cbN22_EveltLog,       &IniConfig.bN22Enable_EventLog,     ECBool,     "ASECL_FTP", "bN22Enable_EventLog",                         bShow, bEnable, bReadFromFile, 0);      //Steven 20181224 : For ASE-CL
        elConfig->Add(cbN22,                &IniConfig.bN22Enable_ASE_CL_FTP,   ECBool,     "ASECL_FTP", "bN22Enable_ASE_CL_FTP",                       bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(ed_N22_UserName,      &IniConfig.sN22ASE_CL_FTPUserName,  ECText,     "ASECL_FTP", "sN22ASE_CL_FTPUserName",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(ed_N22_Password,      &IniConfig.sN22ASE_CL_FTPPassword,  ECPassword, "ASECL_FTP", "sN22ASE_CL_FTPPassword",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(ed_N22_HostAddress,   &IniConfig.sN22ASE_CL_FTPHost,      ECText,     "ASECL_FTP", "sN22ASE_CL_FTPHostc",                         bShow, bEnable, bReadFromFile, "");
        elConfig->Add(ed_N22_DownloadPath,  &IniConfig.sN22ASE_CL_FTPDownlaodPath, ECText,  "ASECL_FTP", "sN22ASE_CL_FTPDownlaodPath",                  bShow, bEnable, bReadFromFile, "");

        cbConfig_byRecipe->Add(cbN23,                    &IniConfig.bN23UploadJHT_Log,           ECBool,  "ASECL_FTP", "bN23UploadJHT_Log",             bShow, bEnable, bReadFromFile, 1);
        cbConfig_byRecipe->Add(cbN35,                    &IniConfig.bN35UploadJHT_Log,           ECBool,  "ASECL_FTP", "bN35UploadJHT_Log",             bShow, bEnable, bReadFromFile, 1);
        elConfig_byRecipe->Add(ed_N22_1_UserName,        &IniConfig.sN22_1ASE_CL_FTPUserName,    ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPUserName",      bShow, bEnable, bReadFromFile, AnsiString("cuauser"));
        elConfig_byRecipe->Add(ed_N22_1_Password,        &IniConfig.sN22_1ASE_CL_FTPPassword,    ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPPassword",      bShow, bEnable, bReadFromFile, AnsiString("cuaasecl"));
        elConfig_byRecipe->Add(ed_N22_1_HostAddress,     &IniConfig.sN22_1ASE_CL_FTPHost,        ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPHost",          bShow, bEnable, bReadFromFile, AnsiString("10.14.97.228"));
        elConfig_byRecipe->Add(ed_N22_1_UploadPath,      &IniConfig.sN22_1ASE_CL_FTPUplaodPath,  ECText,  "ASECL_FTP", "sN22_1ASE_CL_FTPUplaodPath",    bShow, bEnable, bReadFromFile, AnsiString("//TestData//DATA_TRANSFER//FT//UPLOADSPACE//CUA/TRAY_MAP//DATA_IN//"));
        elConfig_byRecipe->Add(ed_N35_1_UserName,        &IniConfig.sN35_1ASE_CL_FTPUserName,    ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPUserName",      bShow, bEnable, bReadFromFile, AnsiString("cuauser"));
        elConfig_byRecipe->Add(ed_N35_1_Password,        &IniConfig.sN35_1ASE_CL_FTPPassword,    ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPPassword",      bShow, bEnable, bReadFromFile, AnsiString("cuaasecl"));
        elConfig_byRecipe->Add(ed_N35_1_HostAddress,     &IniConfig.sN35_1ASE_CL_FTPHost,        ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPHost",          bShow, bEnable, bReadFromFile, AnsiString("10.14.97.228"));
        elConfig_byRecipe->Add(ed_N35_1_UploadPath,      &IniConfig.sN35_1ASE_CL_FTPUplaodPath,  ECText,  "ASECL_FTP", "sN35_1ASE_CL_FTPUplaodPath",    bShow, bEnable, bReadFromFile, AnsiString("//TestData//DATA_TRANSFER//FT//UPLOADSPACE//SQA/TRAY_MAP//DATA_IN//"));
    }
    else if(CUSTOMER_CODE==CC_HANA_MICRON)                                      //Steven 20251007 : Hana Micron Tray Map
    {
        tsN22->TabVisible               =true;
        cbN22_EveltLog->Visible         =false;
        IniConfig.bN22Enable_EventLog   =false;
        IniConfig.bN22Enable_ASE_CL_FTP =false;
        lb_N22_DownloadPath->Caption    ="Upload Path";

        elConfig->Add(cbN22,                &IniConfig.bN22_1_HANA_TrayMapFTP,  ECBool,     "HANA_FTP", "bN22_1_HANA_TrayMapFTP",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20181224 : For ASE-CL
        elConfig->Add(ed_N22_UserName,      &IniConfig.sN22_1_FTPUserName,      ECText,     "HANA_FTP", "sN22_1_FTPUserName",                           bShow, bEnable, bReadFromFile, "traymap");
        elConfig->Add(ed_N22_Password,      &IniConfig.sN22_1_FTPPassword,      ECPassword, "HANA_FTP", "sN22_1_FTPPassword",                           bShow, bEnable, bReadFromFile, "map0206");
        elConfig->Add(ed_N22_HostAddress,   &IniConfig.sN22_1_FTPHost,          ECText,     "HANA_FTP", "sN22_1_FTPHost",                               bShow, bEnable, bReadFromFile, "12.230.55.65");
        elConfig->Add(ed_N22_DownloadPath,  &IniConfig.sN22_1_FTPUploadPath,    ECText,     "HANA_FTP", "sN22_1_FTPUploadPath",                         bShow, bEnable, bReadFromFile, "\\handlerid\\");
    }
    else
    {
        tsN22->TabVisible=false;
        elConfig->Add(cbN22_EveltLog,       &IniConfig.bN22Enable_EventLog,     ECBool,     "ASECL_FTP", "bN22Enable_EventLog",                         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN22,                &IniConfig.bN22Enable_ASE_CL_FTP,   ECBool,     "ASECL_FTP", "bN22Enable_ASE_CL_FTP",                       bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN23,                &IniConfig.bN23UploadJHT_Log,       ECBool,    "ASECL_FTP", "bN23UploadJHT_Log",                           bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN35,                &IniConfig.bN35UploadJHT_Log,       ECBool,    "ASECL_FTP", "bN35UploadJHT_Log",                           bNoShow, bDisable, bFixedValue, 0);
    }

    tsN23->TabVisible  =(CUSTOMER_CODE==CC_Murata || CosFunction.bSortingBy2DList || IniConfig.bSPILFunction==true);
    tsN23_1->TabVisible=(CUSTOMER_CODE==CC_Murata);
    tsN23_2->TabVisible=(CUSTOMER_CODE==CC_Murata);
    tsN23_3->TabVisible=(CUSTOMER_CODE==CC_Murata ||
                        IniConfig.bSPILFunction==true ||                        //JerryYang 20250521 : add
                        CUSTOMER_CODE==CC_SJ_Semiconductor);                    //RogerYang 2026024 : Add for SJSM白名單
    lbledtN23_4_URL->Text=IniConfig.sN23_4_URL;                                 //JerryYang 20241104 : 支援2DID白名單功能
    lbledtN23_5_UploadPath->Text=IniConfig.sN23_5_UploadPath;

    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200409 : Murata 2DID比對功能
    {
        elConfig->Add(chkN23_1,             &IniConfig.bN23_1_Enable2DIDCompare,ECBool,     "Murata Function", "bN23_1_Enable2DIDCompare",              bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkN23_3,             &IniConfig.bN23_3_UploadTestResult, ECBool,     "Murata Function", "bN23_3_UploadTestResult",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(lbledtN23_1_URL,      &IniConfig.sN23_1_URL,              ECText,     "Murata Function", "sN23_1_URL",                            bShow, bEnable, bReadFromFile, "http://172.16.83.190/sm5200/page/TrafficExecute.jsp?pptid=519149&argument=519149.args");
        elConfig->Add(edtN23_2_LineID,      &IniConfig.sN23_2_Line,             ECText,     "Murata Function", "sN23_2_Line",                           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN23_2_ProcessName, &IniConfig.sN23_2_Process,          ECText,     "Murata Function", "sN23_2_Process",                        bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edtN23_2_Product,     &IniConfig.sN23_2_Product,          ECText,     "Murata Function", "sN23_2_Product",                        bShow, bEnable, bReadFromFile, "");
        elConfig->Add(lbledtN23_2_URL,      &IniConfig.sN23_3_URL,              ECText,     "Murata Function", "sN23_3_URL",                            bShow, bEnable, bReadFromFile, "http://172.16.83.190/sm5200/page/TrafficExecute.jsp?pptid=519129&argument=519129.args");
    }
    else
    {
        elConfig->Add(chkN23_1,             &IniConfig.bN23_1_Enable2DIDCompare,ECBool,     "Murata Function", "bN23_1_Enable2DIDCompare",              bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkN23_3,             &IniConfig.bN23_3_UploadTestResult, ECBool,     "Murata Function", "bN23_3_UploadTestResult",               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bSortingBy2DList)
    {
        tsN23->TabVisible=true;
        elConfig->Add(edN23_2UserName,      &IniConfig.cN23FtpUserName,         ECText,     "2DSotingFunction",     "cN23FtpUserName",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2Pwd,           &IniConfig.cN23FtpPassword,         ECPassword, "2DSotingFunction",     "cN23FtpPassword",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2FTPHost,       &IniConfig.cN23FtpHost,             ECText,     "2DSotingFunction",     "cN23FtpHost",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2FTPPath,       &IniConfig.cN23FtpDownloadPath,     ECText,     "2DSotingFunction",     "cN23FtpDownloadPath",              bShow, bEnable, bReadFromFile, "\\");
        elConfig->Add(edN23_3NetDrivePath,  &IniConfig.sN23DownloadDrivePath,   ECText,     "2DSotingFunction",     "sN23DownloadDrivePath",            bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(rgN23_1_2DSorting,    &IniConfig.iN23DownloadMethod,      ECInteger,  "2DSotingFunction",     "iN23DownloadMethod",               bShow, bEnable, bReadFromFile, 0);  //JerryYang 20190313 : 2D sorting

        elConfig->Add(edtN23_4,             &IniConfig.sN23LotInfoPath,         ECText,     "2DSotingFunction",     "sN23LotInfoPath",                  bShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(chkN23_4,             &IniConfig.bN23UseLotInfoFile,      ECBool,     "2DSotingFunction",     "bN23UseLotInfoFile",               bShow, bEnable, bReadFromFile, 0);  //Steven 20240829 : Lot info從檔案讀取
        if(IniConfig.bSPILFunction)
            elConfig->Add(chkN23_5,         &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bShow, bEnable, bReadFromFile, 1);
        else
            elConfig->Add(chkN23_5,         &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bShow, bEnable, bReadFromFile, 0);  //Steven 20240830 : Summary資料夾不要有年月
    }
    else if(CosFunction.bMakeWhite2DIDList==true)                               //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
    {
        tsN23->TabVisible=true;
        elConfig->Add(edN23_2UserName,      &IniConfig.cN23FtpUserName,         ECText,     "2DSotingFunction",     "cN23FtpUserName",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2Pwd,           &IniConfig.cN23FtpPassword,         ECPassword, "2DSotingFunction",     "cN23FtpPassword",                  bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2FTPHost,       &IniConfig.cN23FtpHost,             ECText,     "2DSotingFunction",     "cN23FtpHost",                      bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN23_2FTPPath,       &IniConfig.cN23FtpDownloadPath,     ECText,     "2DSotingFunction",     "cN23FtpDownloadPath",              bShow, bEnable, bReadFromFile, "\\");
        elConfig->Add(edN23_3NetDrivePath,  &IniConfig.sN23DownloadDrivePath,   ECText,     "2DSotingFunction",     "sN23DownloadDrivePath",            bNoShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(rgN23_1_2DSorting,    &IniConfig.iN23DownloadMethod,      ECInteger,  "2DSotingFunction",     "iN23DownloadMethod",               bShow, bDisable, bReadFromFile, 0);  //JerryYang 20190313 : 2D sorting
        IniConfig.iN23DownloadMethod=0;
        elConfig->Add(edtN23_4,             &IniConfig.sN23LotInfoPath,         ECText,     "2DSotingFunction",     "sN23LotInfoPath",                  bNoShow, bEnable, bReadFromFile, IniConfig.asEventLogAutoSavePath);
        elConfig->Add(chkN23_4,             &IniConfig.bN23UseLotInfoFile,      ECBool,     "2DSotingFunction",     "bN23UseLotInfoFile",               bNoShow, bEnable, bReadFromFile, 0);  //Steven 20240829 : Lot info從檔案讀取
        elConfig->Add(chkN23_5,             &IniConfig.bN25FolderWithoutYYMM,   ECBool,     "2DSotingFunction",     "bN25FolderWithoutYYMM",            bNoShow, bEnable, bReadFromFile, 0);  //Steven 20240830 : Summary資料夾不要有年月
    }
    else
    {
        tsN23->TabVisible=false;
    }

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20200409 : JSCC RTM功能
    {
        tsN24->TabVisible=true;
        elConfig->Add(chk24,               &IniConfig.bN24_EnableRTM,           ECBool,     "RTM Function", "bN24_EnableRTM",                           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(lbledtN24,           &IniConfig.iN24_RTMPort,             ECPort,     "Murata Function",      "iN24_RTMPort",                     bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        tsN24->TabVisible=false;
        elConfig->Add(chk24,               &IniConfig.bN24_EnableRTM,           ECBool,     "RTM Function", "bN24_EnableRTM",                           bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(lbledtN23_4_URL,          &IniConfig.sN23_4_URL,              ECText,     "2DID Search Function", "sN23_4_URL",                       bShow, bEnable, bReadFromFile, "D:\\RMS\\");  //JerryYang 20250521 : add
    elConfig->Add(lbledtN23_5_UploadPath,   &IniConfig.sN23_5_UploadPath,       ECText,     "2DID White list",      "sN23_5_UploadPath",                bShow, bEnable, bReadFromFile, "D:\\RMS\\");

    if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                        //Steven 20210413 : 南茂的自動Start功能
    {
        tsN25->TabVisible=true;
        elConfig->Add(chkN25_1,         &IniConfig.bN25_1_EnableStartControl,   ECBool,     "ChipMos Function", "bN25_1_EnableStartControl",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkN25_2,         &IniConfig.bN25_2_EnableUploadLog,      ECBool,     "ChipMos Function", "bN25_2_EnableUploadLog",               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN25_1_Name,        &IniConfig.sN25_1_FTPUserName,      ECText,     "ChipMos Function",     "sN25_1_FTPUserName",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(edtN25_1_Password,    &IniConfig.sN25_1_FTPPassword,      ECPassword, "ChipMos Function",     "sN25_1_FTPPassword",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(edtN25_1_Host,        &IniConfig.sN25_1_FTPHost,          ECText,     "ChipMos Function",     "sN25_1_FTPHost",                   bShow, bEnable, bReadFromFile, "10.20.50.3");
        elConfig->Add(edtN25_1_Path,        &IniConfig.sN25_1_FTPPath,          ECText,     "ChipMos Function",     "sN25_1_FTPPath",                   bShow, bEnable, bReadFromFile, "/summary/Handler/TesterStatus/");
        elConfig->Add(edtN25_2_Name,        &IniConfig.sN25_2_FTPUserName,      ECText,     "ChipMos Function",     "sN25_2_FTPUserName",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(edtN25_2_Password,    &IniConfig.sN25_2_FTPPassword,      ECPassword, "ChipMos Function",     "sN25_2_FTPPassword",               bShow, bEnable, bReadFromFile, "handler");
        elConfig->Add(edtN25_2_Host,        &IniConfig.sN25_2_FTPHost,          ECText,     "ChipMos Function",     "sN25_2_FTPHost",                   bShow, bEnable, bReadFromFile, "10.20.50.3");
        elConfig->Add(edtN25_2_Path,        &IniConfig.sN25_2_FTPPath,          ECText,     "ChipMos Function",     "sN25_2_FTPPath",                   bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(edtN25_2_Interval,    &IniConfig.iN25_2_UploadInterval,   ECInteger,  "ChipMos Function",     "iN25_2_UploadInterval",            bShow, bEnable, bReadFromFile, 10,    false, 1,       100);
        elConfig->Add(chkN25_3,             &IniConfig.bN25_3_EnableULJamLog,   ECBool,     "ChipMos Function",     "bN25_3_EnableULJamLog",            bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(edtN25_3_LogJamPath,  &IniConfig.sN25_3_JamLogFTPPath,    ECText,     "ChipMos Function",     "sN25_3_JamLogFTPPath",             bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(chkN25_4,             &IniConfig.bN25_4_EnableUpload,     ECBool,     "ChipMos Function",     "bN25_4_EnableUpload",              bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(edtN25_4_UploadPath,  &IniConfig.sN25_4_UploadPath,       ECText,     "ChipMos Function",     "sN25_4_UploadPath",                bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
        elConfig->Add(chkN25_5,             &IniConfig.bN25_5_EnableUpload,     ECBool,     "ChipMos Function",     "bN25_5_EnableUpload",              bShow, bEnable, bReadFromFile, 0);                      //JimmyChiu 20241009 : for 南茂Jam List上傳
        elConfig->Add(edtN25_5_UploadPath,  &IniConfig.sN25_5_UploadPath,       ECText,     "ChipMos Function",     "sN25_5_UploadPath",                bShow, bEnable, bReadFromFile, "/Summary/naslfs2/Handler/");
    }
    else
    {
        tsN25->TabVisible=false;
        elConfig->Add(chkN25_1,     &IniConfig.bN25_1_EnableStartControl,       ECBool,     "ChipMos Function", "bN25_1_EnableStartControl",            bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkN25_2,     &IniConfig.bN25_2_EnableUploadLog,          ECBool,     "ChipMos Function", "bN25_2_EnableUploadLog",               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseJamRawData)                                              //Sam 20220922 : 改用 elConfig 方式 //Sam 20210224 : Auto Upload FTP JAMRawData 功能
    {
        tsN26->TabVisible=true;
        elConfig->Add(chkN26_1,     &IniConfig.bN26_UseJamRawDataUpdataToFTP,   ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataUpdataToFTP",   bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(chkN26_2,     &IniConfig.bN26_UseJamRawDataRecord,        ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataRecord",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN26_1,     &IniConfig.sN26_FTPUserName,                ECText,     "JamRawDataUpdataToFTP","sN26_FTPUserName",                 bShow, bEnable, bReadFromFile,  "SGPHANDLER");          //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(edtN26_2,     &IniConfig.sN26_FTPPassword,                ECPassword, "JamRawDataUpdataToFTP","sN26_FTPPassword",                 bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(edtN26_3,     &IniConfig.sN26_FTPHost,                    ECText,     "JamRawDataUpdataToFTP","sN26_FTPHost",                     bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(edtN26_4,     &IniConfig.sN26_FTPUplaodPath,              ECText,     "JamRawDataUpdataToFTP","sN26_FTPUplaodPath",               bShow, bEnable, bReadFromFile,  "\\HT_JAM\\HT-9046HA-8H\\");
    }
    else
    {
        tsN26->TabVisible=false;
        elConfig->Add(chkN26_1,     &IniConfig.bN26_UseJamRawDataUpdataToFTP,   ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataUpdataToFTP",   bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(chkN26_2,     &IniConfig.bN26_UseJamRawDataRecord,        ECBool,     "JamRawDataUpdataToFTP", "bN26_UseJamRawDataRecord",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUseAlarmLogXml)                                             //Sam 20220922 : 改用 elConfig 方式 Mark //Sam 20210621 : 新增 Event Log xml  Updata
    {
        tsN27->TabVisible=true;
        elConfig->Add(cbN27_1,      &IniConfig.bN27_UseAlarmLogXmlUpdataToFTP,  ECBool,     "bUseAlarmLogXml", "bN27_UseAlarmLogXmlUpdataToFTP",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN27_1,      &IniConfig.sN27_FTPUserName,                ECText,     "bUseAlarmLogXml", "sN27_FTPUserName",                      bShow, bEnable, bReadFromFile,  "SGPHANDLER");          //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(edN27_2,      &IniConfig.sN27_FTPPassword,                ECPassword, "bUseAlarmLogXml", "sN27_FTPPassword",                      bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(edN27_3,      &IniConfig.sN27_FTPHost,                    ECText,     "bUseAlarmLogXml", "sN27_FTPHost",                          bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(edN27_4,      &IniConfig.sN27_FTPUplaodPath,              ECText,     "bUseAlarmLogXml", "sN27_FTPUplaodPath",                    bShow, bEnable, bReadFromFile,  "\\HT_JAM\\HT-9046HA-8H\\");
        elConfig->Add(edN27_6,      &IniConfig.sN27_TesterID,                   ECText,     "bUseAlarmLogXml", "sN27_TesterID",                         bShow, bEnable, bReadFromFile,  "");
    }
    else
    {
        tsN27->TabVisible=false;
        elConfig->Add(cbN27_1,      &IniConfig.bN27_UseAlarmLogXmlUpdataToFTP,  ECBool,     "bUseAlarmLogXml", "bN27_UseAlarmLogXmlUpdataToFTP",        bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_SCK)                                                   //Steven 20210608 : JSCK OEE Function.
    {
        tsN28->TabVisible=true;
        elConfig->Add(chkN28,       &IniConfig.bN28_SCK_OEE,                    ECBool,     "JSCK Function",        "bN26_SCK_OEE",                     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN28_Path,  &IniConfig.sN28_Path,                       ECText,     "JSCK Function",        "sN26_Path",                        bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\TestLog\\");
        elConfig->Add(edtN28_IP,    &IniConfig.sN28_IP,                         ECText,     "JSCK Function",        "sN26_IP",                          bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        tsN28->TabVisible=false;
        elConfig->Add(chkN28,       &IniConfig.bN28_SCK_OEE,                    ECBool,     "JSCK Function",        "bN26_SCK_OEE",                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_GM_TEST)                                               //Steven 20220311 : GM Test工作檔比對功能
    {
        tsN29->TabVisible=true;
        elConfig->Add(chkN29,       &IniConfig.bN29_ParameterCheckForGMTest,    ECBool,     "N29", "bN29_ParameterCheck",                               bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edtN29,       &IniConfig.sN29_FilePath,                   ECText,     "N29", "sN29_FilePath",                                     bShow, bEnable, bReadFromFile, "");
    }
    else
    {
        tsN29->TabVisible=false;
        elConfig->Add(chkN29,       &IniConfig.bN29_ParameterCheckForGMTest,    ECBool,     "N29", "bN29_ParameterCheck",                               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bRecordGroundESDByTestIC)                                    //Sam 20220922 : 改用 elConfig 方式   //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    {
        tsN30->TabVisible=true;
        elConfig->Add(cbN30_1,  &IniConfig.bN30_UseGroundESDUpdataToFTP,        ECBool,     "bRecordGroundESDByTestIC", "bN30_UseGroundESDUpdataToFTP", bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN30_1,  &IniConfig.sN30_FTPUserName,                    ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUserName",             bShow, bEnable, bReadFromFile,  "SGPHANDLER");      //Sam 20250122 : 客戶要求預設
        elConfig->Add(edN30_2,  &IniConfig.sN30_FTPPassword,                    ECPassword, "bRecordGroundESDByTestIC", "sN30_FTPPassword",             bShow, bEnable, bReadFromFile,  "CHROMA");
        elConfig->Add(edN30_3,  &IniConfig.sN30_FTPHost,                        ECText,     "bRecordGroundESDByTestIC", "sN30_FTPHost",                 bShow, bEnable, bReadFromFile,  "192.168.119.243");
        elConfig->Add(edN30_4,  &IniConfig.sN30_FTPUploadPath,                  ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUplaodPath",           bShow, bEnable, bReadFromFile,  "\\Handler\\GroundESDLog\\");
        elConfig->Add(edN30_5,  &IniConfig.sN30_FTPUploadPath2,                 ECText,     "bRecordGroundESDByTestIC", "sN30_FTPUplaodPath2",          bShow, bEnable, bReadFromFile,  "\\Handler\\GroundESDLog2\\");
    }
    else
    {
        tsN30->TabVisible=false;
        elConfig->Add(cbN30_1,  &IniConfig.bN30_UseGroundESDUpdataToFTP,        ECBool,     "bRecordGroundESDByTestIC", "bN30_UseGroundESDUpdataToFTP", bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bAutoTmpeOfsByFTP)                                           //Sam 20220922 : 改用 elConfig 方式   //Sam 20220406 : 溫度自動補償功能 By FTP
    {
        tsN31->TabVisible=true;
        elConfig->Add(cbN31_1,  &IniConfig.iN31_UseAutoTempOfsByFTP,            ECInteger,  "bAutoTmpeOfsByFTP", "bN31_UseAutoTempOfsByFTP",            bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN31_1,  &IniConfig.sN31_FTPUserName,                    ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPUserName",                    bShow, bEnable, bReadFromFile, "SGPHANDLER");           //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(edN31_2,  &IniConfig.sN31_FTPPassword,                    ECPassword, "bAutoTmpeOfsByFTP", "sN31_FTPPassword",                    bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(edN31_3,  &IniConfig.sN31_FTPHost,                        ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPHost",                        bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(edN31_4,  &IniConfig.sN31_FTPDownloadPath,                ECText,     "bAutoTmpeOfsByFTP", "sN31_FTPDownloadPath",                bShow, bEnable, bReadFromFile, "\\Handler\\AutoTempOfs\\");
        elConfig->Add(edN31_5,  &IniConfig.iN31_ContactCnt,                     ECInteger,  "bAutoTmpeOfsByFTP", "iN31_ContactCnt",                     bShow, bEnable, bReadFromFile, 10,    false, 1,       100);
        elConfig->Add(edN31_MinOffset,  &IniConfig.dN31_MinOffset,ECDouble, "bAutoTmpeOfsByFTP",            "dN31_MinOffset",       bShow, bEnable, bReadFromFile, -30.0,   false, -30.0,   30.0);                               //Jimmychiu 20241226 : add N31 temp offset limit
        elConfig->Add(edN31_MaxOffset,  &IniConfig.dN31_MaxOffset,ECDouble, "bAutoTmpeOfsByFTP",            "dN31_MaxOffset",       bShow, bEnable, bReadFromFile, 30.0,    false, -30.0,   30.0);
        if(CUSTOMER_CODE==CC_UTAC_TW)                                           //Sam 20230815 : 聯測改為 Interval time
            labN31_5->Caption="Intetval Time";
    }
    else
    {
        tsN31->TabVisible=false;
        elConfig->Add(cbN31_1,  &IniConfig.iN31_UseAutoTempOfsByFTP,            ECInteger,  "bAutoTmpeOfsByFTP", "bN31_UseAutoTempOfsByFTP",         bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bDownloadUpdateAutomatically)                                //Sam 20220922 : 改用 elConfig 方式 //Sam 20220824 : FTP 自動下載安裝更新包
    {
        tsN32->TabVisible=true;
        elConfig->Add(cbN32_1,  &IniConfig.bN32_DownloadUpdatesAutomatically,   ECBool,     "bDownloadUpdateAutomatically", "bN32_DownloadUpdatesAutomatically",bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbN32_2,  &IniConfig.bN32_CheckForUpdatesOnceDay,         ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckForUpdatesOnceDay;",     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbN32_3,  &IniConfig.bN32_CheckAtInitailStart,            ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtInitailStart;",        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbN32_4,  &IniConfig.bN32_CheckAtTrayFeedFinish,          ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtTrayFeedFinish;",      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN32_1,  &IniConfig.sN32_FTPUserName,                    ECText,     "bDownloadUpdateAutomatically", "sN32_FTPUserName",                 bShow, bEnable, bReadFromFile, "SGPHANDLER");           //Sam 20220831 : 矽格北興俊堯要求修改預設值
        elConfig->Add(edN32_2,  &IniConfig.sN32_FTPPassword,                    ECPassword, "bDownloadUpdateAutomatically", "sN32_FTPPassword",                 bShow, bEnable, bReadFromFile, "CHROMA");
        elConfig->Add(edN32_3,  &IniConfig.sN32_FTPHost,                        ECText,     "bDownloadUpdateAutomatically", "sN32_FTPHost",                     bShow, bEnable, bReadFromFile, "192.168.119.243");
        elConfig->Add(edN32_4,  &IniConfig.sN32_FTPDownloadPath,                ECText,     "bDownloadUpdateAutomatically", "sN32_FTPDownloadPath",             bShow, bEnable, bReadFromFile, "\\Handler\\Installer\\");
        elConfig->Add(edN32_5,  &IniConfig.sN32_NetDownloadPath,                ECText,     "bDownloadUpdateAutomatically", "sN32_NetDownloadPath",             bShow, bEnable, bReadFromFile, "D:\\RMS\\");            //Steven 20221216 : 使用網路硬碟下載安裝包
        elConfig->Add(edN32_6,  &IniConfig.sN32_FTPDownloadPath2,               ECText,     "bDownloadUpdateAutomatically", "sN32_FTPDownloadPath2",            bShow, bEnable, bReadFromFile, "\\Handler\\InstallerForAll\\"); //Sam 20230328 : 自動更新增加版本檢查
        elConfig->Add(rgN32,    &IniConfig.iN32_DownloadMode,                   ECInteger,  "bDownloadUpdateAutomatically", "iN32_DownloadMode",                bShow, bEnable, bReadFromFile, 0);                      //Steven 20221216 : 使用網路硬碟下載安裝包
    }
    else
    {
        tsN32->TabVisible=false;
        elConfig->Add(cbN32_1,  &IniConfig.bN32_DownloadUpdatesAutomatically,   ECBool,     "bDownloadUpdateAutomatically", "bN32_DownloadUpdatesAutomatically",bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN32_2,  &IniConfig.bN32_CheckForUpdatesOnceDay,         ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckForUpdatesOnceDay",      bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN32_3,  &IniConfig.bN32_CheckAtInitailStart,            ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtInitailStart",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN32_4,  &IniConfig.bN32_CheckAtTrayFeedFinish,          ECBool,     "bDownloadUpdateAutomatically", "bN32_CheckAtTrayFeedFinish",       bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbN35_1,      &IniConfig.bN35_Ground_ESD_Upload,              ECBool,     "Rround_ESD_Upload",       "bN35_Ground_ESD_Upload",    bShow, bEnable, bReadFromFile, 0);                          //Sam 20250609 : Record Ground and ESD at intervals and upload
    elConfig->Add(edN35_1,      &IniConfig.sN35_FTPUserName,                    ECText,     "Rround_ESD_Upload",       "sN35_FTPUserName",          bShow, bEnable, bReadFromFile,  "SGPHANDLER");
    elConfig->Add(edN35_2,      &IniConfig.sN35_FTPPassword,                    ECText,     "Rround_ESD_Upload",       "sN35_FTPPassword",          bShow, bEnable, bReadFromFile,  "CHROMA");
    elConfig->Add(edN35_3,      &IniConfig.sN35_FTPHost,                        ECText,     "Rround_ESD_Upload",       "sN35_FTPHost",              bShow, bEnable, bReadFromFile,  "192.168.119.243");
    elConfig->Add(edN35_4,      &IniConfig.sN35_FTPUploadPath,                  ECText,     "Rround_ESD_Upload",       "sN35_FTPUploadPath",        bShow, bEnable, bReadFromFile,  "\\Handler\\GroundandESD");
    elConfig->Add(rgN35_1,      &IniConfig.iN35_Interval,                       ECInteger,  "Rround_ESD_Upload",       "iN35_Interval",             bShow, bEnable, bReadFromFile, 0);          //Sam 20250609 : Record Ground and ESD at intervals and upload

    if(CUSTOMER_CODE==CC_LEADYO)                                                //KenHsieh 20230502 : 利揚要求上傳OCR + BIN Log上傳至Host
    {
        tsN33->TabVisible=true;
        elConfig->Add(cbN33,    &IniConfig.bN33_UpLoadOCRBinLogByNet,           ECBool,     "LEADYO Function",   "bN33_UpLoadOCRBinLogByNet",     bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN33,    &IniConfig.asN33_UploadLogPath,                 ECText,     "LEADYO Function",   "asN33_UploadLogPath",           bShow, bEnable, bReadFromFile, "");
        elConfig->Add(cbN33_1,  &IniConfig.bN33_1_NetChangeFileAndData,         ECBool,     "LEADYO Function",   "bN33_1_NetChangeFileAndData",   bShow, bEnable, bReadFromFile, 0);             //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    }
    else
    {
        tsN33->TabVisible=false;
        elConfig->Add(cbN33,    &IniConfig.bN33_UpLoadOCRBinLogByNet,           ECBool,     "LEADYO Function",   "bN33_UpLoadOCRBinLogByNet",     bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbN33_1,  &IniConfig.bN33_1_NetChangeFileAndData,         ECBool,     "LEADYO Function",   "bN33_1_NetChangeFileAndData",   bNoShow, bDisable, bFixedValue, 0);            //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    }

    if(CUSTOMER_CODE==CC_CYUEAN)                                                //Jimmychiu 20250324 : CC_CYUEAN OEE report
    {
        tsN34->TabVisible=true;
        elConfig->Add(cbN34,    &IniConfig.bN34_GenerateOEEAlarmRpt,            ECBool,     "N34 Function",   "bN34_GenerateOEEAlarmRpt",         bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(edN34,    &IniConfig.sN34_OEEAlarmRptPath,                ECText,     "N34 Function",   "sN34_OEEAlarmRptPath",             bShow, bEnable, bReadFromFile, "D:\\HT9045_Log\\Product_Loader\\OEEAlarmRpt");
    }
    else
    {
        tsN34->TabVisible=false;
        elConfig->Add(cbN34,    &IniConfig.bN34_GenerateOEEAlarmRpt,            ECBool,     "N34 Function",   "bN34_GenerateOEEAlarmRpt",         bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edN34,    &IniConfig.sN34_OEEAlarmRptPath,                ECText,     "N34 Function",   "sN34_OEEAlarmRptPath",             bNoShow, bDisable, bFixedValue, "D:\\HT9045_Log\\OEEAlarmRpt");
    }
    //==> Eastsun 20260520 整合
    if(USE_BU5_Function==true)
    {
        elConfig->Add(cbN40_1,    &IniConfig.bN40_1_HandlerDataBackUpUseFunction,       ECBool,   "FTPUpLoad",         "bN40_1_HandlerDataBackUpUseFunction",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbN41_1,    &IniConfig.bN41_1_HandlerDataBackUpToDiskUseFunction, ECBool,   "FTPUpLoad",         "bN41_1_HandlerDataBackUpToDiskUseFunction",    bShow, bEnable, bReadFromFile, 0);

        elConfig->Add(edN40_1,    &IniConfig.sN40_FTPUserName,                          ECText,   "LEADYO Function",   "sN40FtpUserName",                              bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN40_2,    &IniConfig.sN40_FTPPassword,                          ECText,   "LEADYO Function",   "sN40_FTPPassword",                             bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN40_3,    &IniConfig.sN40_FTPHost,                              ECText,   "LEADYO Function",   "sN40_FTPHost",                                 bShow, bEnable, bReadFromFile, "");
        elConfig->Add(edN40_4,    &IniConfig.sN40_FTPUploadPath,                        ECText,   "LEADYO Function",   "sN40_FTPUploadPath",                           bShow, bEnable, bReadFromFile, "");

        elConfig->Add(edN41_1,    &IniConfig.sN41_DiskUploadPath,                       ECText,   "LEADYO Function",   "sN41_DiskUploadPath",                          bShow, bEnable, bReadFromFile, "");
    }
    //<== Eastsun 20260520 整合
    }

// AI(W906-FW-CFG-W3) 20260825: golden cConfiguration.cpp:4097-4493, transcribed
// VERBATIM (cp950 -> UTF-8). Registration table only -- the port's
// HTEditList::Add signature is argument-for-argument golden's, so no
// adaptation was needed and none was invented. See FW-CFG-W1's banner in
// forms/fConfiguration.h for why this family is the wave's point.
void TfConfiguration::InitConfigEdtList_ItemP()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;
    AnsiString str;

    lblP04->Visible=(CUSTOMER_CODE==CC_SIGURD_PeiXing);                         //Sam 20211124 : 矽格北興 P04 功能開啟，必須也要開啟 P24 不然會 Hang up，北興俊堯要求顯示提示
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20230529 add:KYEC 要求 P04功能強制關閉
    {
        elConfig->Add(cbP04,    &IniConfig.bP04ColorIsEmptyUnloader,            ECBool,     "Tray", "bP04ColorIsEmptyUnloader",                         bShow, bDisable, bFixedValue, 0);
    }
    else
    {
        elConfig->Add(cbP04,    &IniConfig.bP04ColorIsEmptyUnloader,            ECBool,     "Tray", "bP04ColorIsEmptyUnloader",                         bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(cbP05,        &IniConfig.bP05_LoaderCylinderPreOn,            ECBool,     "Tray", "P05_LoaderCylinderPreOn",                          bShow, bEnable, bReadFromFile, 0);      //Steven 20150429 : 預先打兩下Loader汽缸
    elConfig->Add(cbP05_1,      &IniConfig.bP05_1_UnloadCylinderPreOn,          ECBool,     "Tray", "bP05_1_UnloadCylinderPreOn",                       bShow, bEnable, bReadFromFile, 0);      //Steven 20240215 : 退tray前預先打兩下分離汽缸
    if(IniConfig.bUseTrayBlockMode)                                             //Ifor 20181016 (Steven) : add 避免無使用Tray Block模式的客戶跑Tray Block模式
        elConfig->Add(cbP06,    &IniConfig.bP06_LoaderUseCarrierTray,           ECBool,     "Tray", "P06_LoaderUseCarrierTray",                         bShow, bEnable, bReadFromFile, 0);      //Frank 20160921 add
    else
        elConfig->Add(cbP06,    &IniConfig.bP06_LoaderUseCarrierTray,           ECBool,     "Tray", "P06_LoaderUseCarrierTray",                         bNoShow, bDisable, bFixedValue, 0);     //wei 20161125 不使用Tray Block Mode不顯示，避免被誤選

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        elConfig->Add(cbP07,    &IniConfig.bP07NoTrayaAutoTrayFeed,             ECBool,     "Tray", "bP07NoTrayaAutoTrayFeed",                          bShow, bEnable, bReadFromFile, 0);      //wei 20161125 //wei 20161121 TSMC 新增[P07]Loader No Traya Auto TrayFeed
    else
        elConfig->Add(cbP07,    &IniConfig.bP07NoTrayaAutoTrayFeed,             ECBool,     "Tray", "bP07NoTrayaAutoTrayFeed",                          bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbP08,        &IniConfig.bP08TrayFeedCleanLotID,              ECBool,     "Tray", "bP08TrayFeedCleanLotID",                           bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bCleanOutCanTrayEndBySetupFile)                              //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
        elConfig->Add(cbP09,    &IniConfig.bP09TrayEndCanSelectTray,            ECBool,     "Tray", "bP09TrayEndCanSelectTray",                         bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbP09,    &IniConfig.bP09TrayEndCanSelectTray,            ECBool,     "Tray", "bP09TrayEndCanSelectTray",                         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbP10,    &IniConfig.bP10FixedTrayProposeTheInitialQuestion,  ECBool,     "Tray", "bP10FixedTrayProposeTheInitialQuestion",           bShow, bEnable, bReadFromFile, 0);      //Steven 20100205 Add from HT9080A

    if(CUSTOMER_CODE==CC_KYEC_LEE         ||
       CUSTOMER_CODE==CC_KYEC_XILINX      ||
       CUSTOMER_CODE==CC_TERAPOWER        ||                                    //Sam 20191109 : Add UPH
       CUSTOMER_CODE==CC_SIGURD_ChungXing ||                                    //Sam20210513 : 新增矽格中興
       CUSTOMER_CODE==CC_UTAC_TW          ||
       CUSTOMER_CODE==CC_FOREHOPE_NINGBO  )                                     //Jimmychiu 20250304 : P11新增CC_FOREHOPE_NINGBO
        elConfig->Add(cbP11,    &IniConfig.bP11RecordUPH,                       ECBool,     "Count", "RecordUPH",                                       bShow, bEnable, bReadFromFile, 0);      //Frank 20150515
    else
        elConfig->Add(cbP11,    &IniConfig.bP11RecordUPH,                       ECBool,     "Count", "RecordUPH",                                       bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bKnockerSetBySetupFile==false)                               //Steven 20120510 : 全部改用汽缸的Enable判斷
    {
        if(CosFunction.bHiSiliconFunction)                                      //wei 20160309 強制開啟不能更改  //JerryYang 20191029 STM P14要能選擇
        {
            elConfig->Add(cbP13,    &IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop,  ECBool,     "Tray",     "bEnableAutoTrayEdgePushCylinderLoop",  bShow, bDisable, bFixedValue, 1);
            elConfig->Add(cbP14,    &IniConfig.bP14EnableAutoTrayRecevieDelayCount,     ECBool,     "Tray",     "bEnableAutoTrayRecevieDelayCount",     bShow, bDisable, bFixedValue, 1);
        }
        else
        {
            elConfig->Add(cbP13,    &IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop,  ECBool,     "Tray",     "bEnableAutoTrayEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, 0);
            elConfig->Add(cbP14,    &IniConfig.bP14EnableAutoTrayRecevieDelayCount,     ECBool,     "Tray",     "bEnableAutoTrayRecevieDelayCount",     bShow, bEnable, bReadFromFile, 0);
        }

        elConfig->Add(edP13_1,      &IniConfig.iP13EdgePushCylinderLoopDelay,           ECInteger,  "Tray",     "iP13EdgePushCylinderLoopDelay",        bShow, bEnable, bReadFromFile, LastSet.iEdgePushCylinderLoopDelay, false, 3000, 2);
        elConfig->Add(edP13_2,      &IniConfig.iP13EdgePushCylinderOnDelay,             ECInteger,  "Tray",     "iP13EdgePushCylinderOnDelay",          bShow, bEnable, bReadFromFile, 10, false,  10000, 1);

        elConfig->Add(edP14_1,      &IniConfig.iP14AutoTrayRecevieDelayCount,           ECInteger,  "Tray",     "iP14AutoTrayRecevieDelayCount",        bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieDelayCount, false, 10000, 1);
        elConfig->Add(edP14_2,      &IniConfig.iP14AutoTrayRecevieLoopDelayTime,        ECInteger,  "Tray",     "iP14AutoTrayRecevieLoopDelayTime",     bShow, bEnable, bReadFromFile, LastSet.iAutoTrayRecevieLoopDelayTime, false, 3000, 2);

        elConfig->Add(cbP16,        &IniConfig.bP16EnableHotplateEdgePushCylinderLoop,  ECBool,     "Hotplate", "bEnableHotplateEdgePushCylinderLoop",  bShow, bEnable, bReadFromFile, 0);      //kevin 20191219 開啟

        if(TRAY_VIBRATION==VibrationMotor)
        {
            elConfig->Add(edP16_1,  &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger,  "Hotplate", "iHotplateEdgePushCylinderLoopDelay",   bShow, bEnable, bReadFromFile, 5, false, 600, 200);
            elConfig->Add(edP16_2,  &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger,  "Hotplate", "iHotplateEdgePushCylinderOnDelay",     bShow, bEnable, bReadFromFile, 5, false, 50, 2);
        }
        else
        {
            elUdUld->Add(edP16_1,   &IniConfig.iP16HotplateEdgePushCylinderLoopDelay,   ECInteger,  "Tray",    "iHotplateEdgePushCylinderLoopDelay",    bShow, bEnable, bReadFromFile, 10,                                          true,  2,   600);
            elUdUld->Add(edP16_2,   &IniConfig.iP16HotplateEdgePushCylinderOnDelay,     ECInteger,  "Tray",    "iHotplateEdgePushCylinderOnDelay",      bShow, bEnable, bReadFromFile,10,                                          true,  2,   50);
        }
    }

    if(IniConfig.bEnableUnloadTrayFree)                                         //jou 2011-05-27
        elConfig->Add(cbP15,    &IniConfig.bUnloadTrayFree,                     ECBool,     "Tray", "bUnloadTrayFree",                                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbP15,    &IniConfig.bUnloadTrayFree,                     ECBool,     "Tray", "bUnloadTrayFree",                                  bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12)  //kevin 20190923 add
        elConfig->Add(cbP17,    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else if(CUSTOMER_CODE==CC_KYEC_LEE && USE_STM_Function==true)
        elConfig->Add(cbP17,    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else if(CUSTOMER_CODE==CC_AMKOR_Japan)                                      //RogerYang 2051110 : 瑞薩FTCT需求，先放進Shuttle後再回來做一格一格檢查
        elConfig->Add(cbP17,    &IniConfig.bP17InArmFullPickFromLoader,         ECBool,     "Loader", "DisableLoaderFull",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on
    else
        elConfig->Add(cbP17,        &IniConfig.bP17InArmFullPickFromLoader,     ECBool,     "Loader", "DisableLoaderFull",                              bShow, bDisable, bFixedValue, 1);       //Steven 20111026 : In Arm Full Pick from Loader Tray //Steven 20190815 : [P17]改成always on

    elConfig->Add(cbP18,        &IniConfig.bP18FailAutoTrayManual,              ECBool,     "Tray", "FailAutoTrayManual",                               bShow, bEnable, bReadFromFile, 0);      //ChungHung 20120329 FailBin AutoTray Manual
    elConfig->Add(cbP19,        &IniConfig.bP19CatchTrayUpThenCheck,            ECBool,     "Tray", "bCatchTrayUpThenCheck",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20120727 : 夾Tray發生異常時要先把Arm上升再檢查一次如果還是沒夾到才Alarm
    elConfig->Add(cbP20, &IniConfig.bP20ManualClearFixTrayDataAfterInitialStart,ECBool,     "Tray", "bManualClearFixTrayDataAfterInitialStart",         bShow, bEnable, bReadFromFile, 0);      //ChungHung 20130305 add for Amkor Initial Strat後不清Tray盤資料需手動清除後才可Run
    elConfig->Add(chkP20_1, &IniConfig.bP20_1_ManualClrAutoTrayWhenInitialStart,ECBool,     "Tray", "bP20_1_ManualClrAutoTrayWhenInitialStart",         bShow, bEnable, bReadFromFile, 0);      //Steven 20210420 : add for TFME InitialStrat後不清Tray盤資料需手動清除後才可Run
    elConfig->Add(chkP20_2, &IniConfig.bP20_2_ManualClrLoadTrayWhenInitialStart,ECBool,     "Tray", "bP20_2_ManualClrLoadTrayWhenInitialStart",         bShow, bEnable, bReadFromFile, 0);      //Steven 20230117 : add for JSCK InitialStrat後需手動清除Loader Tray後才可Run

    if(CUSTOMER_CODE==CC_ASE_KaohSiung  ||                                      //kevin 20130708 高雄日月光強制TRUE
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12 ||                                   //Steven 20131101 : Add ASE-K12
       CosFunction.bHiSiliconFunction ||
       CUSTOMER_CODE==CC_KYEC_LEE ||                                            //Ifor 20180625 (wei) : Fix KYEC P21ㄧ般版本未強制開啟問題
       CUSTOMER_CODE==CC_Murata ||                                              //Steven 20200706 : Add Murata
       IniConfig.bSPILFunction==true)                                           //JerryYang 20250220 : add
        elConfig->Add(cbP21,    &IniConfig.bP21CheckFixTray,                    ECBool,     "Tray", "bCheckFixTray",                                    bShow, bDisable, bFixedValue, 1);       //kevin 20130430 tray feed 需先取出FIX TRAY
    else
        elConfig->Add(cbP21,    &IniConfig.bP21CheckFixTray,                    ECBool,     "Tray", "bCheckFixTray",                                    bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(chkP21_1,     &IniConfig.bP21_1_CheckFixTray,                 ECBool,     "Tray", "bP21_1_CheckFixTray",                              bShow, bEnable, bReadFromFile, 0);      //Steven 20250321 initial start時偵測fix tray需放入
    elConfig->Add(chkP21_2,     &IniConfig.bP21_2_LoaderTrayFeed,               ECBool,     "Tray", "bP21_2_LoaderTrayFeed",                            bShow, bEnable, bReadFromFile, 0);      //Steven 20250606 : Tray Feed 包含 loader tray

    elConfig->Add(cbP22,        &IniConfig.bP22EnableFirstTrayNeedAlarm,        ECBool,     "Tray", "bEnableFirstTrayNeedAlarm",                        bShow, bEnable, bReadFromFile, 0);
    elConfig->Add(coP22,        &IniConfig.iEnableFirstTrayNeedAlarmNum,        ECInteger,  "Tray", "iEnableFirstTrayNeedAlarmNum",                     bShow, bEnable, bReadFromFile, 0);

    elConfig->Add(edP23_1,      &IniConfig.iOCRByNewTrayIntrvalTray,            ECInteger,  "P23", "iOCRByNewTrayIntrvalTray",                          bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);
    elConfig->Add(edP23_2,      &IniConfig.iOCRMaxInspDevices,                  ECInteger,  "P23", "iOCRMaxInspDevices",                                bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU || IniConfig.bSPILFunction)               //JerryYang 20241122 : SPIL Disable P24                           //Sam 20230317 : 矽格湖口廠不能用會與 P39 功能起衝突
        elConfig->Add(cbP24,    &IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray, ECBool,    "Tray", "bSkipEventNeedRemoveEmptyAndColorTray",        bNoShow, bEnable, bFixedValue, 0);
    else
        elConfig->Add(cbP24,    &IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray, ECBool,    "Tray", "bSkipEventNeedRemoveEmptyAndColorTray",        bShow, bEnable, bReadFromFile, 0);      //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查

    if(CosFunction.bManuallyRemoveForceInColor)                                 //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    {
        elConfig->Add(cbP24_2,  &IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT,   ECBool, "Tray", "bP24SkipEventNeedRemoveColorTrayForIDT",          bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbP24_3,  &IniConfig.bP24TwoTrayMustManuallyRemovedForGenernal,ECBool, "Tray", "bP24TwoTrayMustManuallyRemovedForGenernal",       bShow, bEnable, bReadFromFile, 0);      //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    }
    else
    {
        elConfig->Add(cbP24_2,  &IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT,   ECBool, "Tray", "bP24SkipEventNeedRemoveColorTrayForIDT",          bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(cbP24_3,  &IniConfig.bP24TwoTrayMustManuallyRemovedForGenernal,ECBool, "Tray", "bP24TwoTrayMustManuallyRemovedForGenernal",       bNoShow, bDisable, bFixedValue, 0);     //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(cbP25,  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bShow, bDisable, bFixedValue, 1);       //kevin 20220810 ASE_KH
    }
    else if(AUTO_EMPTY_COLOR!=0)                                                //Alick 20160803 add Empty/Color 為四軸時，[P25]不顯示
        elConfig->Add(cbP25,  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bShow, bEnable, bReadFromFile, 0);      //kevin 20151102  empty or color TRAY 不補AUTO 123 TRAY就不須載入一個空TRAY。
    else
        elConfig->Add(cbP25,  &IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty, ECBool,     "Tray", "bEmptyColorNoSuppleAutoNoLoadEmpty",               bNoShow, bDisable, bFixedValue, 0);

    if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
        elConfig->Add(cbP26,    &IniConfig.bP26_OCRCheckLot,                    ECBool,     "Tray", "bP26_OCRCheckLot",                                 bShow, bEnable, bReadFromFile, 0);      //wei 20151117 OCR Lot check
    else
        elConfig->Add(cbP26,    &IniConfig.bP26_OCRCheckLot,                    ECBool,     "Tray", "bP26_OCRCheckLot",                                 bNoShow, bDisable, bFixedValue, 0);

    if(AUTO3_IS_MAGAZINE==1)                                                    //JerryYang 20230515 : P27跟Magazine衝突，先不使用
    {
        elConfig->Add(cbP27,    &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool,  "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bNoShow, bDisable, bFixedValue, 0);
    }
    else
    {
        if(CosFunction.bSortingBinTraywhenCleanOut ||
           CosFunction.bSortingBinTrayWhenTrayFeed)                             //JerryYang 20170911 (Steven) 整盤功能,執行時機由clean out改至tray feed前
            elConfig->Add(cbP27, &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool, "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bShow, bEnable, bReadFromFile, 0);      //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
        else
            elConfig->Add(cbP27, &IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut, ECBool, "Tray", "bAutoSortingBinTrayByOutArmwhenCleanOut",          bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_UNISEM_M)
        elConfig->Add(cbP28,    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bShow, bDisable, bFixedValue, 1);
    else if(CUSTOMER_CODE==CC_SCC || CosFunction.bUseAuto1OnlyBin1)
        elConfig->Add(cbP28,    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bShow, bEnable, bReadFromFile, 0);      //Alick 20160801 SCC可設定AUTO1 ONLY BIN1
    else
        elConfig->Add(cbP28,    &IniConfig.bP28Auto1OnlyBin1,                   ECBool,     "Function", "bP28Auto1OnlyBin1",                            bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbP29,        &IniConfig.bP29LoaderCheckIsFull,               ECBool,     "Function", "bP29LoaderCheckIsFull",                        bShow, bEnable, bReadFromFile, 0);      //Steven 20160818 : CheckLoader滿盤
    elConfig->Add(edP29,        &IniConfig.dP29LoaderCheckIsFullInterval,       ECDouble,   "Function", "dP29LoaderCheckIsFullInterval",                bShow, bEnable, bReadFromFile,  10,    false,  1.0,      1000.0);   //Steven 20160818 : CheckLoader滿盤

    if(CosFunction.bUseFixTryCheckRemainingAmount)
    {
        elConfig->Add(cbP30,    &IniConfig.bP30FixTryCheckRemainingAmount,      ECBool,     "Function", "bP30FixTryCheckRemainingAmount",               bShow, bEnable, bReadFromFile, 0);      //Ifor 20160829 : Check Fix Try 到達設定剩餘IC數量Alarm不停機
        elConfig->Add(edP30,    &IniConfig.iP30FixCheckRemainingAmountInterval, ECInteger,  "Function", "iP30FixCheckRemainingAmountInterval",          bShow, bEnable, bReadFromFile,  10,    false,  1,      1000);   //Ifor 20160829 : Fix Try 剩餘可放IC數量
    }
    else
    {
        elConfig->Add(cbP30,    &IniConfig.bP30FixTryCheckRemainingAmount,      ECBool,     "Function", "bP30FixTryCheckRemainingAmount",               bNoShow, bDisable, bFixedValue, 0);
        elConfig->Add(edP30,    &IniConfig.iP30FixCheckRemainingAmountInterval, ECInteger,  "Function", "iP30FixCheckRemainingAmountInterval",          bNoShow, bDisable, bFixedValue,  10,    false,  1,      1000);
    }

    elConfig->Add(cbP31,        &IniConfig.bP31LoaderTryLastOneFeedContinueRun, ECBool,     "Function", "bP31LoaderTryLastOneFeedContinueRun",          bShow, bEnable, bReadFromFile, 0);      //Ifor 20160829 : Loader Try 最後一盤入料Alarm不停機

    if(CosFunction.bUseEmptyColorTrayPreAlarm)
        elConfig->Add(cbP32,    &IniConfig.bP32EmptyColorTrayPreAlarm,          ECBool,     "Function", "bP32EmptyColorTrayPreAlarm",                   bShow, bEnable, bReadFromFile, 0);      //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    else
        elConfig->Add(cbP32,    &IniConfig.bP32EmptyColorTrayPreAlarm,          ECBool,     "Function", "bP32EmptyColorTrayPreAlarm",                   bNoShow, bDisable, bFixedValue, 0);

    if(CosFunction.bUseAutoTrayPreAlarm)
        elConfig->Add(cbP33,    &IniConfig.bP33AutoTrayPreAlarm,                ECBool,     "Function", "bP33AutoTrayPreAlarm",                         bShow, bEnable, bReadFromFile, 0);      //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    else
        elConfig->Add(cbP33,    &IniConfig.bP33AutoTrayPreAlarm,                ECBool,     "Function", "bP33AutoTrayPreAlarm",                         bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_TERAPOWER)
        elConfig->Add(cbP34,    &IniConfig.bP34CleanOutChangeInitialMode,       ECBool,     "Function", "bP34CleanOutChangeInitialMode",                bShow, bEnable, bReadFromFile, 0);      //kevin 20170417 (wei) : CleanOut change initial Mode
    else
        elConfig->Add(cbP34,    &IniConfig.bP34CleanOutChangeInitialMode,       ECBool,     "Function", "bP34CleanOutChangeInitialMode",                bNoShow, bDisable, bFixedValue, 0);

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20230608 add:KYEC 要求[P35] Tray Arm home safe pos強制反灰由DB選擇開/關
        elConfig->Add(cbP35,    &IniConfig.bP35TrayArm,                         ECBool,     "Function", "bP35TrayArm",                                  bShow, bDisable, bReadFromFile, 0);      //kevin 20171006 (wei) tray arm home 需遮住sensor
    else
        elConfig->Add(cbP35,    &IniConfig.bP35TrayArm,                         ECBool,     "Function", "bP35TrayArm",                                  bShow, bEnable, bReadFromFile, 0);      //kevin 20171006 (wei) tray arm home 需遮住sensor

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        elConfig->Add(cbP36,    &IniConfig.bP36BufferTrayNoSame,                ECBool,     "Function", "bP36BufferTrayNoSame",                         bShow, bEnable, bReadFromFile, 0);      //kevin 20171117 (wei) Load ,&,& Unload 強制不能使用相同軌道取放盤
    }
    else
    {
        elConfig->Add(cbP36,    &IniConfig.bP36BufferTrayNoSame,                ECBool,     "Function", "bP36BufferTrayNoSame",                         bNoShow, bDisable, bFixedValue, 0);
    }

    if(USE_AUTO_RETEST==eartInstall)                                            //JerryYang 20250826 : 有ART才啟用P37
    {
        elConfig->Add(cbP37,    &IniConfig.bP37bAutoCylinderUP,                 ECBool,     "Function", "bP37bAutoCylinderUP" ,                         bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP37,    &IniConfig.bP37bAutoCylinderUP,                 ECBool,     "Function", "bP37bAutoCylinderUP" ,                         bNoShow, bDisable, bFixedValue, 0);
    }

    elConfig->Add(cbP38,        &IniConfig.bP38UseEmptyFullPutColor,            ECBool,     "Function", "bP38UseEmptyFullPutColor",                     bNoShow, bDisable, bFixedValue, 0);     //wei 20170504 Use Empty Full Put Color

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
    {
        elConfig->Add(cbP39,    &IniConfig.bP39LoaderHasSkipPlaceToEmpty,       ECBool,     "Function", "bP39LoaderHasSkipPlaceToEmpty",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP39,    &IniConfig.bP39LoaderHasSkipPlaceToEmpty,       ECBool,     "Function", "bP39LoaderHasSkipPlaceToEmpty",                bNoShow, bDisable, bFixedValue, 0);
    }

    if(LoaderUnload_StepMotor)                                                  //Sam 20201221 : Tray y step motor by machine
    {
        elConfig->Add(cbP40,    &IniConfig.bP40TrayYSpeedByMachine,             ECBool,     "Function", "bP40TrayYSpeedByMachine",                      bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP40,    &IniConfig.bP40TrayYSpeedByMachine,             ECBool,     "Function", "bP40TrayYSpeedByMachine" ,                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20240403 : Unload Tray Disable Edit
    {
        elConfig->Add(cbP41,        &IniConfig.bP41UnloadTrayDisableEdit,       ECBool,     "Function", "bP41UnloadTrayDisableEdit",                    bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbP41,        &IniConfig.bP41UnloadTrayDisableEdit,       ECBool,     "Function", "bP41UnloadTrayDisableEdit",                    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_FMSH)                                                  //Ifor 20210428 add:Auto 退Tray 完成時報警停機
    {
        elConfig->Add(cbP42,    &IniConfig.bP42AlarmWhenExitTrayComplete,       ECBool,     "Function", "bP42AlarmWhenExitTrayComplete",                bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP42,    &IniConfig.bP42AlarmWhenExitTrayComplete,       ECBool,     "Function", "bP42AlarmWhenExitTrayComplete" ,               bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bUnloadTrayModeByRecipe)                                     //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    {
        elConfig->Add(chkAutoTrayFeed,           &TrayForm.bAutoFeed,              ECBool,  "Flag", "Auto Feed",                                        bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbManuTakeAutoFailTray_FT, &TrayForm.bFailAutoTrayManual_FT, ECBool,  "Flag", "bFailAutoTrayManual_FT",                           bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbManuTakeAutoFailTray_RT, &TrayForm.bFailAutoTrayManual_RT, ECBool,  "Flag", "bFailAutoTrayManual_RT",                           bShow, bEnable, bReadFromFile, 0);
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220331 : 矽品中山要求AUTO SKIP後可由TRAY ARM搬TRAY
    {
        elConfig->Add(cbP44,    &IniConfig.bP44LockLoaderTrayToNone,            ECBool,     "Function", "bP44LockLoaderTrayToNone" ,                    bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP44,    &IniConfig.bP44LockLoaderTrayToNone,            ECBool,     "Function", "bP44LockLoaderTrayToNone" ,                    bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20221008 load 空盤 不TRAY
    {
        elConfig->Add(cbP45,    &IniConfig.bP45LastLoaderNoInSide,              ECBool,     "Function", "bP45LastLoaderNoInSide",                       bShow, bEnable, bReadFromFile, 1); //kevin 20221008
    }
    else
    {
        elConfig->Add(cbP45,    &IniConfig.bP45LastLoaderNoInSide,              ECBool,     "Function", "bP45LastLoaderNoInSide" ,                      bShow, bEnable, bReadFromFile, 0);
    }
    elConfig->Add(cbP46,        &IniConfig.bP46_LoadTrayModeByHandler,          ECBool,     "Tray", "bP46_LoadTrayModeByHandler" ,                      bShow, bEnable, bReadFromFile, 0);        //Steven 20221117 : Loader Tray Mode設定跟著機台變

    elConfig->Add(cbP48,        &IniConfig.bP48UnloaderCylinderLoop,            ECBool,     "Tray", "bP48UnloaderCylinderLoop" ,                        bShow, bEnable, bReadFromFile, 0);        //JimmyChiu 20230512 Unloader cylinder loop
    elConfig->Add(edP48Times,     &IniConfig.iP48UnloaderCylinderLoopTimes,     ECInteger,  "Tray", "iP48UnloaderCylinderLoopTimes",                    bShow, bEnable, bReadFromFile, 0,    false, 0,       3);
    elConfig->Add(edP48DelayTime, &IniConfig.iP48UnloaderCylinderLoopDelay,     ECInteger,  "Tray", "iP48UnloaderCylinderLoopDelay",                    bShow, bEnable, bReadFromFile, 0,    false, 0,       2);

    if(CosFunction.bUseLocalTraySpeed==true)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)
            elConfig->Add(cbP49,&IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bShow, bDisable, bFixedValue, 1);        //Ifor 20200825 add: Use Local Tray Speed   //Ifor 20200827 add: KYEC 強制開啟功能並反灰不可修改
        else
            elConfig->Add(cbP49,&IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bShow, bEnable, bReadFromFile, 0);     //Ifor 20200825 add: Use Local Tray Speed
    }
    else
    {
        elConfig->Add(cbP49,    &IniConfig.bP49UseLocalTraySpeed,               ECBool,    "Function", "bP49UseLocalTraySpeed",                        bNoShow, bDisable, bFixedValue, 0);    //Ifor 20200825 add: Use Local Tray Speed
    }

    if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                      //Sam 20230221 : 矽格中興國桂要求要能關閉
        elConfig->Add(cbP50,        &IniConfig.bP50DisabledAutoTrackSensorDetect, ECBool,          "Function", "bP50DisabledAutoTrackSensorDetect",          bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbP50,        &IniConfig.bP50DisabledAutoTrackSensorDetect, ECBool,          "Function", "bP50DisabledAutoTrackSensorDetect" ,         bNoShow, bDisable, bFixedValue, 0);

    elConfig->Add(cbP51,        &IniConfig.bP51TrayArmPutwaitUnloadOK,          ECBool,     "Function", "bP51TrayArmPutwaitUnloadOK",                   (CUSTOMER_CODE==CC_ASE_KaohSiung), bEnable, bReadFromFile, 1);      //kevin 20230331 add TRAY arm 放 auto 123 等待tray上升避免輸 送帶滑進去 夾tray.
    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                                                                                                                                                                                  //KenHsieh 20230919 : Empty & Color Last Tray Check
        elConfig->Add(cbP52,    &IniConfig.bP52EmptyColorLastTrayCheck,         ECBool,     "Function", "bP51EmptyColorLastTrayCheck",                  bShow, bEnable, bReadFromFile, 0);
    else
        elConfig->Add(cbP52,    &IniConfig.bP52EmptyColorLastTrayCheck,         ECBool,     "Function", "bP51EmptyColorLastTrayCheck",                  bNoShow, bDisable, bFixedValue, 0);

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20231218 : P53防混功能
    {
        elConfig->Add(cbP53,    &IniConfig.bP53_ForcedScanBinCodeOfUnloader,    ECBool,     "Tray", "bP53_ForcedScanBinCodeOfUnloader" ,   bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        elConfig->Add(cbP53,    &IniConfig.bP53_ForcedScanBinCodeOfUnloader,    ECBool,     "Tray", "bP53_ForcedScanBinCodeOfUnloader" ,   bNoShow, bDisable, bFixedValue, 0);
    }

    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20251105 add: ASE SG 客戶要求[P54]強制關閉不顯示
        elConfig->Add(cbP54,        &IniConfig.P54UnloaderTaryCheckHasErrorBinIC,   ECBool,      "Function", "P54UnloaderTaryCheckHasErrorBinIC",  bNoShow, bDisable, bFixedValue, 0);   //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
    else
        elConfig->Add(cbP54,        &IniConfig.P54UnloaderTaryCheckHasErrorBinIC,   ECBool,      "Function", "P54UnloaderTaryCheckHasErrorBinIC",  bShow, bEnable, bReadFromFile, 0);    //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin

    if(CUSTOMER_CODE==CC_KYEC_LEE &&                                            //Ifor 20211014 add: KYEC 要求暫時強制關閉P40功能，反灰不可修改
            AUTO_EMPTY_COLOR==false)                                            //Ifor 20201225 add:六軌模式不顯示並強制關閉
    {
        elConfig->Add(cbP55,        &IniConfig.bP55LdUldUseEmptyAndColorTray,   ECBool,     "Function", "bP40LdUldUseEmptyAndColorTray",                bShow, bDisable, bFixedValue, 0);       //Ifor 20201218 add: Load & Unload Use Empty And Color Tray
    }
    else
    {
        elConfig->Add(cbP55,        &IniConfig.bP55LdUldUseEmptyAndColorTray,   ECBool,     "Function", "bP40LdUldUseEmptyAndColorTray",                bNoShow, bDisable, bFixedValue, 0);         //Ifor 20201218 add: Load & Unload Use Empty And Color Tray
    }

    elConfig->Add(chkP56,           &IniConfig.bP56TrayArmWaitAtColorTrack,     ECBool,     "Function", "bP56TrayArmWaitAtColorTrack",                bShow, bEnable, bReadFromFile, 0);            //Steven 20240516 : Tray Arm等待位置改到Color
    elConfig->Add(cbP57,            &IniConfig.bP57LoaderAutoCleanOutByInputCT, ECBool,     "Function", "bP57LoaderAutoCleanOutByInputCT",            bShow, bEnable, bReadFromFile, 0);            //Sam 20250605 : Loader Count AutoCleanOut
    elConfig->Add(rgP58RunModeAfterFT,&IniConfig.iP58RunModeAfterFT,            ECInteger,  "Function", "iP58RunModeAfterFT",                         bShow, bEnable, bReadFromFile,  0);           //JimmyChiu 20250905 : FT模式結束後切換模式

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警
    {
        elConfig->Add(cbP59, &IniConfig.bP59UnloaderICFloattingAlarmAfterExit,  ECBool,     "Function", "bP57UnloaderICFloattingAlarmAfterExit",                      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbP59, &IniConfig.bP59UnloaderICFloattingAlarmAfterExit,  ECBool,     "Function", "bP57UnloaderICFloattingAlarmAfterExit" ,                     bShow, bEnable, bReadFromFile, 0);
    }

    if(CosFunction.bReadClipCodeFromUnloader)                                   //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
    {
        elConfig->Add(cbP60, &IniConfig.bP60ReadClipCodeFromUnloader,           ECBool,     "Function", "bP60ReadClipCodeFromUnloader",                      bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbP60, &IniConfig.bP60ReadClipCodeFromUnloader,           ECBool,     "Function", "bP60ReadClipCodeFromUnloader" ,                     bNoShow, bDisable, bFixedValue, 0);
    }

    if(CosFunction.bFirstTrayCheckOnUnloader)                                   //Jimmychiu 20251205 : First Tray Check On Unloader
    {
        elConfig->Add(cbP62,       &IniConfig.bP62FirstTrayCheckOnUnloader,     ECBool,     "Function", "bP62FirstTrayCheckOnUnloader",   bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto1, &IniConfig.bP62Auto1,                        ECBool,     "Function", "bP62Auto1",                      bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto2, &IniConfig.bP62Auto2,                        ECBool,     "Function", "bP62Auto2",                      bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP62_Auto3, &IniConfig.bP62Auto3,                        ECBool,     "Function", "bP62Auto3",                      bShow, bEnable, bReadFromFile, 0);
        elConfig->Add(cbP62_1,     &IniConfig.bP62AlwaysEnabledAtLotStart,      ECBool,     "Function", "bP62AlwaysEnabledAtLotStart",    bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(cbP63,       &IniConfig.bP63MachineStopAtIntervalTime,    ECBool,     "Function", "bP63MachineStopAtIntervalTime",  bShow, bEnable, bReadFromFile, 0);
//        elConfig->Add(edP63,       &IniConfig.iP63IntervalTime,                 ECInteger,  "Function", "iP63IntervalTime",               bShow, bEnable, bReadFromFile, 0);
    }
    else
    {
        plP62->Visible=false;
//        cbP63->Visible=false;
//        edP63->Visible=false;
    }

    if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                                        //Ifor 20260407 : [P65] Enable ARM QA Mode
    {
        elConfig->Add(cbP65,    &IniConfig.bP65EnableArmQAMode,                 ECBool,     "Function", "bP65EnableArmQAMode",          bShow, bEnable, bReadFromFile, 0);    //Ifor 20260407 : [P65] Enable ARM QA Mode
        elConfig->Add(edtP65,   &IniConfig.iP65ArmQAModeValue,                  ECInteger,  "Function", "iP65ArmQAModeValue",           bShow, bEnable, bReadFromFile,  0,     false,  0,      30);  //Ifor 20260407 : [P65] ARM QA Mode Value
    }
    else
    {
        elConfig->Add(cbP65,    &IniConfig.bP65EnableArmQAMode,                 ECBool,     "Function", "bP65EnableArmQAMode",          bNoShow, bDisable, bFixedValue, 0);  //Ifor 20260407 : [P65] Enable ARM QA Mode
        elConfig->Add(edtP65,   &IniConfig.iP65ArmQAModeValue,                  ECInteger,  "Function", "iP65ArmQAModeValue",           bNoShow, bDisable, bFixedValue, 0);  //Ifor 20260407 : [P65] ARM QA Mode Value
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        elConfig->Add(cbP66,    &IniConfig.bP66AutoChangingFlashWarn,         ECBool,     "Function", "bP66AutoChangingFlashWarn",                  bShow, bEnable, bReadFromFile, 1);
    }
    else
    {
        elConfig->Add(cbP66,    &IniConfig.bP66AutoChangingFlashWarn,         ECBool,     "Function", "bP66AutoChangingFlashWarn",                  bShow, bEnable, bReadFromFile, 0);
    }
}

// =============================================================================
// FW-CFG-W4b -- ReadLockByFile / ChangeCBListProperty / InitConfigEdtList
// =============================================================================
// AI(W906-FW-CFG-W4b) 20260825: golden cConfiguration.cpp:230-261, transcribed
// VERBATIM (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::ReadLockByFile()                                          //Sam 20221018 : 修正 LockByFile 問題
{
    /*
    Enabled=0 , Active=0
    Checked鎖起來，不勾關閉
    Enabled=0 , Active=1
    Checked鎖起來，打勾啟動
    Enabled=1 , Active=0 or 1(Active無作用)
    由 Config 畫面直接設定開關
    */
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bF06_Active=ReadIniData(sPath, "Specific", "F06_Active",  true);  //Steven 20140627 : Add for ASE-CL -- F06 打勾
    IniConfig.bF06_Enable=ReadIniData(sPath, "Specific", "F06_Enabled", false); //Steven 20140627 : Add for ASE-CL -- F06 Enable

    IniConfig.bF11_Active       =ReadIniData(sPath, "Specific", "F11_Active",   true);      //Sam 20240202 : 新增 F11 Lock by file 功能
    IniConfig.bF11_Enable       =ReadIniData(sPath, "Specific", "F11_Enabled",  false);     //Sam 20240202 : 新增 F11 Lock by file 功能

    IniConfig.bI06_Active=ReadIniData(sPath, "Specific", "I06_Active",  true);  //Sam 20220527 : for 矽格北興 -- I06 打勾
    IniConfig.bI06_Enable=ReadIniData(sPath, "Specific", "I06_Enabled", false); //Sam 20220527 : for 矽格北興 -- I06 Enable

    IniConfig.bP24_Active=ReadIniData(sPath, "Specific", "P24_Active",  true);  //JerryYang 20160220 add for 矽格北興 -- P24 打勾
    IniConfig.bP24_Enable=ReadIniData(sPath, "Specific", "P24_Enabled", false); //JerryYang 20160220 add for 矽格北興 -- P24 Enable

    IniConfig.bRTC_Active       =ReadIniData(sPath, "Specific", "RTC_Active",  true);       //Sam 20240311 : 新增 RTC Lock by file 功能
    IniConfig.bRTC_Enable       =ReadIniData(sPath, "Specific", "RTC_Enabled",  false);     //Sam 20240311 : 新增 RTC Lock by file 功能
    IniConfig.bI02_Enable       =ReadIniData(sPath, "Specific", "I02_Enabled", true);     //JerryYang 20260504 : for SPIL -- I02 Enable
    IniConfig.bA03_Enable       =ReadIniData(sPath, "Specific", "A03_Enabled", true);

    // AI(W906-FW-CFG-W4b) 20260825: GATE (CFG4-seed). Everything above this
    // point is a READ and stays LIVE; these two lines are the method's only
    // WRITES, and they write AuthPath+"config.ini" -- i.e. the shared
    // D:\HT9045\config\config.ini. This campaign does not open write paths
    // unattended, so they are gated and queued.
    // WHY THIS IS NOT THE SAME AS THE CheckAndReadIniData PRECEDENT: that one
    // is a READ whose implementation happens to seed a missing key, and this
    // tree keeps those live (cObserver.cpp:549 and its siblings). These are
    // explicit WriteIniData calls whose whole purpose is to create a key.
    // CONSEQUENCE, stated exactly: the in-memory values are unaffected -- the
    // ReadIniData calls above have already assigned IniConfig.bI02_Enable /
    // bA03_Enable their defaults for this run. What is lost is only the
    // PERSISTENCE: on a machine whose config.ini lacks those two keys, golden
    // would add them and this port will not, so the same defaults are
    // re-derived on every start instead of being written once.
#if 0 // GATE (CFG4-seed) -- writes D:\HT9045\config\config.ini (see above)
    if(!CheckKeyExist(sPath, "Specific", "I02_Enabled")) WriteIniData(sPath, "Specific", "I02_Enabled", IniConfig.bI02_Enable);
    if(!CheckKeyExist(sPath, "Specific", "A03_Enabled")) WriteIniData(sPath, "Specific", "A03_Enabled", IniConfig.bA03_Enable);
#endif // GATE (CFG4-seed)
}

// AI(W906-FW-CFG-W4b) 20260825: golden cConfiguration.cpp:266-363, transcribed
// VERBATIM (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::ChangeCBListProperty()
{
    bool bShow=true, bNoShow=false;
    bool bEnable=true, bDisable=false;
    bool bReadFromFile=true, bFixedValue=false;

    ReadLockByFile();                                                           //Sam 20221018 : 修正 LockByFile 問題

    if(CUSTOMER_CODE==CC_KYEC_LEE && Tri_Temp_Machine!=1)                       //Wenqi 20240516 Use by Arm Close Site Function
        ((THTEdit*)elConfig->FEditList->Items[cbA09->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    else if(CosFunction.bHaveFIFOMode &&
            IniConfig.bI37_EnableFIFOMode &&
            IniConfig.bI37_EnableFIFOSiteOrder)                                 //Steven 20170302 (wei) : FIFO MODE 跟 By Arm Close Site功能衝突
        ((THTEdit*)elConfig->FEditList->Items[cbA09->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[cbA09->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockD42ByFile)                                              //JerryYang 20160220 add for Amkor-Philipine
        ((THTEdit*)elConfig->FEditList->Items[cbD42->Tag])->ChangeProperty(bShow, IniConfig.bD42_Enable, !IniConfig.bD42_Enable, IniConfig.bD42_Active);
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
            CUSTOMER_CODE==CC_ASE_KaohSiung_K12)
        ((THTEdit*)elConfig->FEditList->Items[cbD42->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, true);
    else
        ((THTEdit*)elConfig->FEditList->Items[cbD42->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockD44ByFile)                                              //JerryYang 20160220 add for Amkor-Philipine
        ((THTEdit*)elConfig->FEditList->Items[cbD44->Tag])->ChangeProperty(bShow, IniConfig.bD44_Enable, !IniConfig.bD44_Enable, IniConfig.bD44_Active);
    else if(CUSTOMER_CODE==CC_ASE_CL         ||                                 //JerryYang 20210129 : ASE-CL松諭要求強制開啟
            CUSTOMER_CODE==CC_SIGURD_HUKOU   ||                                 //Sam 20220527 : 矽格湖口 Jonas 要求強制開啟
            CUSTOMER_CODE==CC_SIGURD_PeiXing ||                                 //Sam 20240606 : 北興廠也要強制開啟
            CUSTOMER_CODE==CC_KYEC_LEE && Tri_Temp_Machine==1)                  //Wenqi 20240516 KYEC要求強制反灰開啟
        ((THTEdit*)elConfig->FEditList->Items[cbD44->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, true);
    else
        ((THTEdit*)elConfig->FEditList->Items[cbD44->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockF06ByFile)
        ((THTEdit*)elConfig->FEditList->Items[cbF06->Tag])->ChangeProperty(bShow, IniConfig.bF06_Enable, IniConfig.bF06_Enable, IniConfig.bF06_Active);    //Sam 20221018 : 修正 LockByFile 問題
    else
        ((THTEdit*)elConfig->FEditList->Items[cbF06->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockF11ByFile)  //Sam 20240202 : 新增 F11 Lock by file 功能
        ((THTEdit*)elConfig->FEditList->Items[cbF11->Tag])->ChangeProperty(bShow, IniConfig.bF11_Enable, IniConfig.bF11_Enable, IniConfig.bF11_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[cbF11->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(Cylinder[C_Shuttle_Knocker_1].Enable==false && Cylinder[C_Shuttle_Knocker_2].Enable==false)  //Steven 20160509 : 氣缸沒裝就不要啟用功能
    {
        ((THTEdit*)elConfig->FEditList->Items[cbF14->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
        ((THTEdit*)elConfig->FEditList->Items[cbF14_1->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[cbF14->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
        ((THTEdit*)elConfig->FEditList->Items[cbF14_1->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if((IniConfig.bA09_ByArmCloseSite ||
        bCanAutoCloseSite==true) &&                                             //Steven 20200420 : 整合Auto Site Off
       IniConfig.bL17HeadHeaterOnWhenCloseSite==false)                          //Steven 20160926 : 避免這幾個功能同時開,會導致IC換邊擺,然後可能發生沒加熱就生產
    {
        ((THTEdit*)elConfig->FEditList->Items[cbF17->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[cbF17->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if(CosFunction.bLockF26ByFile)                                              //Sam 20220527 : for 矽格湖口 -- F26 Enable
    {
        ((THTEdit*)elConfig->FEditList->Items[rgF26->Tag])->ChangeProperty(bShow, IniConfig.bF26_Enable, bReadFromFile, false);
    }
    else
    {
        ((THTEdit*)elConfig->FEditList->Items[rgF26->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    }

    if(IniConfig.bShowLotInfo)
        ((THTEdit*)elConfig->FEditList->Items[cbI04->Tag])->ChangeProperty(bShow, bDisable, bFixedValue, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[cbI04->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    if(CosFunction.bLockI06ByFile)                                              //Sam 20220527 : for 矽格北興
        ((THTEdit*)elConfig->FEditList->Items[cbI06->Tag])->ChangeProperty(bShow, IniConfig.bI06_Enable, IniConfig.bI06_Enable, IniConfig.bI06_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[cbI06->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);

    #ifndef SOFT_SIMULTE
    if(CosFunction.bKnockerSetBySetupFile==false && Cylinder[C_HotplateVibration].Enable==true)
        ((THTEdit*)elConfig->FEditList->Items[cbP16->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
    else
        ((THTEdit*)elConfig->FEditList->Items[cbP16->Tag])->ChangeProperty(bNoShow, bDisable, bFixedValue, false);
    #endif

    if(CosFunction.bLockP24ByFile)                                              //JerryYang 20160425 lock P24
        ((THTEdit*)elConfig->FEditList->Items[cbP24->Tag])->ChangeProperty(bShow, IniConfig.bP24_Enable, IniConfig.bP24_Enable, IniConfig.bP24_Active); //Sam 20250814 : 修正 Lock 錯誤
    else
        ((THTEdit*)elConfig->FEditList->Items[cbP24->Tag])->ChangeProperty(bShow, bEnable, bReadFromFile, false);
}

// AI(W906-FW-CFG-W4b) 20260825: golden cConfiguration.cpp:4495-4512, transcribed
// VERBATIM (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::InitConfigEdtList()                                       //Steven 20190614 : 重新整理Edit to 變數的方式
{
    // AI(W906-FW-CFG-W4b) 20260825: DEVIATION -- an `elConfig` null guard golden
    // does not have, for a reason golden does not have either. golden creates
    // that global in WinMain (`elConfig=new HTEditList;`, golden main.cpp:1484)
    // and only ever reaches this form afterwards, so it cannot be NULL there.
    // In this port the global is declared (Public/HTEditList.h:247) but nothing
    // has created it yet, and every one of the thirteen Item functions below
    // dereferences it on its first line. That is the exact shape that cost
    // PT-W2 88 SEGFAULTs: `elLaser` lives one line away, at golden
    // main.cpp:1483, and fLaserSensor's constructor dereferenced it at static
    // init. Guarding on the pointer reproduces golden's own precondition
    // instead of crashing when it is not met; once an owner creates elConfig,
    // this runs exactly like golden.
    if(elConfig == 0 || elConfig_byRecipe == 0 || cbLastSet == 0 || elUdUld == 0)
        return;

    InitConfigEdtList_ItemA();
    InitConfigEdtList_ItemB();
    InitConfigEdtList_ItemC();
    InitConfigEdtList_ItemD();
    InitConfigEdtList_ItemE();
    InitConfigEdtList_ItemF();
    InitConfigEdtList_ItemG();
    InitConfigEdtList_ItemI();
    InitConfigEdtList_ItemL();
    InitConfigEdtList_ItemM();
    InitConfigEdtList_ItemN();
    InitConfigEdtList_ItemO();
    InitConfigEdtList_ItemP();

    ChangeCBListProperty();
}

// =============================================================================
// FW-CFG-W5 -- PORT-ONLY ctor + FormShow (merged 20260825)
// =============================================================================
// AI(W906-FW-CFG-W5) 20260825: PORT-ONLY constructor -- NOT golden's.
// golden's TfConfiguration ctor is :109-225 and is still untranslated (it calls
// ReadLockByFile/InitConfigEdtList/ReadConfigStandard/WriteContactData, does a
// WinSock hostname lookup, and is write-path; forms/fConfiguration.h's own
// INTEGRATION STATUS note defers it). This one exists for a single reason: the
// three widget ARRAYS FormShow dereferences have no .dfm streaming and no
// in-class initializer, so something has to allocate their elements before
// FormShow touches them. golden allocates them inside that deferred ctor.
// Same shape forms/fLotInfo.cpp:250-255 uses for SocketSiteCH_Display/edSocket.
// When golden's ctor is translated, its body belongs BELOW this block, exactly
// as cObserver.cpp separates its port-only section (A) from golden's body (B).
TfConfiguration::TfConfiguration()
{
    for (int i = 0; i < TOTAL_MOTOR; i++)
    {
        edSoftSpeed[i]  = new TEdit();
        labSoftSpeed[i] = new TLabel();
    }
    for (int i = 0; i < 10; i++)
        edOCRTrayLot[i] = new TEdit();
}

// AI(W906-FW-CFG-W5) 20260825: golden cConfiguration.cpp:4514-5365, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::FormShow(void *Sender)
{
    AnsiString str;
    int iATCVerBuf=0;                                                           //KenHsieh 20240216 : add ATC Power Follow Function

    // GATE (W5-Pages) -- `pcConfig->PageCount` / `->Pages[i]` enumerate a
    // TPageControl's child pages. TfConfigurationPageControl models
    // ActivePage and ActivePageIndex only; a page LIST needs the parent/child
    // tree this facade does not have -- forms/fSpeed.h GATE (S4),
    // forms/fHandlerSys.h GATE (H1) and forms/fLotInfo.h GATE (WA-4) all
    // document that same gap. ACTIVE ARM: nothing; the loop body is itself a
    // ChangeCompomentEnabled call, which GATE (W5-CCE) closes anyway.
#if 0 // GATE (W5-Pages)
    for(int i=0; i<pcConfig->PageCount; i++)                                    //Steven 20210810 : 快速搜尋Config加上權限控制
    {
    // ------------------------------------------------------------------
    // GATE (W5-CCE) -- every ChangeCompomentEnabled(...) call in this method.
    // golden's signature is `ChangeCompomentEnabled(TWinControl *PCtrl, ...)`
    // and it walks PCtrl's child controls. In this port every widget class
    // (TPanel / TGroupBox / TTabSheet / TRadioGroup) derives DIRECTLY from
    // vclcompat::TControl -- none has a TWinControl-shaped ancestor. That is
    // the component-tree gap forms/fSpeed.h GATE (S4), forms/fHandlerSys.h
    // GATE (H1) and forms/fLotInfo.h GATE (WA-4) already document, and this
    // project's rule for it is 「跨檔缺口 GATE 不自建 shim」 -- so the calls are
    // gated rather than forced through with a cast.
    // COSTS NOTHING OBSERVABLE TODAY: the function's own BODY is already gated
    // in cAuthority.cpp for the identical reason (cAuthority.h:32), so every
    // one of these calls is a documented no-op even where it would compile.
    // WHAT IS LOST once a real control tree lands: the per-access-level
    // enable/disable pass over the Config tabs. The AccessLevel comparisons
    // themselves are preserved verbatim inside the gate.
    // ------------------------------------------------------------------
#if 0 // GATE (W5-CCE)
        if(pcConfig->Pages[i]!=tsSearchFunction)
            ChangeCompomentEnabled(pcConfig->Pages[i], true, true);
#endif // GATE (W5-CCE)
    }                                                                           //Steven 20120807 End: 先在最上面Enable全部畫面
#endif // GATE (W5-Pages)

    ReadLastSetIni();
    fShow=true;
    tsTempComm->TabVisible=(CUSTOMER_CODE==CC_HONPREC_QC)?true:false;
    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //Steven 20110818
    {
        tsTempComm->TabVisible=true;
    }

    listHeaterMonitor->Items->Clear();
    chkHeater->Checked=false;
    bStopChange=true;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(PageControl1->ActivePageIndex==2)                                    //kevin 20180411 key in password
        {
            PageControl1->ActivePageIndex=0;
        }
    }
    else
    {
        PageControl1->ActivePage=tsConfig;
    }

    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        edSoftSpeed[i]->Text=(LastSet.SoftSpeed[i]==0)?10000:LastSet.SoftSpeed[i];   //JerryYang 20161129 軟體模擬speed 1000->10000 //Steven 20160816 : provide initial soft speed as 1000
        edSoftSpeed[i]->Tag=i;
        labSoftSpeed[i]->Caption=MOT[i].Alias;
        if(MOT[i].Alias=="")                                                    //Steven 20150910 : 改成動態產生
        {
            edSoftSpeed[i]->Visible=false;
            labSoftSpeed[i]->Visible=false;
        }
        edSoftSpeed[i]->Enabled=authConfig[0];                                  //Steven 20090731
    }

    btnAdd1000  ->Enabled=authConfig[0];
    btnDec1000  ->Enabled=authConfig[0];
    btnSetTo1000->Enabled=authConfig[0];
    btnAdd10000 ->Enabled=authConfig[0];
    btnSetToTech->Enabled=authConfig[0];

//#ifndef Carry4
    edSoftSpeed[17] ->Visible=false;
    labSoftSpeed[17]->Visible=false;
    edSoftSpeed[18] ->Visible=false;
    labSoftSpeed[18]->Visible=false;
//#endif

    bStopChange=false;

    if(bHasTrayCSV)                                                             //Steven 20210629 : Tray Form改成CSV
    {
        sbtReloadTray->Click();
    }

    if(bHasPlateCSV)                                                            //Steven 20210629 : Plate Form改成CSV
    {
        sbtReloadHP->Click();
    }

    GetConfAuth();                                                              //Steven 20090731

//[A]-------------------------
    if(IniConfig.bA09_ByArmCloseSite)                                           //ChungHung 20140505 add 加強保護
        CosFunction.bOneCycleCanChangeArm=false;

//[C]-------------------------
    cbC07->Checked=!bGetLotIDFormTester;                                        //ChungHung 20150615 反向

//[D]-------------------------
    double temp;
    temp=LastSet.dIndexLoadRate[1][0]*100;
    tbD25_Index60mm_NS->Position=int(temp);                                     //wei 20150303   京元NS浮動頭

    temp=LastSet.dIndexLoadRate[1][2]*100;
    tbD25_Index40mm_NS->Position=int(temp);

    temp=LastSet.dIndexLoadRate[1][3]*100;
    tbD25_Index30mm_NS->Position=int(temp);

    temp=LastSet.dIndexLoadRate[0][0]*100;
    tbD25_Index60mm->Position   =int(temp);

    temp=LastSet.dIndexLoadRate[0][2]*100;
    tbD25_Index40mm->Position   =int(temp);

    temp=LastSet.dIndexLoadRate[0][3]*100;
    tbD25_Index30mm->Position   =int(temp);

    edD25_60mm->Text            =LastSet.dIndexLoadRate[2][0];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset
    edD25_40mm->Text            =LastSet.dIndexLoadRate[2][2];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset
    edD25_30mm->Text            =LastSet.dIndexLoadRate[2][3];                  //2014-06-26    Dell    for TSMC 高溫Load cell offset

    edD25_60mm->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);      //2014-06-26    Dell    for TSMC 高溫Load cell offset
    edD25_40mm->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
    edD25_30mm->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
    labD25_4->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    labD25_5->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    labD25_6->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
    tbD25_Index60mm_NS->Visible =(CosFunction.bEPUseNSSLK);                     //wei 20150303   京元NS浮動頭
    tbD25_Index40mm_NS->Visible =(CosFunction.bEPUseNSSLK);
    tbD25_Index30mm_NS->Visible =(CosFunction.bEPUseNSSLK);
    labD25_1_NS->Visible        =(CosFunction.bEPUseNSSLK);
    labD25_2_NS->Visible        =(CosFunction.bEPUseNSSLK);
    labD25_3_NS->Visible        =(CosFunction.bEPUseNSSLK);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung || CosFunction.bHiSiliconFunction==true) //kevin 20170328 (Steven) EP 密碼輸入   //Ifor 20170803 (wei) add KYEC EP 密碼  //JerryYang 20170921 (Steven) Hisi版本 EP offset鎖定
    {
        gbD25->Enabled=false;
        gbD25->Color =clRed;
    }

    gbD25->Visible              =(CosFunction.bUseDynamicKitDiameter==false);   //Steven 20170605 : 可以自定義Kit直徑
    btnOpenEP->Visible          =(CosFunction.bUseDynamicKitDiameter==false);   //Ifor 20170911 (Steven) : 無自訂Kit不需顯示Open EP 開關

    if(CosFunction.bLockD41ByFile)                                              //Steven 20140627 : Add for ASE-CL
    {
        edD41->Enabled  =IniConfig.bD41_Enable;
        coD41->Enabled  =IniConfig.bD41_Enable;
    }

    udD46->Position     =IniConfig.iD46WaitIndexDestroyTime;

    // GATE (W5-COM2) -- same missing member as GATE (CFG2-COM2) earlier in
    // this file: COM2 is TCOM2Shim (atester_shims.h:405) and bCCDDummyRum is
    // not one of its members. UNLIKE CFG2-COM2 there is no half to keep here:
    // that one was a DISJUNCTION so dropping an operand narrowed it, this one
    // is a CONJUNCTION so dropping an operand would WIDEN the branch -- the
    // wrong direction. The `else` keyword is gated with it.
    // ACTIVE ARM: golden's else body runs. That is the arm golden itself takes
    // on every machine without real-time CCD (REAL_TIME_CCD==false), so it is
    // the faithful landing, not an arbitrary default: labD53/edD53 stay visible.
#if 0 // GATE (W5-COM2)
    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                              //Steven 20110907 : Real Time CCD - 燈光Start後不是On就是Off
    {
        labD53->Visible=false;
        edD53->Visible=false;
        edD53->Text=0;
    }
    else
#endif // GATE (W5-COM2)
    {
        labD53->Visible=true;
        edD53->Visible=true;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
    {
        gbD60->Visible=true;
        temp=LastSet.dIndexLoadRate[1][1]*100;
        tbD60_Index56mm_NS->Position=int(temp);
        temp=LastSet.dIndexLoadRate[0][1]*100;
        tbD60_Index56mm->Position   =int(temp);

        edD60_56mm->Text            =LastSet.dIndexLoadRate[2][1];
        edD60_56mm->Visible         =(CosFunction.bUseLoadCellOffsetByHeater);
        labD60_1->Visible           =(CosFunction.bUseLoadCellOffsetByHeater);
        tbD60_Index56mm_NS->Visible =(CosFunction.bEPUseNSSLK);
        labD60_NS->Visible          =(CosFunction.bEPUseNSSLK);
        gbD60->Visible              =(CosFunction.bUseDynamicKitDiameter==false);   //Steven 20170605 : 可以自定義Kit直徑
    }
    else
    {
        gbD60->Visible=false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20170328 (Steven) EP 密碼輸入
    {
        gbD60->Enabled=false;
        gbD60->Color  =clRed;
    }

//[E]-------------------------
    //Steven 20090710 Start
    palE30->Visible=(cbE30->Checked)?true:false;  //Steven 20090904
    palE31->Visible=(cbE31->Checked)?true:false;  //Steven 20090904
    palE32->Visible=(cbE32->Checked)?true:false;  //Steven 20090904
    edE30_LodX->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale);
    edE30_HP1X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale[0]);
    edE30_HP2X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale[1]);
    edE30_LodY->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale);
    edE30_HP1Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale[0]);
    edE30_HP2Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale[1]);
//    edE31_Au1X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[0]);
//    edE31_Au2X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[1]);
//    edE31_Au3X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[2]);
//    edE31_Fi1X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[3]);
//    edE31_Fi2X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[4]);
//    edE31_Fi3X->Text=FormatFloat("0.0000", LastSet.fTrayXScale[5]);
//    edE31_Au1Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[0]);
//    edE31_Au2Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[1]);
//    edE31_Au3Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[2]);
//    edE31_Fi1Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[3]);
//    edE31_Fi2Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[4]);
//    edE31_Fi3Y->Text=FormatFloat("0.0000", LastSet.fTrayYScale[5]);
    edE32_IS1X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale[0]);
    edE32_IS2X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale[1]);
    edE32_IS1Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale[0]);
    edE32_IS2Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale[1]);
    edE32_OS1X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale[0]);
    edE32_OS2X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale[1]);
    edE32_OS1Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale[0]);
    edE32_OS2Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale[1]);
    //Steven 20090717 End

    pgcScale->ActivePage=tsE30;
    pgcE31->ActivePage=tsE31_1;

    if(Tri_Temp_Machine==1)                                                     //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
    {
        edE30_1_LodX->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale_Hot) ;
        edE30_1_HP1X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Hot[0]);
        edE30_1_HP2X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Hot[1]);
        edE30_1_LodY->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale_Hot) ;
        edE30_1_HP1Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Hot[0]);
        edE30_1_HP2Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Hot[1]);

        edE32_1_IS1X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Hot[0]) ;
        edE32_1_IS2X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Hot[1]) ;
        edE32_1_IS1Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Hot[0]) ;
        edE32_1_IS2Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Hot[1]) ;
        edE32_1_OS1X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Hot[0]);
        edE32_1_OS2X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Hot[1]);
        edE32_1_OS1Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Hot[0]);
        edE32_1_OS2Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Hot[1]);

        edE30_2_LodX->Text=FormatFloat("0.0000", LastSet.fLoaderTrayXScale_Cold) ;
        edE30_2_HP1X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Cold[0]);
        edE30_2_HP2X->Text=FormatFloat("0.0000", LastSet.fHotPlateXScale_Cold[1]);
        edE30_2_LodY->Text=FormatFloat("0.0000", LastSet.fLoaderTrayYScale_Cold) ;
        edE30_2_HP1Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Cold[0]);
        edE30_2_HP2Y->Text=FormatFloat("0.0000", LastSet.fHotPlateYScale_Cold[1]);

        edE32_2_IS1X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Cold[0]) ;
        edE32_2_IS2X->Text=FormatFloat("0.0000", LastSet.fInShuttleXScale_Cold[1]) ;
        edE32_2_IS1Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Cold[0]) ;
        edE32_2_IS2Y->Text=FormatFloat("0.0000", LastSet.fInShuttleYScale_Cold[1]) ;
        edE32_2_OS1X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Cold[0]);
        edE32_2_OS2X->Text=FormatFloat("0.0000", LastSet.fOutShuttleXScale_Cold[1]);
        edE32_2_OS1Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Cold[0]);
        edE32_2_OS2Y->Text=FormatFloat("0.0000", LastSet.fOutShuttleYScale_Cold[1]);
    }
    else
    {
        pnlE30_Hot->Visible =false;
        pnlE30_Cold->Visible=false;
        tsE31_Hot->TabVisible=false;
        tsE31_Cold->TabVisible=false;
        pnlE32_Hot->Visible=false;
        pnlE32_Cold->Visible=false;
    }

//[F]-------------------------
    if(Cylinder[C_Shuttle_Knocker_1].Enable==false &&
       Cylinder[C_Shuttle_Knocker_2].Enable==false)                             //Steven 20200522 : 氣缸沒裝就不要啟用功能
    {
        pnlF14->Visible=false;
    }

//[G]-------------------------

//[H]-------------------------

//[I]-------------------------
    if(CosFunction.bHaveFIFOMode)
    {
        AnsiString Dir;
        imgI37_3->Tag=IniConfig.iI37_LockLoaderDirection;
        Dir.sprintf("%stype%d.bmp", BmpPath, IniConfig.iI37_LockLoaderDirection);
        try
        {
            // GATE (W5-Picture) -- golden TImage->Picture is a TPicture with
            // LoadFromFile(). A headless build has no image subsystem, so
            // TfConfigurationImage models ->Tag only. Display-only; the file
            // path calculation above it stays live.
#if 0 // GATE (W5-Picture)
            imgI37_3->Picture->LoadFromFile(Dir);
#endif // GATE (W5-Picture)
        }
        catch(...)
        {
            MyDBIProcess("Exception", "TfConfiguration::FormShow", "");   // AI(W906-FW-CFG-W5): 顯式尾引數消歧義（兩個 2-arg 宣告可見），同 cObserver.cpp 的裁定
            ShowMyMessage("''Path :D:\\HT9045\\IMG\\BMP\\'' be delete");
        }
    }

    if(CosFunction.bTestTimeOutShowSkipAndHome)
    {
        // GATE (W5-Controls) -- `rgI22->Controls[1]` reaches a TRadioGroup's
        // design-time child. Same component-tree gap as GATE (W5-CCE);
        // vclcompat::TRadioGroup carries ItemIndex/Items only.
#if 0 // GATE (W5-Controls)
        rgI22->Controls[1]->Enabled=false;
#endif // GATE (W5-Controls)
        // GATE (W5-Controls) -- `rgI22->Controls[1]` reaches a TRadioGroup's
        // design-time child. Same component-tree gap as GATE (W5-CCE);
        // vclcompat::TRadioGroup carries ItemIndex/Items only.
#if 0 // GATE (W5-Controls)
        rgI22->Controls[2]->Enabled=false;
#endif // GATE (W5-Controls)
    }
//[J]-------------------------

//[K]-------------------------

//[L]-------------------------
    //jou 980928 start : High tempture 130 deg Position shift
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            if(abs(LastSet.TempPosShift_Shuttle[i][j])>1001)
                LastSet.TempPosShift_Shuttle[i][j]=0;
        }
    }
    edL09_Sh1L->Text=LastSet.TempPosShift_Shuttle[0][0];
    edL09_Sh1R->Text=LastSet.TempPosShift_Shuttle[0][1];
    edL09_Sh2L->Text=LastSet.TempPosShift_Shuttle[1][0];
    edL09_Sh2R->Text=LastSet.TempPosShift_Shuttle[1][1];

    //Steven 20110214 Start
    if(FIX3_FULL_PLACE==Fix3K_ShortShuttle)
    {
        edL09_Sh1L->Text=15;
        edL09_Sh1R->Text=90;
        edL09_Sh2L->Text=15;
        edL09_Sh2R->Text=90;
    }
    else if(ATC_SYSTEM>eATCUninstall)                                           //wei 20160329 ATC模式 Shuttle偏移
    {
        edL09_Sh1L->Text=15;
        edL09_Sh1R->Text=60;
        edL09_Sh2L->Text=15;
        edL09_Sh2R->Text=60;
    }
    else
    {
        edL09_Sh1L->Text=40;
        edL09_Sh1R->Text=130;
        edL09_Sh2L->Text=40;
        edL09_Sh2R->Text=130;
    }
    //Steven 20110214 End
    //jou 980928 end
    gbL11->Visible=(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber);              //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    if(Temperature.bUseTC2Offset)                                               //KenHsieh 20240311 : add Tc2 Offset
    {
        cbL11_6->Checked=false;
        cbL11_7->Checked=false;
        cbL11_8->Checked=false;
        IniConfig.bL11_6ATCUseTemperatureOutsideAlarm=false;
        IniConfig.bL11_7ATCUseMaxSurgeAlarm=false;
        IniConfig.bL11_8ATCUseTemperatureCompare=false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        // GATE (W5-ATCVer) -- `ATC_InterfaceForm->asATC_SW_Ver`. The global is
        // real (acarry_shims.h:115, TATC_InterfaceFormShim*) but that shim does
        // not carry asATC_SW_Ver -- and including acarry_shims.h here is
        // impossible anyway: it pulls in aHotPlateSubstrate.h, which redefines
        // `class TList` and `class uPlateInfo` against Public/HTEditList.h
        // :128/:321. Measured this wave: that include produced exactly those
        // two redefinitions and nothing else.
#if 0 // GATE (W5-ATCVer)
        if(ATC_SYSTEM==eNewATCSystem &&
           ATC_InterfaceForm->asATC_SW_Ver!="")                                 //KenHsieh 20240216 : add ATC Power Follow Function
        {
            iATCVerBuf=ATC_InterfaceForm->asATC_SW_Ver.Length()-5;
            str=ATC_InterfaceForm->asATC_SW_Ver.SubString(1, 3+iATCVerBuf);
            if(atof(str.c_str())>=2.5)
            {
                cbL43->Visible=true;
            }
            else
            {
                IniConfig.bL43EnableATCPowerFollow=false;
                cbL43->Checked=IniConfig.bL43EnableATCPowerFollow;
                cbL43->Visible=false;
            }
        }
        else                                                                  // AI(W906-FW-CFG-W5): 連 else 關鍵字一起 gate
#endif // GATE (W5-ATCVer)
        // ACTIVE ARM: golden 的 else 主體照跑。那是「沒有新版 ATC 版本字串」
        // 時的預設，對本 port 恆真（版本字串取不到），所以這是忠實的落點，
        // 不是隨便挑的預設值。
        {
            IniConfig.bL43EnableATCPowerFollow=false;
            cbL43->Checked=IniConfig.bL43EnableATCPowerFollow;
            cbL43->Visible=false;
        }
    }

    if(Tri_Temp_Machine!=1)
    {
        grpL31->Visible=false;
        grpL32->Visible=false;
        grpL33->Visible=false;
        grpL34->Visible=false;
        grpL35->Visible=false;
        grpL37->Visible=false;
        cbL37 ->Visible=false;
        cbL38 ->Visible=false;
        grpL39->Visible=false;
        labL40->Visible=false;
        edL40 ->Visible=false;
        labL41->Visible=false;
        edL41 ->Visible=false;
        cbL42 ->Visible=false;
        edtL42->Visible=false;
        cbL46 ->Visible=false;
        cbL44 ->Visible=false;
        edtL44->Visible=false;
        cbL45 ->Visible=false;
        edtL45->Visible=false;
        edtL09_1->Visible=false;
        edtL09_2->Visible=false;
    }
//[M]-------------------------
    bM01Enter=true;
    gbM01->Visible=cbM01->Checked;
    bM01Enter=false;

//[N]-------------------------
    pcN00->Visible                  =false;                                     //Steven 20110421

    if(CosFunction.bHiSiliconFunction  ||
       CosFunction.bUseLogUploadToFTPFunction ||
       CosFunction.bHisiLogUploadNetwork ||
       CUSTOMER_CODE==CC_ASE_KaohSiung ||                                       //kevin 20131009 高雄日月光強制TRUE
       CUSTOMER_CODE==CC_GM_TEST       ||                                       //Steven 20220520 : Add GM Test
       CUSTOMER_CODE==CC_ChipMos_ZHUBEI)
    {
        pcN00->Visible              =true;
    }

    tsN05->TabVisible               =false;                                     //wei 20150511
    gbN05_WebService->Visible       =(CUSTOMER_CODE==CC_SCK);                   //Steven 20161201 : For SCK Web Service
    tsN06->TabVisible               =false;                                     //wei 20150511
    tsN07->TabVisible               =false;                                     //wei 20150511

    edN04_Model->Text=IniConfig.sMachineType;
    edN04_ID->Text   =IniConfig.SocketHandlerID;
    edtN04_TesterID->Text           =IniConfig.RMSTesterID;                     //Ifor 20231219 add Tester ID

    if(CUSTOMER_CODE==CC_ASE_CL || IniConfig.bFTPJamCodeUpload)                 //Steven 20110210
    {
        if(MachineTypeChoice==Type_HT9045)                                      //Steven 20110708
            edN04_Model->Text="HT9045";
        else if(MachineTypeChoice==Type_HT9045_12Site)                          //ChungHung 20130507 add HT9045 updata for 12site 517
            edN04_Model->Text="HT9045_12Site";
        else
            edN04_Model->Text="HT9046";

        edN04_Model->Enabled=false;
        edN04_ID->Text=AnsiString(PC_NAME);
        edN04_ID->Enabled=false;
    }

    cbN05_EnableRMS->Checked=false;                                             //Steven 20110304 : 防呆
    gbN05_1->Visible        =(CosFunction.bUseERMS);                            //Steven 20160711 : 使用進階版RMS

    groupbN05_RTC->Visible=CosFunction.bRTCalarmUnload;                         //jou 20170210 (Steven) : RTC alarm image unload

    cbN06_EnableFTP->Checked=false;                                             //Steven 20110304 : 防呆

    if(EnableRMSFunc())
    {
        pcN00->Visible          =true;                                          //Steven 20110421
        tsN05->TabVisible       =true;
        gbN05->Caption          ="[N05] RMS Setting for Recipe File";
        labN05_1->Caption       ="Upload Path:";
        cbN05_EnableRMS->Caption="Enable RMS Connection";
        cbN05_EnableRMS->Checked=IniConfig.bEnableRms;
        edN05_RmsPath->Text     =IniConfig.sRmsPath;
        edN05_DownPath->Text    =IniConfig.sRmsDownPath;

        edN05_AmbTemp->Text     =IniConfig.fAmbientTemp;                        //Steven 20110421
        cbN05_CheckFile->Checked=IniConfig.bCheckFile;                          //Steven 20110421
        chkN05_TrayFeedClear->Checked=IniConfig.bClearLotInfoWhenTrayFeed;      //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
        btnUploadAll->Caption="Upload All";
    }
    else if(IniConfig.bShowLotInfo)
    {
        pcN00->Visible          =true;                                          //Steven 20110421
        if(CUSTOMER_CODE!=CC_TSMC_TAINAN &&                                     //wei 20161102 TSMC N05不顯示
           CUSTOMER_CODE!=CC_ASE_M &&                                           //Ifor 20170703 (wei) add ASEM 不顯示N05
           CUSTOMER_CODE!=CC_GIGAS)                                             //Isaac 20210128 : add 全智不顯示N05
            tsN05->TabVisible       =true;

        if(CosFunction.bDownloadRecipeLevelMode==true)                          //jou 2016-01-06 download recipe 增加權限模式選擇
        {
            labN05_DownPath->Caption="Engineer Path:";
        }
        else
        {
            edN05_DownPath->Visible =false;
            labN05_DownPath->Visible=false;
        }

        gbN05->Caption          ="[N05] Server Setting for Recipe File";
        labN05_1->Caption       ="Server Path:";
        cbN05_EnableRMS->Caption="Enable Network Connection";
        cbN05_EnableRMS->Checked=IniConfig.bEnableRms;

        edN05_RmsPath->Text     =IniConfig.sRmsPath;
        edN05_AmbTemp->Text     =IniConfig.fAmbientTemp;
        cbN05_CheckFile->Checked=IniConfig.bCheckFile;

        if(IniConfig.bVTESTFunction==true)
        {
            labN05_AmbTemp->Visible=false;
            edN05_AmbTemp->Visible=false;
            btnMesSystem->Visible=true;
            #ifndef SOFT_SIMULTE                                                //RogerYang 20250218 軟體模擬可以修改
            cbN05_EnableRMS->Enabled=false;
            cbN05_CheckFile->Enabled=false;
            #endif
        }

        cbN05_1->Checked        =IniConfig.bEnableErms;
        edN05_1->Text           =IniConfig.sErmsPath;

        checkbN05_RTC->Checked  =IniConfig.bN05_RTCalarmUnload;                 //jou 20170210 (Steven) : RTC alarm image unload
        editN05_RTC->Text       =IniConfig.asN05_RTCalarmUnload;                //jou 20170210 (Steven) : RTC alarm image unload

        if(CosFunction.bDownloadRecipeLevelMode)                                //jou 2016-01-06 download recipe 增加權限模式選擇
            edN05_DownPath->Text=IniConfig.sRmsDownPath;

        chkN05_TrayFeedClear->Checked=IniConfig.bClearLotInfoWhenTrayFeed;      //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        tsN05->TabVisible     =false;                                           //Ifor 20161229 (Steven) KYEC 喬智要求關閉N05 頁面及功能
        labN07_5->Visible     =true;
        edN07_5->Visible      =true;
    }
    else
    {
        labN07_5->Visible     =false;
        edN07_5->Visible      =false;
    }

    if(CosFunction.bFTPFunction)                                                //Steven 20091004 for KYEC 29818 : [I16] Never Enable and Always True    //Steven 20140609 : else if --> if
    {
        pcN00->Visible              =true;                                      //Steven 20110421
        tsN06->TabVisible           =true;
        cbN06_EnableFTP->Checked    =IniConfig.bEnableFTP;
        edN06_UserName->Text        =IniConfig.FtpUserName;
        edN06_Password->Text        =IniConfig.FtpPassword;
        edN06_HostName->Text        =IniConfig.FtpHost;
        edN06_DownPath->Text        =IniConfig.FtpDownloadPath;
        edN06_UpLdPath->Text        =IniConfig.FtpUplaodPath;
        gbN06_HddLevel->ItemIndex   =IniConfig.iHDEnable;
        gbN06_ServerLv->ItemIndex   =IniConfig.iServerEnable;
        if(CUSTOMER_CODE==CC_JSCC_OS)                                           //長電微電子 (JSCC OS部門)
        {
            edN06_TesterMap->Text   =IniConfig.asN06_TesterPath;
        }
        else
        {
            edN06_TestList->Text    =IniConfig.N06_TasterListFile;
            edN06_TesterMap->Text   =IniConfig.N06_TasterListMap;
        }
        edN06_Port->Text            =IniConfig.N06_FtpPort;                     //Ifor 20201015 add:使用者自定義 FTP Port
        cbbN06_Mode->ItemIndex      =IniConfig.FtpTransMode;                    //Steven 20230719 : 加入FTP傳輸模式

        if(CosFunction.PassworDownloadByFTP)                                    //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
        {
            cbN06_1->Visible=true;
            cbN06_1->Checked=IniConfig.bFtpPasswordDownload;
            edN06_1->Visible=true;
            edN06_1->Text=IniConfig.FtpPasswordDownloadPath;
        }
        else
        {
            cbN06_1->Visible=false;
            cbN06_1->Checked=false;
            edN06_1->Visible=false;
        }

        labN06_TesterMap->Visible           =false;
        edN06_TesterMap->Visible            =false;
        btN06_TesterMap->Visible            =false;
        labN06_TesterList->Visible          =false;
        edN06_TestList->Visible             =false;
        btN06_TesterList->Visible           =false;
        btN06_UpdateTesterList->Visible     =false;

        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            labN05_1->Visible               =false;
            edN05_RmsPath->Visible          =false;
            cbN05_CheckFile->Visible        =false;
            cbN05_CheckFile->Enabled        =false;
            labN05_AmbTemp->Visible         =false;
            edN05_AmbTemp->Visible          =false;
        }
        else if(CUSTOMER_CODE==CC_JSCC_OS)                                      //長電微電子 (JSCC OS部門)
        {
            labN06_TesterMap->Visible       =true;
            labN06_TesterMap->Caption       ="OS Tester Path:";
            edN06_TesterMap->Visible        =true;
        }
        else if(CUSTOMER_CODE==CC_PTI)                                          //RogerYang 20170406 (Steven) 力成 add FTP upload for Jam Alarm Data
        {
            gbN06_HddLevel->Visible         =false;
            gbN06_ServerLv->Visible         =false;
            labN06_DownloadPath->Visible    =false;
            edN06_DownPath->Visible         =false;
            labN06_FileName->Visible        =true;
            edN06_FileName->Visible         =true;
            edN06_FileName->Text            =IniConfig.asN06_FileName;
        }
        else if(CUSTOMER_CODE==CC_Greatek)                                      //Sam 20170815 (Steven) add 超豐不顯示TesterMap
        {
            gbN06_HddLevel->Visible         =false;
            gbN06_ServerLv->Visible         =false;
        }
    }

    cbN06_UseSystemCall->Visible=false;

    if(IniConfig.bFTPJamCodeUpload)                                             //ChungHung 20140108 add FTP unload jam code
    {
        pcN00->Visible              =true;                                      //Steven 20110421
        tsN06->TabVisible           =true;
        cbN06_EnableFTP->Checked    =IniConfig.bEnableFTP;
        edN06_UserName->Text        =IniConfig.FtpUserName;
        edN06_Password->Text        =IniConfig.FtpPassword;
        edN06_HostName->Text        =IniConfig.FtpHost;
        edN06_DownPath->Text        =IniConfig.FtpDownloadPath;
        edN06_UpLdPath->Text        =IniConfig.FtpUplaodPath;
        gbN06_HddLevel->ItemIndex   =IniConfig.iHDEnable;
        gbN06_ServerLv->ItemIndex   =IniConfig.iServerEnable;
        edN06_Port->Text            =IniConfig.N06_FtpPort;                     //Ifor 20201015 add:使用者自定義 FTP Port

        gbN06_HddLevel->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        gbN06_ServerLv->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        btN06_UpdateTesterList->Visible =false;                                 //ChungHung 20140108 add FTP unload jam code
        labN06_DownloadPath->Visible    =false;                                 //ChungHung 20140108 add FTP unload jam code
        edN06_DownPath->Visible         =false;                                 //ChungHung 20140108 add FTP unload jam code
        labN06_UploadPath->Caption      = "Server Path:";                       //ChungHung 20140108 add FTP unload jam code
    }

    if(CosFunction.bEnable_SECS_GEM)                                            //ChungHung 20150515 add for SECS GEM 顯示
    {
        bSecsGemStatus=IniConfig.bEnable_SECS_GEM;                              //wei 20160308 開啟SECSGEM功能
        pcN00->Visible                  =true;                                  //wei 20150511
        tsN07->TabVisible               =(CosFunction.bEnable_SECS_GEM);        //Steven 20141006 : SECS GEM使用Remote Start功能
        pnlN07_2->Visible               =(CosFunction.bRCMDStart);
    }
    // GATE (W5-FormHS) -- `FormHS` is another form entirely and
    // GetUploadServerByFTPPath has no port anywhere in the tree. The caption
    // it would fill (lblUploadRealPath) simply stays empty.
#if 0 // GATE (W5-FormHS)
    lblUploadRealPath->Caption=FormHS->GetUploadServerByFTPPath("EventLog", true);
#endif // GATE (W5-FormHS)

//[O]-------------------------
    gbO06->Visible=false;
    if(IniConfig.bEventLogAutoSaveFunction)                                     //Steven 20110221 Start : EventLogAutoSave
    {
        gbO06->Visible              =true;
        dtO06_LastDate->Date        =IniConfig.dtEventLogLastRecordDate;
        dtpO06NextTime->Time        =IniConfig.dtEventLogLastRecordDate;
        for(int i=0; i<7; i++)
        {
            if(IniConfig.bAutoSaveLogWeek[i]==true)
                strngrdAutoSaveLog->Cells[i][1]="On";
            else
                strngrdAutoSaveLog->Cells[i][1]="";
        }
    }
    cbO08->Visible=false;                                                       //Steven 20140221 : Mark

    if(CosFunction.bUseHeadContactCount==true)                                  //Ifor 20191128 : Fix 銦片 Life Time 功能無顯示
    {
        paLifeTime->Visible=!(CUSTOMER_CODE==CC_SCC);
    }

//[P]-------------------------
    #ifndef SOFT_SIMULTE
    pnlP13->Visible=(CosFunction.bKnockerSetBySetupFile==false);                //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    pnlP16->Visible=(CosFunction.bKnockerSetBySetupFile==false &&               //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
                     Cylinder[C_HotplateVibration].Enable==true);               //Steven 20120510 : 全部改用汽缸的Enable判斷
    #endif

    // GATE (W5-WriteContact) -- TfConfiguration::WriteContactData is not
    // translated yet (it is part of the deferred golden ctor cluster, golden
    // :109-225). Calling it is what golden does here; the method lands with
    // that ctor wave.
#if 0 // GATE (W5-WriteContact)
    WriteContactData();                                                         //kevin 20170329 (Steven) 將CONTRACT 資料寫到INI 給 客戶檢查
#endif // GATE (W5-WriteContact)

    if(INSTALL_OCR!=eocrUninstal)                                               //ChungHung 20121002 add OCR Function
        gbP23_OCR->Visible=true;
    else
        gbP23_OCR->Visible=false;

    if(IniConfig.iOCRConditions==0)
        rgP23_OCRByNewTray->Checked=true;
    else if(IniConfig.iOCRConditions==1)
        rgP23_OCRByInitialStart->Checked=true;
    else
        rgP23_OCRByNewTray->Checked=true;

    gbP26_OCRCheck->Visible=CosFunction.bTrayOCR;                               //wei 20161125 因修改不使用IniConfig.iOCRConditions判斷OCR模式，所以移除
    for(int i=0; i<10; i++)                                                     //wei 20151117 OCR Lot check
    {
        edOCRTrayLot[i]->Text=LastSet.TrayCount[i];
        edOCRTrayLot[i]->Tag=i;
    }

    gbUnloadMode->Visible=(CosFunction.bUnloadTrayModeByRecipe);                //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
//------------------------------------------------------------------------------

    // GATE (W5-Geometry) -- form position. `class TfConfiguration` has no TForm
    // base and declares no geometry member; identical to cObserver.cpp GATE
    // (FW3A-3). Nothing in this port reads a form position back.
#if 0 // GATE (W5-Geometry)
    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
#endif // GATE (W5-Geometry)
    bEnterOffset=true;

    if(CUSTOMER_CODE==CC_MTI)                                                   //Steven 20110131
    {
        btResume->Visible=true;
    }
    else
    {
        btResume->Visible=false;
    }

    Timer1->Enabled=true;

//請保持在最下面--------------------------------------------------------------
    // GATE (W5-ShowMemo) -- TfConfiguration::ShowMemo is not translated. Its
    // own content source, InitialMemo, walks a TWinControl control tree this
    // facade does not have (the same gap as GATE (W5-CCE)), so translating it
    // would produce empty memos anyway. Deliberately deferred, see DEVLOG.
#if 0 // GATE (W5-ShowMemo)
    ShowMemo();                                                                 //Steven 20120218 : 這行得在所有的Visible完成後才能執行
#endif // GATE (W5-ShowMemo)
    //Steven 20120807 Start: 在最下面決定權限能不能Enable                       //Steven 20140805 : 低權限可以看不能改
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_A1, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);  //JerryYang 20190928 為了讓[A30] Setup function不被鎖住
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_A2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_A3, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_A4, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_C1, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[68])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_C2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[68])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D3, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D4, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D5, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D6, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D7, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_D8, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[69])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pgcScale, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[3]==false)?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_E2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[11]==false)?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_E5, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[70] || authConf[11]==false)?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_F00,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_F10,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_F20,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[71])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_G,  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[72])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_I01,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_I20,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_I30,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_I40,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[73])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_L1, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_L2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_L3, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[74])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_O1, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[75])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_O2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[75])?false:true);
#endif // GATE (W5-CCE)
    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210128 : 為了讓Enable FTP不被鎖住
    {
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(gbN04  , (AccessLevel<LevelSet.AccessLevel[76])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN05  , (AccessLevel<LevelSet.AccessLevel[76])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_UserName  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_Password  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_HostName  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_DownPath  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_UpLdPath  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_Port      , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_TesterMap , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(edN06_TestList  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(gbN06_HddLevel  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(gbN06_ServerLv  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)

#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN07  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN08  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN09  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN10  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN11  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN12  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN13  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN14  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN15  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN16  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(grpN20 , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN22  , (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[67])?false:true);
#endif // GATE (W5-CCE)
    }
    else
    {
#if 0 // GATE (W5-CCE)
        ChangeCompomentEnabled(tsN00,  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[76])?false:true);
#endif // GATE (W5-CCE)
    }
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_P0, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_P1, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_P2, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_P3, (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[77])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pnlTray,(AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[78])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pnlHP,  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[79])?false:true);
#endif // GATE (W5-CCE)
#if 0 // GATE (W5-CCE)
    ChangeCompomentEnabled(pal_M,  (AccessLevel<LevelSet.AccessLevel[30] || AccessLevel<LevelSet.AccessLevel[96])?false:true);
#endif // GATE (W5-CCE)
    //Steven 20120807 End: 在最下面決定權限能不能Enable
//    CheckEnabled(); //jou 2012-06-27 必須在ChangeCompomentEnabled之後         //Steven 20120912 : 會發生被Disable後,無法Enable的狀況

    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       (CUSTOMER_CODE==CC_KYEC_XILINX && IniConfig.bChangeKitNoHardStop==true && IniConfig.bRemeberAutoHeight==true))   //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
    {
        palD01->Visible=true;
    }
    else
    {
        palD01->Visible=false;
    }
//    gbL09->Enabled=false;                                                     //JerryYang 20230131 : L09功能可以開關
    edL09_Sh1L->Enabled=false;
    edL09_Sh1R->Enabled=false;
    edL09_Sh2L->Enabled=false;
    edL09_Sh2R->Enabled=false;

//    pal_A1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_A2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;       //wei 20160225 分頁
//    pal_C->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D3->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D4->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D5->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_D6->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_E1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_E2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_F00->Enabled=(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_F10->Enabled=(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_G->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_I->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_L1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_L2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_O1->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_O2->Enabled =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_N->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_P->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;
//    pal_M->Enabled  =(AccessLevel<LevelSet.AccessLevel[30])?false:true;

    strngrdAutoSaveLog->Cells[0][0]="Sun";
    strngrdAutoSaveLog->Cells[1][0]="Mon";
    strngrdAutoSaveLog->Cells[2][0]="Tue";
    strngrdAutoSaveLog->Cells[3][0]="Wed";
    strngrdAutoSaveLog->Cells[4][0]="Thu";
    strngrdAutoSaveLog->Cells[5][0]="Fri";
    strngrdAutoSaveLog->Cells[6][0]="Sat";

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160101 海思專用版本 顯示不可修改溫度
    {
        pal_L1->Enabled=false;
        pal_L2->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_JCET)
    {
        btN06_UpdateTesterList->Visible=false;
    }

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //KaiChen 20200623 ：矽格北興，不給修改
    {
        //Sam 20220706 : 北興俊堯說可以再打開修改 Mark
        //==>
        //cbP13       ->Enabled=false;
        //edP13_1     ->Enabled=false;
        //edP13_2     ->Enabled=false;
        //<==
        //Sam 20220706 : 北興俊堯說可以再打開修改 Mark
        cbP14       ->Enabled=false;
        edP14_1     ->Enabled=false;
        edP14_2     ->Enabled=false;
    }

    if(CosFunction.bUseConfigSaveButton)    //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    {
        btnSave->Visible=true;
        bSave=false;
    }
    else
    {
        btnSave->Visible=false;
    }
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, "fConfiguration", asLogPath);                        //kevin 20181025 change//Steven 20100629
}

// =============================================================================
// FW-CFG-W6 -- 17 個 UI 事件處理器（merged 20260825）
// =============================================================================
// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6165-6195, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index60mmChange(void *Sender)
{
    AnsiString buffer="60mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="58 : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="75mm : ";
    else
        buffer="60mm : ";
    labD25_1->Caption=buffer+AnsiString(tbD25_Index60mm->Position/100.0);

    if(old60data==0.0)
        old60data=tbD25_Index60mm->Position;

    if(old60data!=tbD25_Index60mm->Position)
    {
        aEP60OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old60data, tbD25_Index60mm->Position); //Ifor 20170807 (wei) Sb1 =>aEPOldData  //JerryYang 20170922 (Steven) add for EP change log
        bEP60DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag
        if(old60data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old60data=115;
            tbD25_Index60mm->Position=115;                                      //Ifor 20170807 (wei) tbD25_Index30mm => tbD25_Index60mm
        }

        if(old60data<85)
        {
            old60data=85;
            tbD25_Index60mm->Position=85;                                       //Ifor 20170807 (wei) tbD25_Index30mm => tbD25_Index60mm
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6197-6228, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index40mmChange(void *Sender)
{
    AnsiString buffer="40mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="40mm : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="55mm : ";
    else
        buffer="40mm : ";
    labD25_2->Caption=buffer+AnsiString(tbD25_Index40mm->Position/100.0);

    if(old40data==0.0)
        old40data=tbD25_Index40mm->Position;

    if(old40data!=tbD25_Index40mm->Position)
    {
        aEP40OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old40data, tbD25_Index40mm->Position); //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP40DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag

        if(old40data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old40data=115;
            tbD25_Index40mm->Position=115;                                      //kevin 20170609 (wei) add EP 保護
        }

        if(old40data<85)
        {
            old40data=85;
            tbD25_Index40mm->Position=85;                                       //kevin 20170609 (wei) add EP 保護
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6230-6261, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index30mmChange(void *Sender)
{
    AnsiString buffer="30mm : ";                                                //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="28mm : ";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="40mm : ";
    else
        buffer="30mm : ";

   labD25_3->Caption=buffer+AnsiString(tbD25_Index30mm->Position/100.0);
    if(old30data==0.0)
        old30data=tbD25_Index30mm->Position;

    if(old30data!=tbD25_Index30mm->Position)
    {
        aEP30OldData.sprintf("EP %s mm : %4d -> %4d", buffer, old30data, tbD25_Index30mm->Position);   //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP30DataChange=true;                                                   //Ifor 20170807 (wei) add EP Data Change Flag

        if(old30data>115)                                                       //jou 20171227 (Steven) : >= -> > 修正海思EP 拉霸無法調整
        {
            old30data=115;
            tbD25_Index30mm->Position=115;                                      //kevin 20170609 (wei) add EP 保護
        }

        if(old30data<85)
        {
            old30data=85;
            tbD25_Index30mm->Position=85;                                       //kevin 20170609 (wei) add EP 保護
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6454-6483, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index60mm_NSChange(void *Sender)
{
    AnsiString buffer="60mm for NS:";                                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="58mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="75mm for NS:";

    labD25_1_NS->Caption=buffer+AnsiString(tbD25_Index60mm_NS->Position/100.0);

    if(old60data_NS==0.0)
        old60data_NS=tbD25_Index60mm_NS->Position;

    if(old60data_NS!=tbD25_Index60mm_NS->Position)
    {
        aEP60OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old60data_NS, tbD25_Index60mm_NS->Position);   //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP60DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old60data_NS>=115)
        {
            old60data_NS=115;
            tbD25_Index60mm_NS->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old60data_NS<=85)
        {
            old60data_NS=85;
            tbD25_Index60mm_NS->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6485-6514, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index40mm_NSChange(void *Sender)
{
    AnsiString buffer="40mm for NS: ";                                          //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="40mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="55mm for NS:";

    labD25_2_NS->Caption=buffer+AnsiString(tbD25_Index40mm_NS->Position/100.0);

    if(old40data_NS==0.0)
        old40data_NS=tbD25_Index40mm_NS->Position;

    if(old40data_NS!=tbD25_Index40mm_NS->Position)
    {
        aEP40OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old40data_NS, tbD25_Index40mm_NS->Position);  //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP40DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old40data_NS>=115)
        {
            old40data_NS=115;
            tbD25_Index40mm_NS->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old40data_NS<=85)
        {
            old40data_NS=85;
            tbD25_Index40mm_NS->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6516-6544, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD25_Index30mm_NSChange(void *Sender)
{
    AnsiString buffer="28mm for NS:";                                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //wei 20151005 add 56mm
        buffer="28mm for NS:";
    else if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        buffer="40mm for NS:";

    labD25_3_NS->Caption=buffer+AnsiString(tbD25_Index30mm_NS->Position/100.0);
    if(old30data_NS==0.0)
        old30data_NS=tbD25_Index30mm_NS->Position;

    if(old30data_NS!=tbD25_Index30mm_NS->Position)
    {
        aEP30OldData_NS.sprintf("EP %s mm : %4d -> %4d", buffer, old30data_NS, tbD25_Index30mm_NS->Position);  //Ifor 20170807 (wei) Sb1 =>aEPOldData
        bEP30DataChange_NS=true;                                                //Ifor 20170807 (wei) add EP Data Change Flag
        if(old30data_NS>=115)
        {
            old30data_NS=115;
            tbD25_Index30mm_NS->Position=115;                                   //kevin 20170609 (wei) add EP 保護
        }

        if(old30data_NS<=85)
        {
            old30data_NS=85;
            tbD25_Index30mm_NS->Position=85;                                    //kevin 20170609 (wei) add EP 保護
        }
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6546-6561, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD60_Index56mmChange(void *Sender)
{
    AnsiString buffer="56mm : ",Sb1="",Sb2="",Sb3="";                           //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    static int olddata=0.0;

    labD60->Caption=buffer+AnsiString(tbD60_Index56mm->Position/100.0);
    if(olddata==0.0)
        olddata=tbD60_Index56mm->Position;

    if(olddata!=tbD60_Index56mm->Position)
    {
        Sb1.sprintf("EP %s mm : %4d -> %4d", buffer, olddata, tbD60_Index56mm->Position/100.0);
        RecordProcess(Sb1);                                                     //kevin 20170328 (Steven) add EP修改紀錄
        olddata=tbD60_Index56mm->Position;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6563-6578, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::tbD60_Index56mm_NSChange(void *Sender)
{
    AnsiString buffer="56mm for NS : ",Sb1="",Sb2="",Sb3="";                    //kevin 20170328 (Steven) add 顯示 layout kit 浮動頭外徑
    static int olddata=0.0;

    labD60_NS->Caption=buffer+AnsiString(tbD60_Index56mm_NS->Position/100.0);
    if(olddata==0.0)
        olddata=tbD60_Index56mm_NS->Position;

    if(olddata!=tbD60_Index56mm_NS->Position)
    {
        Sb1.sprintf("EP %s mm : %4d -> %4d", buffer, olddata, tbD60_Index56mm_NS->Position/100.0);
        RecordProcess(Sb1);                                                     //kevin 20170328 (Steven) add EP修改紀錄
        olddata=tbD60_Index56mm_NS->Position;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6286-6297, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbD55Click(void *Sender)
{
    if(cbD55->Checked)                                                          //ChungHung 20120710 add SCK Disable IndexCheck
    {
        // GATE (W6-COM2) -- third site of the same missing member: COM2 is
        // TCOM2Shim (atester_shims.h:405) and bCCDDummyRum is not one of its
        // members. golden here is a DISJUNCTION, so in principle the
        // REAL_TIME_CCD half could be kept as GATE (CFG2-COM2) did -- but this
        // if exists only to pop a refusal message, and half a refusal is worse
        // than none: it would refuse on machines golden lets through.
        // ACTIVE ARM: nothing. cbD55 keeps whatever the operator set.
#if 0 // GATE (W6-COM2)
        if(COM2->bCCDDummyRum || REAL_TIME_CCD==false)                          //RTC 沒有開不能Disable IndexCheck
        {
            ShowMyMessage("Must cancel \"Enable Real Time CCD\" first", "");
            cbD55->Checked=false;
        }
#endif // GATE (W6-COM2)
        return;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6299-6310, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbE38Click(void *Sender)
{
    if(cbE38->Checked)
    {
        cbE40->Checked=false;
        cbE40->Visible=false;
    }
    else
    {
        cbE40->Visible=true;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6263-6275, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbE39Click(void *Sender)
{
    if(cbE39->Checked)                                                          //ChungHung 20120206 Hotplate check
    {
        cbE39_1->Visible=true;
        cbE39_1->Checked=IniConfig.bE39_1PutTheDevicesToErrorBin;
    }
    else
    {
        cbE39_1->Visible=false;
        cbE39_1->Checked=false;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6445-6452, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbD36Click(void *Sender)
{
    if(cbD36->Checked==true)                                                    //jou 2014-06-24 RTC 自動進行Model驗證
    {
        cbD33->Checked=false;
        cbD35->Checked=true;
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6425-6438, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbM01Click(void *Sender)
{
    TCheckBox *ptr=(TCheckBox*)Sender;
    if(bM01Enter==true || fShow==false)
        return;
    bM01Enter=true;

    // GATE (W6-DoPassword) -- TfConfiguration::DoPassword (golden :6376-6423)
    // is not translated: it reaches fInput / fMain / fPassword across three
    // other forms. Deliberately deferred, see DEVLOG 20260825 XI.
    // ACTIVE ARM: the checkbox keeps the value the operator just set -- the
    // same outcome golden reaches when the password check PASSES.
#if 0 // GATE (W6-DoPassword)
    if(DoPassword()!=true)
    {
        ptr->Checked=!ptr->Checked;
    }
#endif // GATE (W6-DoPassword)
    gbM01->Visible=cbM01->Checked;
    bM01Enter=false;
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6019-6024, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::cbE30Click(void *Sender)                    //Steven 20090904
{
    palE30->Visible=(cbE30->Checked)?true:false;
    palE31->Visible=(cbE31->Checked)?true:false;
    palE32->Visible=(cbE32->Checked)?true:false;
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:6010-6017, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::pcConfigChange(void *Sender)                //Steven 20090731
{
    if(AccessLevel>=LevelSet.AccessLevel[42])
    {
        // 同 GATE (W5-CCE)：TWinControl 祖先缺口，函式本體本來就是 gated no-op。
#if 0 // GATE (W5-CCE)
        if(pcConfig->ActivePage!=tsSearchFunction)                              //Steven 20210730 : 快速搜尋Config
            ChangeCompomentEnabled(pcConfig->ActivePage, authConf[pcConfig->ActivePageIndex]);
#endif // GATE (W5-CCE)
    }
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:5995-6008, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::PageControl1Change(void *Sender)
{
    if(CUSTOMER_CODE==CC_ASE_KaohSiung && PageControl1->ActivePageIndex==2)     //kevin 20180411 key in password
    {
        // GATE (W6-bPassWord) -- TfConfiguration::bPassWord (golden :6781-6798)
        // is not translated yet. Same posture as GATE (W6-DoPassword): the
        // ACTIVE arm is the password-accepted path.
#if 0 // GATE (W6-bPassWord)
        if(bPassWord(2)==false)
        {
            PageControl1->ActivePageIndex=0;
            return;
        }
#endif // GATE (W6-bPassWord)
    }

        // 同 GATE (W5-CCE)：TWinControl 祖先缺口，函式本體本來就是 gated no-op。
#if 0 // GATE (W5-CCE)
    if(pcConfig->ActivePage!=tsSearchFunction)                                  //Steven 20210730 : 快速搜尋Config
        ChangeCompomentEnabled(PageControl1->ActivePage, authConfig[PageControl1->ActivePageIndex]);
#endif // GATE (W5-CCE)
}

// AI(W906-FW-CFG-W6) 20260825: golden cConfiguration.cpp:5988-5993, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::udD46Click(void *Sender)                    // AI(W906-FW-CFG-W6): golden 的第二個參數
                                                                                // TUDBtnType Button 全函式從未被讀，
                                                                                // 依本 port「只留有讀到的參數」規則移除；
                                                                                // TUDBtnType 本身在本樹也沒有 port。
{
    edD46->Text=udD46->Position;
    IniConfig.iD46WaitIndexDestroyTime=udD46->Position;
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:60 的檔案層陣列。
// golden 在建構子 :153 用 `new TCheckBox(this)` 逐一配置，然後 :154 立刻
// `->Parent=gbSendTemp`——那條路走 TWinControl 控制項樹，本 facade 沒有
// （同 GATE (W5-CCE)），所以本樹保持零初始化。
// 這不是新加的降級：golden 自己的 btHeaterSelectAll/ClearSelect 就寫了
// `if(cbTempSelsct[i]!=NULL)`（golden :5470、:5481），對 NULL 的容忍是 golden 的。
TCheckBox *cbTempSelsct[tcTotalCount];

// =============================================================================
// FW-CFG-W7 -- 16 個 UI 處理器（merged 20260825；
//              btnSetToTechClick 與 LoadConfiguration 退出，理由見各自位置）
// =============================================================================
// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:7113-7146, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::edtSearchFunctionChange(void *Sender)       //Steven 20210730 : 快速搜尋Config
{
    if(edtSearchFunction->Text.Length()==0)
    {
        for(int i=0; i<elConfig->FEditList->Count; i++)
        {
            THTEdit *Temp;
            Temp=(THTEdit*)elConfig->FEditList->Items[i];
            Temp->SetToDefaultPosition();
        }
    }
    else if(edtSearchFunction->Text.Length()>=2)
    {
        for(int i=0; i<elConfig->FEditList->Count; i++)
        {
            THTEdit *Temp;
            Temp=(THTEdit*)elConfig->FEditList->Items[i];
            TCheckBox    *Chb = dynamic_cast <TCheckBox   *>(Temp->SourceControl);
            if(Chb!=NULL)
            {
                AnsiString Str=Chb->Caption.UpperCase();
                if(Str.AnsiPos(edtSearchFunction->Text.UpperCase())!=0)
                {
#if 0 // GATE (W7-Search) -- golden 原文保留
                    // `class TScrollBox` 全樹零 port（forms/fSetup.h:324-325 為了
                    // golden cSetUp.h:84 的 scrlbxSocketSensor 已經量過同一件事），
                    // 而 vclcompat::TControl 沒有 Parent / Align，`alTop` 也沒有。
                    // 量測 20260825:
                    //   grep -rn "class TScrollBox" --include=*.h --include=*.cpp  -> 0
                    //   grep -rn "alTop"            --include=*.h                 -> 0
                    // 這兩行是「把命中搜尋字串的控制項搬進搜尋捲軸並靠上排列」，
                    // 純版面。不命中的那一支 SetToDefaultPosition() 保持 live，
                    // 所以搜尋框仍會把不符的項目歸位，只是不會把符合的搬過去。
                    Temp->SourceControl->Parent=scrlbxSearch;
                    Temp->SourceControl->Align=alTop;
#endif
                }
                else
                {
                    Temp->SetToDefaultPosition();
                }
            }
        }
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5444-5466, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::SetSoftSpeedSpeed(bool clear,int value)
{
    if(clear)
    {
        for(int i=0; i<TOTAL_MOTOR; i++)
        {
            edSoftSpeed[i]->Text="1000";
            LastSet.SoftSpeed[i]=1000;
        }
    }
    else
    {
        for(int i=0; i<TOTAL_MOTOR; i++)
        {
            LastSet.SoftSpeed[i]+=value;
            if(LastSet.SoftSpeed[i]>100000)
                LastSet.SoftSpeed[i]=100000;
            if(LastSet.SoftSpeed[i]<1000)
                LastSet.SoftSpeed[i]=1000;
            edSoftSpeed[i]->Text=LastSet.SoftSpeed[i];
        }
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6687-6698, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::sbN15UserLevelByTxtReadFilePathClick(
      void *Sender)                                                          //Sam 20170824 (Steven) 移植超豐 ESD Control 功能 form HT-7045
{
    OpenDialog2->Filter="*.TXT|*.TXT";
    OpenDialog2->DefaultExt="TXT";
    OpenDialog2->InitialDir="c:\\";

    if(OpenDialog2->Execute())
    {
        edtN15_1->Text=OpenDialog2->FileName;
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6705-6716, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::spbA25_RunExecutFilePathChoiceClick(
      void *Sender)
{
    OpenDialog3->Filter="*.EXE|*.EXE";
    OpenDialog3->DefaultExt="EXE";
    OpenDialog3->InitialDir="c:\\";

    if(OpenDialog3->Execute())
    {
        edA25_1->Text=OpenDialog3->FileName;
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5468-5475, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btHeaterSelectAllClick(void *Sender)
{
    for(int i=0; i<tcTotalCount; i++)
    {
        if(cbTempSelsct[i]!=NULL)                                               //Steven 20200609 : 加上保護
            cbTempSelsct[i]->Checked=true;
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5477-5484, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btHeaterClearSelectClick(void *Sender)
{
    for(int i=0; i<tcTotalCount; i++)
    {
        if(cbTempSelsct[i]!=NULL)                                               //Steven 20200609 : 加上保護
            cbTempSelsct[i]->Checked=false;
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6585-6591, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnRecordJamRateByTimeClearClick(void *Sender)      // 2015.11.11 , Joye , Add Jam Rate Record
{
    iRecordJamRateByTime_LoaderCount    = 0;
    iRecordJamRateByTime_JamCount       = 0;

    bRecordJamRateByTime_Clear = true;
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5956-5960, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btD47Click(void *Sender)
{
    LastSet.iD47SocketTestedCount=0;
    edD47_3->Text=LastSet.iD47SocketTestedCount;
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:7148-7152, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnSetIPSCQtyClick(void *Sender)
{
#if 0 // GATE (W7-IPSC) -- golden 原文保留
    // 跨表單：`fProductionInfo` 是 ProductionInfo/ProductionInfo.h:572 的
    // `extern PACKAGE TfProductionInfo *fProductionInfo`，本樹沒有對應的 port
    // 全域（forms/fCounterClear.h:47-53 為了同一個物件已經開過 gate）。
    // 量測 20260825: grep -rn "TfProductionInfo \*fProductionInfo" 於 port 樹 -> 0
    fProductionInfo->iIPSCFlag_TriggerMode=1;
    fProductionInfo->iIPSCFlag_CountdownQty=atoi(edtSetIPSCQty->Text.c_str());
#endif
}

// AI(W906-FW-CFG-W7) 20260825: LoadConfiguration (golden :7591-7595) 本波刻意不翻。
// 本體只有兩行 `ReadLastDataFile(); ReadLastSetIni();`，名字全是 Read，但
// ReadLastSetIni（cprod.cpp:2958-3071）**會寫回產線設定檔**：
//     :2993 WriteIniData(szDir, "Hotplate Form", "Using Flag", ...)
//     :2998 WriteIniData(sPath, "Tray", "bRecordSkipPosition", ...)
//     :3037 WriteIniDataGeneral("System", "USE_SOCKET_SENSOR", ...)   <- system\Gerneral.ini
// 最後那個就是本樹 20260817 被整檔重寫過的那支檔案。write path，佇列。
// （ReadLastDataFile 本身三個 CreateFile 都是 GENERIC_READ/OPEN_EXISTING，
//   對檔案是唯讀的；但它 :1769 呼叫 fMain->ModifyTester(OFF_LINE) 切 tester 上下線。）
//
// 這條是新加的 deep pass（tools/wavescan/freefunc_index.py）抓到的，
// 人工讀本體讀不出來——兩行都叫 Read。

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5434-5437, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnDec1000Click(void *Sender)
{
    SetSoftSpeedSpeed(false, -1000);
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5439-5442, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnSetTo1000Click(void *Sender)
{
    SetSoftSpeedSpeed(true, 0);
}

// AI(W906-FW-CFG-W7) 20260825: btnSetToTechClick (golden :5978-5981) 本波刻意不翻。
// 它整個本體就是呼叫 cinitial.cpp:13925 的自由函式 SetOffsetToTech()，而那支會
//     Tech.iInArmLoadStageX += InArmOffSet[InOfsLoader]->GetX();   (golden :13958 起)
// 把目前的 offset 累加進 Tech.*——那是入料/出料手臂與 shuttle 的教導座標，
// 馬達實際會走到的位置。golden 自己也知道，:13950 先跳一個
// MessageDlg("Sure to Set Offset To Tech?") 要人按確認。
// 安全項，佇列等使用者，不自己做。
//
// 順帶記一筆篩選器的第三個缺口：screen_methods.py 只掃 handler 自己的本體，
// **不跟進自由函式**，所以這支的本體「只有一行函式呼叫」被判成乾淨。
// 前兩個缺口是跨表單呼叫（btnN31_Manual 那批）與 CopyFile（ReadConfigStandard）。

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5429-5432, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnAdd1000Click(void *Sender)
{
    SetSoftSpeedSpeed(false, 1000);
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:5973-5976, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnAdd10000Click(void *Sender)
{
    SetSoftSpeedSpeed(false, 10000);
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6331-6337, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btnAutoSaveSetAllClick(void *Sender)
{
    for(int i=0; i<7; i++)
    {
        strngrdAutoSaveLog->Cells[i][1]="On";
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6157-6163, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btN06_TesterListClick(void *Sender)
{
    if(OpenDialog1->Execute())
    {
        edN06_TestList->Text=OpenDialog1->FileName;
    }
}

// AI(W906-FW-CFG-W7) 20260825: golden cConfiguration.cpp:6339-6345, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfConfiguration::btN06_TesterMapClick(void *Sender)
{
    if(OpenDialog1->Execute())
    {
        edN06_TesterMap->Text=OpenDialog1->FileName;
    }
}
