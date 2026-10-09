//---------------------------------------------------------------------------
//  Interface/TesterTCP_N06_St02.cpp -- the N06 recipe-sync family of golden 913 Interface/TesterTCP.cpp:1057-1191
//  (= V912, same lines and bytes; 0618 had only the two fire-and-forget void bodies, :1058-1097):
//    LogN06Result / Get7zPath / RunSevenZipSync (file statics there, here too) and the bool
//    TfTesterTCP::CopyRecipeToTester / CopyRecipeFromTester -> TesterTCP_CopyRecipeToTester / TesterTCP_CopyRecipeFromTester
//    (declared in Interface/TesterTCP.h; the 0618 bodies left Interface/TesterTCP.cpp with this card).
//  AI(W906-W195) 20261009 (St02-E). Laptop card W-195 (3) "KYEC"; Steven 1009: a = B (a new St02 file), g = A (follow 913:
//  synchronous -- "要功能打開才要等／換工作檔的時候，本來就是要等變更完成，才能再做新的事").
//  Golden origin: AI(ht9045-v899) 20260625, case CASE-ARDENTEC-20260625-001 (D:\HT9045\docs\mg_w15_analysis.md:198-232):
//  the old ShellExecute was fire-and-forget, void, silent on failure; 913 waits for 7z (<= 10 s), reads its exit code,
//  checks the zip / share / 7z exist, and logs the result once per change (N06000001 OK / N06000002 NG) in the EventLog.
//  No customer gate: only IniConfig.bN06_CopyTesterFile (Configuration [N06]); off -> false before any side effect.
//
//  [W906] deviations (each marked at its line):
//    1. Application->ExeName -> GetModuleFileNameA(NULL) (vclcompat has no TApplication; cpublic.cpp:2153 precedent).
//    2. Test seams, unset in production = golden behaviour:
//       W906_N06ExecHook  -- when non-null, RunSevenZipSync calls it instead of ShellExecuteEx (the ctest never starts 7z);
//       W906_7Z_EXE (env) -- replaces only the golden fallback literal "D:\\HT9045\\7z.exe" (so "7Z_MISSING" is testable
//                            without touching D:\HT9045).
//    3. Steven 1009 c = B / 1 = B: CreateProcessA (no shell32) instead of golden's ShellExecuteEx, with 7z's output piped
//       ("-bsp1 -bb1") into its own day file <as9045LogPath>\N06\N06_7z_yyyymmdd.log (production D:\HT9045_Log\N06\); the
//       10 s wait, the exit-code rule and the N06 event-log lines stay golden's.  7z inherits only that pipe (handle list;
//       ShellExecuteEx inherited nothing) and gets no stdin.
//    4. Steven 1009 (RULINGS 1006 #21, his decision; beyond golden): a failed sync fails the recipe change -- bilingual alarm
//       (answer 6 = A, his text) and START refused until a successful re-change (answers 3 / 4); see W906_N06SyncRecipeOrBlock.
//    5. Steven 1009 answer 2 = B: on the 10 s timeout 7z is terminated (golden left it running) and the day file names the
//       last file it reached.
//---------------------------------------------------------------------------
#include "Interface/TesterTCP.h"
#include "Interface/TesterTCP_N06_St02.h"

#include "Config.h"                  // IniConfig.bN06_CopyTesterFile / asN06_TesterPath
#include "common.h"                  // DataPath / as9045LogPath / MyForceDirectories; vclcompat FileExists / DirectoryExists / ExtractFilePath / IncludeTrailingBackslash
#include "vclcompat/vcl_compat.h"    // AnsiString

#include "canary_support.h"          // ShowMyMessage (the fail-and-stop alarm)

#include <windows.h>                 // CreateProcessA, CreatePipe, PeekNamedPipe, WaitForSingleObject, GetExitCodeProcess, GetModuleFileNameA
#include <cstdio>                    // fopen / fputs (the N06 day file)
#include <cstdlib>                   // getenv
#include <cstring>                   // memset
#include <string>
#include <vector>

// golden cMyDB.h:135 -- declared here, not by #include "cMyDB.h": that header re-declares RecordProcess's default
// argument which this target already gets from canary_support.h (FTPClientForm_St02.cpp:25 notes the same trap).
void __fastcall SaveEventLogInfo(AnsiString aAlarmCode, AnsiString aMess, int iType, AnsiString aStatus, AnsiString sErrPart);

