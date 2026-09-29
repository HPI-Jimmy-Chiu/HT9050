// ===========================================================================
//  EventLogAnalysis/ElaFileUtil.cpp -- see ElaFileUtil.h.  AI(W906-ELA-R1) 20260927 (St02-E).
// ===========================================================================
#include "EventLogAnalysis/ElaFileUtil.h"
#include "EventLogAnalysis/ElaCore.h"   // BcbGetCommaText (MyDBIProcess's TStringList CommaText)

#include <windows.h>

#include <atomic>
#include <cstdlib>
#include <cstring>

namespace ela {

// ===========================================================================
//  Delphi 6 SysUtils
// ===========================================================================
// SysUtils.ByteTypeTest (SysLocale.FarEast): idx (0-based) is a trail byte when an odd number of lead bytes runs right
// before it.  ::IsDBCSLeadByte uses the system ANSI code page, as Delphi's LeadBytes (GetCPInfo(CP_ACP)).
static bool IsTrailByte(const std::string& s, size_t idx)
{
    if (idx == 0 || idx >= s.size() || s[idx] == '\0')
        return false;
    long i = (long)idx - 1;
    while (i >= 0 && ::IsDBCSLeadByte((BYTE)s[(size_t)i]))
        --i;
    return (((long)idx - i) % 2) == 0;
}

int LastDelimiterA(const std::string& delimiters, const std::string& s)
{
    // SysUtils.LastDelimiter: from the end; a match on a trail byte skips its lead byte too
    int result = (int)s.size();
    while (result > 0)
    {
        const char c = s[(size_t)result - 1];
        if (c != '\0' && std::strchr(delimiters.c_str(), c) != 0)
        {
            if (IsTrailByte(s, (size_t)result - 1))
                --result;
            else
                return result;
        }
        --result;
    }
    return 0;
}

std::string ExtractFilePathA(const std::string& fileName)
{
    const int i = LastDelimiterA("\\:", fileName);            // PathDelim + DriveDelim
    return fileName.substr(0, (size_t)i);
}

std::string ExtractFileNameA(const std::string& fileName)
{
    const int i = LastDelimiterA("\\:", fileName);
    return fileName.substr((size_t)i);
}

bool DirectoryExistsA(const std::string& name)
{
    const DWORD code = ::GetFileAttributesA(name.c_str());
    return code != INVALID_FILE_ATTRIBUTES && (code & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static std::string ExcludeTrailingPathDelimiter(const std::string& s)
{
    // IsPathDelimiter(S, Length(S)): the last byte is '\' and a single byte (not a trail byte)
    if (!s.empty() && s[s.size() - 1] == '\\' && !IsTrailByte(s, s.size() - 1))
        return s.substr(0, s.size() - 1);
    return s;
}

// SysUtils.ForceDirectories (Delphi 6): 1 = True, 0 = False, -1 = it raised (an empty name, which the recursion reaches
// for a relative name with no '\', e.g. "abc" -> ExtractFilePath "" -> raise SCannotCreateDir)
static int ForceDirs(const std::string& dirIn)
{
    if (dirIn.empty())
        return -1;
    const std::string dir = ExcludeTrailingPathDelimiter(dirIn);
    if (dir.size() < 3 || DirectoryExistsA(dir) || ExtractFilePathA(dir) == dir)
        return 1;                                             // avoid 'xyz:\' problem
    const int parent = ForceDirs(ExtractFilePathA(dir));
    if (parent < 0)
        return -1;
    return (parent == 1 && ::CreateDirectoryA(dir.c_str(), NULL) != 0) ? 1 : 0;
}

bool ForceDirectoriesA(const std::string& dir)
{
    return ForceDirs(dir) == 1;
}

bool SaveBytesToFile(const std::string& fileName, const std::string& bytes, std::string* error)
{
    // Classes.TFileStream.Create(FileName, fmCreate) -> SysUtils.FileCreate
    HANDLE h = ::CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        if (error) *error = "Cannot create file " + fileName;      // golden EFCreateError (SFCreateError)
        return false;
    }
    DWORD written = 0;
    const BOOL ok = bytes.empty() ? TRUE : ::WriteFile(h, bytes.data(), (DWORD)bytes.size(), &written, NULL);
    ::CloseHandle(h);
    if (!ok || written != (DWORD)bytes.size())
    {
        if (error) *error = "Stream write error";                  // golden EWriteError (SWriteError)
        return false;
    }
    return true;
}

// ===========================================================================
//  golden Common.cpp
// ===========================================================================
bool WriteDataToFile(const std::string& cFilePath, const std::string& cData, bool bOverWrite)
{
    DWORD dwWritten;
    const DWORD dwCreationDisposition = bOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS;
    HANDLE hFile = ::CreateFileA(cFilePath.c_str(),                    // 檔案路徑
                                 GENERIC_WRITE,                        // 寫入權限
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,   // 共享模式
                                 NULL,                                 // 安全屬性
                                 dwCreationDisposition,                // 創建模式（覆寫或追加）
                                 FILE_ATTRIBUTE_NORMAL,                // 檔案屬性
                                 NULL);                                // 模板檔案
    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    if (!bOverWrite)
        ::SetFilePointer(hFile, 0, NULL, FILE_END);
    const char* cData_ = cData.c_str();
    ::WriteFile(hFile, cData_, (DWORD)std::strlen(cData_), &dwWritten, NULL);   // 寫入資料 (golden strlen: stops at a NUL)
    ::WriteFile(hFile, "\n", 1, &dwWritten, NULL);                             // 加入換行符
    ::CloseHandle(hFile);                                                      // 關閉檔案
    return true;
}

static std::atomic<DbiProcessHook>& DbiHookRef()
{
    static std::atomic<DbiProcessHook> hook(static_cast<DbiProcessHook>(0));
    return hook;
}

void SetDbiProcessHook(DbiProcessHook hook) { DbiHookRef().store(hook); }
DbiProcessHook GetDbiProcessHook() { return DbiHookRef().load(); }

void MyDBIProcess(std::vector<std::string>* mmoException, const std::string& asTable, const std::string& S1,
                  const std::string& S2, const std::string& S3, const std::string& S4, const std::string& S5,
                  const std::string& S6)
{
    Row SL;                                                   // golden TStringList *SL (seven Add calls)
    SL.push_back(asTable);
    SL.push_back(S1);
    SL.push_back(S2);
    SL.push_back(S3);
    SL.push_back(S4);
    SL.push_back(S5);
    SL.push_back(S6);
    const std::string commaText = BcbGetCommaText(SL);
    if (mmoException) mmoException->push_back(commaText);     // frmELA->mmoException->Lines->Add(SL->CommaText)
    const DbiProcessHook hook = GetDbiProcessHook();
    if (hook) hook(commaText);                                // frmELA->slFTPLog->AddTextWithDateTime(...) -- ElaFtp (R4)
}

int MyForceDirectories(const std::string& Directory, const std::string& Function, std::vector<std::string>* mmoException)
{
    std::string Str;
    if (Directory == "")
    {
        Str = "Directory value is NULL!";                     // golden Str.sprintf("Directory value is NULL!", ...)
        if (mmoException) mmoException->push_back(Str);
        return -1;
    }
    else
    {
        if (DirectoryExistsA(Directory) == false)
        {
            if (ForceDirs(Directory) < 0)                    // catch(Exception& e): SysUtils raised SCannotCreateDir
            {
                Str = Directory + " -- " + Function;
                if (mmoException)
                {
                    mmoException->push_back("Unable to create directory");   // e.Message
                    mmoException->push_back(Str);
                }
                MyDBIProcess(mmoException, "Exception", "MyForceDirectories", "Create directory fail!", Str, Directory,
                             "Unable to create directory");   // a third mmoException line, as golden
                return -1;
            }
        }
    }
    if (DirectoryExistsA(Directory))
        return 1;
    else
        return -1;
}

// ===========================================================================
//  golden FileInfo.cpp
// ===========================================================================
void GetNameAndExtension(const std::string& asFileName, std::string& asFile, std::string& asExtension)
{
    const int dot_pos = LastDelimiterA(".", asFileName);      // AnsiString::LastDelimiter('.')
    asFile = asExtension = "";
    if (dot_pos > 0)
    {
        asFile = asFileName.substr(0, (size_t)dot_pos - 1);   // SubString(1, dot_pos-1)
        asExtension = asFileName.substr((size_t)dot_pos);     // SubString(dot_pos+1, Length()-dot_pos+1): to the end
    }
}

std::string PathCombin(const std::string& sPath, const std::string& sFile)
{
    std::string combinedPath = sPath;
    if (!combinedPath.empty())
    {
        if (combinedPath[combinedPath.size() - 1] == '/' || combinedPath.find('/') != std::string::npos)   // FTP path format
        {
            if (combinedPath[combinedPath.size() - 1] != '/')
                combinedPath += "/";
        }
        else                                                                                            // Local path
        {
            if (combinedPath[combinedPath.size() - 1] != '\\')
                combinedPath += "\\";
        }
    }
    combinedPath += sFile;
    return combinedPath;
}

void EnsureDirectoriesExist(std::string sPath)
{
    if (sPath.empty())
        return;                                               // golden: sPath[0] raises ERangeError
    if (sPath[sPath.size() - 1] == '\\')
        sPath = sPath.substr(0, sPath.size() - 1);
    size_t pos = 1;                                           // golden 1-based indexes below
    while (pos <= sPath.size())
    {
        const size_t hit = sPath.find('\\', pos - 1);        // SubString(pos, ...).Pos("\\")
        const size_t nextPos = (hit != std::string::npos) ? hit + 1 : sPath.size() + 1;
        const std::string currentPath = sPath.substr(0, nextPos - 1);
        if (::GetFileAttributesA(currentPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            if (!::CreateDirectoryA(currentPath.c_str(), NULL))
                return;
        }
        pos = nextPos + 1;
    }
}

// ===========================================================================
//  golden Analyzer.cpp (RogerYang 20251104 : 偉測MTBF文件生成)
// ===========================================================================
bool IsValidFileName(const std::string& fileName)
{
    //檢查是否為空
    if (fileName.empty())
        return false;

    //檢查是否包含非法字元
    const std::string justName = ExtractFileNameA(fileName);
    const char* illegalChars = "\\/:*?\"<>|";
    for (size_t i = 0; i < justName.size(); i++)
        if (std::strchr(illegalChars, justName[i]))           // (a NUL matches the terminator: false, as golden)
            return false;

    //檢查資料夾是否存在
    const std::string dir = ExtractFilePathA(fileName);
    if (!dir.empty() && !DirectoryExistsA(dir))
    {
        if (ForceDirs(dir) < 0)
            return false;                                     // golden: ForceDirectories raised, the save is abandoned
        return true;
    }

    //正常
    return true;
}

std::string FixFolderPath(std::string path)
{
    if (path.empty())
        return path;
    if (path[path.size() - 1] != '\\')
        path += "\\";
    return path;
}

// ===========================================================================
//  V906 seam
// ===========================================================================
std::string Ht9045LogRoot()
{
    const char* e = std::getenv("W906_HT9045LOG_ROOT");
    return (e != 0 && *e != 0) ? std::string(e) : std::string("D:\\HT9045_Log");
}

}  // namespace ela
