// =============================================================================
//  forms/fBuilder.cpp  --  definitions for the fBuilder facade
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (1 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cBuilder.cpp (573 lines)
//  + cBuilder.h (67 lines), read with `io.open(p, encoding='cp950')`.
//  SPAN: 22 golden `TfBuilder::` member bodies, 531 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 22-member denominator:
//     13 ACTIVE           123 of 531 golden span lines (23.2%)
//      9 GATED-WITH-BODY  408 golden span lines carried as `#if 0` transcript
//                         below -- every one of the nine is the READ-ONLY-
//                         DIRECTION safety rule, NOT a link boundary.
//
//  See forms/fBuilder.h for the full GATE REGISTER (B-1)..(B-9), the ACTIVE
//  reachability evidence, DEVIATIONS D-1..D-9, the FIELD LIST (including the 8
//  widgets deliberately NOT declared) and the ZERO-WRITER notes about
//  `asBackupCreate`, `DirectoryListBox1->Directory` and `Checked[]`.
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall` and the unused FormClose
//  parameters), so it is a faithful TRANSCRIPT -- not verified code.  Many of
//  the identifiers it names (oFile, Handle, Application, FileInfo,
//  fMain->cbSetupFileName, fMain->LookForFile, fHotPlate, fContact, FTestIF,
//  fYieldMonitoring, fSetup, fTemp_Set, fSpeed, fTrayAssignment, fBinSel,
//  fTrayForm, fLd_ULd, fCCLink, SaveTestMode, HSys, ...) are NOT members of
//  this facade and/or have no reachable definition; un-gating requires
//  supplying them first AND a decision from the user that this tree may write
//  to D:\HT9045\.
//
//  ⚠ GOLDEN BUG PRESERVED VERBATIM in GATE (B-5): `bool bDelOK=SHFileOperation
//  (&oFile);` then `if(bDelOK==true) ShowMyMessage("Delete Setup File ERROR")`.
//  SHFileOperation returns 0 on SUCCESS, so the variable's NAME is inverted
//  relative to its meaning.  Not "fixed" -- 改行為要留給使用者決定.
//
//  BACKSLASH-COMMENT SCAN (the -Wcomment line-splice trap): all five golden
//  files of this wave were scanned 20260828 for a `//` comment whose line ends
//  in a backslash.  ZERO hits in cBuilder.cpp -- so no comment delimiter was
//  changed anywhere in this file, and every transcript below is byte-faithful
//  to golden apart from the `__fastcall` / parameter reductions named above.
//
//  Steven 20260925 (Data.Builder web page): the "#if 0 ... NEVER BEEN COMPILED"
//  statement above is history for (B-2)..(B-8) -- they are compiled and live now
//  (Steven authorised web create/delete, 20260925), see the section after
//  FormClose and forms/fBuilder.h's 20260925 block. (B-1) and (B-9) are still
//  `#if 0`. This file now WRITES under D:\HT9045\IniData\Data and
//  D:\HT9045\IniData\Offset (create / delete / import) and under whatever
//  directory the operator browsed to (export).
// =============================================================================
#include "forms/fBuilder.h"
#include "forms/fQwertyKey.h"    // fQwertyKey->ShowQwertyKey (forms/fQwertyKey.cpp, ht9045_forms)
#include "common.h"              // DataPath / OnlyMakeFileDataInPut (common.cpp, ht9045_core)
#include "cmydef.h"              // N_NO_SYMBOL (extern const int, cmydef.cpp, ht9045_globals)
#include "canary_support.h"      // ShowMyMessage -- body canary_support.cpp (ht9045_sm), one of the four sanctioned forms->sm exceptions
#include "forms/fMain.h"         // Steven 20260925: fMain->cbSetupFileName / LookForFile (forms/fMain.cpp, same ht9045_forms target) -- (B-2)/(B-4)
#include "CosFunction.h"         // Steven 20260925: CosFunction.bBuilderImportSingleFolder (CosFunction.cpp, ht9045_globals) -- (B-6)

#include <windows.h>             // WIN32_FIND_DATA / FindFirstFile / FindNextFile / FindClose (ACTIVE InitCompData); same include forms/fMesSystem.cpp:24 already uses in this target
#include <shellapi.h>            // Steven 20260925: SHFILEOPSTRUCT / SHFileOperation (shell32, in the default MinGW and MSVC link libraries)
#include <cstring>               // strcmp (ACTIVE InitCompData, golden :379-380)
#include <string>

// AI(W906-FW3-BTQ1) 20260828: TfBuilder/fBuilder were FREE tree-wide -- same
// idiom as forms/fCounterSel.cpp:38 / forms/fLd_ULd.cpp:43 /
// forms/fTesterIF.cpp:24.  Golden's ctor body (:17-21) is a single own-field
// assignment, so this static-init `new` touches no global -- no SIOF risk
// (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL 全域"; the fLaserSensor
// incident that rule comes from turned 88 of 134 ctest binaries into
// SEGFAULTs).  The two facade-local widget stand-ins constructed by the field
// initialisers are leaf containers (a TStringList and an AnsiString) and call
// nothing translated.
TfBuilder *fBuilder = new TfBuilder();

TfBuilder::TfBuilder()   // golden :17-21
{
    asBackupCreate="";

    // Steven 20260925: dfm/VCL defaults the live bodies depend on -- own widgets
    // only (ctor-safety rule above). vclcompat widgets start Visible=Enabled=false
    // (vclcompat/Controls.h DEFAULT-VALUE RULE); golden cBuilder.dfm sets neither
    // on these controls, so they are the VCL default TRUE, and a TComboBox starts
    // with no selection (ItemIndex -1).
    cbSourceFile->Visible=true;       cbSourceFile->Enabled=true;       cbSourceFile->ItemIndex=-1;
    edNewFileName->Visible=true;      edNewFileName->Enabled=true;
    btCreateSetupFile->Visible=true;  btCreateSetupFile->Enabled=true;
    cbDeleteFile->Visible=true;       cbDeleteFile->Enabled=true;       cbDeleteFile->ItemIndex=-1;
    btDeleteSetupFile->Visible=true;  btDeleteSetupFile->Enabled=true;
    spbImport->GroupIndex=1;          spbExport->GroupIndex=1;          // dfm :158/:174 (AllowAllUp)
    // VCL TDirectoryListBox.Create: `GetDir(0, FDirectory); { initially use current
    // dir on default drive }` (BCB6 Source\vcl\filectrl.pas:875). A Win32 query, no
    // other global touched.
    char cwd[MAX_PATH]="";
    const DWORD n=GetCurrentDirectoryA(MAX_PATH, cwd);
    if(n>0 && n<MAX_PATH)
        DirectoryListBox1->Directory=cwd;
}

// ---------------------------------------------------------------------------
//  golden :23-33.  ACTIVE -- and it is ACTIVE only because InitCompData and
//  ShowDirBoxPath are.  Golden's FormShow is a pure display refresh: it opens
//  no file and creates no directory.
// ---------------------------------------------------------------------------
void TfBuilder::FormShow(TObject *Sender)
{
    (void)Sender;
    InitCompData();
    ShowDirBoxPath();
    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
    asBackupCreate="";

    btCreateSetupFile->Top=88;
    fShow=true;
}

// ---------------------------------------------------------------------------
void TfBuilder::cbSourceFileChange(TObject *Sender)   // golden :35-41
{
    (void)Sender;
    if(cbSourceFile->Text!="")
        edNewFileName->Enabled=true;
    else
        edNewFileName->Enabled=false;
}

// ---------------------------------------------------------------------------
void TfBuilder::edNewFileNameChange(TObject *Sender)   // golden :43-49
{
    (void)Sender;
    if(edNewFileName->Text!="")
        btCreateSetupFile->Enabled=true;
    else
        btCreateSetupFile->Enabled=false;
}

// ---------------------------------------------------------------------------
void TfBuilder::cbDeleteFileChange(TObject *Sender)   // golden :194-200
{
    (void)Sender;
    if(cbDeleteFile->Text!="")
        btDeleteSetupFile->Enabled=true;
    else
        btDeleteSetupFile->Enabled=false;
}

// ---------------------------------------------------------------------------
void TfBuilder::DirectoryListBox1Change(TObject *Sender)   // golden :262-265
{
    (void)Sender;
    ShowDirBoxPath();
}

// ---------------------------------------------------------------------------
//  golden :267-272.  ACTIVE.
//  ⚠ CONSEQUENCE, not a defect: `DirectoryListBox1->Directory` has no writer in
//  this port (golden's writer is the VCL control reacting to the operator's
//  browse), so this always renders "Path:".  See the header's ZERO-WRITER note.
//  Steven 20260925: superseded -- the ctor seeds Directory with the current
//  directory (VCL default) and WebBuilder.cpp writes it when the operator browses.
// ---------------------------------------------------------------------------
void TfBuilder::ShowDirBoxPath()
{
    AnsiString str;
    str.sprintf("Path:%s", DirectoryListBox1->Directory);
    labDir->Caption=str;
}

// ---------------------------------------------------------------------------
//  golden :337-348.  ACTIVE.  Pure read; no side effect of any kind.
//  ⚠ CONSEQUENCE: nothing ticks CheckListBox1 in this port, so this always
//  returns false.  Its only caller is GATE (B-7) anyway.
//  Steven 20260925: superseded -- WebBuilder.cpp ticks Checked[] from the page
//  before (B-7) spbExportClick, which is live now.
// ---------------------------------------------------------------------------
bool TfBuilder::NeedExport(AnsiString sName)
{
    for(int i=0; i<CheckListBox1->Items->Count; i++)
    {
        if(CheckListBox1->Checked[i])
        {
            if(CheckListBox1->Items->Strings[i]==sName)
                return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
//  golden :350-394.  ACTIVE.  READ-ONLY directory enumeration -- it opens
//  nothing, creates nothing, writes nothing; it only lists the sub-folders of
//  DataPath into the three list widgets.
//
//  ⚠ golden :373 really does append a SECOND wildcard: :370 has already made
//  SDataPath end in "\\*.*", and :373 then searches `SDataPath+"*"`, i.e.
//  "...\\*.**".  Transcribed verbatim (改行為要留給使用者決定).
//  ⚠ WHAT THIS ACTUALLY TOUCHES: `DataPath` is NOT empty offline -- it is a
//  HARD-CODED initialiser, common.cpp:104
//  `AnsiString DataPath = "D:\\HT9045\\IniData\\Data\\";` -- and that
//  directory EXISTS on this machine (checked 20260828).  So this body really
//  enumerates the shared recipe folder and fills the three list widgets with
//  the real recipe names.  It stays ACTIVE because enumeration is a READ: no
//  handle is opened for writing, no directory is created, nothing is deleted.
//  Golden's "File Path Lost" branch fires only if that directory is missing.
// ---------------------------------------------------------------------------
void TfBuilder::InitCompData()
{
    WIN32_FIND_DATA filedata;                                                   // Structure for file data
    HANDLE filehandle;                                                          // Handle for searching

    AnsiString szFile="", szDir="";
    AnsiString SDataPath=DataPath;

    cbSourceFile->Clear();
    cbDeleteFile->Clear();
    CheckListBox1->Clear();
    // Steven 20260925: VCL TCustomCombo.Clear is `SetTextBuf(''); FItems.Clear;`
    // (BCB6 Source\vcl\stdctrls.pas:2338-2343) -- the text and the selection go too.
    // vclcompat's TComboBox::Clear only empties Items, so the rest is done here.
    cbSourceFile->Text=""; cbSourceFile->ItemIndex=-1;
    cbDeleteFile->Text=""; cbDeleteFile->ItemIndex=-1;
    if(!DirectoryExists(SDataPath))
    {
        ShowMyMessage("File Path Lost");
        return;
    }
    else if(DirectoryExists(SDataPath))
    {
        if(SDataPath.SubString(SDataPath.Length(), 1)!="\\")
        {
            SDataPath=SDataPath+"\\*.*";
        }

        filehandle=FindFirstFile((SDataPath+"*").c_str(), &filedata);
        if(filehandle!=INVALID_HANDLE_VALUE)                                    //Steven 20101118 Start : 不要顯示資料夾以外的檔案
        {
            do
            {
                if((filedata.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)!=0 ||
                    strcmp(filedata.cFileName, ".")==0 ||                       /* 不處理隱藏檔及 . 跟 .. */
                    strcmp(filedata.cFileName, "..")==0)
                {
                    continue;
                }
                else if(filedata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)     //如果該檔案為資料夾
                {
                    cbSourceFile->Items->Add(filedata.cFileName);
                    cbDeleteFile->Items->Add(filedata.cFileName);
                    CheckListBox1->Items->Add(filedata.cFileName);
                }
            } while(FindNextFile(filehandle, &filedata));
            FindClose(filehandle);
        }
    }
    (void)szFile;
    (void)szDir;
}

// ---------------------------------------------------------------------------
void TfBuilder::spbExitClick(TObject *Sender)   // golden :484-487
{
    (void)Sender;
    Close();   // DEVIATION D-3 -- port-only no-op
}

// ---------------------------------------------------------------------------
//  golden :489-493.  ACTIVE.  DEVIATION D-6: golden's unused TMouseButton /
//  TShiftState / int X / int Y parameters are dropped (neither type has a port
//  and the body reads none of them).
// ---------------------------------------------------------------------------
void TfBuilder::edNewFileNameMouseDown(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL);
}

// ---------------------------------------------------------------------------
void TfBuilder::edNewFileNameKeyPress(TObject *Sender, char &Key)   // golden :495-500
{
    (void)Sender;
    if(OnlyMakeFileDataInPut(Key)==false)
        Key=0;   // golden `Key=NULL;` on a char -- faithful as 0, DEVIATION D-7
}

// ---------------------------------------------------------------------------
void TfBuilder::FormClose()   // golden :502-505, DEVIATION D-4
{
    fShow=false;
}

// ===========================================================================
//  Steven 20260925 (Data.Builder web page) -- WRITE DIRECTION.
//  Golden re-read: HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cBuilder.cpp (cp950).
//  (B-2)..(B-8) are LIVE below (Steven 20260925: "可以從網頁做" create/delete);
//  (B-1) and (B-9) stay `#if 0` -- reasons in forms/fBuilder.h's 20260925 block.
//  The golden text is kept line for line; every changed line carries a
//  "golden ..." note. The port-only helpers come first.
// ===========================================================================

// golden common.cpp:162 `SHFILEOPSTRUCT oFile;` (extern in common.h:190), shared by
// the five SHFileOperation bodies below. ht9045_core (common.cpp) does not define
// it in this tree, so this TU keeps its own; nothing outside this file uses it.
static SHFILEOPSTRUCT oFile;

// golden V912 ProductionInfo/FileInfo.cpp:290-313 FileInfo::PathCombin, verbatim.
// FileInfo lives in ht9045_sm, which ht9045_forms may not link; common.cpp's
// Common_PathCombin is the same kind of TU-local copy (of the 906 text -- V912
// added the `sFile.AnsiPos("\\")!=1` protection, copied here).
static AnsiString W906_PathCombin(const AnsiString& sPath, const AnsiString& sFile)
{
    AnsiString combinedPath=sPath;
    if(!combinedPath.IsEmpty())
    {
        if(combinedPath[combinedPath.Length()]=='/' || combinedPath.Pos("/"))   // FTP path format
        {
            if(combinedPath[combinedPath.Length()]!='/')
            {
                combinedPath+="/";
            }
        }
        else                                                                    // Local path
        {
            if(combinedPath[combinedPath.Length()]!='\\' &&
               sFile.AnsiPos("\\")!=1)                                          //Steven 20260708 : Add protection
            {
                combinedPath+="\\";
            }
        }
    }
    combinedPath+=sFile;
    return combinedPath;
}

// ---------------------------------------------------------------------------
//  W906ShFileOp -- every golden `SHFileOperation(&oFile)` below goes through here.
//  Two port-only changes, nothing else:
//   (1) pFrom/pTo are re-packed with an explicit double NUL. SHFileOperation takes
//       NUL-separated lists ended by an empty string. golden builds most pFrom in
//       zeroed char[] buffers (fine), but CopySourTarget passes
//       `oFile.pTo=DataPath.c_str();` (:294/:322) -- one NUL, the second one is
//       whatever byte follows the AnsiString buffer.
//   (2) FOF_NOERRORUI is added. With error UI, SHFileOperation opens a modal box
//       on the machine's desktop and does not return until someone clicks it.
//       wb_serve runs this on its one tick thread (canary_support.h, the
//       W906_DiagnosticsWindowOpen_Hook threading note), so a locked file would
//       stall the machine loop behind a box the web operator -- possibly at another
//       PC -- cannot see. The return code goes to the browser instead
//       (vW906ShOps). golden's create path already passes FOF_NOERRORUI (:121).
//  FOF_ALLOWUNDO / FOF_NOCONFIRMATION / FOF_NOCONFIRMMKDIR stay as golden set them
//  (FO_DELETE with FOF_ALLOWUNDO sends the folder to the Recycle Bin).
// ---------------------------------------------------------------------------
static int W906ShFileOp(TfBuilder *f, SHFILEOPSTRUCT *op)
{
    std::string from = op->pFrom ? std::string(op->pFrom) : std::string();
    std::string to   = op->pTo   ? std::string(op->pTo)   : std::string();
    std::string fromList = from; fromList.push_back('\0');   // + c_str()'s own NUL = double NUL
    std::string toList   = to;   toList.push_back('\0');

    SHFILEOPSTRUCT o = *op;
    o.pFrom  = fromList.c_str();
    o.pTo    = op->pTo ? toList.c_str() : NULL;
    o.fFlags = (FILEOP_FLAGS)(op->fFlags | FOF_NOERRORUI);
    o.fAnyOperationsAborted = FALSE;
    const int ret = SHFileOperation(&o);
    op->fAnyOperationsAborted = o.fAnyOperationsAborted;

    TfBuilderShOp rec;
    rec.wFunc   = (int)op->wFunc;
    rec.from    = from.c_str();
    rec.to      = to.c_str();
    rec.flags   = (int)o.fFlags;
    rec.ret     = ret;
    rec.aborted = (o.fAnyOperationsAborted != FALSE);
    f->vW906ShOps.push_back(rec);
    return ret;
}

// ---------------------------------------------------------------------------
//  W906_MessageBox -- stands in for golden `Application->MessageBox(text, caption,
//  flags)` at :95, :218, :407, :472, :480. wb_serve has no modal box and its
//  dispatch cannot wait for one, so the browser asks the OK/CANCEL question first
//  (two-phase, WebBuilder.cpp): phase 1 runs the handler with the answer preset to
//  IDCANCEL and learns from bW906MsgBoxReached where golden stopped to ask; phase 2
//  re-runs it with IDOK. MB_OK notices are recorded and answered IDOK at once.
// ---------------------------------------------------------------------------
int TfBuilder::W906_MessageBox(const AnsiString &text, const char *caption, int flags)
{
    if((flags & MB_TYPEMASK)==MB_OKCANCEL)
    {
        bW906MsgBoxReached=true;
        asW906MsgBoxText=text;
        asW906MsgBoxCaption=caption ? caption : "";
        return iW906MsgBoxAnswer;
    }
    vW906InfoBoxes.push_back(text);
    return IDOK;
}

void TfBuilder::W906_ResetTrace()
{
    bW906MsgBoxReached=false;
    asW906MsgBoxText="";
    asW906MsgBoxCaption="";
    vW906InfoBoxes.clear();
    vW906ShOps.clear();
}

#if 0 // GATE (B-1) Create2DCodeWorkFile -- golden :51-70.  WRITES DISK:
      // `CopyFile(str2.c_str(), str1.c_str(), false)` at golden :66, into
      // D:\HT9045\Barcode_File\<port>\<recipe>.cfg.  Also needs FileInfo()
      // (no port resolved) and HSys.
      // Steven 20260925: STILL GATED -- its only caller (btCreateSetupFileClick
      // :97) runs it for CUSTOMER_CODE==CC_KYEC_XILINX with a 2D-code reader, and
      // Steven's instruction for this page is 客戶專屬條件先跳過、只註記.
void TfBuilder::Create2DCodeWorkFile()                                          //wei 20160802 add 2DCodeCreateWorkFile
{
    AnsiString SPath="D:\\HT9045\\Barcode_File\\";
    AnsiString OrgPath=cbSourceFile->Text+AnsiString(".cfg");
    AnsiString NewPath=edNewFileName->Text+AnsiString(".cfg");
    AnsiString asStr;
    AnsiString str1, str2;

    for(int i=0; i<4; i++)
    {
        asStr=FileInfo().PathCombin(SPath, HSys.asBarCodeComPort[i]);
        str2=FileInfo().PathCombin(asStr, OrgPath);
        if(FileExists(str2))
        {
            str1=FileInfo().PathCombin(asStr, NewPath);
            CopyFile(str2.c_str(), str1.c_str(), false);
            MySleep(50);
        }
    }
}
#endif // GATE (B-1)

// ---------------------------------------------------------------------------
//  btCreateSetupFileClick -- golden :72-159. LIVE (was GATE B-2).
//  WRITES: MyForceDirectories(DataPath\<new>) and (OffsetPath\<new>) (:106), then
//  FO_COPY of <source>\*.* into each (:115-122). A <new> that already exists is
//  copied INTO (FOF_NOCONFIRMATION overwrites same-named files) -- golden has no
//  "already exists" check and none is added; the page warns before asking.
//  Then the three lists get the name appended (not re-enumerated) and fMain's
//  recipe combo is refreshed (LookForFile is the facade's counter stub; the Text
//  it restores is the current recipe, unchanged).
// ---------------------------------------------------------------------------
void TfBuilder::btCreateSetupFileClick(TObject *Sender)
{
    (void)Sender;
    if(cbSourceFile->Text=="")
    {
        ShowMyMessage("Must select Source Setup data","必須選擇來源檔");
        return;
    }

    if(edNewFileName->Text=="")
    {
        ShowMyMessage("Must set setup name","必須設定setup名字");
        return;
    }

    AnsiString str;
    char cStr1[256]="", cStr2[256]="";                                          //20111026 jou
    AnsiString OrgPath="", NewPath="", sString, JOBFILEPath="";
    AnsiString SPath[2]={DataPath, OffsetPath};

    AnsiString SBuffer=edNewFileName->Text;
    SBuffer=SBuffer.UpperCase();
    (void)SBuffer;                                                              // golden computes it and never reads it

    str.sprintf("Do you create ''%s'' as the name", edNewFileName->Text);
    if(W906_MessageBox(str, "Builder", MB_OKCANCEL|MB_TOPMOST)==IDOK)          // golden :95 Application->MessageBox(str.c_str(), ...)
    {
#if 0   // GATE (B-1) -- customer CC_KYEC_XILINX (Steven 20260925: 客戶專屬條件先跳過)
        if(CUSTOMER_CODE==CC_KYEC_XILINX && BAR_CODE_INSTALL==ebctInShtIntel)   //wei 20160802 add 2DCodeCreateWorkFile
        {
            Create2DCodeWorkFile();
        }
#endif

        for(int i=0; i<2; i++)
        {
            OrgPath=W906_PathCombin(SPath[i], cbSourceFile->Text);              // golden FileInfo().PathCombin
            NewPath=W906_PathCombin(SPath[i], edNewFileName->Text);             // golden FileInfo().PathCombin
            MyForceDirectories(NewPath);

            //------------------20111026    jou------------------
            OrgPath+="\\*.*";
            ZeroMemory(&cStr1, sizeof(cStr1));
            ZeroMemory(&cStr2, sizeof(cStr2));
            strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
            strncpy(cStr2, NewPath.c_str(), sizeof(cStr2));
            //複製工作檔
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=NULL;                                                    // golden Handle (no form window)
            oFile.wFunc=FO_COPY;
            oFile.pFrom=cStr1;                                                  //The list of names must be double null-terminated.
            oFile.pTo=cStr2;
            oFile.hNameMappings=NULL;
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
            W906ShFileOp(this, &oFile);                                         // golden SHFileOperation(&oFile)
            #ifdef ASE_KaohSiung                                                //kevin 20150105 start
            JOBFILEPath=W906_PathCombin(SPath[i], "JOBFILE");
            if(JOBFILEPath.Pos("JOBFILE"))                                      //kevin 20150120
                continue;

            MyForceDirectories(JOBFILEPath);
            ZeroMemory(&cStr1, sizeof(cStr1));
            ZeroMemory(&cStr2, sizeof(cStr2));
            strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
            strncpy(cStr2, JOBFILEPath.c_str(), sizeof(cStr2));

            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=NULL;                                                    //複製工作檔
            oFile.wFunc=FO_COPY;
            oFile.pFrom=cStr1;
            oFile.pTo=cStr2;
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
            W906ShFileOp(this, &oFile);
            #endif                                                              //kevin 20150105 end

            //---------------------------------------------------

            if(i==2)                                                            //Steven 20101118 : 強制要加加
                i++;
        }
        (void)JOBFILEPath;
        cbSourceFile->Items->Add(edNewFileName->Text);
        cbDeleteFile->Items->Add(edNewFileName->Text);
        CheckListBox1->Items->Add(edNewFileName->Text);
        asBackupCreate=edNewFileName->Text;
        edNewFileName->Text="";

        sString=fMain->cbSetupFileName->Text;
        fMain->cbSetupFileName->Clear();
        fMain->LookForFile();
        fMain->cbSetupFileName->Text=sString;
    }
}

// ---------------------------------------------------------------------------
//  bSaveAsJobFile -- golden :163-192. LIVE (was GATE B-3). RESERVED NAME.
//  On this build only :190-191 run (memory): the copy block is
//  `#ifdef ASE_KaohSiung` (customer macro, not defined in CMakeLists.txt), and so
//  are all six callers (FileRW/HotPlateForm_File.cpp:237, FileRW/TestIF_File.cpp:
//  496/2629/4565, forms/fLd_ULd.cpp:344, forms/fTesterIF.cpp:1573).
// ---------------------------------------------------------------------------
// KEVIN 20150107 Save As JobFile
void TfBuilder::bSaveAsJobFile(AnsiString SourceFileFileName, AnsiString FileName)
{
    AnsiString OrgPath="", NewPath="", sString, JOBFILEPath="";
    AnsiString SPath[2]={DataPath, OffsetPath}, sBuffer=SourceFileFileName.UpperCase();

    if(sBuffer.Pos(FileName))                                                   //檔案名稱比較
        return;

    for(int i=0; i<2; i++)
    {
        OrgPath=W906_PathCombin(SPath[i], SourceFileFileName)+"\\*.*";          // golden FileInfo().PathCombin
        #ifdef ASE_KaohSiung                                                    //kevin 20150105 start
        JOBFILEPath=W906_PathCombin(SPath[i], FileName);
        MyForceDirectories(JOBFILEPath);
        ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
        oFile.hwnd=NULL;                                                        // golden Handle
        oFile.wFunc=FO_COPY;
        oFile.pFrom=OrgPath.c_str();
        oFile.pTo=JOBFILEPath.c_str();
        oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
        W906ShFileOp(this, &oFile);                                             // golden SHFileOperation(&oFile)
        #endif                                                                  //kevin 20150105 end

        //---------------------------------------------------
        if(i==2)                                                                //Steven 20101118 : 強制要加加
            i++;
    }
    (void)NewPath; (void)sString; (void)JOBFILEPath; (void)OrgPath;
    CheckListBox1->Items->Add(FileName);
    asBackupCreate=FileName;
}

// ---------------------------------------------------------------------------
//  btDeleteSetupFileClick -- golden :202-230. LIVE (was GATE B-4).
//  golden's own guards run first: empty selection (:205-209) and the IN-USE
//  check against fMain->cbSetupFileName->Text (:211-215; wb_serve sets that Text
//  to GetLastOpenFN() at boot, tools/wb_serve.cpp "golden's startup mirror").
//  After the question -- answered either way -- golden refreshes fMain's combo,
//  re-enumerates the three lists and clears the selection (:223-229).
// ---------------------------------------------------------------------------
void TfBuilder::btDeleteSetupFileClick(TObject *Sender)
{
    (void)Sender;
    AnsiString str, sString;
    if(cbDeleteFile->Text=="")
    {
        ShowMyMessage("Must select delete Setup data","必須選擇刪除檔");
        return;
    }

    if(cbDeleteFile->Text==fMain->cbSetupFileName->Text)                        //2012-01-13    Dell    工作檔del後,程式無法開啟
    {
        ShowMyMessage("File in use, cannot delete", "工作檔正在使用用中");
        return;
    }

    str.sprintf("Do you delete ''%s'' as the name", cbDeleteFile->Text);
    if(W906_MessageBox(str, "Builder", MB_OKCANCEL|MB_TOPMOST)==IDOK)          // golden :218 Application->MessageBox(str.c_str(), ...)
    {
        DeleteSetupFile(cbDeleteFile->Text);
    }

    sString=fMain->cbSetupFileName->Text;
    fMain->cbSetupFileName->Clear();
    fMain->LookForFile();
    fMain->cbSetupFileName->Text=sString;

    InitCompData();
    cbDeleteFile->Text="";
}

// ---------------------------------------------------------------------------
//  DeleteSetupFile -- golden :232-260. LIVE (was GATE B-5).
//  DELETES DataPath\<name> and OffsetPath\<name> (FO_DELETE, FOF_ALLOWUNDO ->
//  Recycle Bin). IsFileInUse on a directory path: CreateFile without
//  FILE_FLAG_BACKUP_SEMANTICS fails with ACCESS_DENIED, not a sharing violation,
//  so the 5 s MySleep only fires for a path that is really held open.
//  ⚠ GOLDEN BUG PRESERVED VERBATIM: `bool bDelOK=SHFileOperation(&oFile);` is true
//  on FAILURE (SHFileOperation returns 0 on success), so the error message fires
//  on failure -- correct behaviour through an inverted name. Not "fixed".
// ---------------------------------------------------------------------------
void TfBuilder::DeleteSetupFile(AnsiString DeleteFileName)                       //Steven 20110305
{
    char str1[256]="";
    AnsiString OrgPath="";
    AnsiString SPath[2]={DataPath, OffsetPath};

    for(int i=0; i<2; i++)
    {
        OrgPath=W906_PathCombin(SPath[i], DeleteFileName);                     // golden FileInfo().PathCombin
        if(DirectoryExists(OrgPath))
        {
            if(IsFileInUse(OrgPath.c_str())==true)                              //Jimmychiu 20241121 : Verify whether the file is currently in use
            {
                MySleep(5000);
            }
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=NULL;                                                    // golden Handle
            oFile.wFunc=FO_DELETE;
            strncpy(str1, OrgPath.c_str(), sizeof(str1));
            oFile.pFrom=str1;
            oFile.fFlags=FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
            bool bDelOK=W906ShFileOp(this, &oFile);                             // golden SHFileOperation(&oFile)
            if(bDelOK==true)
            {
                ShowMyMessage("Delete Setup File ERROR");
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  CopySourTarget -- golden :274-335. LIVE (was GATE B-6). WRITES into DataPath.
//  CosFunction.bBuilderImportSingleFolder (customer TSMC switch in CosFunction.cpp)
//  copies the browsed folder itself; otherwise, once PER SUB-FOLDER found in it
//  (golden quirk, kept: N sub-folders = N identical copies), everything under it
//  (`<dir>\*.*`, files and folders) is copied into DataPath. Same-named recipes
//  are overwritten (FOF_NOCONFIRMATION) -- including the one in use; golden has
//  no guard and none is added.
//  ⚠ golden's source buffer is `char Orgstr[128]`: a <dir>\*.* of 128 chars or
//  more is truncated without a terminator in golden. WebBuilder.cpp refuses such
//  a directory before calling (golden limit, not a new one).
// ---------------------------------------------------------------------------
void TfBuilder::CopySourTarget(AnsiString Sour)
{
    char Orgstr[128]="";
    TSearchRec SearchRec;
    AnsiString SDataPath=Sour;
    int iAttr=faDirectory;                                                      //檔案
    if(!DirectoryExists(SDataPath))
    {
        ShowMyMessage("File Path Lost");
        return;
    }
    else if(DirectoryExists(SDataPath))
    {
        if(CosFunction.bBuilderImportSingleFolder)                              //ChungHung 20150413 add for TSMC //ChungHung 20150415 add for TSMC
        {
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=NULL;                                                    // golden Handle
            oFile.wFunc=FO_COPY;
            strncpy(Orgstr, SDataPath.c_str(), sizeof(Orgstr));
            oFile.pFrom=Orgstr;
            oFile.pTo=DataPath.c_str();
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION);
            W906ShFileOp(this, &oFile);                                         // golden SHFileOperation(&oFile)
        }
        else
        {
            if(SDataPath.SubString(SDataPath.Length(), 1)!="\\")
            {
                SDataPath=SDataPath+"\\*.*";
            }
            else
            {
                SDataPath=SDataPath+"*.*";                                      //Steven 20110621
            }

            if(FindFirst(SDataPath, iAttr, SearchRec)==0)
            {
                do
                {
                    if((SearchRec.Attr&iAttr)==faDirectory)
                    {
                        if(SearchRec.Name!="." && SearchRec.Name!="..")
                        {
                            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
                            oFile.hwnd=NULL;                                    // golden Handle
                            oFile.wFunc=FO_COPY;
                            strncpy(Orgstr, SDataPath.c_str(), sizeof(Orgstr));
                            oFile.pFrom=Orgstr;
                            oFile.pTo=DataPath.c_str();
                            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION);
                            W906ShFileOp(this, &oFile);                         // golden SHFileOperation(&oFile)
                        }
                    }
                }
                while(FindNext(SearchRec)==0);
                FindClose(SearchRec);
            }
        }
    }

    InitCompData();
}

// ---------------------------------------------------------------------------
//  spbExportClick -- golden :396-473. LIVE (was GATE B-7).
//  For every CHECKED recipe: FO_DELETE <browsed dir>\<name> if it exists (:439-449,
//  Recycle Bin), then FO_COPY DataPath\<name> -> <browsed dir>\<name>. Writes and
//  deletes OUTSIDE D:\HT9045 -- wherever DirectoryListBox1 points.
//  WebBuilder.cpp refuses, before calling, a browsed directory that is (or is
//  inside) DataPath/OffsetPath or that would make <dir>\<name> contain them: in
//  golden that browse target makes :448 delete the recipe being exported (port
//  guard, reported to Steven).
// ---------------------------------------------------------------------------
void TfBuilder::spbExportClick(TObject *Sender)
{
    (void)Sender;
    spbExport->Down=false;
    AnsiString str="";
    TSearchRec SearchRec;
    AnsiString SDataPath=DataPath;
    AnsiString OrgPath="", NewPath="";
    int iAttr=faDirectory;
    char cStr1[256]="", cStr2[256]="";

    str.sprintf("Make sure export data to ''%s''", DirectoryListBox1->Directory);
    if(W906_MessageBox(str, "Export", MB_OKCANCEL|MB_TOPMOST)!=IDOK)           // golden :407 Application->MessageBox(str.c_str(), ...)
        return;
    if(!DirectoryExists(SDataPath))
    {
        ShowMyMessage("File Path Lost");
        return;
    }
    else if(DirectoryExists(SDataPath))
    {
        if(SDataPath.SubString(SDataPath.Length(), 1)!="\\")
        {
            SDataPath=SDataPath+"\\*.*";
        }
        else
        {
            SDataPath=SDataPath+"*.*";                                          //Steven 20110621
        }

        if(FindFirst(SDataPath, iAttr, SearchRec)==0)
        {
            do
            {
                if((SearchRec.Attr&iAttr)==faDirectory)
                {
                    if(NeedExport(SearchRec.Name))
                    {
                        NewPath=DirectoryListBox1->Directory;
                        OrgPath=DataPath;
                        OrgPath+=SearchRec.Name;
                        NewPath+=AnsiString ("\\");
                        NewPath+=SearchRec.Name;

                        if(DirectoryExists(NewPath))                            //jou 2012-01-13 修正Export不能使用
                        {
                            memset(cStr1, 0, sizeof(cStr1));
                            strncpy(cStr1, NewPath.c_str(), sizeof(cStr1));
                            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
                            oFile.hwnd=NULL;                                    // golden Handle
                            oFile.wFunc=FO_DELETE;
                            oFile.pFrom=cStr1;
                            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR);
                            W906ShFileOp(this, &oFile);                         // golden SHFileOperation(&oFile)
                        }

                        memset(cStr1, 0, sizeof(cStr1));
                        memset(cStr2, 0, sizeof(cStr2));
                        strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
                        strncpy(cStr2, NewPath.c_str(), sizeof(cStr2));

                        ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
                        oFile.hwnd=NULL;                                        //複製工作檔  (golden Handle)
                        oFile.wFunc=FO_COPY;
                        oFile.pFrom=cStr1;
                        oFile.pTo=cStr2;
                        oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR);
                        W906ShFileOp(this, &oFile);                             // golden SHFileOperation(&oFile)
                    }
                }
            }
            while(FindNext(SearchRec)==0);
            FindClose(SearchRec);
        }
    }
    DirectoryListBox1->Update();

    W906_MessageBox("Export data finish!!", "Export", MB_OK|MB_TOPMOST);        // golden :472 Application->MessageBox(...)
}

// ---------------------------------------------------------------------------
//  spbImportClick -- golden :475-482. LIVE (was GATE B-8) -> CopySourTarget.
// ---------------------------------------------------------------------------
void TfBuilder::spbImportClick(TObject *Sender)
{
    (void)Sender;
    spbImport->Down=false;
    AnsiString str="";
    str.sprintf("Make sure Import data form ''%s''", DirectoryListBox1->Directory);
    if(W906_MessageBox(str, "Import", MB_OKCANCEL)==IDOK)                      // golden :480 Application->MessageBox(str.c_str(), ...)
        CopySourTarget(DirectoryListBox1->Directory);
}

#if 0 // GATE (B-9) bSaveAllFillOrFile -- golden :509-572.  WRITES DISK with the
      // widest blast radius in the wave: it calls SaveSetupFile(...) /
      // SaveOther(...) on ELEVEN other forms, i.e. it rewrites the entire
      // recipe on disk.  RESERVED NAME (SECSGEM/uHGemHT9045.cpp:3710, itself
      // inside that file's own `#if 0`).  Independently it also has real
      // reachability blockers (fSpeed/fBinSel/fTrayForm/fCCLink/fSetup/
      // fYieldMonitoring have no facade carrying these methods; GetRecipePath
      // common.h:317 and SaveTestMode cprod.h:3289 ARE reachable) -- but
      // SAFETY is the operative gate and would remain so even if every
      // collaborator existed.
      // Steven 20260925: STILL GATED for the Data.Builder page -- cBuilder.dfm
      // binds no control to it (the page cannot reach it), its one caller is dead
      // code, and fCCLink has no facade anywhere in this tree.
// KEVIN 20180824 add  Save File
void TfBuilder::bSaveAllFillOrFile(AnsiString SourceFileFileName, int iFile)
{
    AnsiString S=GetLastOpenFN();
    AnsiString szDir=GetRecipePath();

    fHotPlate->DoIniDataToForm();
    fContact->DoIniDataToForm();
    fContact->DutCount();                                                       //jou 2014-09-06 修正開啟程式的時候EP異常
    FTestIF->DoIniDataToForm();
    fYieldMonitoring->DoIniDataToForm();
    fSetup->DoIniDataToForm();
    fTemp_Set->DoIniDataToForm(true);                                           //Steven 20110930 : 得在fSetup後面
    fSpeed->DoIniDataToForm();
    fTrayAssignment->DoIniDataToForm();                                         //wei 20150317 : 得在fSpeed後面

    if(iFile==0)
    {
       if(SourceFileFileName.Pos("ArmCondition"))
          fSpeed->SaveSetupFile(szDir);                                         //USPEED
       else if(SourceFileFileName.Pos("Binasgn"))
          fBinSel->SaveOther(szDir);                                            //cBinSel
       if(SourceFileFileName.Pos("Contact"))
          fContact->SaveSetupFile(szDir, S);                                    //Contract
       if(SourceFileFileName.Pos("HandlerCondition"))                           //SETUP
          fSetup->SaveSetupFile(szDir);
       if(SourceFileFileName.Pos("HotPlate"))                                   //hotplate
          fHotPlate->SaveSetupFile(szDir, S);
       if(SourceFileFileName.Pos("Temperature"))
           fTemp_Set->SaveSetupFile(szDir, S);                                  //TEMP
       if(SourceFileFileName.Pos("Tester"))
       {
          FTestIF->SaveSetupFile(szDir, S);
          fYieldMonitoring->SaveSetupFile(szDir, S);
       }

       if(SourceFileFileName.Pos("TestMode"))
          SaveTestMode();                                                       //USPEED
       if(SourceFileFileName.Pos("Tray"))
       {
          fTrayAssignment->SaveSetupFile(szDir, S);
          fTrayForm->SaveSetupFile(szDir, S);
       }

       if(SourceFileFileName.Pos("UdUld"))
          fLd_ULd->SaveSetupFile(szDir, S);
    }
    else
    {                                                                           //all fil save
         fSpeed->SaveSetupFile(szDir);
         fBinSel->SaveOther(szDir);
         fContact->SaveSetupFile(szDir, S);
         fSetup->SaveSetupFile(szDir);
         fHotPlate->SaveSetupFile(szDir, S);
         fTemp_Set->SaveSetupFile(szDir, S);
         FTestIF->SaveSetupFile(szDir, S);
         fYieldMonitoring->SaveSetupFile(szDir, S);
         fTrayForm->SaveSetupFile(szDir, S);
         fTrayAssignment->SaveSetupFile(szDir, S);
         fLd_ULd->SaveSetupFile(szDir, S);

         if(CosFunction.bCCLinkValueSaveFile)                                   //kevin 20180827 add
            fCCLink->SaveSetupFile();
    }
}
#endif // GATE (B-9)
