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
// =============================================================================
#include "forms/fBuilder.h"
#include "forms/fQwertyKey.h"    // fQwertyKey->ShowQwertyKey (forms/fQwertyKey.cpp, ht9045_forms)
#include "common.h"              // DataPath / OnlyMakeFileDataInPut (common.cpp, ht9045_core)
#include "cmydef.h"              // N_NO_SYMBOL (extern const int, cmydef.cpp, ht9045_globals)
#include "canary_support.h"      // ShowMyMessage -- body canary_support.cpp (ht9045_sm), one of the four sanctioned forms->sm exceptions

#include <windows.h>             // WIN32_FIND_DATA / FindFirstFile / FindNextFile / FindClose (ACTIVE InitCompData); same include forms/fMesSystem.cpp:24 already uses in this target
#include <cstring>               // strcmp (ACTIVE InitCompData, golden :379-380)

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
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  ALL NINE ARE THE READ-ONLY-DIRECTION SAFETY RULE.  See forms/fBuilder.h
//  for the per-entry reasoning.
// ===========================================================================

#if 0 // GATE (B-1) Create2DCodeWorkFile -- golden :51-70.  WRITES DISK:
      // `CopyFile(str2.c_str(), str1.c_str(), false)` at golden :66, into
      // D:\HT9045\Barcode_File\<port>\<recipe>.cfg.  Also needs FileInfo()
      // (no port resolved) and HSys.
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

#if 0 // GATE (B-2) btCreateSetupFileClick -- golden :72-159 (88L, the biggest
      // body in the file).  WRITES DISK: MyForceDirectories(NewPath) at :106
      // CREATES DIRECTORIES and SHFileOperation(FO_COPY) at :122 recursively
      // copies a whole recipe folder, twice (DataPath and OffsetPath).
      // Secondary blockers: `oFile` (golden file-scope SHFILEOPSTRUCT, no
      // port), `Handle` (removed with the base class, DEVIATION D-2),
      // `Application->MessageBox` (no port), and fMain->cbSetupFileName /
      // fMain->LookForFile() -- fMain IS real and in this same ht9045_forms
      // target, but `rg "LookForFile" forms/fMain.h` -> 0 hits (20260828).
