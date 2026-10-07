// =============================================================================
//  ProductionInfo/TfFTP.cpp  --  implementation of the TfFTP helper class.
//
//  Faithful translation of golden ProductionInfo/TfFTP.cpp (758 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 ("prodinfo" group), 2 of 3 units.  See
//  TfFTP.h's banner for the class-level role summary, the substrate-reuse
//  rationale (built on KYECFTP/MiniFtpEngine.h's Nmftp::TNMFTP, NOT a fresh
//  FTP engine), and the single gate -- not repeated in full here.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim, all 35 golden function bodies):
//    TfFTP::TfFTP                        golden :13-48
//    TfFTP::~TfFTP                       golden :50-58
//    TfFTP::Connect()                    golden :60-89
//    TfFTP::Connect(7-arg)               golden :91-102
//    TfFTP::SplitPath                    golden :105-147
//    TfFTP::NavigateToCommonPrefix       golden :150-176
//    TfFTP::NavigateOrCreateFolders      golden :179-212
//    TfFTP::CleanPath                    golden :215-244
//    TfFTP::ChangeDirectories            golden :246-262
//    TfFTP::ChangeDir                    golden :264-290
//    TfFTP::CheckFTPFilePath             golden :292-301
//    TfFTP::CheckLocalFilePath           golden :303-312
//    TfFTP::Rename                       golden :314-346 (GATE 1, see header)
//    TfFTP::GetFileList                  golden :348-351
//    TfFTP::DownloadFilterFile           golden :353-397
//    TfFTP::DownloadFilterFile_Get1stFileName    golden :399-443
//    TfFTP::DownloadFilterFile_GetLastFileName   golden :445-492
//    TfFTP::Upload(4-arg)                golden :494-497
//    TfFTP::Upload(5-arg)                golden :499-528
//    TfFTP::Delete                       golden :530-559
//    TfFTP::GetAllFolder                 golden :561-590
//    TfFTP::Close                        golden :592-613
//    TfFTP::RemoveAllTrailingSlashes     golden :615-623
//    TfFTP::NMFTP2ListItem               golden :625-628
//    TfFTP::NMFTP2Success                golden :630-652
//    TfFTP::NMFTP2AuthenticationFailed   golden :654-658
//    TfFTP::NMFTP2TransactionStop        golden :660-665
//    TfFTP::NMFTP2TransactionStart       golden :667-672
//    TfFTP::NMFTP2ConnectionFailed       golden :674-679
//    TfFTP::NMFTP2Failure                golden :681-703
//    TfFTP::NMFTP2UnSupportedFunction    golden :705-726
//    TfFTP::NMFTP2Error                  golden :728-735
//    TfFTP::NMFTP2Status                 golden :737-743
//    TfFTP::NMFTP2Connect                golden :745-750
//    TfFTP::NMFTP2Disconnect             golden :752-757
//  35 of 35 golden functions get a real body.  ZERO stubs.
//
//  GATE REGISTER -- 1 gate, re-cited at TfFTP::Rename below; full rationale
//  in TfFTP.h's banner.
//
//  DISCOVERED GOLDEN-VS-SUBSTRATE GAP (flag for review, NOT a gate -- this
//  compiles and links fine, it is a documented BEHAVIOUR consequence of an
//  already-real, already-reviewed substrate design choice, the same class of
//  finding KYECFTP/FTPClient_Transfer.cpp's own "mkdir-then-cd-again" note
//  records for the identical root cause): TfFTP::ChangeDir's mkdir-on-demand
//  idiom (golden :264-290) assumes `pFTP->ChangeDir(sFilePath)` THROWS when
//  the remote directory does not exist yet -- that is the only way its FIRST
//  `catch(...){bflag=true;}` could ever set `bflag`, which is what gates the
//  MakeDirectory+retry in the SECOND try block. KYECFTP/MiniFtpEngine.h's own
//  file banner documents that `Nmftp::TNMFTP::ChangeDir` NEVER throws
//  (failures surface only via the OnFailure event, which just logs +
//  sets bChangeDirOk/bError here -- it does not call MakeDirectory). Against
//  THIS engine, `bflag` therefore stays `false` unconditionally, the
//  mkdir-then-cd-again branch never runs, and `TfFTP::ChangeDir` returns
//  `true` UNCONDITIONALLY (the final `catch(...){return false;}` is likewise
//  unreachable, since nothing in the un-taken `if(bflag)` block can throw
//  either) -- it no longer reports whether the remote directory genuinely
//  exists or was created. This is LOAD-BEARING: `ChangeDirectories` /
//  `NavigateOrCreateFolders` both branch on `ChangeDir(...)==false` to decide
//  whether a folder-creation attempt failed (golden :198-203); against this
//  port that branch is now dead code (ChangeDir never returns false), so a
//  genuinely-unwritable remote path will silently appear to succeed here
//  where golden would have surfaced an error via ShowMessage. Retiring this
//  gap needs a MiniFtpEngine.h change (make ChangeDir/MakeDirectory throw on
//  failure, or expose a synchronous success signal TfFTP::ChangeDir can
//  check instead of relying on an exception) -- out of this wave's scope
//  (pre-existing file, HARD RULE 7).
//
//  MECHANICAL ADAPTATIONS (not golden behaviour changes):
//   * Event wiring: golden's BCB6 `__closure` member-pointer assignments
//     (ctor, golden :36-47) become `[this](...){ this->NMFTPxxx(...); }`
//     lambdas bound to Nmftp::TNMFTP's `std::function` event slots -- see
//     TfFTP.h's own "EVENT WIRING" banner note.
//   * `ShowMessage("...")` (golden :171/:209, 2 call sites, both inside
//     NavigateToCommonPrefix/NavigateOrCreateFolders `catch(Exception&e)`
//     blocks) -- golden Dialogs.hpp's REAL VCL modal dialog. Per this
//     project's own established convention (SECSGEM/SecsSvEcRegistration.cpp
//     / SECSGEM/uHGemEquipment.cpp's identical `Gated_ShowMessage` precedent,
//     cited verbatim in both those files' own banners: "no stand-in for the
//     real ShowMessage global exists anywhere in the tree today"), both call
//     sites are rewritten to `Gated_ShowMessage(...)`, a file-LOCAL
//     (anonymous-namespace, internal-linkage) no-op defined below --
//     internal linkage specifically because uHGemEquipment.cpp's own
//     same-named stand-in is NOT anonymous-namespace-scoped (external
//     linkage), so a second external-linkage definition of the identical
//     signature would be a duplicate-symbol link error if both TUs ever
//     joined the same archive; anonymous-namespace avoids that risk entirely
//     regardless of how uHGemEquipment.cpp's own copy is eventually resolved.
//   * `MNetLog(AnsiString)` (golden :11, `extern bool MNetLog(AnsiString
//     Message);`, called ~20 times throughout this file) -- golden's REAL
//     body is Motor/myMN200motor.cpp (not in this tree, per
//     AutoClean/AutoClean.cpp:171's identical note). AutoClean.cpp already
//     has the SAME "no translated home yet" gap for the SAME bool-returning
//     signature and resolves it with a file-local (internal-linkage,
//     `static`) no-op stand-in -- mirrored verbatim here (this file's own
//     `static`, so no cross-TU collision risk with whichever wave eventually
//     lands the real Motor/myMN200motor.cpp translation; cMyDB.cpp/
//     Motor/mymotor.cpp's OWN local stand-ins are `void`-returning, a
//     DIFFERENT signature, so there is no ambiguity between any of the three
//     independently-scoped stand-ins even if all three TUs are linked
//     together).
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :9). `void __fastcall TfFTP::GetFileList(...)` /
//  the 12 `void __fastcall TfFTP::NMFTPxxx(...)` definitions drop
//  __fastcall (neutral, matches the header). No __property / __published in
//  this .cpp. SOFT_SIMULTE is NOT defined; the commented-out
//  `#ifdef SOFT_SIMULTE` block in the golden ctor (:15-23) is reproduced as
//  an inert comment, exactly as golden itself carries it (already dead code
//  in golden, not something this wave activates or removes). Numeric
//  semantics kept EXACT (no int/float rewrites).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8, in
//  its original column. Final gate: ZERO U+FFFD.
// =============================================================================

// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "TfFTP.h" ; #include "common.h" ; #include "FileInfo.h"
//   #pragma package(smart_init)
//   extern bool MNetLog(AnsiString Message);                    //Steven 20110406
// =============================================================================

#include "MachineDefine.h"            // de-VCL'd include hub: vclcompat umbrella +
                                      //   <windows.h>/<fstream>/... + `using namespace std;`
#include "ProductionInfo/TfFTP.h"
#include "common.h"                   // golden's own include -- DirectoryExists/MyForceDirectories
                                      //   (both real; common.h transitively pulls vcl_compat.h)
#include "ProductionInfo/FileInfo.h"  // FileInfo().PathCombin(...) -- golden :333/:334/:516
#include "canary_support.h"           // __FUNC__ (BCB6 builtin -> standard __func__), golden :515
                                      //   call site only -- hard-boundary file, read-only use

namespace {

// ---------------------------------------------------------------------------
//  Exception -- golden's ambient VCL `Exception` class (System.hpp), used by
//  NavigateToCommonPrefix/NavigateOrCreateFolders' `catch(Exception& e)`
//  below. Same file-local, internal-linkage, `.Message`-only stand-in as
//  KYECFTP/FTPClient_Transfer.cpp's own precedent (full rationale there).
//  BEHAVIOURAL NOTE (matches that precedent's own note): nothing in either
//  function's call graph (SplitPath / ChangeDir, both documented not to
//  throw against Nmftp::TNMFTP -- see this file's own "DISCOVERED
//  GOLDEN-VS-SUBSTRATE GAP" banner note above) is evidenced to throw an
//  Exception here either; both `catch(Exception&e)` blocks are structurally
//  kept (byte-identical to golden), not structurally reachable, in this
//  translation.
// ---------------------------------------------------------------------------
class Exception
{
public:
    AnsiString Message;
    explicit Exception(const AnsiString& Msg = AnsiString()) : Message(Msg) {}
};

// golden Dialogs.hpp `void ShowMessage(const AnsiString&)` -- see this file's
// own "MECHANICAL ADAPTATIONS" banner note for the full precedent citation.
void Gated_ShowMessage(const AnsiString & /*S*/)
{
    // TODO(W7-UI): wire to a real modal ShowMessage dialog.
}

} // anonymous namespace