// [W906] 2: the ctest's stand-in for ShellExecuteEx + wait (null in production)
bool (*W906_N06ExecHook)(const char* s7z, const char* sParam, unsigned long* pdwExit) = 0;

namespace {

// golden calls IncludeTrailingPathDelimiter (BCB6 synonym of IncludeTrailingBackslash); TU-local, as TesterTCP.cpp:105
inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// [W906] 1: golden Application->ExeName
AnsiString W906_ExeName()
{
    char buf[MAX_PATH] = { 0 };
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    return (n > 0 && n < MAX_PATH) ? AnsiString(buf) : AnsiString("");
}

// [W906] 4 (Steven 1009, fail-and-stop): what the alarm reports -- the last LogN06Result reason and 7z's last error line
AnsiString s_sN06LastReason;
AnsiString s_sN06Last7zError;                                                   // the last 7z line that reads like an error
AnsiString s_sN06LastFile;                                                      // [W906] 5: the last "- name" / "+ name" line (-bb1)
bool       s_bN06Blocked = false;                                               // START refused until a successful re-change
std::vector<std::string> s_vN06DayLines;                                        // [W906] 3: this run's lines, written by N06FlushDayLog

AnsiString N06Now()
{
    SYSTEMTIME t;
    GetLocalTime(&t);
    AnsiString s;
    s.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%03d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    return s;
}

void N06DayLine(const AnsiString& s)
{
    s_vN06DayLines.push_back(std::string((N06Now() + "  " + s).c_str()));
}

// [W906] 3: one line of 7z's output -- Steven 1009: the day file shows which files were done, where a timeout stopped and
//   7z's own error text; the alarm appends the last error line.
void N06Log7zLine(const AnsiString& sLine)
{
    AnsiString s = sLine.Trim();
    if(s.Length()==0)
        return;
    N06DayLine("  " + s);
    if(s.Length() > 2 && (s.SubString(1, 2)=="- " || s.SubString(1, 2)=="+ "))
        s_sN06LastFile = s.SubString(3, s.Length() - 2);
    const AnsiString sLow = s.LowerCase();
    if(sLow.Pos("error") || sLow.Pos("can not") || sLow.Pos("cannot"))
        s_sN06Last7zError = s;
}

// [W906] 3: <as9045LogPath>\N06\N06_7z_yyyymmdd.log, one block per 7z run (header, 7z's lines, result)
void N06FlushDayLog()
{
    SYSTEMTIME t;
    GetLocalTime(&t);
    const AnsiString sDir = as9045LogPath + "\\N06";
    MyForceDirectories(sDir, __func__);
    AnsiString sFile;
    sFile.sprintf("%s\\N06_7z_%04d%02d%02d.log", sDir.c_str(), t.wYear, t.wMonth, t.wDay);
    FILE* fp = std::fopen(sFile.c_str(), "ab");
    if(fp)
    {
        for(size_t i=0; i<s_vN06DayLines.size(); ++i)
        {
            std::fputs(s_vN06DayLines[i].c_str(), fp);
            std::fputs("\r\n", fp);
        }
        std::fclose(fp);
    }
    s_vN06DayLines.clear();
}

// Steven 1009 answer 6 = A: the plain reason of the alarm's S2
AnsiString N06PlainReason(const AnsiString& sCode)
{
    if(sCode=="SRC_ZIP_MISSING")      return "找不到要複製的 OS_Setting.zip";
    if(sCode=="DEST_PATH_MISSING")    return "測試機共用資料夾不存在或沒連上";
    if(sCode=="7Z_MISSING")           return "找不到 7z.exe";
    if(sCode=="EXEC_FAIL_OR_TIMEOUT") return "7z 無法執行或超過 10 秒（已強制結束）";
    if(sCode.Pos("7Z_EXIT_")==1)      return "7z 回報錯誤（代碼 " + sCode.SubString(9, sCode.Length() - 8) + "）";
    return sCode;
}

// [W906] 3: read what 7z wrote so far without blocking; lines end at \r (the -bsp1 progress) or \n
void N06DrainPipe(HANDLE hRead, std::string& sPending)
{
    for(;;)
    {
        DWORD dwAvail=0;
        if(PeekNamedPipe(hRead, NULL, 0, NULL, &dwAvail, NULL)==FALSE || dwAvail==0)
            return;
        char buf[512];
        DWORD dwGot=0;
        if(ReadFile(hRead, buf, dwAvail < sizeof(buf) ? dwAvail : (DWORD)sizeof(buf), &dwGot, NULL)==FALSE || dwGot==0)
            return;
        for(DWORD i=0; i<dwGot; ++i)
        {
            const char c = buf[i];
            if(c=='\r' || c=='\n')
            {
                if(!sPending.empty()) N06Log7zLine(AnsiString(sPending.c_str()));
                sPending.clear();
            }
            else if((unsigned char)c >= 0x20 || c=='\t')
                sPending += c;                                                  // the progress line's backspaces are dropped
        }
    }
}

// [W906] 3: only the pipe's write end goes to 7z (PROC_THREAD_ATTRIBUTE_HANDLE_LIST, Vista+).  With plain bInheritHandles 7z would
//   also inherit every inheritable handle of the Handler (Winsock sockets are inheritable by default), and a 7z left running after
//   the 10 s timeout would keep the tester / SECS connections open.  MinGW.org's headers predate the API, so it is looked up in
//   kernel32 at run time under our own names (no clash with MSVC's); not found -> plain inheritance.
struct N06StartupInfoEx { STARTUPINFOA StartupInfo; void* lpAttributeList; };
typedef BOOL (WINAPI *N06PfnInitAttr)(void*, DWORD, DWORD, SIZE_T*);
typedef BOOL (WINAPI *N06PfnUpdAttr)(void*, DWORD, DWORD_PTR, void*, SIZE_T, void*, SIZE_T*);
typedef void (WINAPI *N06PfnDelAttr)(void*);
const DWORD     N06_EXTENDED_STARTUPINFO_PRESENT     = 0x00080000;
const DWORD_PTR N06_PROC_THREAD_ATTRIBUTE_HANDLE_LIST = 0x00020002;

} // namespace

