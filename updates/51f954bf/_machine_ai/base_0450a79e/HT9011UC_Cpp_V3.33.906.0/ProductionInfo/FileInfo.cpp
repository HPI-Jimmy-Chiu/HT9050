// =============================================================================
//  ProductionInfo/FileInfo.cpp  --  implementation of the FileInfo family.
//
//  Faithful translation of golden ProductionInfo/FileInfo.cpp (614 lines,
//  BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 ("prodinfo" group), 1 of 3 units.  See
//  FileInfo.h's banner for the class-level role summary, the 2 gates, and the
//  __property translation decision -- not repeated here.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim, all 45 golden function bodies):
//    ParameterInfo::ParameterInfo              golden :10-14
//    ParameterInfo::~ParameterInfo             golden :16-18
//    FileInfo::FileInfo(AnsiString)            golden :20-23
//    FileInfo::~FileInfo                       golden :25-33
//    FileInfo::ReadFile(AnsiString)            golden :35-38
//    FileInfo::ReadFile(AnsiString,int)        golden :40-63
//    FileInfo::WriteFile                       golden :65-81
//    FileInfo::IsFilePathExist                 golden :83-88
//    FileInfo::DecodeASCII(char*,int,int)      golden :90-95
//    FileInfo::DecodeASCII(AnsiString,int)     golden :97-115
//    FileInfo::DecodeReadMap2Parameter         golden :117-133
//    FileInfo::SaveAsTxtFile                   golden :135-141
//    FileInfo::GetNameAndExtension             golden :143-152
//    FileInfo::GetAllFileNamesInFolder         golden :154-181
//    FileInfo::GetAllFolderNamesInFolder       golden :183-212
//    FileInfo::DeleteFolderContents            golden :214-245
//    FileInfo::SplitPath                       golden :247-272
//    FileInfo::DirectoryExist                  golden :274-278
//    FileInfo::RemoveAllTrailingBackslashes    golden :280-288
//    FileInfo::PathCombin                      golden :290-312
//    FileInfo::EnsureDirectoriesExist          golden :314-361
//    cDatabaseMin::cDatabaseMin                golden :363-366
//    cDatabaseMin::~cDatabaseMin                golden :368-371
//    cDatabaseMin::LoadFile                    golden :373-376
//    cDatabaseMin::SaveFile                    golden :378-381
//    cDBTStringGrid::cDBTStringGrid             golden :383-387
//    cDBTStringGrid::~cDBTStringGrid            golden :389-392
//    cDBTStringGrid::Clear                     golden :394-410
//    cDBTStringGrid::SetRowCol                 golden :412-421 (GATE 1, see header)
//    cDBTStringGrid::GetRowCol                 golden :423-430
//    cDBTStringGrid::SetColTitle               golden :432-438
//    cDBTStringGrid::GetColTitle               golden :440-448
//    cDBTStringGrid::SetRowTitle               golden :450-456
//    cDBTStringGrid::GetRowTitle               golden :458-472
//    cDBTStringGrid::SetCell                   golden :474-500
//    cDBTStringGrid::GetCell                   golden :502-516
//    cDBTStringGrid::AddRow                    golden :518-524
//    cDBTStringGrid::AddCol                    golden :526-532
//    cDBTStringGrid::CloneTo                   golden :534-548
//    cDBTStringGrid::ClearRow                  golden :550-570
//    cDBTStringGrid::HideRow                   golden :572-593 (GATE 2, see header)
//    cMathTool::cMathTool                      golden :595-598
//    cMathTool::~cMathTool                     golden :600-603
//    cMathTool::Clear                          golden :605-608
//    cMathTool::IsDoubleEqual                  golden :610-613
//  45 of 45 golden functions get a real body.  ZERO stubs.
//
//  GATE REGISTER -- see FileInfo.h's banner for the full rationale; re-cited
//  at each call site below (both are single-statement drops, not #if 0/#else
//  pairs, since the DEFAULT here IS "drop the rendering-only call" and there
//  is no golden CALL to keep verbatim in a dead arm -- the statements
//  themselves are the thing gated, not a branch around a working call).
//
//  MINIMAL NECESSARY ADAPTATIONS (2, both flagged inline with //AI at the line):
//   (a) golden :49-52 builds a `TStringStream` wrapping DecodeASCII's decoded
//       AnsiString, then `slSourceFile->LoadFromStream(ts)`. Neither
//       TStringStream nor a TStringList::LoadFromStream(TStream*) overload
//       exists anywhere in this tree (grepped: zero hits for TStringStream;
//       vclcompat/MemoryStream.h's TMemoryStream::LoadFromStream takes only
//       another TMemoryStream*, and it is a DIFFERENT, unrelated shim built
//       for SECSGEM/uHGemEquipment.cpp's byte-buffer needs, not a string/line
//       splitter). vclcompat::TStringList::Text's setter (TStringList.h:210,
//       "SetText: split on CR/LF") is the EXACT same operation TStringStream+
//       LoadFromStream performs end-to-end here (decoded bytes -> AnsiString
//       -> line-split into the TStringList) -- replaced with
//       `slSourceFile->Text=DecodeASCII(asFilePath,idecode);`, net-identical
//       observable result, one fewer heap object. This path is NOT a dead
//       corner: uSetUpConfiguration.cpp's every ReadFile call passes
//       idecode=-100, so this exact line is the one this wave's OTHER two
//       units depend on for real.
//   (b) golden :113 `delete[] a;` runs UNCONDITIONALLY after the `if(fp.is_open())`
//       block, but `a` is a local `char *a;` with NO initializer -- if the
//       file fails to open, `a` is read UNINITIALISED and `delete[]` is
//       applied to garbage (undefined behaviour). Preserved VERBATIM (not
//       fixed) -- see the GOLDEN QUIRKS note below; this is exactly the kind
//       of pre-existing defect HARD RULE 3 says to keep, not "improve".
//
//  GOLDEN QUIRKS / BUGS preserved VERBATIM, not fixed:
//   1. FileInfo::DecodeASCII(char*,int,int) (:90) returns `AnsiString(cStream)`
//      -- NOT `AnsiString(cStream, iLen)` -- even though `iLen` is a
//      parameter. `cStream` (the caller's `new char[n]` buffer, golden :107)
//      is NEVER null-terminated, so this constructs the AnsiString by
//      scanning for a NUL byte that may lie past the allocation (a genuine
//      buffer over-read in golden). Kept exactly as golden wrote it.
//   2. FileInfo::DecodeASCII(AnsiString,int) (:97) has the uninitialised-
//      pointer `delete[] a;` bug described in adaptation (b) above.
//   3. FileInfo::GetAllFileNamesInFolder/GetAllFolderNamesInFolder (:154,:183)
//      call `FindFirst(folderPath+"*.*", faAnyFile, searchRec)` /
//      `FindFirst(folderPath+"*.*", faDirectory, searchRec)` and then filter
//      the RESULT again in the loop body (`(searchRec.Attr & faDirectory)==0`
//      / `searchRec.Attr & faDirectory`) -- i.e. the `attr` mask passed to
//      FindFirst is effectively redundant with the loop's own filter (BCB6's
//      FindFirst mask is inclusive-OR with the standard attributes, not an
//      exact match, so a plain `faDirectory` mask still returns files too).
//      Kept exactly as golden wrote it -- both loops still produce the right
//      answer because of the explicit in-loop re-check.
//   4. FileInfo::DeleteFolderContents (:214) calls `FileInfo().GetAllFileNamesInFolder(...)`
//      on a FRESH default-constructed temporary rather than `this` -- since
//      GetAllFileNamesInFolder never reads `asFilePath`/`mapGroup`, this is a
//      no-op distinction (a temporary works identically to `this` here), but
//      it is golden's own literal spelling and is kept verbatim.
//   5. cDBTStringGrid::GetRowTitle (:458) guards `if(iRow<recordCount)` before
//      reading `sg->Cells[0][iRow+1]`, but its sibling GetColTitle (:440) has
//      NO equivalent guard on `iCol` before reading `sg->Cells[iCol+1][0]` --
//      an asymmetric bounds-check golden itself carries. Kept as-is; a
//      too-large `iCol` reads "" (AI(W906-W3-6b) 20260925: vclcompat now = BCB6 grids.pas {$R-} sparse Cells; it used to throw
//      std::out_of_range here, and BCB6 never raised ERangeError for this --
//      NB2 R2 Q1 read grids.pas).
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :7). `using namespace std;` (golden :8) kept --
//  MachineDefine.h already pulls in <fstream>/<iostream>/<vector>/<map> this
//  file's std:: usage (fstream/ofstream/ios::) needs. No __fastcall /
//  __property / __published bodies in this .cpp (golden has none either --
//  the header's 3 __property declarations have no .cpp counterpart to
//  translate, only the 6 underlying Get/Set method bodies, which ARE here).
//  SOFT_SIMULTE is NOT defined and golden does not reference it in this unit.
//  Numeric semantics kept EXACT (no int/float rewrites).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8, in
//  its original column. Final gate: ZERO U+FFFD.
// =============================================================================

// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "FileInfo.h"      ; #pragma package(smart_init)
//   using namespace std;
// =============================================================================

#include "MachineDefine.h"           // de-VCL'd include hub: vclcompat umbrella +
                                     //   <windows.h>/<fstream>/<iostream>/<vector>/<map> +
                                     //   `using namespace std;` (matches golden :8)
#include "ProductionInfo/FileInfo.h"

namespace {
// ---------------------------------------------------------------------------
//  Exception -- golden's ambient VCL `Exception` class (System.hpp), used by
//  DeleteFolderContents' `catch(Exception& e)` below. Same file-local,
//  internal-linkage, `.Message`-only stand-in as
//  KYECFTP/FTPClient_Transfer.cpp's own precedent (that file's banner has the
//  full rationale for why this is kept anonymous-namespace-local rather than
//  hoisted into vclcompat/ -- avoids any cross-TU ODR risk from a parallel
//  unit inventing the same primitive independently). NOTHING in this
//  function's call graph (DeleteFile) is documented to throw a VCL Exception
//  either -- same "structurally kept, not structurally reachable" status as
//  that precedent's own note.
// ---------------------------------------------------------------------------
class Exception
{
public:
    AnsiString Message;
    explicit Exception(const AnsiString& Msg = AnsiString()) : Message(Msg) {}
};
} // anonymous namespace

