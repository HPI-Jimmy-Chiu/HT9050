// ===========================================================================
//  forms/fLotInfo_Download_St02.cpp -- see forms/fLotInfo_Download_St02.h.
//
//  AI(W906-P14) 20261010 (St02-E) POOL-14 MR-B1 (RULINGS_20261010 #8).  Golden = 913 uLotInfo.cpp, translated line by line in golden
//  order ("// :N" = golden 913 line).  Every deviation is marked [W906]:
//    * TfLotInfo method -> St02 free function; `this` / FileListBox2 / iAutoClean are not on the facade (header).
//    * golden ShellExecute / system / CopyFile go through the KYEC FTP exec seams (KYECFTP/KyecFtpSeams_St02.h; null hook = the same
//      Win32 / CRT call), so a ctest runs no 7z, cmd.exe or copy outside its sandbox.
//    * golden FileListBox2 (a TFileListBox on the Lot Info form, Mask *.*): W906_CountFiles counts the same entries (normal files of
//      that directory; folders and hidden / system files are not listed by a TFileListBox with FileType [ftNormal]).
//    * RMS download (IniConfig.bEnableRms: RMSDownloadByNetwork / RMSDownloadByFTP, golden :4480-4505) is another feature (N05 RMS) and
//      is not in the port: refused before the retry loop (golden would loop on "WAR1684 Retry" with nothing that can succeed).
//    * ATC_InterfaceForm->IsConnect(): the port has no ATC link (TATC_InterfaceFormShim, acarry_shims.h:115; the same GATE WB-16 as
//      KYECFTP/FTPClientUpload_St02.cpp:97), so golden's "connected" arm (:4807-4812) is never taken, the "not connected" arm runs.
//  Customer branches (S25, RULINGS_20261010 #8 Q5): the condition stays, a body that does customer work is gated and named --
//    here: CC_TERAPOWER (:4688-4699, only flips the restore decision: kept as golden), CC_KYEC_LEE / CC_KYEC_XILINX forced Auto Clean
//    (:4743-4758) and CC_SIGURD_PeiXing heating mode (:4760-4789) write the downloaded recipe's own ini (gated), CC_GIGAS (:4794-4798,
//    a log instead of a box: kept as golden), CC_SCC / CC_SCK RMS paths (RMS, above).
// ===========================================================================
#include "forms/fLotInfo_Download_St02.h"

#include <cstdio>
#include <string>
#include <direct.h>            // _rmdir (golden rmdir)
#include <windows.h>           // SetCurrentDirectoryA, DeleteFileA, FindFirstFileA

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"       // CC_*, eByNetwork / eByFTP, eNewATCSystem, eATC60
#include "cmydef.h"            // CUSTOMER_CODE, ATC_SYSTEM, K_RETRY / K_SKIP, MMSystem, bStartATCRun, iATC_RecipeFileTransfer, sATCPath, System*
#include "Config.h"            // IniConfig
#include "CosFunction.h"       // CosFunction
#include "cprod.h"             // Temperature, TestIF, TestIF_File
#include "common.h"            // DataPath, OffsetPath, AuthPath, asATCFileTransferPath, sFTPSetupFileLogPath, MyForceDirectories, MySleep, ini helpers
#include "cpublic.h"           // CompareMD5ByFolder, GetTimeInfo
#include "canary_support.h"    // ShowErrorMessage, ShowMyMessage, RecordProcess
#include "forms/fMain.h"       // fMain->cbSetupFileName
#include "forms/fOffSet.h"     // fOffSet->GetOffsetPath
#include "KYECFTP/FTPClientForm_St02.h"   // fFTPClient (bControlBySECSGEM / bControlByGPIB / memoFTP)
#include "KYECFTP/KyecFtpSeams_St02.h"    // W906_KyecFtpShellExec / _System / _CopyFile

extern void NewRecordProcess(AnsiString S1, AnsiString S2 = "", AnsiString S3 = "");   // golden cMyDB.h (as KYECFTP/FTPClient_Transfer.cpp:77)