//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260625: 新增去重式記錄器，N06 recipe 同步成敗寫入既有 EventLog，避免靜默失敗且不膨脹日誌
static void LogN06Result(AnsiString FileName, bool bOk, AnsiString sReason)       // golden 913 TesterTCP.cpp:1059-1069
{
    static AnsiString sLastKey="";
    s_sN06LastReason = bOk ? AnsiString("") : sReason;                          // [W906] 4: the fail-and-stop alarm names it (before the dedup)
    AnsiString sKey = FileName + "|" + (bOk ? AnsiString("OK") : sReason);
    if(sKey==sLastKey) return;                                                  // 連續相同結果只記第一次
    sLastKey=sKey;
    if(bOk)
        SaveEventLogInfo("N06000001", AnsiString("Recipe synced to OS Tester: ")+FileName, 20, "OK", "");
    else
        SaveEventLogInfo("N06000002", AnsiString("Recipe sync to OS Tester FAIL [")+sReason+"]: "+FileName, 0, "NG", "");
}
//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260625: 取 7z 完整路徑(不硬編碼)，FileExists 為 false 才 fallback
static AnsiString Get7zPath()                                                     // golden 913 :1072-1078
{
    AnsiString s7z = ExtractFilePath(W906_ExeName())+"7z.exe";                   // [W906] 1: golden Application->ExeName
    if(FileExists(s7z)==false)
    {
        const char* env = std::getenv("W906_7Z_EXE");                           // [W906] 2: unset = golden literal
        s7z = (env != 0 && env[0] != 0) ? AnsiString(env) : AnsiString("D:\\HT9045\\7z.exe");
    }
    return s7z;
}
//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260625: 同步執行 7z 並取得 exit code(沿用 main.cpp ShellExecuteEx+WaitForSingleObject 模式)；回傳 false 代表失敗，pdwExit 帶回程序結束碼
static bool RunSevenZipSync(AnsiString s7z, AnsiString sParam, unsigned long *pdwExit)   // golden 913 :1081-1106
{
    if(W906_N06ExecHook)                                                        // [W906] 2: ctest stand-in, never set in production
        return W906_N06ExecHook(s7z.c_str(), sParam.c_str(), pdwExit);

    // [W906] 3 (Steven 1009 c = B, 1 = B): CreateProcess instead of golden's ShellExecuteEx (no shell32), with 7z's stdout / stderr
    //   on a pipe and "-bsp1 -bb1" so every processed file, where a timeout stopped and 7z's own error text reach the N06 day file
    //   (N06Log7zLine / N06FlushDayLog).  The 10 s wait, the exit-code rule and LogN06Result are golden's.
    s_vN06DayLines.clear();
    N06DayLine("7z " + sParam);
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength=sizeof(sa);
    sa.bInheritHandle=TRUE;
    HANDLE hRead=NULL, hWrite=NULL;
    if(CreatePipe(&hRead, &hWrite, &sa, 0)==FALSE)
    {
        AnsiString sE; sE.sprintf("result: CreatePipe failed, GetLastError=%lu", (unsigned long)GetLastError());
        N06DayLine(sE);
        N06FlushDayLog();
        return false;
    }
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);                        // only the child's end is inherited

    N06StartupInfoEx six;
    memset(&six, 0, sizeof(six));
    STARTUPINFOA& si = six.StartupInfo;
    si.cb=sizeof(si);
    si.dwFlags=STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_HIDE;                                                     // golden nShow=SW_HIDE
    si.hStdOutput=hWrite;
    si.hStdError=hWrite;
    si.hStdInput=NULL;                                                          // -y: 7z asks nothing; a password prompt fails at once instead of waiting 10 s
    DWORD dwCreate=CREATE_NO_WINDOW;
    HMODULE hK32=GetModuleHandleA("kernel32.dll");
    N06PfnInitAttr pInit = hK32 ? (N06PfnInitAttr)GetProcAddress(hK32, "InitializeProcThreadAttributeList") : 0;
    N06PfnUpdAttr  pUpd  = hK32 ? (N06PfnUpdAttr) GetProcAddress(hK32, "UpdateProcThreadAttribute") : 0;
    N06PfnDelAttr  pDel  = hK32 ? (N06PfnDelAttr) GetProcAddress(hK32, "DeleteProcThreadAttributeList") : 0;
    std::vector<char> attr;
    if(pInit && pUpd && pDel)
    {
        SIZE_T cb=0;
        pInit(NULL, 1, 0, &cb);                                                 // asks for the size (fails with ERROR_INSUFFICIENT_BUFFER)
        attr.resize(cb ? cb : 1);
        if(cb && pInit(attr.data(), 1, 0, &cb))
        {
            if(pUpd(attr.data(), 0, N06_PROC_THREAD_ATTRIBUTE_HANDLE_LIST, &hWrite, sizeof(HANDLE), NULL, NULL))
            {
                six.lpAttributeList=attr.data();
                si.cb=sizeof(six);
                dwCreate|=N06_EXTENDED_STARTUPINFO_PRESENT;
            }
            else
                pDel(attr.data());
        }
    }
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));
    AnsiString sCmd = "\"" + s7z + "\" " + sParam + " -bsp1 -bb1";
    std::vector<char> cmd(sCmd.c_str(), sCmd.c_str() + sCmd.Length() + 1);      // CreateProcessA may write into the command line
    const BOOL bStarted = CreateProcessA(NULL, cmd.data(), NULL, NULL, TRUE, dwCreate, NULL, NULL, &si, &pi);
    if(six.lpAttributeList) pDel(six.lpAttributeList);
    CloseHandle(hWrite);                                                        // the child holds its own copy
    if(bStarted==FALSE)
    {
        AnsiString sE; sE.sprintf("result: CreateProcess failed, GetLastError=%lu", (unsigned long)GetLastError());
        N06DayLine(sE);
        N06FlushDayLog();
        CloseHandle(hRead);
        return false;
    }

    std::string sPending;                                                       // a line not yet ended by \r or \n
    const DWORD t0 = GetTickCount();
    unsigned long dwWait = WAIT_TIMEOUT;
    for(;;)
    {
        N06DrainPipe(hRead, sPending);
        dwWait = WaitForSingleObject(pi.hProcess, 50);
        if(dwWait==WAIT_OBJECT_0 || GetTickCount()-t0 >= 10000)                // timeout 10s
            break;
    }
    if(dwWait!=WAIT_OBJECT_0)                                                    // [W906] 5 (Steven 1009 answer 2 = B): golden left 7z running
    {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 2000);
    }
    N06DrainPipe(hRead, sPending);
    if(!sPending.empty()) N06Log7zLine(AnsiString(sPending.c_str()));
    if(dwWait!=WAIT_OBJECT_0)
        N06DayLine("result: TIMEOUT -- no exit after 10 s, 7z terminated; last file: " +
                   (s_sN06LastFile.Length() ? s_sN06LastFile : AnsiString("(none)")));

    bool bOk=false;
    unsigned long dwExit=0xFFFFFFFF;
    if(dwWait==WAIT_OBJECT_0)
    {
        DWORD dwCode=0;
        if(GetExitCodeProcess(pi.hProcess, &dwCode))
        {
            dwExit=dwCode;
            bOk=true;                                                           // 順利取得 exit code，由呼叫端判定 0=OK
        }
    }
    if(dwWait==WAIT_OBJECT_0)
    {
        AnsiString sR; sR.sprintf("result: exit code %lu", (unsigned long)dwExit);
        N06DayLine(sR);
    }
    N06FlushDayLog();
    if(pdwExit) *pdwExit=dwExit;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hRead);
    return bOk;
}
//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260625: 改回傳 bool + 同步等待 + 取 exit code + 存在性檢查，避免 N06 上傳靜默失敗
bool TesterTCP_CopyRecipeToTester(AnsiString FileName)                            //Steven 20250612 : for OS Tester.   golden 913 :1109-1151
{
    AnsiString str1, str2, str3;
    if(IniConfig.bN06_CopyTesterFile==false)                                    //Steven 20250327 : OS測試機的工作檔也要上傳
        return false;

    str1.sprintf("%s%s\\OS_Setting.zip", DataPath.c_str(), FileName.c_str());   //Steven 20230710 : OS測試機的工作檔也要上傳
    str2.sprintf("%s", IncludeTrailingPathDelimiter(IniConfig.asN06_TesterPath).c_str());

    if(FileExists(str1)==false)                                                 // 來源 zip 不存在 -> 記錄後返回，不可靜默
    {
        LogN06Result(FileName, false, "SRC_ZIP_MISSING");
        return false;
    }
    if(DirectoryExists(IniConfig.asN06_TesterPath)==false)                      // 目的網路磁碟不存在 -> 記錄後返回
    {
        LogN06Result(FileName, false, "DEST_PATH_MISSING");
        return false;
    }

    AnsiString s7z = Get7zPath();
    if(FileExists(s7z)==false)
    {
        LogN06Result(FileName, false, "7Z_MISSING");
        return false;
    }

    str3.sprintf("e \"%s\" -o\"%s\" -y", str1.c_str(), str2.c_str());
    unsigned long dwExit=0xFFFFFFFF;
    if(RunSevenZipSync(s7z, str3, &dwExit)==false)                              // 啟動或等待逾時失敗
    {
        LogN06Result(FileName, false, "EXEC_FAIL_OR_TIMEOUT");
        return false;
    }
    if(dwExit!=0)                                                               // 7z: 0=OK,1=warn,>=2=error
    {
        AnsiString sR; sR.sprintf("7Z_EXIT_%u", (unsigned)dwExit);
        LogN06Result(FileName, false, sR);
        return false;
    }
    LogN06Result(FileName, true, "");
    return true;
}
//------------------------------------------------------------------------------
//AI(ht9045-v899) 20260625: 同 CopyRecipeToTester，改 bool 回傳 + 同步取 exit code + 存在性檢查 + 去重記錄
bool TesterTCP_CopyRecipeFromTester(AnsiString FileName)                          //Steven 20250612 : for OS Tester.   golden 913 :1154-1191
{
    AnsiString str1, str2, str3;
    if(IniConfig.bN06_CopyTesterFile==false)                                    //Steven 20250327 : OS測試機的工作檔也要上傳
        return false;

    str1.sprintf("%s%s\\OS_Setting.zip", DataPath.c_str(), FileName.c_str());   //Steven 20230710 : OS測試機的工作檔也要上傳
    str2.sprintf("%s%s.ini", IncludeTrailingPathDelimiter(IniConfig.asN06_TesterPath).c_str(), FileName.c_str());

    if(FileExists(str2)==false)                                                 //AI(ht9045-v899) 20260625: 原註解掉的 log 改為呼叫去重記錄器
    {
        LogN06Result(FileName, false, "SRC_INI_MISSING");
        return false;
    }

    AnsiString s7z = Get7zPath();
    if(FileExists(s7z)==false)
    {
        LogN06Result(FileName, false, "7Z_MISSING");
        return false;
    }

    str3.sprintf("a -tzip \"%s\" \"%s\"", str1.c_str(), str2.c_str());
    unsigned long dwExit=0xFFFFFFFF;
    if(RunSevenZipSync(s7z, str3, &dwExit)==false)                              // 啟動或等待逾時失敗
    {
        LogN06Result(FileName, false, "EXEC_FAIL_OR_TIMEOUT");
        return false;
    }
    if(dwExit!=0)                                                               // 7z: 0=OK,1=warn,>=2=error
    {
        AnsiString sR; sR.sprintf("7Z_EXIT_%u", (unsigned)dwExit);
        LogN06Result(FileName, false, sR);
        return false;
    }
    LogN06Result(FileName, true, "");
    return true;
}
//------------------------------------------------------------------------------
//===========================================================================
//  [W906] 4 -- Steven 1009 (RULINGS 1006 #21, his own decision; beyond golden): a failed sync FAILS the recipe change.
//  Golden's callers ignore the bool (913 main.cpp:22267 / :25955); in V906 the two callers go through this function instead:
//  flag off -> silently continue (true, as golden "要功能打開才要等"); OK -> true, START allowed again; failure -> START is
//  refused (W906_N06RecipeSyncBlocked) until a successful re-change, and a bilingual alarm (answer 6 = A, Steven's text) names
//  the reason (the N06000002 event-log line is LogN06Result's, unchanged).
//===========================================================================
// [W906] 4: the file of the failed change, for the START refusal's text
static AnsiString s_sN06BlockedFile;

