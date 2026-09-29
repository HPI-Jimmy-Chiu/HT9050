// ===========================================================================
//  EventLogAnalysis/ElaFileUtil.h -- the analyzer's file helpers (ELA reports plan R1), no UI.
//  AI(W906-ELA-R1) 20260927 (St02-E).  Golden: EventlogAnalyzer Rev891.0 (SVN r891,
//  D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code): Common.cpp WriteDataToFile (:584-613), MyForceDirectories (:469-509),
//  MyDBIProcess (:23-48); FileInfo.cpp GetNameAndExtension (:144-153), PathCombin (:260-282), EnsureDirectoriesExist
//  (:284-312); Analyzer.cpp IsValidFileName (:2572-2601), FixFolderPath (:2603-2612); and the Delphi 6 SysUtils calls
//  they make (DirectoryExists, ForceDirectories, ExtractFilePath / ExtractFileName, LastDelimiter, TStrings.SaveToFile).
//  Plan: D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md §4 R1.
//  Ledger: docs/ELA_PORT_LEDGER.md "R1 / R2".
//
//  Byte strings in, byte strings out: paths are the ANSI bytes the A-functions (CreateFileA ...) take -- on the machines
//  that is cp950, as golden.  LastDelimiter / ExtractFilePath / ExtractFileName are MBCS-aware like Delphi's (a cp950
//  trail byte 0x5C is not a path delimiter; ::IsDBCSLeadByte = Delphi's LeadBytes, both from the system ANSI code page).
//  PathCombin, FixFolderPath, IsValidFileName's character test and EnsureDirectoriesExist index bytes, as golden does.
//
//  NOT here: FTP_Log (golden TfrmELA::slFTPLog, D:\HT9045_Log\UploadFile\FTP_Log) -- that belongs to ElaFtp (R4).  Golden
//  MyDBIProcess writes to it; here MyDBIProcess reaches it through SetDbiProcessHook, which ElaFtp installs.
// ===========================================================================
#ifndef HT9045_ELA_ELAFILEUTIL_H
#define HT9045_ELA_ELAFILEUTIL_H

#include <string>
#include <vector>

namespace ela {

// ---- Delphi 6 SysUtils (the parts golden's helpers call) ----
int LastDelimiterA(const std::string& delimiters, const std::string& s);   // 1-based, 0 = none; MBCS-aware
std::string ExtractFilePathA(const std::string& fileName);    // up to and including the last '\' or ':'
std::string ExtractFileNameA(const std::string& fileName);    // after the last '\' or ':'
bool DirectoryExistsA(const std::string& name);               // GetFileAttributes: exists and is a directory
bool ForceDirectoriesA(const std::string& dir);               // recursive CreateDir ("" = false; golden raises there)
// TStrings.SaveToFile -> TFileStream fmCreate: CreateFile(GENERIC_READ|GENERIC_WRITE, no sharing, CREATE_ALWAYS), the
// bytes, close.  false = golden EFCreateError / EWriteError (*error says which; may be null).
bool SaveBytesToFile(const std::string& fileName, const std::string& bytes, std::string* error);

// ---- golden Common.cpp ----
// WriteDataToFile(cFilePath, cData, bOverWrite=false) :584-613: CreateFile(GENERIC_WRITE, FILE_SHARE_READ|WRITE,
// OPEN_ALWAYS -- or CREATE_ALWAYS when bOverWrite), seek to the end unless bOverWrite, strlen(cData) bytes, then "\n"
// (LF only).  Golden returns nothing and ignores a failure; here false = the file could not be opened.
bool WriteDataToFile(const std::string& cFilePath, const std::string& cData, bool bOverWrite = false);

// MyDBIProcess(asTable, S1..S6) :23-48: the seven strings as one TStringList CommaText, added to mmoException and to
// slFTPLog (FTP_Log, AddTextWithDateTime).  The FTP_Log half goes to the hook (ElaFtp, R4); no hook = mmoException only.
typedef void (*DbiProcessHook)(const std::string& commaText);
void SetDbiProcessHook(DbiProcessHook hook);                  // ElaFtp (R4) installs its FTP_Log writer; 0 = none
DbiProcessHook GetDbiProcessHook();
void MyDBIProcess(std::vector<std::string>* mmoException, const std::string& asTable, const std::string& S1,
                  const std::string& S2 = "", const std::string& S3 = "", const std::string& S4 = "",
                  const std::string& S5 = "", const std::string& S6 = "");

// MyForceDirectories(Directory, Function) :469-509 -> 1 = the directory exists afterwards, -1 = not.  An empty Directory
// adds "Directory value is NULL!" to mmoException (may be null) and returns -1.  Golden's two catch blocks (exception
// -> mmoException + MyDBIProcess) are unreachable: Delphi 6 ForceDirectories returns False, it does not raise, for a
// non-empty name -- so a directory that cannot be made is a silent -1, as golden.
int MyForceDirectories(const std::string& Directory, const std::string& Function, std::vector<std::string>* mmoException);

// ---- golden FileInfo.cpp ----
// GetNameAndExtension :144-153: split at the LAST '.' of the whole string (not only the name); no '.' = both "".
void GetNameAndExtension(const std::string& asFileName, std::string& asFile, std::string& asExtension);
// PathCombin :260-282: a path that ends in '/' or holds a '/' anywhere is an FTP path (joined with '/'), else '\'; an
// empty path gives sFile alone.
std::string PathCombin(const std::string& sPath, const std::string& sFile);
// EnsureDirectoriesExist :284-312: one trailing '\' dropped, then every prefix up to each '\' made with
// CreateDirectory; stops at the first one that cannot be made (so "\\server\share\x" stops at once -- golden).  An empty
// path: golden's sPath[sPath.Length()] raises ERangeError; here it does nothing.
void EnsureDirectoriesExist(std::string sPath);

// ---- golden Analyzer.cpp (RogerYang 20251104, VTEST MTBF) ----
// IsValidFileName :2572-2601: false = empty, or the name part (ExtractFileName) holds one of \ / : * ? " < > |;
// otherwise true -- and a missing folder (ExtractFilePath) is made on the way (ForceDirectories, result ignored).
bool IsValidFileName(const std::string& fileName);
// FixFolderPath :2603-2612: "" stays ""; else a '\' is added when the last byte is not one.
std::string FixFolderPath(std::string path);

// ---- V906 seam (not golden) ----
// golden literal "D:\HT9045_Log" (N10SaveSummaryData's EventLogSummary folder, Analyzer.cpp:3170; FTP_Log's
// D:\HT9045_Log\UploadFile): W906_HT9045LOG_ROOT when set (getenv not NULL and not ""), the Handler's own seam
// (common.cpp :240 / :425 as9045LogPath).  ctests point it into %TEMP%.
std::string Ht9045LogRoot();

}  // namespace ela

#endif