AnsiString W906_St02_LotInfo_iAutoClean[69];                                    // golden 913 uLotInfo.h:1366 (header)

namespace {

// BCB6 SysUtils IncludeTrailingPathDelimiter = IncludeTrailingBackslash (not in vclcompat; as cprod.cpp:194)
inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// [W906] golden FileListBox2->Directory=dir; ->Update(); ->Items->Count (a TFileListBox, Mask *.*, FileType [ftNormal])
int W906_CountFiles(const AnsiString& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "*.*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    int n = 0;
    do
    {
        if ((fd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) == 0) ++n;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return n;
}

}  // namespace

// ---------------------------------------------------------------------------
//  golden 913 TfLotInfo::WriteFTPSetupFileChangeLog :14024-14033 (Sam 20210803 : FTP SetFile Change Log)
// ---------------------------------------------------------------------------
void W906_St02_LotInfo_WriteFTPSetupFileChangeLog(AnsiString msg)
{
    AnsiString Path="",Log="";
    GetTimeInfo();                                                              // :14027
    Path.sprintf("%s\\%04d_%02d_%02d", sFTPSetupFileLogPath, SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(Path);
    Log.sprintf("%04d-%02d-%02d, %02d:%02d:%02d:%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, msg);
    Path.sprintf("%s\\%04d_%02d_%02d\\%04d_%02d_%02d_%02d.txt", sFTPSetupFileLogPath, SystemYear, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate, SystemHour);
    WriteDataToFile(Path.c_str() , Log.c_str());                                // :14032
}

// ---------------------------------------------------------------------------
//  golden 913 DoBackupSetupFile :3630-3907 / DoOverWriteSetupFile :3908-4307 -- POOL-14 MR-B2.  Until then (no live caller: MR-C
//  plSLoadClick is the only one) a named stub: the download of a recipe that already exists would not back up / restore the local
//  parameters.
// ---------------------------------------------------------------------------
int W906_St02_LotInfo_DoBackupSetupFile(AnsiString DataPath, AnsiString sDLFileName1)
{
    (void)DataPath;
    W906_St02_FtpGated("golden 913 uLotInfo.cpp:3630-3907 DoBackupSetupFile [" + std::string(sDLFileName1.c_str()) + "] -- POOL-14 MR-B2, not yet");
    return 0;
}

int W906_St02_LotInfo_DoOverWriteSetupFile(AnsiString DataPath, AnsiString sDLFileName)
{
    (void)DataPath;
    W906_St02_FtpGated("golden 913 uLotInfo.cpp:3908-4307 DoOverWriteSetupFile [" + std::string(sDLFileName.c_str()) + "] -- POOL-14 MR-B2, not yet");
    return 0;
}

// ---------------------------------------------------------------------------
//  golden 913 TfLotInfo::DownloadFromServer :4308-4830
// ---------------------------------------------------------------------------
bool W906_St02_LotInfo_DownloadFromServer(AnsiString sDLFileName, bool bFromFTP)
{
    (void)bFromFTP;                                                             // golden never reads it either
    int iCheckSumResult;
    bool bCheckHasAllFile=false;
    AnsiString sLog="";
    AnsiString FileName[8]={"ArmCondition.Data", "Binasgn.Data", "Contact.Data", "HandlerCondition.Data",
                            "HotPlate.Data", "Temperature.Data", "Tester.Data", "Tray.Data"};
    int iHeatingMode=0;                                                         //Sam 20210224 : 北興俊堯要求 NoChamber Head Only，Chamber Head Only;NoChamber Head+Socket，Chamber Head+Chamber
    (void)iHeatingMode;

    SetCurrentDirectoryA("D://");                                               // :4317 golden SetCurrentDirectory(_T("D://"))
    //----------------------
    //檢查是不是有斷線
    //----------------------

    if(IniConfig.bEnableRms)                                                    // :4322 CosFunction.bFTPFunction==false)                 //Steven 20110211 : 使用FTP的不需要檢查
    {
        if(IniConfig.iN05_UpDLMethod==eByFTP)
        {
            //pass
        }
        else if(CUSTOMER_CODE==CC_SCC ||
                CUSTOMER_CODE==CC_SCK)                                          //ChungHung 20130621 add SCK RMS
        {
            if(DirectoryExists(IniConfig.sRmsDownPath)==false)
            {
                ShowErrorMessage("WAR1683", 0, MMSystem);                       //Connection Fail.
                return false;
            }
        }
        else if(IniConfig.bSPILFunction==false)                                 //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            if(DirectoryExists(IniConfig.sRmsPath)==false)
            {
                ShowErrorMessage("WAR1683", 0, MMSystem);                       //Connection Fail.
                return false;
            }
        }
    }

    AnsiString str, str1, str2, str3;
    AnsiString sConfigPath=AuthPath+"Security_new.def";                         // :4348 (golden never reads it)
    (void)sConfigPath;
    AnsiString sNewFilePath, sDLOffsetPath, sCurrOffsetPath;
    AnsiString SPath[2]={IncludeTrailingPathDelimiter(DataPath), IncludeTrailingPathDelimiter(OffsetPath)};

    int ret=0, ret2=0;
    static int iCount=0;
    bool bHasSetUpFile=false, bHasSetUpFile2=false;
    bool bHasATCFileTransfer=false;                                             //Eastsun 20260522 整合: ATC Recipe File Transfer (Dual-Socket) flag
    bool bFilecomplete=false;                                                   //Eastsun 20260522 整合
    if(CosFunction.bUseATCFileTransfer==true &&                                 //Eastsun 20260522 整合
       ATC_SYSTEM==eNewATCSystem )
    {
        bHasATCFileTransfer=true;
    }
    AnsiString sDLFileName1=sDLFileName.SubString(1, sDLFileName.Length()-4);   //找沒有_NET的
    int hInstance;                                                              // [W906] golden HINSTANCE (the ShellExecute seam returns it as int)

    sDLOffsetPath=fOffSet->GetOffsetPath(sDLFileName);                          // :4365
    sCurrOffsetPath=fOffSet->GetOffsetPath();

    if(sDLFileName.AnsiPos(AnsiString("_NET"))==0)
    {
        sDLFileName1=sDLFileName;
    }

    //----------------------
    //首先判斷工作檔有沒有
    //----------------------
    if(CosFunction.bFTPDownloadAlwaysCover)                                     // :4376 JerryYang 20190523 KYEC download工作檔時不要還原本機的參數
    {
        if(!DirectoryExists(SPath[0]+sDLFileName))
        {
            MyForceDirectories(SPath[0]+sDLFileName);
            bHasSetUpFile=false;
        }
        else
        {
            bHasSetUpFile=true;
        }
    }
    else
    {
        if(!DirectoryExists(SPath[0]+sDLFileName1))                             //Ifor 20190520 : Fix  sDLFileName1 -> sDLFileName 避免FTP DownLoad 資料被還原
        {                                                                       //Steven 20200514 : 要放在sDLFileName前面, 避免沒有 _NET的版本誤判
            MyForceDirectories(SPath[0]+sDLFileName1);                          //Ifor 20190520 : Fix  sDLFileName1 -> sDLFileName 避免FTP DownLoad 資料被還原
            bHasSetUpFile=false;
        }
        else
        {
            bHasSetUpFile=true;
        }

        MyForceDirectories(SPath[0]+sDLFileName);                               //Ifor 20190520 : Fix  sDLFileName -> sDLFileName1 避免FTP DownLoad 資料被還原
    }

    if(bHasSetUpFile==false)                                                    // :4403 沒有的話,就先建立新的資料夾
    {
        MyForceDirectories(sDLOffsetPath);
        MyForceDirectories(SPath[0]+sDLFileName);
        fMain->cbSetupFileName->Items->Add(sDLFileName);
    }

    if(IniConfig.bE45_AllSetupFileUseOneFile==false &&                          // :4410
       CosFunction.bUseLocalRecipeOffset==false)                                //Steven 20190109 : 修正統一Offset時,不下載Offset
    {
        if(CosFunction.bFTPFunction)                                            //jou 2012-12-22 下載 Offset 檔案 start
        {
            str3=OffsetPath+sDLFileName+".Offset";
            if(FileExists(str3))
            {
                str1="e \""+str3+"\" -o\""+OffsetPath+sDLFileName+"\\\" -y";
                hInstance=W906_KyecFtpShellExec("d:\\HT9045\\7z.exe", str1.c_str());   // :4419 golden ShellExecute(this, "open", ..., SW_HIDE)
                MySleep(200);
                ::DeleteFileA(str3.c_str());
            }
        }
    }
    else
    {
        str1.sprintf("%s%s", OffsetPath, sDLFileName);                          // :4427
        MyForceDirectories(str1);
    }

    //----------------------
    //檢查Offset檔
    //----------------------
    str1.sprintf("%s\\Position Offset.Data", sDLOffsetPath);                    // :4434
    if(!FileExists(str1))                                                       //如果沒有就從當下的工作檔複製過來
    {
        str2.sprintf("%s\\Position Offset.Data", sCurrOffsetPath);
        W906_KyecFtpCopyFile(str2.c_str(), str1.c_str(), false);                // :4438 golden CopyFile
    }

    str1.sprintf("%s\\Position Offset Hot.Data", sDLOffsetPath);                // :4441
    if(!FileExists(str1))
    {
        str2.sprintf("%s\\Position Offset Hot.Data",  sCurrOffsetPath);
        W906_KyecFtpCopyFile(str2.c_str(), str1.c_str(), false);                // :4445
    }

    if(CosFunction.bFTPFunction==false ||                                       // :4448 Steven 20110211 : 使用FTP的不需要下載
       IniConfig.bEnableRms==true)                                              //Steven 20190109 : 修正FTP跟RMS同時打開的問題
    {
        //----------------------
        //開始下載檔案
        //----------------------
        AnsiString sSourcesFilePath="";
        AnsiString sTargetFilePath=DataPath;
        AnsiString sFileName=sDLFileName;
        (void)sTargetFilePath; (void)sFileName;
        do
        {
            if(FileExists("d:\\HT9045\\7z.exe")==false)                         // :4459
            {
                W906_KyecFtpCopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false);
            }

            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {
                ret2=0;
            }
            else
            {
                if(CUSTOMER_CODE==CC_SCC ||
                   CUSTOMER_CODE==CC_SCK )
                {
                    sSourcesFilePath=IniConfig.sRmsDownPath;
                }
                else
                {
                    sSourcesFilePath=IniConfig.sRmsPath;
                }
                (void)sSourcesFilePath;

                if(IniConfig.iN05_UpDLMethod==eByNetwork ||                     // :4480 / :4493
                   IniConfig.iN05_UpDLMethod==eByFTP)
                {
                    // [W906] golden :4480-4543 RMSDownloadByNetwork / RMSDownloadByFTP (+ the ATC recipe from RMS) -- RMS (N05) is not in
                    //   the port.  Golden would show "WAR1684 下載 %s.zip 失敗" with Retry / Skip and loop on Retry; nothing can succeed, so
                    //   the download is refused here instead (file banner).
                    W906_St02_FtpGated("golden 913 uLotInfo.cpp:4480-4543 RMS download (RMSDownloadByNetwork / RMSDownloadByFTP) -- RMS is not in the port");
                    return false;
                }
            }
        }
        while(ret2==1);                                                         // :4546
    }

    //----------------------
    //把要還原的資料先備份
    //----------------------
    if(bHasSetUpFile)                                                           // :4552 前提是該資料夾已經有資料了
    {
        W906_St02_LotInfo_DoBackupSetupFile(DataPath, sDLFileName1);            //Steven 20191101 : 整合下載工作檔的覆蓋方式
    }
    else                                                                        //Sam 20210803 : FTP SetFile Change Log
    {
        sLog.sprintf("No backup [%s] ", sDLFileName1);
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    }

    //----------------------
    //解壓縮檔案並覆蓋到原本的資料夾
    //----------------------
    if(FileExists(DataPath+sDLFileName+".zip"))                                 // :4565 Steven 20110603
    {
        sNewFilePath=DataPath+sDLFileName+"\\";
        // :4568 FileListBox2->Directory=sNewFilePath;  [W906] W906_CountFiles(sNewFilePath) below

        hInstance=0;                                                            // :4570 golden hInstance=NULL;
        if(IniConfig.FtpUseSystemCallToUnZip)                                   //Steven 20140609
        {
            str1.sprintf("d:\\HT9045\\7z.exe e \"%s%s.zip\" -o\"%s%s\\\" -y", DataPath, sDLFileName, DataPath, sDLFileName);
            ret=W906_KyecFtpSystem(str1.c_str());                               // :4574 golden system()
            if(CosFunction.bUseFTPDownLoadATCRecipe==true   &&                  //JerryYang 20190906 ATC工作檔經由handler上傳/下載
               ATC_SYSTEM==eNewATCSystem                    &&                  //Ifor 20191115 : add FTP DownLoad ATC Recipe
               Temperature.bATCActiveCooling==true          &&
               CosFunction.bHiSiliconFunction==false        )                   //Ifor 20200716 add:Hisi版不下載ATC工作檔
            {
                if(DirectoryExists(sATCPath))
                {
                    if(FileExists(DataPath+sDLFileName+"ATC_Recipe"+".zip"))
                    {
                        str1.sprintf("d:\\HT9045\\7z.exe e \"%s%s.zip\" -o\"%s\\\" -y", DataPath, sDLFileName+"ATC_Recipe", sATCPath);
                        ret=W906_KyecFtpSystem(str1.c_str());                   // :4585
                    }
                    else
                    {
                        ShowMyMessage("Download ATC recipe fail", "ATC recipe not exist in server");
                        return false;
                    }
                }
                else
                {
                    ShowMyMessage("Download ATC recipe fail, check below path", sATCPath);
                    return false;
                }
            }

            if(bHasATCFileTransfer==true)                                       // :4600 Eastsun 20260522 整合
            {
                if(DirectoryExists(asATCFileTransferPath))
                {
                    if(FileExists(DataPath+sDLFileName+"\\"+sDLFileName1+".dat"))
                    {
//                        str1.sprintf("d:\\HT9045\\7z.exe e \"%s%s.dat\" -o\"%s\\\" -y", DataPath, sDLFileName+"ATC_Recipe", asATCFileTransferPath);
//                        ret=system(str1.c_str());
                        str1=DataPath+sDLFileName+"\\"+sDLFileName1+".dat";
                        str2=asATCFileTransferPath+sDLFileName1+".dat";

                        W906_KyecFtpCopyFile(str1.c_str(), str2.c_str() , false);   // :4611 golden CopyFile(..., FALSE)
                            bFilecomplete=true;
                    }
                    else
                    {
                        RecordProcess("Download ATC recipe fail", "ATC recipe not exist in server");
//                        ShowMyMessage("Download ATC recipe fail", "ATC recipe not exist in server");
                        return false;
                    }
                }
                else
                {
                    RecordProcess("Download ATC recipe fail, check below path", asATCFileTransferPath);
//                    ShowMyMessage("Download ATC recipe fail, check below path", asATCFileTransferPath);
                    return false;
                }
            }
        }
        else
        {
            str1="e \""+DataPath+sDLFileName+".zip\" -o\""+DataPath+sDLFileName+"\\\" -y";
            hInstance=W906_KyecFtpShellExec("d:\\HT9045\\7z.exe", str1.c_str());   // :4632 jou 2012-12-14 system改採用ShellExecute
        }
    }
    else
    {
        ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, sDLFileName);  // :4637 下載 %s.zip 失敗
        return false;
    }