bool W906_N06SyncRecipeOrBlock(AnsiString FileName)
{
    if(IniConfig.bN06_CopyTesterFile==false)
    {
        s_bN06Blocked=false;
        return true;
    }
    s_sN06Last7zError="";
    s_sN06LastFile="";
    if(TesterTCP_CopyRecipeToTester(FileName))
    {
        s_bN06Blocked=false;
        return true;
    }
    s_bN06Blocked=true;
    s_sN06BlockedFile=FileName;
    // Steven 1009 answer 6 = A (his text); "Append 7z's last error line when there is one"
    const AnsiString sCode = s_sN06LastReason;
    AnsiString sS1 = "N06 recipe sync to OS Tester FAILED: " + sCode + " -- " + FileName +
                     ". Fix it and change the recipe again. START is blocked until then.";
    AnsiString sS2 = "OS 測試機工作檔同步失敗：" + N06PlainReason(sCode) + "（" + sCode + "，" + FileName +
                     "）。請排除後重新換工作檔；完成前不能按 START。";
    if(s_sN06Last7zError.Length())
    {
        sS1 += " (7z: " + s_sN06Last7zError + ")";
        sS2 += "（7z：" + s_sN06Last7zError + "）";
    }
    ShowMyMessage(sS1, sS2, "W906_N06SyncRecipeOrBlock");
    return false;
}

bool W906_N06RecipeSyncBlocked()
{
    return s_bN06Blocked && IniConfig.bN06_CopyTesterFile;
}

// [W906] 4 -- the START side of the same decision (Steven 1009 answer 4 = B): TfMainWeb::StartFromWeb calls this; while the
// last sync failed, START is refused with the reason (the operator would otherwise see START do nothing).  St02's wording,
// in the style of answer 6.
bool W906_N06CheckStartAllowed()
{
    if(W906_N06RecipeSyncBlocked()==false)
        return true;
    ShowMyMessage("START refused: the last N06 recipe sync to OS Tester FAILED: " + s_sN06LastReason + " -- " + s_sN06BlockedFile +
                  ". Fix it and change the recipe again.",
                  "不能按 START：上一次 OS 測試機工作檔同步失敗：" + N06PlainReason(s_sN06LastReason) + "（" + s_sN06LastReason + "，" +
                  s_sN06BlockedFile + "）。請排除後重新換工作檔。",
                  "W906_N06CheckStartAllowed");
    return false;
}
//------------------------------------------------------------------------------