void TfBuilder::btCreateSetupFileClick(TObject *Sender)
{
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

    str.sprintf("Do you create ''%s'' as the name", edNewFileName->Text);
    if(Application->MessageBox(str.c_str(), "Builder", MB_OKCANCEL|MB_TOPMOST)==IDOK)
    {
        if(CUSTOMER_CODE==CC_KYEC_XILINX && BAR_CODE_INSTALL==ebctInShtIntel)   //wei 20160802 add 2DCodeCreateWorkFile
        {
            Create2DCodeWorkFile();
        }

        for(int i=0; i<2; i++)
        {
            OrgPath=FileInfo().PathCombin(SPath[i], cbSourceFile->Text);
            NewPath=FileInfo().PathCombin(SPath[i], edNewFileName->Text);
            MyForceDirectories(NewPath);

            //------------------20111026    jou------------------
            OrgPath+="\\*.*";
            ZeroMemory(&cStr1, sizeof(cStr1));
            ZeroMemory(&cStr2, sizeof(cStr2));
            strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
            strncpy(cStr2, NewPath.c_str(), sizeof(cStr2));
            //複製工作檔
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=Handle;
            oFile.wFunc=FO_COPY;
            oFile.pFrom=cStr1;                                                  //The list of names must be double null-terminated.
            oFile.pTo=cStr2;
            oFile.hNameMappings=NULL;
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
            SHFileOperation(&oFile);
            #ifdef ASE_KaohSiung                                                //kevin 20150105 start
            JOBFILEPath=FileInfo().PathCombin(SPath[i], "JOBFILE");
            if(JOBFILEPath.Pos("JOBFILE"))                                      //kevin 20150120
                continue;

            MyForceDirectories(JOBFILEPath);
            ZeroMemory(&cStr1, sizeof(cStr1));
            ZeroMemory(&cStr2, sizeof(cStr2));
            strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
            strncpy(cStr2, JOBFILEPath.c_str(), sizeof(cStr2));

            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=Handle;                                                  //複製工作檔
            oFile.wFunc=FO_COPY;
            oFile.pFrom=cStr1;
            oFile.pTo=cStr2;
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
            SHFileOperation(&oFile);
            #endif                                                              //kevin 20150105 end

            //---------------------------------------------------

            if(i==2)                                                            //Steven 20101118 : 強制要加加
                i++;
        }
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
#endif // GATE (B-2)

#if 0 // GATE (B-3) bSaveAsJobFile -- golden :163-192.  WRITES DISK:
      // MyForceDirectories + SHFileOperation(FO_COPY) at golden :176-183.
      // RESERVED NAME (uTemp_Set.cpp:4656, forms/fLd_ULd.cpp:342,
      // forms/fTesterIF.cpp:1567, VacuumUnit/VacuumUnit.cpp:575 -- all four
      // dead today; the VacuumUnit one is the pre-flight's FALSE LIVE, see the
      // header).  ⚠ Golden's whole copy block is itself inside
      // `#ifdef ASE_KaohSiung`, which this port does NOT define, so on this
      // build only :190-191 would execute.  Gated WHOLE regardless: splitting
      // on an undefined macro would be inventing a build variant.
// KEVIN 20150107 Save As JobFile
void TfBuilder::bSaveAsJobFile(AnsiString SourceFileFileName, AnsiString FileName)
{
    AnsiString OrgPath="", NewPath="", sString, JOBFILEPath="";
    AnsiString SPath[2]={DataPath, OffsetPath}, sBuffer=SourceFileFileName.UpperCase();

    if(sBuffer.Pos(FileName))                                                   //檔案名稱比較
        return;

    for(int i=0; i<2; i++)
    {
        OrgPath=FileInfo().PathCombin(SPath[i], SourceFileFileName)+"\\*.*";
        #ifdef ASE_KaohSiung                                                    //kevin 20150105 start
        JOBFILEPath=FileInfo().PathCombin(SPath[i], FileName);
        MyForceDirectories(JOBFILEPath);
        ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
        oFile.hwnd=Handle;
        oFile.wFunc=FO_COPY;
        oFile.pFrom=OrgPath.c_str();
        oFile.pTo=JOBFILEPath.c_str();
        oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI);
        SHFileOperation(&oFile);
        #endif                                                                  //kevin 20150105 end

        //---------------------------------------------------
        if(i==2)                                                                //Steven 20101118 : 強制要加加
            i++;
    }
    CheckListBox1->Items->Add(FileName);
    asBackupCreate=FileName;
}
#endif // GATE (B-3)

#if 0 // GATE (B-4) btDeleteSetupFileClick -- golden :202-230.  DELETES FILES
      // transitively: :220 calls DeleteSetupFile (GATE B-5).  Also
      // Application->MessageBox and fMain->cbSetupFileName / LookForFile as in
      // (B-2).
void TfBuilder::btDeleteSetupFileClick(TObject *Sender)
{
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
    if(Application->MessageBox(str.c_str(), "Builder", MB_OKCANCEL|MB_TOPMOST)==IDOK)
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
#endif // GATE (B-4)

#if 0 // GATE (B-5) DeleteSetupFile -- golden :232-260.  DELETES FILES:
      // `oFile.wFunc=FO_DELETE` at :249 + SHFileOperation at :253, applied to
      // DataPath\<name> AND OffsetPath\<name>.  The most destructive body in
      // this wave.  (IsFileInUse at :243 IS reachable -- common.h:346,
      // common.cpp, ht9045_core -- so it is not a blocker; safety is.)
      // ⚠ GOLDEN BUG PRESERVED VERBATIM: `bool bDelOK=SHFileOperation(&oFile);`
      // then error-on-true.  SHFileOperation returns 0 on SUCCESS, so the
      // NAME is inverted relative to the meaning.  NOT "fixed".
void TfBuilder::DeleteSetupFile(AnsiString DeleteFileName)                       //Steven 20110305
{
    char str1[256]="";
    AnsiString OrgPath="";
    AnsiString SPath[2]={DataPath, OffsetPath};

    for(int i=0; i<2; i++)
    {
        OrgPath=FileInfo().PathCombin(SPath[i], DeleteFileName);
        if(DirectoryExists(OrgPath))
        {
            if(IsFileInUse(OrgPath.c_str())==true)                              //Jimmychiu 20241121 : Verify whether the file is currently in use
            {
                MySleep(5000);
            }
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
            oFile.hwnd=Handle;
            oFile.wFunc=FO_DELETE;
            strncpy(str1, OrgPath.c_str(), sizeof(str1));
            oFile.pFrom=str1;
            oFile.fFlags=FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
            bool bDelOK=SHFileOperation(&oFile);
            if(bDelOK==true)
            {
                ShowMyMessage("Delete Setup File ERROR");
            }
        }
    }
}
#endif // GATE (B-5)

#if 0 // GATE (B-6) CopySourTarget -- golden :274-335.  WRITES DISK:
      // SHFileOperation(FO_COPY) into DataPath at :296 and :324.
      // (CosFunction.bBuilderImportSingleFolder at :287 IS reachable --
      // CosFunction.cpp, ht9045_globals -- so it is not a blocker; safety is.)
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
            oFile.hwnd=Handle;
            oFile.wFunc=FO_COPY;
            strncpy(Orgstr, SDataPath.c_str(), sizeof(Orgstr));
            oFile.pFrom=Orgstr;
            oFile.pTo=DataPath.c_str();
            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION);
            SHFileOperation(&oFile);
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
                            oFile.hwnd=Handle;
                            oFile.wFunc=FO_COPY;
                            strncpy(Orgstr, SDataPath.c_str(), sizeof(Orgstr));
                            oFile.pFrom=Orgstr;
                            oFile.pTo=DataPath.c_str();
                            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION);
                            SHFileOperation(&oFile);
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
#endif // GATE (B-6)

