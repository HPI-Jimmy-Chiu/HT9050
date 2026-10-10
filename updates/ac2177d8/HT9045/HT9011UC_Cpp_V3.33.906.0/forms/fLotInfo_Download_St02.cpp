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
#include "forms/fLotInfo.h"    // MR-B2: fLotInfo->fTempUserOffset / fTempATCOffset / fContactHeight / iShuttleMode / bART / iART / iIndexHeatingMode
#include "Motor/mymotor.h"     // MR-B2: MOT[MMAutoCleanKit].Tray (the Auto Clean pad counts)
#include "forms/fBuilder.h"    // MR-B3: fBuilder->DeleteSetupFile
#include "forms/fSetup.h"      // MR-B3: fSetup->bFirstTime
#include "Public/ExternFunction.h"   // MR-B3: DeleteDirectory
#include "ATC/ATCInterface.h"  // MR-B3: ATCInterfaceForm (eATCHonPrecType)
#include "SECSGEM/SecsEventReport.h" // MR-B3: EventReport
#include "SECSGEM/SecsEventType.h"   // MR-B3: SECS_EVENT.SwitchSetupFile
#include "LastSet.h"           // MR-B3: LastSet.iRunStartMode
#include <shellapi.h>          // MR-B3: SHFileOperationA (btSaveSetupFileClick)
#include <cstring>             // MR-B3: strncpy / strcmp
#include "KYECFTP/FTPClientForm_St02.h"   // fFTPClient (bControlBySECSGEM / bControlByGPIB / memoFTP)
#include "KYECFTP/KyecFtpSeams_St02.h"    // W906_KyecFtpShellExec / _System / _CopyFile

extern void NewRecordProcess(AnsiString S1, AnsiString S2 = "", AnsiString S3 = "");   // golden cMyDB.h (as KYECFTP/FTPClient_Transfer.cpp:77)
extern void SetRunStartMode(eRunStartMode Mode, AnsiString ModeText);              // aHotPlateSubstrate.h:945 (MR-B3; no defaults restated)