    str1.sprintf("%s%s\\%s", DataPath, sDLFileName, sDLFileName);               // :4641
    if(DirectoryExists(str1))
        _rmdir(str1.c_str());                                                   // [W906] golden rmdir

    iCount=0;
    do
    {
        // :4648 FileListBox2->Update();
        iCount++;
        MySleep(500);                                                           //Steven 20140609 : 1000 --> 100
        if(iCount>100)
        {
            ret=-1;
            break;
        }
    }while(W906_CountFiles(sNewFilePath)<8);                                    // :4656 golden FileListBox2->Items->Count<8 (Steven 20140609 : 確認下載解壓縮後的檔案最少8個)

    MySleep(500);                                                               //Steven 20140609 : 1000 --> 100

    iCheckSumResult=CompareMD5ByFolder(DataPath+sDLFileName);                   // :4660 Steven 20170927 (wei) : 比對工作檔的檢查碼是否正確
    if(IniConfig.bN20_CheckMD5 && iCheckSumResult==0)
    {
        ShowErrorMessage("WAR16118", 0, MMSystem, 0, sDLFileName);              //MD5 check fail!
        return false;
    }

    bCheckHasAllFile=true;                                                      // :4667
    for(int i=0; i<8; i++)
    {
        str1.sprintf("%s%s\\%s", DataPath, sDLFileName, FileName[i]);
        if(FileExists(str1)==false)
        {
            bCheckHasAllFile=false;
        }
    }