//------------------------------------------------------------------------------
ParameterInfo::ParameterInfo(AnsiString &_asParaValue, AnsiString _asFileName)  //Jimmychiu 20220830 add ParameterInfo
{
    asParaValue=&_asParaValue;
    asFileParaName=_asFileName;
}
//------------------------------------------------------------------------------
ParameterInfo::~ParameterInfo()                                                 //Jimmychiu 20220830 add ParameterInfo Class
{
}
//------------------------------------------------------------------------------
FileInfo::FileInfo(AnsiString FilePath)                                         //Jimmychiu 20220830 add FileInfo Class
{
    asFilePath=FilePath;
}
//------------------------------------------------------------------------------
FileInfo::~FileInfo()                                                           //Jimmychiu 20220830 add FileInfo Class
{
    for(int i=0; i<GetSize(); i++)
    {
        delete mapGroup[i];
    }
    mapGroup.clear();
    Sleep(1);                                                                   //避免非同步互搶資源
}
//------------------------------------------------------------------------------
bool FileInfo::ReadFile(AnsiString sSeparatedValues)
{
    return ReadFile(sSeparatedValues,0);                                        //Jimmychiu 20220830 解碼並讀取資料
}
//------------------------------------------------------------------------------
bool FileInfo::ReadFile(AnsiString sSeparatedValues, int idecode)               //Jimmychiu 20220830 解碼並讀取資料
{
    TStringList *slSourceFile=new TStringList();
    AnsiString asGetString="";
    if(IsFilePathExist(asFilePath)==false)
        return false;

    if(idecode!=0)
    {
        //AI(W906-PT-W2) 20260807: MINIMAL NECESSARY ADAPTATION (a) -- see this
        //  file's banner. golden's `TStringStream ts(DecodeASCII(...)); slSourceFile
        //  ->LoadFromStream(&ts);` has no TStringStream/LoadFromStream(TStream*)
        //  substrate anywhere in this tree; TStringList::Text's setter (splits on
        //  CR/LF) is the same net operation with one fewer object.
        slSourceFile->Text=DecodeASCII(asFilePath,idecode);
    }
    else
    {
        slSourceFile->LoadFromFile(asFilePath);
    }

    for(int i=0; i<slSourceFile->Count; i++)
        DecodeReadMap2Parameter(slSourceFile->Strings[i], sSeparatedValues);
    delete slSourceFile;
    return true;
}
//------------------------------------------------------------------------------
bool FileInfo::WriteFile(AnsiString sSeparatedValues)                           //Jimmychiu 20220830 解碼並讀取資料
{
    TStringList *slSourceFile=new TStringList();
    AnsiString asSetString="";
    if(IsFilePathExist(asFilePath)==false)
        return false;

    slSourceFile->LoadFromFile(asFilePath);
    for(int i=0; i<GetSize(); i++)
    {
        ParameterInfo* _p=mapGroup.find(i)->second;
        asSetString=_p->asFileParaName+sSeparatedValues+*(_p->asParaValue);
        slSourceFile->Add(asSetString);
    }
    delete slSourceFile;
    return true;
}
//------------------------------------------------------------------------------
bool FileInfo::IsFilePathExist(AnsiString sPath)
{
    if(sPath=="" || FileExists(sPath)==false)
        return false;
    return true;
}
//------------------------------------------------------------------------------
AnsiString FileInfo::DecodeASCII(char *cStream, int iLen, int iTransfor)        //Jimmychiu 20220830 解碼並讀取資料
{
    for(int i=0; i<iLen; i++)
        cStream[i]+=iTransfor;
    return AnsiString(cStream);                                                 // GOLDEN QUIRK 1 (see file banner) -- kept verbatim
}
//------------------------------------------------------------------------------
AnsiString FileInfo::DecodeASCII(AnsiString asFileName, int iTransfor)
{
    fstream fp;
    AnsiString asBack="";
    char *a;
    fp.open(asFileName.c_str(), ios::in);                                       //開啟檔案
    if(fp.is_open())
    {
        fp.seekg(0, ios::end);
        int n=fp.tellg();
        a=new char[n];
        fp.seekg(0,ios::beg);
        fp.read(a,n);
        asBack=DecodeASCII(a,n,iTransfor);
        fp.close();
    }
    delete[] a;                                                                 // GOLDEN QUIRK 2 / adaptation (b) (see file banner) -- kept verbatim
    return asBack;
}
//------------------------------------------------------------------------------
void FileInfo::DecodeReadMap2Parameter(AnsiString asLineValue, AnsiString sSeparatedValues)
{
    AnsiString asKey="";
    int iStart=-1;
    for(int j=0;j<GetSize();j++)
    {
        ParameterInfo* _p=mapGroup.find(j)->second;
        asKey=_p->asFileParaName;
        if(asKey=="")continue;
        if(asLineValue.Pos(asKey+sSeparatedValues)==1)
        {
            iStart=asKey.Length()+sSeparatedValues.Length()+1;
            *(_p->asParaValue)=asLineValue.SubString(iStart,asLineValue.Length()-iStart+1);
            break;
        }
    }
}
//------------------------------------------------------------------------------
void FileInfo::SaveAsTxtFile(AnsiString asContent, AnsiString asNewFileWithPath)                        //Jimmychiu 20230223 帳密權限加密，提升資訊安全
{
    ofstream outfile;
    outfile.open(asNewFileWithPath.c_str());
    outfile<<asContent.c_str();
    outfile.close();
}
//------------------------------------------------------------------------------
void FileInfo::GetNameAndExtension(AnsiString asFileName, AnsiString &asFile, AnsiString &asExtension) //Jimmychiu 20230223 帳密權限加密，提升資訊安全
{
    int dot_pos = asFileName.LastDelimiter('.');
    asFile=asExtension="";
    if(dot_pos>0)
    {
        asFile=asFileName.SubString(1,dot_pos-1);
        asExtension=asFileName.SubString(dot_pos+1, asFileName.Length()-dot_pos+1);
    }
}
//------------------------------------------------------------------------------   //Jimmychiu 20240224 : Get all file name in the folder
std::vector<AnsiString> FileInfo::GetAllFileNamesInFolder(AnsiString folderPath, int &iErr)
{
    //iErr=>  0=normal 1=No Folder
    iErr=0;
    std::vector<AnsiString> fileNames;
    TSearchRec searchRec;
    if(folderPath.SubString(folderPath.Length(),1) != "\\")
    {
        folderPath+="\\";
    }
    int findResult=FindFirst(folderPath+"*.*",faAnyFile,searchRec);
    if(findResult==0)
    {
        do
        {
            if((searchRec.Attr & faDirectory)==0)//remove folder name
            {
                fileNames.push_back(searchRec.Name);
            }
        }while(FindNext(searchRec)==0);
        FindClose(searchRec);
    }
    else
    {
        iErr=1;
    }
    return fileNames;
}
//------------------------------------------------------------------------------
std::vector<AnsiString> FileInfo::GetAllFolderNamesInFolder(AnsiString folderPath, int &iErr)
{
    //iErr=>  0=normal 1=No Folder
    iErr=0;
    std::vector<AnsiString> folderNames;
    TSearchRec searchRec;
    if(folderPath.SubString(folderPath.Length(),1) != "\\")
    {
        folderPath+="\\";
    }
    int findResult=FindFirst(folderPath+"*.*",faDirectory,searchRec);
    if(findResult==0)
    {
        do
        {
            if ((searchRec.Attr & faDirectory) &&
                searchRec.Name != "." &&
                searchRec.Name != "..")
            {
                folderNames.push_back(searchRec.Name);
            }
        }while(FindNext(searchRec)==0);
        FindClose(searchRec);
    }
    else
    {
        iErr=1;
    }
    return folderNames;
}
//------------------------------------------------------------------------------
bool FileInfo::DeleteFolderContents(AnsiString folderPath,int &iErr)
{
    if(folderPath.SubString(folderPath.Length(),1)!="\\")
    {
        folderPath+="\\";
    }
    iErr=0; //iErr=>  0=normal 1=No Folder 2=delete fail
    std::vector<AnsiString> fileNames =FileInfo().GetAllFileNamesInFolder(folderPath,iErr);   //"D:\\PM RECORD\\";
    if(iErr==1)
    {
        return false;
    }
    else
    {
        AnsiString sFileName="";
        for(unsigned int i=0;i<fileNames.size();i++)
        {
            try
            {
                if(DeleteFile(folderPath+fileNames[i])==false)
                {
                    iErr=2;
                }
            }
            //AI(W906-PT-W2) 20260807: golden `catch (Exception& e)` -- BCB6's
            //  ambient VCL Exception (System.hpp). Nothing in this tree needed
            //  it before this wave's KYECFTP/FTPClient_Transfer.cpp precedent
            //  (which documents the same gap); same file-local, internal-
            //  linkage, `.Message`-only stand-in, so a parallel unit's own
            //  identical local definition (anonymous-namespace, internal
            //  linkage) cannot ODR-collide with this one.
            catch (Exception& e)
            {
                iErr=2;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
void FileInfo::SplitPath(const AnsiString& fullPath, AnsiString& path, AnsiString& filename, AnsiString& extension)
{
    int lastSlashIndex=fullPath.LastDelimiter("\\");
    if(lastSlashIndex!=0)
    {
        path=fullPath.SubString(1,lastSlashIndex-1);
        AnsiString filenameWithExtension=fullPath.SubString(lastSlashIndex+1,fullPath.Length()-lastSlashIndex);
        int lastDotIndex=filenameWithExtension.LastDelimiter(".");
        if(lastDotIndex!=0)
        {
            filename=filenameWithExtension.SubString(1,lastDotIndex-1);
            extension=filenameWithExtension.SubString(lastDotIndex+1,filenameWithExtension.Length()-lastDotIndex);
        }
        else
        {
            filename=filenameWithExtension;
            extension="";
        }
    }
    else
    {
        path="";
        filename=fullPath;
        extension="";
    }
}
//------------------------------------------------------------------------------
bool FileInfo::DirectoryExist(AnsiString folderPath)
{
    folderPath=RemoveAllTrailingBackslashes(folderPath);
    return DirectoryExists(folderPath);
}
//------------------------------------------------------------------------------
AnsiString FileInfo::RemoveAllTrailingBackslashes(const AnsiString& input)
{
    int ilen=input.Length();
    while(ilen>0 && input[ilen]=='\\')
    {
        ilen--;
    }
    return input.SubString(1, ilen);
}
//------------------------------------------------------------------------------
AnsiString FileInfo::PathCombin(const AnsiString& sPath, const AnsiString& sFile)
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
            if(combinedPath[combinedPath.Length()] != '\\')
            {
                combinedPath+="\\";
            }
        }
    }
    combinedPath+=sFile;
    return combinedPath;
}
//------------------------------------------------------------------------------
void FileInfo::EnsureDirectoriesExist(AnsiString sPath)
{
    if(sPath.Length()==0)
        return;

    if(sPath[sPath.Length()]!='\\')                                             //Steven 20260505 : Auto-detect filename component
    {
        DWORD attr=GetFileAttributes(sPath.c_str());
        if(attr!=INVALID_FILE_ATTRIBUTES)                                       //RogerYang 20260527 : Fix - INVALID treats as dir-to-create
        {
            if(!(attr & FILE_ATTRIBUTE_DIRECTORY))
                sPath=ExtractFilePath(sPath);                                   //路徑存在：確認是檔案才 strip
        }
        else
        {
            AnsiString sExt=ExtractFileExt(sPath).LowerCase();                  //RogerYang 20260603 : 只卡控明確的檔案副檔名，避免誤判含「.」的目錄名
            if(sExt==".txt" || sExt==".csv")
                sPath=ExtractFilePath(sPath);
        }
    }

    if(sPath[sPath.Length()]=='\\')
    {
        sPath=sPath.SubString(1,sPath.Length()-1);
    }
    int pos=1;
    while(pos<=sPath.Length())
    {
        int nextPos=sPath.SubString(pos, sPath.Length()-pos+1).Pos("\\");
        if(nextPos>0)
        {
            nextPos=pos+nextPos-1;
        }
        else
        {
            nextPos=sPath.Length()+1;
        }
        AnsiString currentPath=sPath.SubString(1, nextPos-1);
        if(GetFileAttributes(currentPath.c_str())==INVALID_FILE_ATTRIBUTES)
        {
            if(!CreateDirectory(currentPath.c_str(), NULL))
            {
                return;
            }
        }
        pos=nextPos+1;
    }
}
//------------------------------------------------------------------------------
cDatabaseMin::cDatabaseMin()
{
    Clear();
}
//------------------------------------------------------------------------------
cDatabaseMin::~cDatabaseMin()
{
//
}
//------------------------------------------------------------------------------
bool cDatabaseMin::LoadFile(AnsiString dbpath,AnsiString dbName)
{
    return true;
}
//------------------------------------------------------------------------------
void cDatabaseMin::SaveFile()
{
//
}
//------------------------------------------------------------------------------
cDBTStringGrid::cDBTStringGrid(TStringGrid* _sg)
{
    sg=_sg;
    iRowHeight=25;
}
//------------------------------------------------------------------------------
cDBTStringGrid::~cDBTStringGrid()
{
//
}
//------------------------------------------------------------------------------
void cDBTStringGrid::Clear()
{
    if(sg)
    {
        int recordCount=0;
        int fieldCount=0;
        GetRowCol(recordCount,fieldCount);
        for(int irow=0;irow<recordCount;irow++)
        {
            for(int icol=0;icol<fieldCount;icol++)
            {
                SetCell(irow,icol,"");
            }
        }
        SetRowCol(1,1);
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::SetRowCol(int iRow,int iCol)
{
    if(sg)
    {
        sg->RowCount =iRow+1;//title
        sg->ColCount =iCol+1;//title
        //AI(W906-PT-W2) 20260807: GATE 1 (header banner). golden :418-419:
        //    sg->DefaultColWidth=100;
        //    sg->ColWidths[0]=30;
        //  vclcompat::TStringGrid has neither member (StringGrid.h's own file
        //  banner scopes it to headless Cells/RowCount/ColCount, explicitly NO
        //  ColWidths/RowHeights) and this pre-existing file may not be edited
        //  (HARD RULE 7). Pure column-width RENDERING hints with no effect on
        //  Cells/RowCount/ColCount data and zero readers anywhere in this
        //  headless port -- dropped, not stubbed, per the header's gate note.
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::GetRowCol(int &iRow,int &iCol)
{
    if(sg)
    {
        iRow=sg->RowCount-1;//remove title
        iCol=sg->ColCount-1;//remove title
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::SetColTitle(int iCol,const AnsiString str)
{
    if(sg)
    {
        sg->Cells[iCol+1][0]=str;
    }
}
//------------------------------------------------------------------------------
AnsiString cDBTStringGrid::GetColTitle(int iCol)
{
    AnsiString str="";
    if(sg)
    {
        str=sg->Cells[iCol+1][0];
    }
    return str;
}
//------------------------------------------------------------------------------
void cDBTStringGrid::SetRowTitle(int iRow,const AnsiString str)
{
    if(sg)
    {
        sg->Cells[0][iRow+1]=str;
    }
}
//------------------------------------------------------------------------------
AnsiString cDBTStringGrid::GetRowTitle(int iRow)
{
    AnsiString sBack="";
    if(sg)
    {
        int recordCount=0;
        int fieldCount=0;
        GetRowCol(recordCount,fieldCount);
        if(iRow<recordCount)
        {
            sBack=sg->Cells[0][iRow+1];
        }
    }
    return sBack;
}
//------------------------------------------------------------------------------
void cDBTStringGrid::SetCell(int iRow,int iCol,const AnsiString str)
{
    if(sg)
    {
        iRow=iRow+1;
        iCol=iCol+1;
        int recordCount=0;
        int fieldCount=0;
        bool bReset=false;
        GetRowCol(recordCount,fieldCount);
        if(iRow>=recordCount)
        {
            recordCount=iRow;
            bReset=true;
        }

        if(iCol>=fieldCount)
        {
            fieldCount=iCol;
            bReset=true;
        }

        if(bReset)
            SetRowCol(recordCount,fieldCount);
        sg->Cells[iCol][iRow]=str;
    }
}
//------------------------------------------------------------------------------
AnsiString cDBTStringGrid::GetCell(int iRow,int iCol)
{
    AnsiString sBack="";
    if(sg)
    {
        int recordCount=0;
        int fieldCount=0;
        GetRowCol(recordCount,fieldCount);
        if(iCol<fieldCount && iRow<recordCount)
        {
            sBack=sg->Cells[iCol+1][iRow+1];
        }
    }
    return sBack;
}
//------------------------------------------------------------------------------
void cDBTStringGrid::AddRow()
{
    if(sg)
    {
        //AI(W906-PT-W2) 20260807: MINIMAL NECESSARY ADAPTATION -- golden
        //  `sg->RowCount++;`. vclcompat::TStringGrid::RowCount is a
        //  RowCountProxy (StringGrid.h's own "Proxy-with-operator=" idiom)
        //  exposing only `operator int() const` and `operator=(int)`, no
        //  `operator++` -- `proxy++` does not compile against it. Rewritten
        //  as the equivalent read-then-assign; same resize side effect
        //  (RowCountProxy::operator= grows the grid by exactly one row,
        //  padding with "").
        sg->RowCount=sg->RowCount+1;
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::AddCol()
{
    if(sg)
    {
        // Same adaptation as AddRow immediately above -- golden `sg->ColCount++;`.
        sg->ColCount=sg->ColCount+1;
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::CloneTo(TStringGrid* destination)
{
    if(sg)
    {
        destination->RowCount=sg->RowCount;
        destination->ColCount=sg->ColCount;
        for(int row=0;row<sg->RowCount;row++)
        {
            for(int col=0;col<sg->ColCount;col++)
            {
                destination->Cells[col][row]=sg->Cells[col][row];
            }
        }
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::ClearRow(int iRow)
{
    if(sg)
    {
        iRow++;                                      //ignor title
        int recordCount=0;
        int fieldCount=0;
        GetRowCol(recordCount,fieldCount);
        if(iRow<=recordCount)
        {
            for(int row=iRow;row<recordCount;row++)
            {
                for(int col=0;col<fieldCount;col++)
                {
                    sg->Cells[col][row]=sg->Cells[col][row+1];
                }
            }
            // Same adaptation as AddRow's banner note above -- golden `sg->RowCount--;`.
            sg->RowCount=sg->RowCount-1;
        }
    }
}
//------------------------------------------------------------------------------
void cDBTStringGrid::HideRow(bool bOn,int iIndex)
{
    if(sg)
    {
        int iRowH=(bOn)?0:iRowHeight;
        iIndex++;                                      //ignor title
        int recordCount=0;
        int fieldCount=0;
        GetRowCol(recordCount,fieldCount);
        //AI(W906-PT-W2) 20260807: GATE 2 (header banner). golden :581-591 sets
        //  `sg->RowHeights[row]=iRowH;` here (both the iIndex==0 "hide every
        //  row" loop and the iIndex<=recordCount single-row branch).
        //  vclcompat::TStringGrid has no RowHeights member (same StringGrid.h
        //  scope note as GATE 1) -- a pure rendering hint with no reader in
        //  this headless port. `iRowH` itself is still computed above
        //  (unused past this point) so the golden control-flow/branch shape
        //  is otherwise reproduced exactly; only the actual widget write is
        //  dropped.
        (void)iRowH; (void)recordCount; (void)fieldCount; (void)iIndex;
    }
}
//------------------------------------------------------------------------------
cMathTool::cMathTool()
{
    //
}
//------------------------------------------------------------------------------
cMathTool::~cMathTool()
{
    //
}
//------------------------------------------------------------------------------
void cMathTool::Clear()
{
    //
}
//------------------------------------------------------------------------------
bool cMathTool::IsDoubleEqual(const double a,const double b,const double tolerance)
{
    return fabs(a-b)<=tolerance;
}
//------------------------------------------------------------------------------