AnsiString W906_St02_LotInfo_iAutoClean[69];                                    // golden 913 uLotInfo.h:1366 (header)
AnsiString W906_St02_LotInfo_strCleanCnt[20][10];                               // golden 913 uLotInfo.h:1370 (header; MR-B2)

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
//  golden 913 TfLotInfo::DoBackupSetupFile :3630-3906 / DoOverWriteSetupFile :3908-4306 (Steven 20191101 : 整合下載工作檔的覆蓋方式)
//  POOL-14 MR-B2, AI(W906-P14) 20261010 (St02-E).  Generated from golden 913 by C:\AI_TempFile\st02e-scratch\p14\gen_p14b2.py, every line in
//  golden order; the only changes ([W906]): TfLotInfo members on the facade -> fLotInfo->X; iAutoClean / strCleanCnt (not on the facade)
//  -> the St02 stand-ins in forms/fLotInfo_Download_St02.h; WriteFTPSetupFileChangeLog -> the St02 function above; CopyFile ->
//  W906_KyecFtpCopyFile (null hook = CopyFileA); DeleteFile -> ::DeleteFileA.
//  Backup reads the current settings of the recipe that is about to be overwritten (Temperature / Contact / HandlerCondition / Tester)
//  into those members and copies HotPlate / UdUld / ArmCondition.Data to DataPath; OverWrite writes them back into the downloaded
//  recipe for every [Network] item of AuthPath Security_new.def that says "do not cover" (CheckAndReadIniData writes the default when
//  the key is missing, as golden), copies the backed-up files back and deletes the DataPath copies.
//  GCC SPLICES, BCC32 DOES NOT (RULINGS_20261010 #9): golden 913 :3679 / :3777 end their // comments with a backslash; bcc32 runs
//    :3680 / :3778 anyway (both backups happen on the machines), GCC would not, so the port drops those two backslashes (notes there).
//  GOLDEN NOTE (kept): OverWrite restores TestMode.Data / Binasgn.Data / BinasgnOff.Data from DataPath (golden :4259-4303) but Backup
//    never copies them there, so those CopyFile calls copy a leftover file or nothing.
//  GOLDEN NOTE (kept): strCleanCnt is [20][10] (golden 913 uLotInfo.h:1370) and both loops index it [X][Y] with X < Auto Clean kit
//    XItem, Y < YItem (golden :3896-3902, :4239-4246) -- a kit tray wider than 20 or taller than 10 would write past the array.
// ---------------------------------------------------------------------------
int W906_St02_LotInfo_DoBackupSetupFile(AnsiString DataPath, AnsiString sDLFileName1)  //Steven 20191101 : 整合下載工作檔的覆蓋方式   // golden 913 :3630
{
    AnsiString str, str1, str2, sLog="";;
    int ret=0;

    //Sam 20210803 : FTP SetFile Change Log
    //==>
    sLog.sprintf("Backup [%s] ", sDLFileName1);
    W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    //<==
    //Sam 20210803 : FTP SetFile Change Log

    //----------------------
    //把溫度Offset資料備份
    //----------------------
    str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName1);

    for(int i=0; i<tcTotalCount; i++)                                           //ChungHung 20140804 add 修正 RMS 原本 只還原到前十個 uer Offset
    {
        str2.printf("CH%d", i+1);
        fLotInfo->fTempUserOffset[i]=ReadIniData(str1, "User OffSet", str2, 0.0);
        if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                        //Steven 20221209 : ATC offset要不要覆蓋
        {
            str2.sprintf("ATCTempOffset[%d]", i);
            fLotInfo->fTempATCOffset[i]=ReadIniData(str1, "ATC", str2, 0.0);
        }
    }
    fLotInfo->iIndexHeatingMode=CheckAndReadIniData(str1, "Index",   "Heating Mode",   0);                                        //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式
    MySleep(100);

    //Sam 20210803 : FTP SetFile Change Log
    //==>
    for(int i=0;i<tcTotalCount;i++)
    {
        sLog.sprintf("fLotInfo->fTempUserOffset[%d]=%3.3f", i, fLotInfo->fTempUserOffset[i]);
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);

        if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                        //Steven 20221209 : ATC offset要不要覆蓋
        {
            sLog.sprintf("fLotInfo->fTempATCOffset[%d]=%3.3f", i, fLotInfo->fTempATCOffset[i]);
            W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
        }
    }
    //<==
    //Sam 20210803 : FTP SetFile Change Log

    //----------------------
    //把HotPlate資料備份
    //----------------------
    // GCC SPLICES, BCC32 DOES NOT (RULINGS_20261010 #9, AI(W906-P14) MR-B2): in golden 913 the next line's comment ends with a backslash;
    //   bcc32 still runs :3680 `str2.sprintf("%sHotPlate.Data", DataPath);`, so the HotPlate backup goes to DataPath.  GCC would splice
    //   :3680 into the comment (str2 = "CH<tcTotalCount>" in the cwd), so the port drops that backslash.  Not a golden bug: no ledger row.
    str1.sprintf("%s%s\\HotPlate.Data", DataPath, sDLFileName1);                //Ifor 20190520 :add HotPlate.Data路徑少  // AI(W906-P14): trailing '\' dropped -- GCC splices, bcc32 does not (RULINGS_20261010 #9)
    str2.sprintf("%sHotPlate.Data", DataPath);
    ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
    MySleep(100);

    //----------------------
    //把Contact高度備份
    //----------------------
    if(CosFunction.bContactHeightSaveToContactIni)                              //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
    {
        str1="D:\\HT9045\\system\\Contact.ini";
        if(FileExists(str1)==false || CheckIniData(str1, sDLFileName1, "Test Arm1")==false)
        {
            fLotInfo->fContactHeight[0]=0.0;
            fLotInfo->fContactHeight[1]=0.0;
            fLotInfo->fContactHeight[2]=0.0;
            fLotInfo->fContactHeight[3]=0.0;
            fLotInfo->fContactHeight[4]=0.0;
            fLotInfo->fContactHeight[5]=0.0;
            fLotInfo->fContactHeight[6]=0.0;
            fLotInfo->fContactHeight[7]=0.0;
            fLotInfo->fContactHeight[20]=0.0;
            fLotInfo->fContactHeight[21]=0.0;
            fLotInfo->fContactHeight[22]=0.0;
            fLotInfo->fContactHeight[23]=0.0;
        }
        else
        {
            fLotInfo->fContactHeight[0]=ReadIniData(str1, sDLFileName1, "Pick Up1", 0.0);
            fLotInfo->fContactHeight[1]=ReadIniData(str1, sDLFileName1, "Test Arm1", 0.0);
            fLotInfo->fContactHeight[2]=ReadIniData(str1, sDLFileName1, "Drop1",    0.0);
            fLotInfo->fContactHeight[3]=ReadIniData(str1, sDLFileName1, "Place1",   0.0);
            fLotInfo->fContactHeight[4]=ReadIniData(str1, sDLFileName1, "Pick Up2", 0.0);
            fLotInfo->fContactHeight[5]=ReadIniData(str1, sDLFileName1, "Test Arm2", 0.0);
            fLotInfo->fContactHeight[6]=ReadIniData(str1, sDLFileName1, "Drop2",    0.0);
            fLotInfo->fContactHeight[7]=ReadIniData(str1, sDLFileName1, "Place2",   0.0);
            fLotInfo->fContactHeight[20]=ReadIniData(str1, sDLFileName1, "ContactBackUp1",     0.0);                              //Steven 20190822 : download cover file include ContactBackUp and ShuttlePickBackUp
            fLotInfo->fContactHeight[21]=ReadIniData(str1, sDLFileName1, "ContactBackUp2",     0.0);
            fLotInfo->fContactHeight[22]=ReadIniData(str1, sDLFileName1, "ShuttlePickBackUp1",     0.0);
            fLotInfo->fContactHeight[23]=ReadIniData(str1, sDLFileName1, "ShuttlePickBackUp2",     0.0);
        }
    }
    else
    {
        str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName1);
        fLotInfo->fContactHeight[0]=ReadIniData(str1, "Test Arm1", "Pick Up", 0.0);
        fLotInfo->fContactHeight[1]=ReadIniData(str1, "Test Arm1", "Contact", 0.0);
        fLotInfo->fContactHeight[2]=ReadIniData(str1, "Test Arm1", "Drop",    0.0);
        fLotInfo->fContactHeight[3]=ReadIniData(str1, "Test Arm1", "Place",   0.0);
        fLotInfo->fContactHeight[4]=ReadIniData(str1, "Test Arm2", "Pick Up", 0.0);
        fLotInfo->fContactHeight[5]=ReadIniData(str1, "Test Arm2", "Contact", 0.0);
        fLotInfo->fContactHeight[6]=ReadIniData(str1, "Test Arm2", "Drop",    0.0);
        fLotInfo->fContactHeight[7]=ReadIniData(str1, "Test Arm2", "Place",   0.0);
        fLotInfo->fContactHeight[20]=ReadIniData(str1, "Test Arm1", "ContactBackUp",     0.0);                                    //Steven 20190822 : download cover file include ContactBackUp and ShuttlePickBackUp
        fLotInfo->fContactHeight[21]=ReadIniData(str1, "Test Arm2", "ContactBackUp",     0.0);
        fLotInfo->fContactHeight[22]=ReadIniData(str1, "Test Arm1", "ShuttlePickBackUp",     0.0);
        fLotInfo->fContactHeight[23]=ReadIniData(str1, "Test Arm2", "ShuttlePickBackUp",     0.0);
    }

    str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName1);
    fLotInfo->fContactHeight[8]=ReadIniData(str1, "Torque Control", "Pin Number",    0.0);
    fLotInfo->fContactHeight[9]=ReadIniData(str1, "Torque Control", "Force Per Pin", 0.0);
    fLotInfo->fContactHeight[19]=ReadIniData(str1,"Torque Control", "Torque",        0.0);

    fLotInfo->fContactHeight[10]=ReadIniData(str1, "Wait Time", "Drop Wait",   1.0);
    fLotInfo->fContactHeight[11]=ReadIniData(str1, "Wait Time", "Drop Speed",  1.0);
    fLotInfo->fContactHeight[12]=ReadIniData(str1, "Mode", "Contact",                          0.0);
    fLotInfo->fContactHeight[13]=ReadIniData(str1, "Mode", "Vacuum",                           0.0);
    fLotInfo->fContactHeight[14]=ReadIniData(str1, "Mode", "Dummy Contact",                    0.0);
    fLotInfo->fContactHeight[15]=ReadIniData(str1, "Mode", "Head Device Mode",                 0.0);
    fLotInfo->fContactHeight[16]=ReadIniData(str1, "Mode", "Kit Diameter",                     3.0);
    fLotInfo->fContactHeight[17]=ReadIniData(str1, "Mode", "Suck Shuttle Device After Tested", 0.0);
    fLotInfo->fContactHeight[18]=ReadIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber", 0.0);

    fLotInfo->fContactHeight[24]=ReadIniData(str1, "Mode", "Die Force Kit Diameter",           2.0);                              //Ifor 20191003 : add Die Force 可以自定義Kit直徑

   //Sam 20210803 : FTP SetFile Change Log
    //==>
    for(int i=0;i<25;i++)
    {
        sLog.sprintf("fLotInfo->fContactHeight[%d]=%3.3f",i,fLotInfo->fContactHeight[i]);
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    }
    //<==
    //Sam 20210803 : FTP SetFile Change Log

    //----------------------
    //把Ld/Uld速度備份
    //----------------------
    str1.sprintf("%s%s\\UdUld.Data", DataPath, sDLFileName1);
    str2.sprintf("%sUdUld.Data", DataPath);
    ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
    MySleep(100);

    //----------------------
    //把速度設定備份
    //----------------------
    str1.sprintf("%s%s\\ArmCondition.Data", DataPath, sDLFileName1);
    // GCC SPLICES, BCC32 DOES NOT (RULINGS_20261010 #9, AI(W906-P14) MR-B2): in golden 913 the next line's comment ends with a backslash;
    //   bcc32 still runs :3778 `ret=CopyFile(str1.c_str(), str2.c_str(), false);`, so ArmCondition.Data is backed up and OverWrite
    //   (golden :4068-4076) restores it.  GCC would splice :3778 into the comment, so the port drops that backslash.
    str2.sprintf("%sArmCondition.Data", DataPath);                              //Ifor 20190520 :add ArmCondition.Data路徑多了  // AI(W906-P14): trailing '\' dropped -- GCC splices, bcc32 does not (RULINGS_20261010 #9)
    ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
    MySleep(100);

    //----------------------
    //備份開啟單一Shuttle選項
    //----------------------
    str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName1);
    fLotInfo->iShuttleMode[0]=ReadIniData(str1, "Configuration", "Shuttle Mode", 0);
    fLotInfo->iShuttleMode[1]=ReadIniData(str1, "Configuration", "Shuttle1 Cancel", 0);

   //Sam 20210803 : FTP SetFile Change Log
    //==>
    for(int i=0; i<2; i++)
    {
        sLog.sprintf("fLotInfo->iShuttleMode[%d]=%d", i, fLotInfo->iShuttleMode[i]);
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    }
    //<==
    //Sam 20210803 : FTP SetFile Change Log

    //----------------------
    //備份Auto Retest選項
    //----------------------                                                    //Steven 20190918 : ART設定下載不覆蓋
    if(CosFunction.bUseSCKART)
    {
        str1.sprintf("%s%s\\Tester.Data", DataPath, sDLFileName1);
        fLotInfo->bART[0]=ReadIniData(str1, "AutoRetest", "Enable ART",              false);
        fLotInfo->bART[1]=ReadIniData(str1, "AutoRetest", "Run ART Without Cmd",     false);
        fLotInfo->bART[2]=ReadIniData(str1, "AutoRetest", "Auto Socket Off",         false);
        fLotInfo->iART=ReadIniData(str1, "AutoRetest", "Try Count",               3);
    }

    //----------------------
    //備份Auto Clean選項
    //----------------------                                                    //Steven 20161116 : ATC說要加上Auto Clean
    str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName1);        //Steven 20220729 : Add

    W906_St02_LotInfo_iAutoClean[ 0]=ReadIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",  AnsiString("1000"));
    W906_St02_LotInfo_iAutoClean[ 1]=ReadIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",  AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 2]=ReadIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 3]=ReadIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",  AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 4]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",  AnsiString("-200"));
    W906_St02_LotInfo_iAutoClean[ 5]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 6]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 7]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[ 8]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",  AnsiString("-200"));
    W906_St02_LotInfo_iAutoClean[ 9]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[10]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[11]=ReadIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[12]=ReadIniData(str1, "Configuration", "HotplatlXOffset",                AnsiString("0.0"));
    W906_St02_LotInfo_iAutoClean[13]=ReadIniData(str1, "Configuration", "HotplatlYOffset",                AnsiString("0.0"));
    W906_St02_LotInfo_iAutoClean[14]=ReadIniData(str1, "Configuration", "HotplatlPickOffset",             AnsiString("0.0"));
    W906_St02_LotInfo_iAutoClean[15]=ReadIniData(str1, "Configuration", "HotplatlPlaceOffset",            AnsiString("0.0"));             //kevin 20150209 end add offset
    W906_St02_LotInfo_iAutoClean[16]=ReadIniData(str1, "Configuration", "HotplatlPitchOffset",            AnsiString("0.0"));             //kevin 20150526
    W906_St02_LotInfo_iAutoClean[17]=ReadIniData(str1, "Configuration", "ShuttlePitchOffset",             AnsiString("0.0"));             //kevin 20150526
    W906_St02_LotInfo_iAutoClean[18]=ReadIniData(str1, "Configuration", "iAutoClean_Function",            AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[19]=ReadIniData(str1, "Configuration", "iAutoClean_Mode",                AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[20]=ReadIniData(str1, "Configuration", "iAutoClean_Tray",                AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[21]=ReadIniData(str1, "Configuration", "iAutoClean_AlarmCount",          AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[22]=ReadIniData(str1, "Configuration", "iAutoClean_iPadThickness",       AnsiString("10"));              //kevin 20180630 add clean pad - device
    W906_St02_LotInfo_iAutoClean[23]=ReadIniData(str1, "Configuration", "iAutoClean_MotorSpeed[0]",       AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[24]=ReadIniData(str1, "Configuration", "iAutoClean_MotorSpeed[1]",       AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[25]=ReadIniData(str1, "Configuration", "iAutoClean_MotorSpeed[2]",       AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[26]=ReadIniData(str1, "Configuration", "iAutoClean_MotorSpeed[3]",       AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[27]=ReadIniData(str1, "Configuration", "iAutoClean_ContactMode",         AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[28]=ReadIniData(str1, "Configuration", "iAutoClean_DropHigh",            AnsiString("0"));               //kevin 20180717 autoClean drop high
    W906_St02_LotInfo_iAutoClean[29]=ReadIniData(str1, "Configuration", "iAutoClean_ContactTime",         AnsiString("5"));
    W906_St02_LotInfo_iAutoClean[30]=ReadIniData(str1, "Configuration", "iAutoClean_ContactCount",        AnsiString("3"));
    W906_St02_LotInfo_iAutoClean[31]=ReadIniData(str1, "Configuration", "iAutoClean_DevicePinCount",      AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[32]=ReadIniData(str1, "Configuration", "fAutoClean_ForcePerPin",         AnsiString("0.0"));             //kevin 20150826
    W906_St02_LotInfo_iAutoClean[33]=ReadIniData(str1, "Configuration", "fAutoClean_DevicePinForceGf",    AnsiString("0.0"));
    W906_St02_LotInfo_iAutoClean[34]=ReadIniData(str1, "Configuration", "fAutoClean_AireForce",           AnsiString("0.0"));
    W906_St02_LotInfo_iAutoClean[35]=ReadIniData(str1, "Configuration", "dAutoClean_XPitch_Kit",          AnsiString("2200.0"));
    W906_St02_LotInfo_iAutoClean[36]=ReadIniData(str1, "Configuration", "dAutoClean_YPitch_Kit",          AnsiString("2200.0"));
    W906_St02_LotInfo_iAutoClean[37]=ReadIniData(str1, "Configuration", "dAutoClean_XStart_Kit",          AnsiString("3300.0"));
    W906_St02_LotInfo_iAutoClean[38]=ReadIniData(str1, "Configuration", "dAutoClean_YStart_Kit",          AnsiString("1800.0"));
    W906_St02_LotInfo_iAutoClean[39]=ReadIniData(str1, "Configuration", "iAutoClean_XDivision_Kit",       AnsiString("8"));
    W906_St02_LotInfo_iAutoClean[40]=ReadIniData(str1, "Configuration", "iAutoClean_YDivision_Kit",       AnsiString("2"));
    W906_St02_LotInfo_iAutoClean[41]=ReadIniData(str1, "Configuration", "dAutoClean_XPitch_Tray",         AnsiString("2200.0"));
    W906_St02_LotInfo_iAutoClean[42]=ReadIniData(str1, "Configuration", "dAutoClean_YPitch_Tray",         AnsiString("2200.0"));
    W906_St02_LotInfo_iAutoClean[43]=ReadIniData(str1, "Configuration", "dAutoClean_XStart_Tray",         AnsiString("3300.0"));
    W906_St02_LotInfo_iAutoClean[44]=ReadIniData(str1, "Configuration", "dAutoClean_YStart_Tray",         AnsiString("1800.0"));
    W906_St02_LotInfo_iAutoClean[45]=ReadIniData(str1, "Configuration", "iAutoClean_XDivision_Tray",      AnsiString("8"));
    W906_St02_LotInfo_iAutoClean[46]=ReadIniData(str1, "Configuration", "iAutoClean_YDivision_Tray",      AnsiString("2"));
    W906_St02_LotInfo_iAutoClean[47]=ReadIniData(str1, "Configuration", "cAutoClean_PackageTray",         AnsiString("BGA"));
    W906_St02_LotInfo_iAutoClean[48]=ReadIniData(str1, "Configuration", "iAutoClean_IntervalContact",     AnsiString("20"));
    W906_St02_LotInfo_iAutoClean[49]=ReadIniData(str1, "Configuration", "iAutoClean_DeveicePices",        AnsiString("8"));
    W906_St02_LotInfo_iAutoClean[50]=ReadIniData(str1, "Configuration", "iCleanIndexOtherArm",            AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[51]=ReadIniData(str1, "Configuration", "iAutoClean_SelectArm",           AnsiString("0"));               //0:Arm1 1:Arm2 2:Arm1 & Arm2
    W906_St02_LotInfo_iAutoClean[52]=ReadIniData(str1, "Configuration", "bAutoClean_FailAlarmLowYield",   AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[53]=ReadIniData(str1, "Configuration", "iAutoClean_LowYieldLimit",       AnsiString("80"));
    W906_St02_LotInfo_iAutoClean[54]=ReadIniData(str1, "Configuration", "iAutoClean_LowYieldCount",       AnsiString("1000"));
    W906_St02_LotInfo_iAutoClean[55]=ReadIniData(str1, "Configuration", "bAutoClean_FailAlarmSiteYieldDifferent", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[56]=ReadIniData(str1, "Configuration", "iAutoClean_FailAlarmSiteYield", AnsiString("80"));
    W906_St02_LotInfo_iAutoClean[57]=ReadIniData(str1, "Configuration", "iAutoClean_FailAlarmSiteYieldDifferentCount", AnsiString("1000"));
    W906_St02_LotInfo_iAutoClean[58]=ReadIniData(str1, "Configuration", "bAutoClean_ConseFailureBySocket_Normal", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[59]=ReadIniData(str1, "Configuration", "iAutoClean_ConseFailureCountBySocket_Normal", AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[60]=ReadIniData(str1, "Configuration", "bAutoClean_ConseFailureBySocket_Retest", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[61]=ReadIniData(str1, "Configuration", "iAutoClean_ConseFailureCountBySocket_Retest", AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[62]=ReadIniData(str1, "Configuration", "bAutoClean_ConseFailureByHead_Normal", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[63]=ReadIniData(str1, "Configuration", "iAutoClean_ConseFailureCountByHead_Normal", AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[64]=ReadIniData(str1, "Configuration", "bAutoClean_ConseFailureByHead_Retest", AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[65]=ReadIniData(str1, "Configuration", "iAutoClean_ConseFailureCountByHead_Retest", AnsiString("10"));
    W906_St02_LotInfo_iAutoClean[66]=ReadIniData(str1, "Configuration", "bAutoClean_UseTray",     AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[67]=ReadIniData(str1, "Configuration", "bAutoClean_UseNSKit",    AnsiString("0"));
    W906_St02_LotInfo_iAutoClean[68]=ReadIniData(str1, "Configuration", "iAutoCleanShuttle",      AnsiString("1"));                       //kevin 20120710

    //Sam 20210803 : FTP SetFile Change Log
    //==>
    for(int i=0; i<69; i++)                                                     //Steven 20240911 : 24 --> 69
    {
        sLog.sprintf("W906_St02_LotInfo_iAutoClean[%d]=%s", i, W906_St02_LotInfo_iAutoClean[i]);
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    }
    //<==
    //Sam 20210803 : FTP SetFile Change Log

    AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");                //JerryYang 20190703 auto clean清潔次數備份
    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
    {
        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
        {
            str.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
            W906_St02_LotInfo_strCleanCnt[X][Y]=ReadIniData(szDir, "Configuration", str, AnsiString(0));
        }
    }

    return ret;
}
//---------------------------------------------------------------------------
int W906_St02_LotInfo_DoOverWriteSetupFile(AnsiString DataPath, AnsiString sDLFileName)   // golden 913 :3908
{
    bool bNeedCover;
    int ret=0;
    AnsiString str, str1, str2, sLog="";;
    AnsiString sConfigPath=AuthPath+"Security_new.def";

    //Sam 20210803 : FTP SetFile Change Log
    //==>
    sLog.sprintf("Restore [%s] ",sDLFileName);
    W906_St02_LotInfo_WriteFTPSetupFileChangeLog(sLog);
    //<==
    //Sam 20210803 : FTP SetFile Change Log
    //----------------------
    //不覆蓋就是必須要還原
    // 0 : 不覆蓋
    // 1 : 要覆蓋
    //----------------------

    //----------------------
    //把溫度Offset資料還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Temp Offset", true);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Temp Offset use local setting");            //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName);
        //for(int i=0; i<10; i++)
        for(int i=0; i<tcTotalCount; i++)                                       //ChungHung 20140804 add 修正 RMS 原本 只還原到前十個 uer Offset
        {
            str2.printf("CH%d", i+1);
            WriteIniData(str1, "User OffSet", str2, fLotInfo->fTempUserOffset[i]);
            if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                    //Steven 20221209 : ATC offset要不要覆蓋
            {
                str2.sprintf("ATCTempOffset[%d]", i);
                WriteIniData(str1, "ATC", str2, fLotInfo->fTempATCOffset[i]);
            }
        }
    }

    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Index Heat Mode", true);                                    //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式
    if(bNeedCover==false)
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Index Heat Mode use local setting");        //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Temperature.Data", DataPath, sDLFileName);
        WriteIniData(str1, "Index",   "Heating Mode",   fLotInfo->iIndexHeatingMode);
    }

    //----------------------
    //把Contact高度還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact High", false);                                      //預設不覆蓋
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Contact High use local setting");           //Sam 20210803 : FTP SetFile Change Log
        if(CosFunction.bContactHeightSaveToContactIni)                          //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
        {
            str1="D:\\HT9045\\system\\Contact.ini";
            WriteIniData(str1, sDLFileName, "Pick Up1",             fLotInfo->fContactHeight[0]);
            WriteIniData(str1, sDLFileName, "Test Arm1",            fLotInfo->fContactHeight[1]);
            WriteIniData(str1, sDLFileName, "Drop1",                fLotInfo->fContactHeight[2]);
            WriteIniData(str1, sDLFileName, "Place1",               fLotInfo->fContactHeight[3]);
            WriteIniData(str1, sDLFileName, "Pick Up2",             fLotInfo->fContactHeight[4]);
            WriteIniData(str1, sDLFileName, "Test Arm2",            fLotInfo->fContactHeight[5]);
            WriteIniData(str1, sDLFileName, "Drop2",                fLotInfo->fContactHeight[6]);
            WriteIniData(str1, sDLFileName, "Place2",               fLotInfo->fContactHeight[7]);
            WriteIniData(str1, sDLFileName, "ContactBackUp1",       fLotInfo->fContactHeight[20]);                                //Steven 20190822 : download cover file include ContactBackUp and ShuttlePickBackUp
            WriteIniData(str1, sDLFileName, "ContactBackUp2",       fLotInfo->fContactHeight[21]);
            WriteIniData(str1, sDLFileName, "ShuttlePickBackUp1",   fLotInfo->fContactHeight[22]);
            WriteIniData(str1, sDLFileName, "ShuttlePickBackUp2",   fLotInfo->fContactHeight[23]);
        }
        else
        {
            str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
            WriteIniData(str1, "Test Arm1", "Pick Up",              fLotInfo->fContactHeight[0]);
            WriteIniData(str1, "Test Arm1", "Contact",              fLotInfo->fContactHeight[1]);
            WriteIniData(str1, "Test Arm1", "Drop",                 fLotInfo->fContactHeight[2]);
            WriteIniData(str1, "Test Arm1", "Place",                fLotInfo->fContactHeight[3]);
            WriteIniData(str1, "Test Arm2", "Pick Up",              fLotInfo->fContactHeight[4]);
            WriteIniData(str1, "Test Arm2", "Contact",              fLotInfo->fContactHeight[5]);
            WriteIniData(str1, "Test Arm2", "Drop",                 fLotInfo->fContactHeight[6]);
            WriteIniData(str1, "Test Arm2", "Place",                fLotInfo->fContactHeight[7]);
            WriteIniData(str1, "Test Arm1", "ContactBackUp",        fLotInfo->fContactHeight[20]);                                //Steven 20190822 : download cover file include ContactBackUp and ShuttlePickBackUp
            WriteIniData(str1, "Test Arm2", "ContactBackUp",        fLotInfo->fContactHeight[21]);
            WriteIniData(str1, "Test Arm1", "ShuttlePickBackUp",    fLotInfo->fContactHeight[22]);
            WriteIniData(str1, "Test Arm2", "ShuttlePickBackUp",    fLotInfo->fContactHeight[23]);
        }
    }

    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact Force", true);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Contact High use local setting");           //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
        WriteIniData(str1, "Torque Control", "Pin Number",    fLotInfo->fContactHeight[8]);
        WriteIniData(str1, "Torque Control", "Force Per Pin", fLotInfo->fContactHeight[9]);
        WriteIniData(str1, "Torque Control", "Torque"       , fLotInfo->fContactHeight[19]);
    }

    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact Mode", true);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Contact High use local setting");           //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Contact.Data", DataPath, sDLFileName);
        WriteIniData(str1, "Wait Time", "Drop Wait",                    fLotInfo->fContactHeight[10]);
        WriteIniData(str1, "Wait Time", "Drop Speed",                   fLotInfo->fContactHeight[11]);
        WriteIniData(str1, "Mode", "Contact",                           fLotInfo->fContactHeight[12]);
        WriteIniData(str1, "Mode", "Vacuum",                            fLotInfo->fContactHeight[13]);
        WriteIniData(str1, "Mode", "Dummy Contact",                     fLotInfo->fContactHeight[14]);
        WriteIniData(str1, "Mode", "Head Device Mode",                  fLotInfo->fContactHeight[15]);
        WriteIniData(str1, "Mode", "Kit Diameter",                      fLotInfo->fContactHeight[16]);
        WriteIniData(str1, "Mode", "Suck Shuttle Device After Tested",  fLotInfo->fContactHeight[17]);
        WriteIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber",  fLotInfo->fContactHeight[18]);

        WriteIniData(str1, "Mode", "Die Force Kit Diameter",            fLotInfo->fContactHeight[24]);                            //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    }

    //----------------------
    //把HotPlate資料還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "HotPlate", false);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("HotPlate use local setting");               //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sHotPlate.Data", DataPath);
        str2.sprintf("%s%s\\HotPlate.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sHotPlate.Data", DataPath);                                  //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);

    //----------------------
    //把Ld/Uld速度還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Load Unload", false);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Load Unload use local setting");            //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sUdUld.Data", DataPath);
        str2.sprintf("%s%s\\UdUld.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sUdUld.Data", DataPath);                                     //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);

    //----------------------
    //把速度設定備份
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Speed Setting", true);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Load Unload use local setting");            //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sArmCondition.Data", DataPath);
        str2.sprintf("%s%s\\ArmCondition.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sArmCondition.Data", DataPath);                              //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);

    //----------------------
    //備份開啟單一Shuttle選項
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Shuttle Mode", false);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Shuttle Mode use local setting");           //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName);
        WriteIniData(str1, "Configuration", "Shuttle Mode", fLotInfo->iShuttleMode[0]);
        WriteIniData(str1, "Configuration", "Shuttle1 Cancel", fLotInfo->iShuttleMode[1]);
    }

    //----------------------
    //備份Auto Retest選項
    //----------------------
    if(CosFunction.bUseSCKART)                                                  //Steven 20190918 : ART設定下載不覆蓋
    {
        bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Auto Retest", true);
    }
    else
    {
        bNeedCover=true;
    }

    if(bNeedCover==false)
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Auto Retest use local setting");            //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Tester.Data", DataPath, sDLFileName);
        WriteIniData(str1, "AutoRetest", "Enable ART",              fLotInfo->bART[0]);
        WriteIniData(str1, "AutoRetest", "Run ART Without Cmd",     fLotInfo->bART[1]);
    }

    if(CosFunction.bUseSCKART)                                                  //Steven 20191101 : ART RT count不覆蓋
    {
        bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "ART_RT_Count ", true);
    }
    else
    {
        bNeedCover=true;
    }

    if(bNeedCover==false)
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("ART_RT_Count use local setting");           //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\Tester.Data", DataPath, sDLFileName);
        WriteIniData(str1, "AutoRetest", "Try Count",              fLotInfo->iART);
        WriteIniData(str1, "AutoRetest", "Auto Socket Off",        fLotInfo->bART[2]);
    }

    //----------------------
    //備份Auto Clean選項
    //----------------------                                                    //Steven 20161116 : ATC說要加上Auto Clean
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean", true);

    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20190712 : 李國旗說要寫死不覆蓋
        bNeedCover=false;                                                       //JerryYang 20190919 : 曹沖說除了Height，其他的都需要正常download覆蓋本機參數

    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Auto Clean use local setting");             //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%s%s\\HandlerCondition.Data", DataPath, sDLFileName);
        if(CUSTOMER_CODE==CC_SCC)                                               //JerryYang 20190919 : 曹沖說除了Height，其他的都需要正常download覆蓋本機參數
        {
            WriteIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",    W906_St02_LotInfo_iAutoClean[ 0]);
            WriteIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",    W906_St02_LotInfo_iAutoClean[ 1]);
            WriteIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",       W906_St02_LotInfo_iAutoClean[ 2]);
            WriteIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",    W906_St02_LotInfo_iAutoClean[ 3]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",    W906_St02_LotInfo_iAutoClean[ 4]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset",   W906_St02_LotInfo_iAutoClean[ 5]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",       W906_St02_LotInfo_iAutoClean[ 6]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",       W906_St02_LotInfo_iAutoClean[ 7]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",    W906_St02_LotInfo_iAutoClean[ 8]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset",   W906_St02_LotInfo_iAutoClean[ 9]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",       W906_St02_LotInfo_iAutoClean[10]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",       W906_St02_LotInfo_iAutoClean[11]);
            WriteIniData(str1, "Configuration", "HotplatlXOffset",                  W906_St02_LotInfo_iAutoClean[12]);
            WriteIniData(str1, "Configuration", "HotplatlYOffset",                  W906_St02_LotInfo_iAutoClean[13]);
            WriteIniData(str1, "Configuration", "HotplatlPickOffset",               W906_St02_LotInfo_iAutoClean[14]);
            WriteIniData(str1, "Configuration", "HotplatlPlaceOffset",              W906_St02_LotInfo_iAutoClean[15]);
            WriteIniData(str1, "Configuration", "HotplatlPitchOffset",              W906_St02_LotInfo_iAutoClean[16]);
            WriteIniData(str1, "Configuration", "ShuttlePitchOffset",               W906_St02_LotInfo_iAutoClean[17]);
        }
        else
        {
            WriteIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",                W906_St02_LotInfo_iAutoClean[ 0]);
            WriteIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",                W906_St02_LotInfo_iAutoClean[ 1]);
            WriteIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",                   W906_St02_LotInfo_iAutoClean[ 2]);
            WriteIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",                W906_St02_LotInfo_iAutoClean[ 3]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",                W906_St02_LotInfo_iAutoClean[ 4]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset",               W906_St02_LotInfo_iAutoClean[ 5]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",                   W906_St02_LotInfo_iAutoClean[ 6]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",                   W906_St02_LotInfo_iAutoClean[ 7]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",                W906_St02_LotInfo_iAutoClean[ 8]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset",               W906_St02_LotInfo_iAutoClean[ 9]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",                   W906_St02_LotInfo_iAutoClean[10]);
            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",                   W906_St02_LotInfo_iAutoClean[11]);
            WriteIniData(str1, "Configuration", "HotplatlXOffset",                              W906_St02_LotInfo_iAutoClean[12]);
            WriteIniData(str1, "Configuration", "HotplatlYOffset",                              W906_St02_LotInfo_iAutoClean[13]);
            WriteIniData(str1, "Configuration", "HotplatlPickOffset",                           W906_St02_LotInfo_iAutoClean[14]);
            WriteIniData(str1, "Configuration", "HotplatlPlaceOffset",                          W906_St02_LotInfo_iAutoClean[15]);
            WriteIniData(str1, "Configuration", "HotplatlPitchOffset",                          W906_St02_LotInfo_iAutoClean[16]);
            WriteIniData(str1, "Configuration", "ShuttlePitchOffset",                           W906_St02_LotInfo_iAutoClean[17]);
            WriteIniData(str1, "Configuration", "iAutoClean_Function",                          W906_St02_LotInfo_iAutoClean[18]);
            WriteIniData(str1, "Configuration", "iAutoClean_Mode",                              W906_St02_LotInfo_iAutoClean[19]);
            WriteIniData(str1, "Configuration", "iAutoClean_Tray",                              W906_St02_LotInfo_iAutoClean[20]);
            WriteIniData(str1, "Configuration", "iAutoClean_AlarmCount",                        W906_St02_LotInfo_iAutoClean[21]);
            WriteIniData(str1, "Configuration", "iAutoClean_iPadThickness",                     W906_St02_LotInfo_iAutoClean[22]);
            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[0]",                     W906_St02_LotInfo_iAutoClean[23]);
            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[1]",                     W906_St02_LotInfo_iAutoClean[24]);
            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[2]",                     W906_St02_LotInfo_iAutoClean[25]);
            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[3]",                     W906_St02_LotInfo_iAutoClean[26]);
            WriteIniData(str1, "Configuration", "iAutoClean_ContactMode",                       W906_St02_LotInfo_iAutoClean[27]);
            WriteIniData(str1, "Configuration", "iAutoClean_DropHigh",                          W906_St02_LotInfo_iAutoClean[28]);
            WriteIniData(str1, "Configuration", "iAutoClean_ContactTime",                       W906_St02_LotInfo_iAutoClean[29]);
            WriteIniData(str1, "Configuration", "iAutoClean_ContactCount",                      W906_St02_LotInfo_iAutoClean[30]);
            WriteIniData(str1, "Configuration", "iAutoClean_DevicePinCount",                    W906_St02_LotInfo_iAutoClean[31]);
            WriteIniData(str1, "Configuration", "fAutoClean_ForcePerPin",                       W906_St02_LotInfo_iAutoClean[32]);
            WriteIniData(str1, "Configuration", "fAutoClean_DevicePinForceGf",                  W906_St02_LotInfo_iAutoClean[33]);
            WriteIniData(str1, "Configuration", "fAutoClean_AireForce",                         W906_St02_LotInfo_iAutoClean[34]);
            WriteIniData(str1, "Configuration", "dAutoClean_XPitch_Kit",                        W906_St02_LotInfo_iAutoClean[35]);
            WriteIniData(str1, "Configuration", "dAutoClean_YPitch_Kit",                        W906_St02_LotInfo_iAutoClean[36]);
            WriteIniData(str1, "Configuration", "dAutoClean_XStart_Kit",                        W906_St02_LotInfo_iAutoClean[37]);
            WriteIniData(str1, "Configuration", "dAutoClean_YStart_Kit",                        W906_St02_LotInfo_iAutoClean[38]);
            WriteIniData(str1, "Configuration", "iAutoClean_XDivision_Kit",                     W906_St02_LotInfo_iAutoClean[39]);
            WriteIniData(str1, "Configuration", "iAutoClean_YDivision_Kit",                     W906_St02_LotInfo_iAutoClean[40]);
            WriteIniData(str1, "Configuration", "dAutoClean_XPitch_Tray",                       W906_St02_LotInfo_iAutoClean[41]);
            WriteIniData(str1, "Configuration", "dAutoClean_YPitch_Tray",                       W906_St02_LotInfo_iAutoClean[42]);
            WriteIniData(str1, "Configuration", "dAutoClean_XStart_Tray",                       W906_St02_LotInfo_iAutoClean[43]);
            WriteIniData(str1, "Configuration", "dAutoClean_YStart_Tray",                       W906_St02_LotInfo_iAutoClean[44]);
            WriteIniData(str1, "Configuration", "iAutoClean_XDivision_Tray",                    W906_St02_LotInfo_iAutoClean[45]);
            WriteIniData(str1, "Configuration", "iAutoClean_YDivision_Tray",                    W906_St02_LotInfo_iAutoClean[46]);
            WriteIniData(str1, "Configuration", "cAutoClean_PackageTray",                       W906_St02_LotInfo_iAutoClean[47]);
            WriteIniData(str1, "Configuration", "iAutoClean_IntervalContact",                   W906_St02_LotInfo_iAutoClean[48]);
            WriteIniData(str1, "Configuration", "iAutoClean_DeveicePices",                      W906_St02_LotInfo_iAutoClean[49]);
            WriteIniData(str1, "Configuration", "iCleanIndexOtherArm",                          W906_St02_LotInfo_iAutoClean[50]);
            WriteIniData(str1, "Configuration", "iAutoClean_SelectArm",                         W906_St02_LotInfo_iAutoClean[51]);
            WriteIniData(str1, "Configuration", "bAutoClean_FailAlarmLowYield",                 W906_St02_LotInfo_iAutoClean[52]);
            WriteIniData(str1, "Configuration", "iAutoClean_LowYieldLimit",                     W906_St02_LotInfo_iAutoClean[53]);
            WriteIniData(str1, "Configuration", "iAutoClean_LowYieldCount",                     W906_St02_LotInfo_iAutoClean[54]);
            WriteIniData(str1, "Configuration", "bAutoClean_FailAlarmSiteYieldDifferent",       W906_St02_LotInfo_iAutoClean[55]);
            WriteIniData(str1, "Configuration", "iAutoClean_FailAlarmSiteYield",                W906_St02_LotInfo_iAutoClean[56]);
            WriteIniData(str1, "Configuration", "iAutoClean_FailAlarmSiteYieldDifferentCount",  W906_St02_LotInfo_iAutoClean[57]);
            WriteIniData(str1, "Configuration", "bAutoClean_ConseFailureBySocket_Normal",       W906_St02_LotInfo_iAutoClean[58]);
            WriteIniData(str1, "Configuration", "iAutoClean_ConseFailureCountBySocket_Normal",  W906_St02_LotInfo_iAutoClean[59]);
            WriteIniData(str1, "Configuration", "bAutoClean_ConseFailureBySocket_Retest",       W906_St02_LotInfo_iAutoClean[60]);
            WriteIniData(str1, "Configuration", "iAutoClean_ConseFailureCountBySocket_Retest",  W906_St02_LotInfo_iAutoClean[61]);
            WriteIniData(str1, "Configuration", "bAutoClean_ConseFailureByHead_Normal",         W906_St02_LotInfo_iAutoClean[62]);
            WriteIniData(str1, "Configuration", "iAutoClean_ConseFailureCountByHead_Normal",    W906_St02_LotInfo_iAutoClean[63]);
            WriteIniData(str1, "Configuration", "bAutoClean_ConseFailureByHead_Retest",         W906_St02_LotInfo_iAutoClean[64]);
            WriteIniData(str1, "Configuration", "iAutoClean_ConseFailureCountByHead_Retest",    W906_St02_LotInfo_iAutoClean[65]);
            WriteIniData(str1, "Configuration", "bAutoClean_UseTray",                           W906_St02_LotInfo_iAutoClean[66]);
            WriteIniData(str1, "Configuration", "bAutoClean_UseNSKit",                          W906_St02_LotInfo_iAutoClean[67]);
            WriteIniData(str1, "Configuration", "iAutoCleanShuttle",                            W906_St02_LotInfo_iAutoClean[68]);
        }
    }

    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Cleaning Count", bNeedCover);                               //KenHsieh 20230518 : Auto Clean count不覆蓋
    if(bNeedCover==false)                                                       //KenHsieh 20230518 : Auto Clean count不覆蓋
    {
        AnsiString szDir=GetRecipeFileName("HandlerCondition.Data");            //JerryYang 20190703 auto clean清潔次數備份
        for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
        {
            for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
            {
                str.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
                WriteIniData(szDir, "Configuration", str, W906_St02_LotInfo_strCleanCnt[X][Y]);
            }
        }
    }

    //----------------------
    //把TestMode還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Test Mode", false);
    if(CUSTOMER_CODE==CC_TERAPOWER)
        bNeedCover=false;                                                       //Sam 20230110 : 晶兆成建勳要強制

    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Auto Clean use local setting");             //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sTestMode.Data", DataPath);
        str2.sprintf("%s%s\\TestMode.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sTestMode.Data", DataPath);                                  //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);

    //----------------------
    //把Binasgn還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Binasgn", false);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("Binasgn use local setting");                //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sBinasgn.Data", DataPath);
        str2.sprintf("%s%s\\Binasgn.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sBinasgn.Data", DataPath);                                   //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);

    //----------------------
    //把Binasgn還原
    //----------------------
    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "BinasgnOff", false);
    if(bNeedCover==false)                                                       //不覆蓋就要還原
    {
        W906_St02_LotInfo_WriteFTPSetupFileChangeLog("BinasgnOff use local setting");             //Sam 20210803 : FTP SetFile Change Log
        str1.sprintf("%sBinasgnOff.Data", DataPath);
        str2.sprintf("%s%s\\BinasgnOff.Data", DataPath, sDLFileName);
        ret=W906_KyecFtpCopyFile(str1.c_str(), str2.c_str(), false);
        MySleep(100);
    }

    //jou 2012-12-14 system改採用DeleteFile
    str1.sprintf("%sBinasgnOff.Data", DataPath);                                //刪除備份
    ::DeleteFileA(str1.c_str());
    MySleep(100);
    return ret;
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

// ===========================================================================
//  POOL-14 MR-B3 (AI(W906-P14) 20261010 (St02-E), RULINGS_20261010 #8): golden 913 TfLotInfo::btSaveSetupFileClick (:5254-5349) and
//  TfLotInfo::ClearAllSetupFile (:12231-12288).  [W906]: the form's btSaveSetupFile is not on the facade -> the St02 stand-in
//  W906_St02_LotInfo_btSaveSetupFileVisible (header); SHFileOperation runs with hwnd NULL (no owner form); fMain->LookForFile ->
//  W906_St02_Ftp.LookForFile (Q4; wb_serve = WebRecipeChange.cpp W906_RC_LookForFile, NULL = the facade's stub); fMain->ChangeSetUpFile
//  -> W906_St02_Ftp.ChangeSetUpFile (as F1's plLoadClick; NULL = not changed, noted); fMain->ShowTestHeadComp is the facade's empty
//  stub (forms/fMain.cpp:247; Q4: called as golden, human review).  ClearAllSetupFile's FindClose guard is Q3 (laptop-approved).
//  S25: the CC_TSMC_TAINAN tail of btSaveSetupFileClick (:5337-5348, online / normal switch) is gated and named.
// ===========================================================================
bool W906_St02_LotInfo_btSaveSetupFileVisible = false;                          // golden fLotInfo->btSaveSetupFile->Visible (header)

void W906_St02_LotInfo_btSaveSetupFileClick()
{
    SetCurrentDirectoryA("D://");                                               // :5256 golden SetCurrentDirectory(_T("D://"))
//  char str[256]="";
    char cStr1[256]="", cStr2[256]="";                                          //20111026 jou
    AnsiString OrgPath="", NewPath="", str1, str2;
    AnsiString SPath[2]={IncludeTrailingPathDelimiter(DataPath), IncludeTrailingPathDelimiter(OffsetPath)};
    SHFILEOPSTRUCTA oFile;                                                      // [W906] golden TfLotInfo member oFile

    str2=fMain->cbSetupFileName->Text;                                          // :5262
    str1=str2.SubString(1, str2.Length()-4);
    int SPath_length=sizeof(SPath)/sizeof(AnsiString);
    for(int i=0; i<SPath_length; i++)                                           //Jimmychiu 20230307 Replace constant with array length
    {
        if(IniConfig.bE45_AllSetupFileUseOneFile && i==1)                       //Steven 20190117 : 沒有要複製Offset
            continue;

        OrgPath=SPath[i];
        NewPath=OrgPath;
        OrgPath+=str2;
        NewPath+=str1;
        MyForceDirectories(NewPath);

        //------------------20111026    jou------------------
        OrgPath+="\\*.*";
        if(CUSTOMER_CODE!=CC_TSMC_TAINAN ||                                     // :5278
           ((CUSTOMER_CODE==CC_TSMC_TAINAN || IniConfig.bAMDFunction) && i!=1))                             //wei 20160726 TSMC offset不複製
        {
            strncpy(cStr1, OrgPath.c_str(), sizeof(cStr1));
            strncpy(cStr2, NewPath.c_str(), sizeof(cStr2));
            //複製工作檔
            ZeroMemory(&oFile, sizeof(SHFILEOPSTRUCTA));
            oFile.hwnd=NULL;                                                    // [W906] golden Handle
            oFile.wFunc=FO_COPY;
            oFile.pFrom=cStr1;
            oFile.pTo=cStr2;
            oFile.fFlags=FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
            SHFileOperationA(&oFile);                                           // :5290
            //---------------------------------------------------
        }

        if(i==2)                                                                //Steven 20101118 : 強制要加加  (golden quirk: never true, SPath has 2)
            i++;
    }
    MySleep(500);                                                               //landam

    W906_St02_LotInfo_ClearAllSetupFile(str1, str2);                            // :5299 Steven 20200512 : 刪除全部工作檔, 只留下當下的

    if(bNoSendSiteOnOff==true)                                                  // :5301
        EventReport(SECS_EVENT.SwitchSetupFile);

    bNoSendSiteOnOff=false;                                                     //wei 20160511 Send Site On Off
    //Ifor 20161221 (Steven) add KYEC 要求 Close Open Site By Setup File FTP 下載後不重置
    //==>
    if(CosFunction.bFTPDownLoadSiteBySetupFile==true)                           // :5307 Ifor 20171123 (Steven) : add FTP DownLoad Site By SetupFile
    {
        if(bDownloadFTP && bFTPDownLoadHasTestMode)
        {
            fMain->ShowTestHeadComp(false);
        }
        else
        {
            fMain->ShowTestHeadComp(true);
        }
    }
    else
    {
        fMain->ShowTestHeadComp(true);                                          //20140218  WEI
    }
    //<==
    //Ifor 20161221 add KYEC 要求 Close Open Site By Setup File FTP 下載後不重置
    W906_St02_LotInfo_btSaveSetupFileVisible=false;                             // :5324 golden btSaveSetupFile->Visible=false;

    if(ATC_SYSTEM==eATCHonPrecType)                                             // :5326 Ifor 20160101 工作檔更換重新連線ATC
    {
        if(ATCInterfaceForm->IsOnLine()==false)
            ATCInterfaceForm->OnLine();
        if(ATCInterfaceForm->ATC_SYS.IsChillerRun()==false)
            ATCInterfaceForm->ATCChillerSwitch(true);
        if(ATCInterfaceForm->ATC_SYS_PAL[0]->bATCRunSetting==false)
            ATCInterfaceForm->SetRunATC(true);
        bRunATC=true;                                                           //ChungHung 20160118 add for Hisi V102
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           // :5337 wei 20160621 FTPDOWNLOAD後切換Online和Normal
    {
        W906_St02_FtpGated("golden 913 uLotInfo.cpp:5339-5347 CC_TSMC_TAINAN: switch to ON_LINE / REALLY after the download (S25)");
    }
}

// ---------------------------------------------------------------------------
//  golden 913 TfLotInfo::ClearAllSetupFile :12231-12288 (Steven 20200512 : 刪除全部工作檔, 只留下當下的)
// ---------------------------------------------------------------------------
void W906_St02_LotInfo_ClearAllSetupFile(AnsiString sSetupFile, AnsiString sSetupFile_Net)
{
    WIN32_FIND_DATAA filedata;                                                  // Structure for file data
    // GOLDEN BUG (RULINGS_20261010 #8 Q3, a laptop-approved deviation): golden 913 :12234 leaves filehandle uninitialised and :12273
    //   FindClose(filehandle)s it after the loop -- with bKeepOnly1SetupFile false that closes a random handle, with it true it closes
    //   the handle :12268 already closed.  Here: INVALID_HANDLE_VALUE at the declaration, reset after :12268, and :12273 only closes a
    //   valid one.  GOLDEN_DEFECT_LEDGER row: requested through St02-M (the ledger is the laptop's).
    HANDLE filehandle=INVALID_HANDLE_VALUE;                                     // :12234 Handle for searching ([W906] Q3: initialised)
    AnsiString SPath[2]={IncludeTrailingPathDelimiter(DataPath), IncludeTrailingPathDelimiter(OffsetPath)};
    AnsiString SDataPath, sFileName;
    if(sSetupFile_Net!="")                                                      // :12237
        fBuilder->DeleteSetupFile(sSetupFile_Net);

    if(CosFunction.bKeepOnly1SetupFile)                                         // :12240 Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    {
        for(int i=0; i<2; i++)
        {
            if(DirectoryExists(SPath[i]))
            {
                SDataPath=SPath[i]+"*.*";
                filehandle=FindFirstFileA((SDataPath+"*").c_str(), &filedata);   //Steven 20101118 Start : 不要顯示資料夾以外的檔案
                if(filehandle!=INVALID_HANDLE_VALUE)
                {
                    do                                                          /* 不處理隱藏檔及 . 跟 .. */
                    {
                        if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                            strcmp(filedata.cFileName, ".")==0 ||
                            strcmp(filedata.cFileName, "..")==0)
                        {
                            continue;
                        }
                        else if(filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)                                   //如果該檔案為資料夾
                        {
                            sFileName=AnsiString(filedata.cFileName);
                            if(sFileName!=sSetupFile)
                            {
                                sFileName=SPath[i]+sFileName;
                                DeleteDirectory(sFileName);
                            }
                        }
                    } while(FindNextFileA(filehandle, &filedata));
                    FindClose(filehandle);                                      // :12268
                    filehandle=INVALID_HANDLE_VALUE;                            // [W906] Q3
                }
            }
        }
    }
    if(filehandle!=INVALID_HANDLE_VALUE)                                        // [W906] Q3 guard
        FindClose(filehandle);                                                  // :12273 Jimmychiu 20220901 釋放記憶體

    if(sSetupFile==fMain->cbSetupFileName->Text)                                // :12275 Steven 20220818 : Fixed for bD31RTCChangeRecipeNeedreCreateModel
        fSetup->bFirstTime=true;
    else
        fSetup->bFirstTime=false;

    RecordProcess("Clear All Setup File");                                      // :12280
    fMain->cbSetupFileName->Clear();
    if(W906_St02_Ftp.LookForFile) W906_St02_Ftp.LookForFile(); else fMain->LookForFile();   // :12282 golden fMain->LookForFile() (Q4)
    fMain->cbSetupFileName->Text=sSetupFile;
    WriteLastDataFN(sSetupFile);                                                // :12284 jou 2015-01-16 修正FTP download error
    if(W906_St02_Ftp.ChangeSetUpFile)                                           // :12285 golden fMain->ChangeSetUpFile(sSetupFile); Steven 20211013
        W906_St02_Ftp.ChangeSetUpFile(sSetupFile);
    else
        W906_St02_FtpNote("ChangeSetUpFile is not installed (W906_St02_InstallLotInfoFtp not called) -- the recipe is not changed");
    if(CosFunction.bLastSetInSetUpFile)                                         // :12286
        SetRunStartMode((eRunStartMode)LastSet.iRunStartMode, "");              //Steven 20211028 : 要重新設定模式,避免OnLine/OffLine Bin沒切換
}