    if(bCheckHasAllFile &&                                                      // :4677
       ((IniConfig.FtpUseSystemCallToUnZip && ret==0) ||
        hInstance>32))
    {
        //----------------------
        //把壓縮檔砍掉
        //----------------------
        str1.sprintf("%s%s.zip", DataPath, sDLFileName);
        ::DeleteFileA(str1.c_str());                                            // :4685 jou 2012-12-14 system改採用DeleteFile
        MySleep(100);

        if(CUSTOMER_CODE==CC_TERAPOWER)                                         // :4688 Sam 20191210 : 修正下載檔案被還原的問題) //如果不是新的工作檔就要進行資料還原
        {
            bHasSetUpFile2=bHasSetUpFile;
            if(IniConfig.bEnableRms==true ||
               IniConfig.bEnableErms==true)
            {
            }
            else
            {
                bHasSetUpFile=false;
            }
        }

        if(bHasSetUpFile)                                                       // :4701 如果不是新的工作檔就要進行資料還原
        {
            W906_St02_LotInfo_DoOverWriteSetupFile(DataPath, sDLFileName);      //Steven 20191101 : 整合下載工作檔的覆蓋方式
        }
        else
        {
            if(CosFunction.bContactHeightSaveToContactIni)                      // :4707 Steven 20200616 : JSCC要求把Contact Height放到別的檔案
            {
                str1="D:\\HT9045\\system\\Contact.ini";
                if(FileExists(str1)==false || CheckIniData(str1, sDLFileName, "Test Arm1")==false)
                {
                    WriteIniData(str1, sDLFileName, "Pick Up1",             0.0);
                    WriteIniData(str1, sDLFileName, "Test Arm1",            0.0);
                    WriteIniData(str1, sDLFileName, "Drop1",                0.0);
                    WriteIniData(str1, sDLFileName, "Place1",               0.0);
                    WriteIniData(str1, sDLFileName, "Pick Up2",             0.0);
                    WriteIniData(str1, sDLFileName, "Test Arm2",            0.0);
                    WriteIniData(str1, sDLFileName, "Drop2",                0.0);
                    WriteIniData(str1, sDLFileName, "Place2",               0.0);
                    WriteIniData(str1, sDLFileName, "ContactBackUp1",       0.0);
                    WriteIniData(str1, sDLFileName, "ContactBackUp2",       0.0);
                    WriteIniData(str1, sDLFileName, "ShuttlePickBackUp1",   0.0);
                    WriteIniData(str1, sDLFileName, "ShuttlePickBackUp2",   0.0);
                }
            }
        }

        if(CosFunction.bSmartAutoClean && TestIF.bACSmart)                      // :4728 Sam 20240726 : AI Clean
        {
            str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName);
            int iACInterval=ReadIniData(str1, "Configuration",  "iAutoClean_IntervalContact",     20);
            AnsiString sLog="";
            sLog.sprintf("AI clean load default interval : %d", iACInterval);
            NewRecordProcess("", sLog, "");
            if(bHasSetUpFile2)
            {
                // GOLDEN NOTE: golden 913 :4737 prints the AnsiString iAutoClean[24] with %d (it became AnsiString in Steven 20240911, the
                //   format was not changed), so the log shows the string's address; kept as golden (only the log text is affected).
                sLog.sprintf("AI clean load previous interval : %d", W906_St02_LotInfo_iAutoClean[24]);   // [W906] golden iAutoClean[24] (header)
                NewRecordProcess("", sLog, "");
                WriteIniData(str1, "Configuration", "iAutoClean_IntervalContact",     W906_St02_LotInfo_iAutoClean[24]);
            }
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE ||                                        // :4743
           CUSTOMER_CODE==CC_KYEC_XILINX)                                       //Steven 20140826 : KYEC要Auto Clean強制ON
        {
            W906_St02_FtpGated("golden 913 uLotInfo.cpp:4746-4757 CC_KYEC_LEE / CC_KYEC_XILINX: force [Configuration] iAutoClean_Function in the "
                               "downloaded HandlerCondition.Data (S25)");
        }