#if 0 // GATE (B-7) spbExportClick -- golden :396-473 (78L).  WRITES **AND
      // DELETES** OUTSIDE D:\HT9045: :448 SHFileOperation(FO_DELETE) removes
      // the destination folder under the browsed DirectoryListBox1->Directory
      // before :462 SHFileOperation(FO_COPY) writes over it.  A wrong browse
      // target destroys unrelated data.  [T] on ACTIVE NeedExport at :431.
void TfBuilder::spbExportClick(TObject *Sender)
{
    spbExport->Down=false;
    AnsiString str="";
    TSearchRec SearchRec;
    AnsiString SDataPath=DataPath;
    AnsiString OrgPath="", NewPath="";
    int iAttr=faDirectory;
    char cStr1[256]="", cStr2[256]="";

    str.sprintf("Make sure export data to ''%s''", DirectoryListBox1->Directory);
    if(Application->MessageBox(str.c_str(), "Export", MB_OKCANCEL|MB_TOPMOST)!=IDOK)
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
                            oFile.hwnd=Handle;
                            oFile.wFunc=FO_DELETE;
                            oFile.pFrom=cStr1;
                            oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR);
                            SHFileOperation(&oFile);
                        }

                        memset(cStr1, 0, sizeof(cStr1));
                        memset(cStr2, 0, sizeof(cStr2));
                        strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
                        strncpy(cStr2, NewPath.c_str(), sizeof(cStr2));

                        ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCT));
                        oFile.hwnd=Handle;                                      //複製工作檔
                        oFile.wFunc=FO_COPY;
                        oFile.pFrom=cStr1;
                        oFile.pTo=cStr2;
                        oFile.fFlags=(FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR);
                        SHFileOperation(&oFile);
                    }
                }
            }
            while(FindNext(SearchRec)==0);
            FindClose(SearchRec);
        }
    }
    DirectoryListBox1->Update();

    Application->MessageBox("Export data finish!!", "Export", MB_OK|MB_TOPMOST);
}
#endif // GATE (B-7)

#if 0 // GATE (B-8) spbImportClick -- golden :475-482.  WRITES DISK
      // transitively via CopySourTarget (GATE B-6) at :481; also
      // Application->MessageBox (no port).
void TfBuilder::spbImportClick(TObject *Sender)
{
    spbImport->Down=false;
    AnsiString str="";
    str.sprintf("Make sure Import data form ''%s''", DirectoryListBox1->Directory);
    if(Application->MessageBox(str.c_str(), "Import", MB_OKCANCEL)==IDOK)
        CopySourTarget(DirectoryListBox1->Directory);
}
#endif // GATE (B-8)

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