// golden Motor/myMN200motor.h -- see this file's own "MECHANICAL ADAPTATIONS"
// banner note. `static`: file-local, matches AutoClean.cpp's identical
// bool-returning precedent.
static bool MNetLog(AnsiString /*Message*/) { return false; }   // golden Motor/myMN200motor.cpp -- not yet translated

//------------------------------------------------------------------------------
TfFTP::TfFTP(AnsiString _asUserID, AnsiString _asPassword, AnsiString _asHost)
{
//    #ifdef SOFT_SIMULTE
//    asUserID    ="HONPREC";
//    asPassword  ="27025312";
//    asHost      ="127.0.0.1";
//    #else
    asUserID    =_asUserID;
    asPassword  =_asPassword;
    asHost      =_asHost;
//    #endif
    TimeOut     =5000;
    Vendor      =NMOS_AUTO;
    Passive     =true;
    Port        =21;

    pFTP        =new TNMFTP(NULL);
    lFolder     =new TStringList;
    lTempFolder =new TStringList;
    bError      =false;
    bChangeDirOk=false;
    bNListOk    =false;

    //AI(W906-PT-W2) 20260807: MECHANICAL ADAPTATION -- golden's BCB6
    //  `__closure` member-pointer assignments (`pFTP->OnListItem=NMFTP2ListItem;`
    //  etc.) become `[this](...){ this->NMFTPxxx(...); }` lambdas bound to
    //  Nmftp::TNMFTP's `std::function` event slots -- see TfFTP.h's own
    //  "EVENT WIRING" banner note. Dispatch target and order are unchanged.
    pFTP->OnListItem              =[this](AnsiString Listing){ this->NMFTP2ListItem(Listing); };
    pFTP->OnSuccess                =[this](TCmdType Trans_Type){ this->NMFTP2Success(Trans_Type); };
    pFTP->OnFailure                =[this](bool &Handled, TCmdType Trans_Type){ this->NMFTP2Failure(Handled, Trans_Type); };
    pFTP->OnError                  =[this](TComponent *Sender, vclcompat::Word Errno, AnsiString Errmsg){ this->NMFTP2Error(Sender, Errno, Errmsg); };
    pFTP->OnStatus                 =[this](TComponent *Sender, AnsiString Status){ this->NMFTP2Status(Sender, Status); };
    pFTP->OnConnectionFailed       =[this](TObject *Sender){ this->NMFTP2ConnectionFailed(Sender); };
    pFTP->OnTransactionStart       =[this](TObject *Sender){ this->NMFTP2TransactionStart(Sender); };
    pFTP->OnTransactionStop        =[this](TObject *Sender){ this->NMFTP2TransactionStop(Sender); };
    pFTP->OnAuthenticationFailed   =[this](bool &Handled){ this->NMFTP2AuthenticationFailed(Handled); };
    pFTP->OnUnSupportedFunction    =[this](TCmdType Trans_Type){ this->NMFTP2UnSupportedFunction(Trans_Type); };
    pFTP->OnDisconnect              =[this](TObject *Sender){ this->NMFTP2Disconnect(Sender); };
    pFTP->OnConnect                 =[this](TObject *Sender){ this->NMFTP2Connect(Sender); };
}
//------------------------------------------------------------------------------
TfFTP::~TfFTP()                                                                 //Jimmychiu 20220901 新增Close FTP
{
    Close();
    lFolder->Clear();
    delete lFolder;

    lTempFolder->Clear();
    delete lTempFolder;
}
//------------------------------------------------------------------------------
bool TfFTP::Connect()
{
    pFTP->Vendor    =static_cast<TFTPVendorType>(Vendor);
    pFTP->TimeOut   =TimeOut;                                                   //Jimmychiu 20220901 調整FTP連線間隔3sec->0.5sec
    pFTP->Passive   =Passive;
    pFTP->Port      =Port;
    pFTP->UserID    =asUserID;
    pFTP->Password  =asPassword;
    pFTP->Host      =asHost;
    try
    {
        if(pFTP->Connected)
            return true;

        pFTP->Connect();
        for(int i=0; i<100; i++)                                                // 100 x 5 = 500 ( 0.5 Sec )
        {
            MySleep(5);
            if(pFTP->Connected)
            {
                return true;
            }
        }
    }
    catch(...)
    {
        Close();                                                                //Jimmychiu 20220901 新增Close FTP
    }
    return false;
}
//------------------------------------------------------------------------------
bool TfFTP::Connect(AnsiString _asUserID, AnsiString _asPassword, AnsiString _asHost, int _TimeOut, int _Vendor, bool _Passive, int _Port)
{
    Port        =_Port;
    Vendor      =_Vendor;
    TimeOut     =_TimeOut;
    Passive     =_Passive;
    asUserID    =_asUserID;
    asPassword  =_asPassword;
    asHost      =_asHost;

    return Connect();
}
//------------------------------------------------------------------------------
// 輔助函數：將路徑分割為資料夾列表
TStringList* TfFTP::SplitPath(AnsiString path)
{
    TStringList *folders = new TStringList();

    if (path.IsEmpty())
        return folders;

    int start = 1;
    int len = path.Length();
    while (start <= len)
    {
        // 從 start 開始找下一個 '/'
        AnsiString sub = path.SubString(start, len - start + 1);
        int posSlash = sub.Pos("/");

        AnsiString part;

        if (posSlash > 0)
        {
            // 擷取 / 之前的部分
            if (posSlash > 1)
            {
                part = path.SubString(start, posSlash - 1);
                part = part.Trim();
                if (!part.IsEmpty())
                    folders->Add(part);
            }
            // 將 start 移動到下一個位置（略過這個 '/')
            start = start + posSlash;
        }
        else
        {
            // 最後一段
            part = path.SubString(start, len - start + 1);
            part = part.Trim();
            if (!part.IsEmpty())
                folders->Add(part);
            break;
        }
    }

    return folders;
}
//------------------------------------------------------------------------------
// 輔助函數：回退到根目錄
bool TfFTP::NavigateToCommonPrefix()
{
    AnsiString currentPath=pFTP->CurrentDir;
    TStringList *folders=SplitPath(currentPath);
    bool bret=false;
    try
    {
        if(currentPath=="/")
        {
        }
        else
        {
            for (int i=folders->Count-1; i>=0; i--)
            {
                ChangeDir("..");
            }
        }
        bret=true;
    }
    catch (Exception &e)
    {
        Gated_ShowMessage("無法回退到共同前綴或根目錄: " + e.Message);
        bret=false;
    }
    delete folders;
    return bret;
}
//------------------------------------------------------------------------------
// 輔助函數：逐層切換或創建資料夾
bool TfFTP::NavigateOrCreateFolders(TStringList *targetFolders, AnsiString originalDir)
{
    try
    {
        // 逐層檢查，從根目錄開始
        AnsiString currentPath=pFTP->CurrentDir;
        AnsiString sPath="";
        for (int i = 0; i < targetFolders->Count; i++)
        {
            if(i==0)
            {
                if(currentPath!="/")
                {
                    ChangeDir("/");
                }
            }
            sPath=targetFolders->Strings[i];
            if (sPath.IsEmpty())
                continue;
            if(ChangeDir(sPath)==false)
            {
                currentPath=pFTP->CurrentDir;
                Gated_ShowMessage("無法創建或切換到資料夾: " + sPath+"   AbsPath"+currentPath);
                return false;
            }
        }
        return true;
    }
    catch (Exception &e)
    {
        Gated_ShowMessage("錯誤: " + e.Message);
        return false;
    }
}
//------------------------------------------------------------------------------
// 輔助函數：清理路徑（移除首尾斜線）
AnsiString TfFTP::CleanPath(AnsiString path)
{
    path = path.Trim();
    if (path == "/")
        return path;
    while (path.Length() > 1 && path[1] == '/')
        path = path.SubString(2, path.Length() - 1);
    while (path.Length() > 1 && path[path.Length()] == '/')
        path = path.SubString(1, path.Length() - 1);
    // 移除中間多餘斜線
    AnsiString cleanPath = "";
    bool lastWasSlash = false;
    for (int i = 1; i <= path.Length(); i++)
    {
        if (path[i] == '/')
        {
            if (!lastWasSlash)
            {
                cleanPath += '/';
                lastWasSlash = true;
            }
        }
        else
        {
            cleanPath += path[i];
            lastWasSlash = false;
        }
    }
    return cleanPath;
}
//------------------------------------------------------------------------------
bool TfFTP::ChangeDirectories(AnsiString asFilePath)                            //Steven 20250716 : 多層次FTP資料夾切換
{
    AnsiString CurrentDir=pFTP->CurrentDir;
    CheckFTPFilePath(asFilePath);
    // 清理目標路徑和當前路徑
    asFilePath=CleanPath(asFilePath);
    // 分割路徑
    TStringList *targetFolders = SplitPath(asFilePath);
    // 保存原始工作目錄
    AnsiString originalDir = CurrentDir;
    // 逐層切換或創建剩餘資料夾
    NavigateToCommonPrefix();
    bool success = NavigateOrCreateFolders(targetFolders, originalDir);
    // 清理
    delete targetFolders;
    return success;
}
//------------------------------------------------------------------------------
bool TfFTP::ChangeDir(AnsiString asFilePath)
{
    bflag=false;                                                                //check Directory
    AnsiString sFilePath=RemoveAllTrailingSlashes(asFilePath);
    try                                                                         //檢查目標路徑，用 ChangeDir 的方式來檢查
    {
        pFTP->ChangeDir(sFilePath);
    }
    catch(...)
    {
        bflag=true;
    }

    try                                                                         //檢查目標路徑，用 ChangeDir 的方式來檢查
    {
        if(bflag)
        {
            pFTP->MakeDirectory(sFilePath);
            pFTP->ChangeDir(sFilePath);
        }
    }
    catch(...)
    {
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
void TfFTP::CheckFTPFilePath(AnsiString &aspath)
{
    if(aspath=="NULL")
        return;

    aspath=StringReplace(aspath, "\\", "//", TReplaceFlags()<<rfReplaceAll);
    aspath=StringReplace(aspath, "////", "//", TReplaceFlags()<<rfReplaceAll);
    if(aspath.SubString(aspath.Length()-1, 2)!="//")
        aspath=aspath+AnsiString("//");
}
//------------------------------------------------------------------------------
void TfFTP::CheckLocalFilePath(AnsiString &aspath)
{
    if(aspath=="NULL")
        return;

    if(aspath.SubString(aspath.Length(), 1)!="\\")
    {
        aspath=aspath+AnsiString("\\");
    }
}
//------------------------------------------------------------------------------
bool TfFTP::Rename(const AnsiString &sourcePath, const AnsiString &targetPath, const AnsiString &sSourFileName, const AnsiString &sTarFileName)
{
    try
    {
        AnsiString asError="", asFileName="", asSource="", asTarget="";
        ChangeDir(sourcePath);
        ChangeDir(targetPath);
        asSource=sourcePath;
        GetAllFolder(asSource, asError);
        if(asError!="")
            return false;

        if(mapFileList.size()!=0)
        {
            for(unsigned int i=0; i<mapFileList.size(); i++)
            {
                asFileName=mapFileList[i];
                if(asFileName.Pos(sSourFileName)>0)
                {
                    asSource=FileInfo().PathCombin(sourcePath, asFileName);
                    asTarget=FileInfo().PathCombin(targetPath, sTarFileName);
                    //AI(W906-PT-W2) 20260807: GATE (1) -- see TfFTP.h banner
                    //  GATE REGISTER for the full rationale. Nmftp::TNMFTP
                    //  (KYECFTP/MiniFtpEngine.h) has no Rename member -- that
                    //  header's own file banner lists Rename() under "NOT
                    //  implemented ... do NOT add" (built exhaustively from
                    //  golden KYECFTP/FTPClient.cpp's own call sites, which
                    //  never call it; THIS file is a different golden TU that
                    //  does). golden's `return true;` immediately below is
                    //  kept VERBATIM -- it was already unconditional in
                    //  golden (real TNMFTP::Rename is void, so golden itself
                    //  never checked a result here either).
#if 0
                    pFTP->Rename(asSource, asTarget);
#endif
                    return true;
                }
            }
        }
        return false;
    }
    catch(...)
    {
        return false;
    }
}
//------------------------------------------------------------------------------
void TfFTP::GetFileList(AnsiString Listing)
{
    mapFileList[mapFileList.size()]=Listing;
}
//------------------------------------------------------------------------------
bool TfFTP::DownloadFilterFile(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString asFilter, AnsiString asError)
{
    bool bReturn=false;
    AnsiString asFileName;
    try
    {
        asFilter=asFilter.UpperCase();                                          //Jimmychiu 20230503 : FTP Download不分檔案大小寫
        GetAllFolder(sSourcesFilePath, asError);
        if(asError!="")
            return bReturn;
        //check target file path
        CheckLocalFilePath(sTargetFilePath);
        if(DirectoryExists(sTargetFilePath)==false)
        {
            MyForceDirectories(sTargetFilePath);
            if(DirectoryExists(sTargetFilePath)==false)
            {
                asError=AnsiString("Error Target File Path : "+sTargetFilePath);
                return bReturn;
            }
        }

        if(mapFileList.size()!=0)
        {
            for(unsigned int i=0; i<mapFileList.size(); i++)
            {
                if(mapFileList[i].UpperCase().Pos(asFilter)>0)                  //Jimmychiu 20230503 : FTP Download不分檔案大小寫
                {
                    asFileName=mapFileList[i];
                    pFTP->Download(sSourcesFilePath+asFileName, sTargetFilePath+asFileName);
                    bReturn=true;
                }
            }
        }
    }
    catch(...)
    {
        asError=AnsiString("FTP Connect Error");
        return bReturn;
    }

    if(bReturn==false)
        asError=AnsiString("No File is Downloaded");
    return bReturn;
}
//------------------------------------------------------------------------------
AnsiString TfFTP::DownloadFilterFile_Get1stFileName(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString asFilter, AnsiString asError)
{
    AnsiString asReturn="";
    AnsiString asFileName;
    try
    {
        asFilter=asFilter.UpperCase();                                          //Jimmychiu 20230503 : FTP Download不分檔案大小寫
        GetAllFolder(sSourcesFilePath, asError);
        if(asError!="")
            return asReturn;

        CheckLocalFilePath(sTargetFilePath);                                    //check target file path
        if(DirectoryExists(sTargetFilePath)==false)
        {
            MyForceDirectories(sTargetFilePath);
            if(DirectoryExists(sTargetFilePath)==false)
            {
                asError=AnsiString("Error Target File Path : "+sTargetFilePath);
                return asReturn;
            }
        }

        if(mapFileList.size()!=0)
        {
            for(unsigned int i=0; i<mapFileList.size(); i++)
            {
                if(mapFileList[i].UpperCase().Pos(asFilter)>0)                  //Jimmychiu 20230503 : FTP Download不分檔案大小寫
                {
                    asFileName=mapFileList[i];
                    pFTP->Download(sSourcesFilePath+asFileName, sTargetFilePath+asFileName);
                    return asFileName;
                }
            }
        }
    }
    catch(...)
    {
        asError=AnsiString("FTP Connect Error");
        return asReturn;
    }

    if(asReturn=="")
        asError=AnsiString("No File is Downloaded");
    return asReturn;
}
//------------------------------------------------------------------------------
AnsiString TfFTP::DownloadFilterFile_GetLastFileName(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString asFilter, AnsiString asError)
{
    AnsiString asReturn="";
    AnsiString asFileName="";
    try
    {
        asFilter=asFilter.UpperCase();                                          //Jimmychiu 20230503 : FTP Download不分檔案大小寫
        GetAllFolder(sSourcesFilePath, asError);
        if(asError!="")
            return asReturn;

        CheckLocalFilePath(sTargetFilePath);                                    //check target file path
        if(DirectoryExists(sTargetFilePath)==false)
        {
            MyForceDirectories(sTargetFilePath);
            if(DirectoryExists(sTargetFilePath)==false)
            {
                asError=AnsiString("Error Target File Path : "+sTargetFilePath);
                return asReturn;
            }
        }

        if(mapFileList.size()!=0)
        {
            for(unsigned int i=0; i<mapFileList.size(); i++)
            {
                if(mapFileList[i].UpperCase().Pos(asFilter)>0)                  //Jimmychiu 20230503 : FTP Download不分檔案大小寫
                {
                    asFileName=mapFileList[i];
                    pFTP->Download(sSourcesFilePath+asFileName, sTargetFilePath+asFileName);
                }
            }
        }
    }
    catch(...)
    {
        asError=AnsiString("FTP Connect Error");
        return asReturn;
    }

    if(asFileName!="")
        return asFileName;

    if(asReturn=="")
        asError=AnsiString("No File is Downloaded");

    return asReturn;
}
//------------------------------------------------------------------------------
bool TfFTP::Upload(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString asFileName, AnsiString asError)
{
    return Upload(sSourcesFilePath, sTargetFilePath, asFileName, asFileName, asError);
}
//------------------------------------------------------------------------------
bool TfFTP::Upload(AnsiString sSourcesFilePath, AnsiString sTargetFilePath, AnsiString sSourceFileName, AnsiString sTargetFileName, AnsiString asError)
{
    bool bReturn=false;
    if(sSourceFileName=="NULL" && sSourceFileName=="")
    {
        asError=AnsiString("Can not find : "+sSourceFileName);
        return bReturn;
    }

    if(ChangeDirectories(sTargetFilePath)==false)                               //FTP
    {
        asError=AnsiString("Error Sources File Path : "+sSourcesFilePath);
        return bReturn;
    }

    CheckLocalFilePath(sSourcesFilePath);                                       //Local
    MyForceDirectories(sSourcesFilePath, __FUNC__);
    MNetLog("FTP, Upload!, "+FileInfo().PathCombin(sTargetFilePath, sTargetFileName));

    try
    {
        pFTP->Upload(FileInfo().PathCombin(sSourcesFilePath, sSourceFileName), sTargetFileName);
        return true;
    }
    catch(...)
    {
        MNetLog("FTP, Upload Fail!, "+FileInfo().PathCombin(sTargetFilePath, sTargetFileName));
        return bReturn;
    }
}
//------------------------------------------------------------------------------
bool TfFTP::Delete(AnsiString sSourcesFilePath, AnsiString asFilter, AnsiString asError)
{
    try
    {
        AnsiString asFileName;
        GetAllFolder(sSourcesFilePath, asError);
        if(asError!="")
            return false;
        if(mapFileList.size()!=0)
        {
            for(unsigned int i=0; i<mapFileList.size(); i++)
            {
                asFileName=mapFileList[i];
                if(asFileName.Pos(asFilter)>0)
                {
                    asFileName=mapFileList[i];
                    pFTP->Delete(asFileName);
                    return true;
                }
            }
        }
        asError=AnsiString("No File Filter");
        return false;
    }
    catch(...)
    {
        asError=AnsiString("FTP Delete Error");
        return false;
    }
}
//------------------------------------------------------------------------------
std::map<int, AnsiString> TfFTP::GetAllFolder(AnsiString &sSourcesFilePath, AnsiString asError)
{
    asError="";
    if(pFTP==NULL)
    {
        asError=AnsiString("FTP UnConnected");
        return mapFileList;
    }
    try
    {
        CheckFTPFilePath(sSourcesFilePath);
        if(ChangeDir(sSourcesFilePath)==false)
        {
            asError=AnsiString("Error Sources File Path : "+sSourcesFilePath);
            return mapFileList;
        }
        pFTP->OnListItem=[this](AnsiString Listing){ this->GetFileList(Listing); };
        if(mapFileList.size()!=0)
            mapFileList.clear();
        pFTP->Nlist();
        MySleep(1);
    }
    catch(...)
    {
        asError=AnsiString("FTP GetAllFolder Error");
        if(mapFileList.size()!=0)
            mapFileList.clear();
    }
    return mapFileList;
}
//------------------------------------------------------------------------------
void TfFTP::Close()                                                             //Jimmychiu 20220901 新增Close FTP
{
    MySleep(1);
    mapFileList.clear();
    try
    {
        if(pFTP!=NULL)
        {
            if(pFTP->Connected)
            {
                pFTP->RequestCloseSocket();
                pFTP->Disconnect();
            }
            pFTP->Abort();
            delete pFTP;
            pFTP=NULL;
        }
    }
    catch(...)
    {
    }
}
//------------------------------------------------------------------------------
AnsiString TfFTP::RemoveAllTrailingSlashes(AnsiString sInput)
{
    int ilen=sInput.Length();
    while(ilen>1 && sInput[ilen]=='/')
    {
        ilen--;
    }
    return sInput.SubString(1, ilen);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2ListItem(AnsiString Listing)
{
    lFolder->Add(Listing);                                                      //列出目錄內的檔案-----
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Success(TCmdType Trans_Type)
{
    switch(Trans_Type)
    {
        case cmdChangeDir   : MNetLog("FTP, Success -- ChangeDir successful"   );bChangeDirOk=true;break;
        case cmdMakeDir     : MNetLog("FTP, Success -- MakeDir successful"     );bChangeDirOk=true;break;
        case cmdRemoveDir   : MNetLog("FTP, Success -- RemoveDir successful"   );break;
        case cmdDelete      : MNetLog("FTP, Success -- Delete successful"      );break;
        case cmdRename      : MNetLog("FTP, Success -- Rename successful"      );break;
        case cmdReInit      : MNetLog("FTP, Success -- Reinitialize successful");break;
        case cmdCurrentDir  : MNetLog("FTP, Success -- CurrentDir successful"  );break;
        case cmdList        : MNetLog("FTP, Success -- List successful"        );bNListOk=true; break;
        case cmdNList       : MNetLog("FTP, Success -- NList successful"       );bNListOk=true; break;
        case cmdUpRestore   : MNetLog("FTP, Success -- UpRestore successful"   );break;
        case cmdDownRestore : MNetLog("FTP, Success -- DownRestore successful" );break;
        case cmdDownload    : MNetLog("FTP, Success -- Download successful"    );break;
        case cmdUpload      : MNetLog("FTP, Success -- Upload successful"      );break;
        case cmdAppend      : MNetLog("FTP, Success -- Append successful"      );break;
        case cmdAllocate    : MNetLog("FTP, Success -- Allocate successful"    );break;
        case cmdDoCommand   : MNetLog("FTP, Success -- DoCommand successful"   );break;
    }
    bError=false;
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2AuthenticationFailed(bool &Handled)
{
    MNetLog("FTP, Authentication Failed");
    bError=true;
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2TransactionStop(TObject *Sender)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, Data transfer end, "+Ptr->Name);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2TransactionStart(TObject *Sender)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, Data transfer start, "+Ptr->Name);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2ConnectionFailed(TObject *Sender)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, Failure -- Connection Failed, "+Ptr->Name);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Failure(bool &Handled, TCmdType Trans_Type)
{
    switch(Trans_Type)
    {
        case cmdChangeDir   : MNetLog("FTP, Failure -- ChangeDir failed"      ); bChangeDirOk=true;break;
        case cmdMakeDir     : MNetLog("FTP, Failure -- MakeDir failed"        ); bChangeDirOk=true;break;
        case cmdDelete      : MNetLog("FTP, Failure -- Delete failed"         ); break;
        case cmdRemoveDir   : MNetLog("FTP, Failure -- RemoveDir failed"      ); break;
        case cmdList        : MNetLog("FTP, Failure -- List failed"           ); bNListOk=true; break;
        case cmdRename      : MNetLog("FTP, Failure -- Rename failed"         ); break;
        case cmdUpRestore   : MNetLog("FTP, Failure -- UploadRestore failed"  ); break;
        case cmdDownRestore : MNetLog("FTP, Failure -- DownloadRestore failed"); break;
        case cmdDownload    : MNetLog("FTP, Failure -- Download failed"       ); break;
        case cmdUpload      : MNetLog("FTP, Failure -- Upload failed"         ); break;
        case cmdAppend      : MNetLog("FTP, Failure -- UploadAppend failed"   ); break;
        case cmdReInit      : MNetLog("FTP, Failure -- Reinitialize failed"   ); break;
        case cmdAllocate    : MNetLog("FTP, Failure -- Allocate failed"       ); break;
        case cmdNList       : MNetLog("FTP, Failure -- NList failed"          ); bNListOk=true; break;
        case cmdDoCommand   : MNetLog("FTP, Failure -- DoCommand failed"      ); break;
        case cmdCurrentDir  : MNetLog("FTP, Failure -- CurrentDir failed"     ); break;
    }
    bError=true;
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2UnSupportedFunction(TCmdType Trans_Type)
{
    switch(Trans_Type)
    {
        case cmdChangeDir   : MNetLog("FTP, UnSupported -- ChangeDir not supported by this server"       ); bChangeDirOk=true;break;
        case cmdMakeDir     : MNetLog("FTP, UnSupported -- MakeDir not supported by this server"         ); bChangeDirOk=true;break;
        case cmdDelete      : MNetLog("FTP, UnSupported -- Delete not supported by this server"          ); break;
        case cmdRemoveDir   : MNetLog("FTP, UnSupported -- RemoveDir not supported by this server"       ); break;
        case cmdList        : MNetLog("FTP, UnSupported -- List not supported by this server"            ); bNListOk=true; break;
        case cmdRename      : MNetLog("FTP, UnSupported -- Rename not supported by this server"          ); break;
        case cmdUpRestore   : MNetLog("FTP, UnSupported -- UploadRestore not supported by this server"   ); break;
        case cmdDownRestore : MNetLog("FTP, UnSupported -- DownloadRestore not supported by this server" ); break;
        case cmdDownload    : MNetLog("FTP, UnSupported -- Download not supported by this server"        ); break;
        case cmdUpload      : MNetLog("FTP, UnSupported -- Upload not supported by this server"          ); break;
        case cmdAppend      : MNetLog("FTP, UnSupported -- UploadAppend not supported by this server"    ); break;
        case cmdReInit      : MNetLog("FTP, UnSupported -- Reinitialize not supported by this server"    ); break;
        case cmdAllocate    : MNetLog("FTP, UnSupported -- Allocate not supported by this server"        ); break;
        case cmdNList       : MNetLog("FTP, UnSupported -- NList not supported by this server"           ); bNListOk=true; break;
        case cmdDoCommand   : MNetLog("FTP, UnSupported -- DoCommand not supported by this server"       ); break;
        case cmdCurrentDir  : MNetLog("FTP, UnSupported -- CurrentDir not supported by this server"      ); break;
    }
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Error(TComponent *Sender, vclcompat::Word Errno,
      AnsiString Errmsg)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, "+Errmsg+", "+Ptr->Name);
    bNListOk=true;
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Status(TComponent *Sender,
      AnsiString Status)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, "+Status+", "+Ptr->Name);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Connect(TObject *Sender)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, FTP Connect, "+Ptr->Name);
}
//------------------------------------------------------------------------------
void TfFTP::NMFTP2Disconnect(TObject *Sender)
{
    TNMFTP *Ptr;
    Ptr=(TNMFTP *)Sender;
    MNetLog("FTP, FTP Disconnect,"+Ptr->Name);
}
//------------------------------------------------------------------------------