        if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                    // :4760 Sam 20210224 : 北興俊堯要求 NoChamber Head Only，Chamber Head Only;NoChamber Head+Socket，Chamber Head+Chamber
        {
            W906_St02_FtpGated("golden 913 uLotInfo.cpp:4762-4788 CC_SIGURD_PeiXing: rewrite [Index] Heating Mode / [Index Arm] Counter Air ON Time "
                               "in the downloaded recipe (S25)");
        }

        if(fFTPClient->bControlBySECSGEM==false &&                              // :4791 ChungHung 20150515 add Control by SECSGEM
           fFTPClient->bControlByGPIB==false)                                   //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        {
            if(CUSTOMER_CODE==CC_GIGAS)                                         //Isaac 20200723 : 全智要求不要顯示，有log就好
            {
                str1="UnZip "+sDLFileName+".zip OK.";
                RecordProcess(str1);                                            //UnZip %s.zip OK.
            }
            else
            {
                ShowErrorMessage("MES1687", 0, MMSystem, 0, sDLFileName);       //UnZip %s.zip OK.
            }
        }

        if(bHasATCFileTransfer==true && bFilecomplete==true)                    // :4805 Eastsun 20260522 整合
        {
            static const bool kW906_ATCIsConnect = false;                       // [W906] golden ATC_InterfaceForm->IsConnect(): no ATC link (file banner)
            if(bStartATCRun && kW906_ATCIsConnect)                              // :4807
            {
                RecordProcess("Download ATC PID FileTransfer Start!!");
                fFTPClient->memoFTP->Lines->Add("FTP Download ATC PID FileTransfer Start!!");
                iATC_RecipeFileTransfer=1;                                          //Ifor 20251229 add: ATC Rrcipe 傳送與接收 0:無需傳送 1:需要傳送 2: 傳送中 6:檔案接收中
            }
            else
            {
                RecordProcess("ATC System Connect Error, Download PID FileTransfer Fail!!");
                fFTPClient->memoFTP->Lines->Add("ATC System Connect Error, Download PID FileTransfer Fail!!");
            }
        }
        return true;                                                            // :4819
    }
    else
    {
        if(fFTPClient->bControlBySECSGEM==false &&                              // :4823 ChungHung 20150515 add Control by SECSGEM
           fFTPClient->bControlByGPIB==false)                                   //KaiChen 20181129 ：Add FTP Downlaod Setup File by GPIB Command
        {
            ShowErrorMessage("WAR1686", 0, MMSystem, 0, sDLFileName);           //UnZip %s.zip Fail.
        }
        return false;
    }
}
