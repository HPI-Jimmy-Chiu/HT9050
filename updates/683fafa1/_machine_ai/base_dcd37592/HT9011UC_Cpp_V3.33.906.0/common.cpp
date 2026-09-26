// ===========================================================================
//  common.cpp  -- PARTIAL translation of BCB6 common.cpp
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\common.cpp
//
//  THIS BATCH (W3 TranslateHelpers): INI-helper family only.
//  All other common.cpp regions are gated with #if 0 // TODO(wave-N).
//
//  INCLUDES
//  --------
//  #include "vclcompat/vcl_compat.h" replaces the BCB6 #include <vcl.h> /
//  #include "MachineDefine.h" / #pragma hdrstop trio.
//
//  FAITHFULNESS NOTES
//  ------------------
//  * Signatures are VERBATIM copies of the BCB6 originals.
//  * __fastcall is neutralized to empty by vcl_compat.h.
//  * BCB6 dangling-pointer bugs in CloseIniFile / CloseIniFileMem are
//    FAITHFULLY preserved (INIFile / INIFileMem not set to NULL after delete;
//    see notes at each function).
//  * RecordProcess / RecordChangeLogProcess / TempChangeLog / InitialOK /
//    bSysLotStart / CosFunction / FormHS are all out-of-batch dependencies
//    and MUST NOT be called from code compiled in this wave.  In the
//    WriteIniData* bodies the change-log side-effect blocks are gated with
//    #if 0 // TODO(wave-logging/UI) and the RecordProcess open-fail paths
//    are also gated.  RecordProcess FORWARD STUB below provides a no-op so
//    common.cpp itself compiles as a standalone TU; the real RecordProcess
//    (from cMyDB) replaces it at link time in a later wave.
//  * TryStrToFloat: BCB6 SysUtils function; provided in vclcompat/SysUtils.h.
//  * CheckRange: BCB6 template in MachineType.h:1520 (T CheckRange(T,T,T)).
//    The ReadWriteIni overloads that use it are translated here; CheckRange
//    is already visible via MachineType.h.
// ===========================================================================

// ---------------------------------------------------------------------------
//  BCB6 preamble (common.cpp:1-16)
//  Was:  #include "MachineDefine.h"
//        #pragma hdrstop
//        #include "common.h"
//        #include "mymessbox.h"
//        ... (other BCB6-specific includes)
//        #pragma package(smart_init)
//  The BCB6 pragmas and VCL-specific includes are dropped; the compat umbrella
//  provides everything the in-batch body needs.
// ---------------------------------------------------------------------------
#include "vclcompat/vcl_compat.h"   // was: #include <vcl.h> / MachineDefine.h
#include "common.h"                 // common.h:4 (translated header)
#include "MachineType.h"            // common.cpp:8 -- for CheckRange template

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
//  Forward stub for RecordProcess (common.cpp:374,402,...).
//  cMyDB.h declares RecordProcess(AnsiString, AnsiString="").  That header
//  pulls untranslated BCB6 UI forms and is out of scope this wave.  Provide
//  a no-op here so this TU compiles standalone.
//
//  ⚠⚠ AI(W906-ST-LEDGER) 20260923 -- THE OLD VERSION OF THIS COMMENT WAS IMPOSSIBLE.
//      It said: "a weak no-op ... At final link time the real RecordProcess from
//      cMyDB.cpp resolves and overrides this stub."  It cannot.  `static` is
//      INTERNAL LINKAGE: every call below binds to this empty body at COMPILE time
//      and no linker anywhere can reach in and change that.  "weak" was wishful --
//      there is no __attribute__((weak)) here, and MSVC (this tree's second oracle)
//      has no portable equivalent, so weak is not the escape hatch either.
//
//      Two more things that comment got wrong:
//        * cMyDB.cpp's RecordProcess is NOT the body that would win.  It sits inside
//          `#if 0 // TODO(GA1-B4-integrate)` (cMyDB.cpp:1830) and cMyDB.cpp:1823-1828
//          says so itself: the live external body is canary_support.cpp:116, declared
//          canary_support.h:70.  (That same note cites ":102" and "canary_support.h:256"
//          -- both have drifted; measured 20260923.)
//        * So the TODO's exit condition ("once cMyDB is translated and linked") is
//          the wrong condition.  cMyDB is not what is missing.
//
//  ⇒ MEASURED CONSEQUENCE: all 41 RecordProcess calls in this TU (27 INI + 11
//    Exception + 2 directory) are permanently swallowed -- in EVERY target,
//    including wb_serve.  This TU is the INI read/write layer, so the failure
//    traces INBOX T4 is looking for ("a section or key name is misspelled and the
//    default is silently taken") cannot be observed from here at all.
//
//  ⇒ THE REAL FIX, AND WHY IT IS NOT DONE HERE (measured 20260923, do not redo):
//    Deleting this stub and including canary_support.h is correct for production
//    targets, but it BREAKS 11 test executables.  They link ht9045_core (which owns
//    this TU) WITHOUT ht9045_sm (which owns canary_support.cpp), so they would get an
//    undefined reference:
//        test_common, test_ini_helpers, test_ContactForce, test_MainCalcCore,
//        test_cContact, test_globals, test_cMyNUDN1, test_ptw1_textprocess,
//        test_w0tail_headers, test_SecsSvEcRegistration, test_SecsWireCodec
//    (method: transitive closure of target_link_libraries over CMakeLists.txt +
//     tests/CMakeLists.txt; 152 executables scanned, these 11 have core-without-sm.)
//    ⇒ The stub's REASON was always right ("so this TU compiles standalone").
//      Only its EXPLANATION was wrong.  Fixing it properly means giving those 11
//      targets the symbol -- which edits tests/CMakeLists.txt, a file both the other
//      laptop and Steven have been committing to, and needs a build to verify.
//      That is a coordinated change, not a drive-by.  Left as-is deliberately.
// ---------------------------------------------------------------------------
static void RecordProcess(AnsiString /*S*/, AnsiString /*S2*/ = AnsiString("")) {}

// ---------------------------------------------------------------------------
//  Forward stub for ShowMyMessage (golden mymessbox.h:58; called from
//  GetLastOpenFN common.cpp:1262/1266/1275 and MyForceDirectories :1709/1715).
//  mymessbox.h pulls untranslated BCB6 UI forms and is out of scope this wave.
//  ht9045_core (this library) must NOT depend on ht9045_sm -- ht9045_sm already
//  has a real sim stand-in for this in canary_support.h, but ht9045_sm
//  transitively depends on ht9045_core, so linking the other way would create
//  a CMake link cycle. Mirror the RecordProcess stub immediately above instead:
//  a local no-op with the same signature golden's call sites need.
//
//  AI(W906-CommonWaveFile) 20260721: added, mirroring the RecordProcess
//  forward-stub precedent above.
//  TODO(wave-UI): remove this stub once mymessbox / a real message-box shim is
//  translated and linked into ht9045_core (or ht9045_core no longer needs it).
// ---------------------------------------------------------------------------
static void ShowMyMessage(AnsiString /*S1*/, AnsiString /*S2*/ = AnsiString(""), AnsiString /*S3*/ = AnsiString(""), bool /*Ok*/ = false, bool /*bServoOff*/ = false) {}

//AI(W906-LOT-W1) 20260919: 同樣形狀的第二個重導 —— `AuthPath`。
//
// 為什麼現在才需要：那五個被洗掉的鍵全樹只有兩個寫者——
// `TfLotInfo::SetLotID`（forms/fLotInfo.cpp:5385/:5395）與
// `::SetLotStart`（:5404）——而它們到今天早上都還是空樁
// （舊位置 forms/fLotInfo.cpp:716-717）。P0-2 換上真本體之後，
// **20260919 的 gate 跑完就把機台真實的 `D:\HT9045\config\config.ini`
// 洗掉了**（Lot ID=1 -> 空、Operator=1 -> 空、時間戳變 9999）。
// 已用 tools/realfile_guard.py 的備份還原。
//
// ⚠ 我第一次的歸因是錯的：我寫 `tests/test_auto9045.cpp:293`，實測
//   把那支測試單獨建起來、手動帶 `W906_AUTH_PATH` 指向一個空目錄跑，
//   **一個字都沒寫**——因為離線鏈在 auto9045.cpp:1479 就 return 90，
//   距離 :1484 的 SetLotID 還差五行。真正的寫者在其餘 8 個呼叫點裡。
//   圓堡不依賴「是誰」：它架在路徑上，不是架在呼叫者上。
//
// ⚠ 這正是 memory `v906-ctest-writes-production-machine-data` 說的那件事，
//   而它也說了「圍堵要兩層」—— 下面 InitCommonString 那一處同樣要補，
//   理由與 W906IniDataRedirect 上方那則註解一字不差：
//   **只補一處會被另一處靜靜蓋掉**。
//
// production 行為零改變：環境變數沒設就回 golden 的字面值。
static AnsiString W906AuthPathRedirect(const char* goldenLiteral)
{
    const char* env = getenv("W906_AUTH_PATH");
    if (env == 0 || *env == '\0')
        return AnsiString(goldenLiteral);                 // production: verbatim golden
    return AnsiString(env);
}
static AnsiString W906EnvPathOr(const char* envName, const char* goldenLiteral) { const char* e = getenv(envName); return (e != 0 && *e != 0) ? AnsiString(e) : AnsiString(goldenLiteral); }   //AI(W906-IOWEB-P11) 20260924: same shape as W906AuthPathRedirect -- UNSET => the golden literal byte for byte (production zero shift). Used for asGeneralPath / LastDataPath at BOTH sites (decl + InitCommonString), because a seam on one site is silently overwritten by the other. On the old blank line, so nothing below moves.
// ---------------------------------------------------------------------------
//  Global path strings (common.cpp:18-174)
//  Big5 source comments are preserved as-is (raw bytes; ASCII portions only
//  shown where needed for provenance).
//
//  NOTE: asKeyStr is file-local in the BCB6 source (no extern in common.h);
//  kept static here so it does not clash with a future cMyDB link unit.
// ---------------------------------------------------------------------------
AnsiString asGeneralPath            =W906EnvPathOr("W906_GENERAL_INI_PATH", "D:\\HT9045\\system\\Gerneral.ini");             // common.cpp:18   //AI(W906-IOWEB-P11) 20260924: seam, see :146
AnsiString asBarCodeLogPath         ="D:\\HT9045_Log\\2DBarCode";                    // common.cpp:19
AnsiString asBarCodeLot             ="D:\\HT9045_Log\\2DBarCode\\LotData.txt";       // common.cpp:20
AnsiString asTrayIDByLot            ="D:\\HT9045_Log\\2DBarCode\\TrayIDByLot.txt";   // common.cpp:21
AnsiString asBarCodeCommLogPath     ="D:\\HT9045_Log\\2DBarCodeComm";                // common.cpp:22
AnsiString asTrayMapCommLogPath     ="D:\\HT9045_Log\\TrayMapComm";                  // common.cpp:23
AnsiString asKeyenceCommLogPath     ="D:\\HT9045_Log\\KeyenceComm";                  // common.cpp:24
AnsiString asTrayMapLogPath         ="D:\\HT9045_Log\\TrayMapLog";                   // common.cpp:25
AnsiString asTrayMapDataPath        ="D:\\HT9045_Log\\TrayMapLog\\TrayData";         // common.cpp:26
AnsiString asSummaryPath            ="D:\\HT9045_Log\\Summary";                      // common.cpp:27
AnsiString asFix2AICCDDataPath      ="D:\\HT9045_Log\\Fix2AICCDLog\\Fix2AICCDData";  // common.cpp:28
AnsiString asSystemPath             ="d:\\HT9045\\system\\";                         // common.cpp:29
AnsiString asDBPath                 ="D:\\HT9045\\MDB\\Handler.db3";                 // common.cpp:30
AnsiString AuthPath                 =W906AuthPathRedirect("D:\\HT9045\\config\\");                         // common.cpp:31
// ===========================================================================
//  AI(W906-BA-BU0g) 20260911: IniData containment seam, migrated from the
//  D:\HT9050 tree (its AI(W906-BU-0g) 20260908). A never received it, and on
//  20260911 A reproduced the incident -- worse than the original.
//
//  MEASURED, not precautionary. A's ba0 baseline gate wrote three files in the
//  machine's own live recipe tree for the ACTIVE work order
//  D169_FCBGA1097_23X23_1X2_FT1_25_V05:
//      IniData\Data\D169_...\HandlerCondition.Data   17:42
//      IniData\Data\D169_...\Temperature.Data        17:44
//      IniData\Data\Tester.Data                      17:44
//  and the changes were REAL VALUES, not reformatting:
//      iIndexArmAutoCleanCnt          4062 -> 0
//      iAutoCleanPad_CountTime_0_0/1     1 -> 0
//      iAutoClean_IndexTime              (added) = 42
//      Temperature                   125.0 -> 25.0000
//  HT9045.exe was not running at the time, so the handler did not do it.
//  The other tree's 20260908 incident names the identical keys as "the
//  AutoClean settings save path"; its version merely created a new
//  "Fail Open" folder, while A's landed on the recipe in use.
//
//  EVERY TEST THAT DID THIS PASSED. That is the danger: a passing test is the
//  most dangerous writer there is, because nothing ever suspects it.
//
//  BOTH SITES MUST CARRY THE SEAM -- here and InitCommonString()'s
//  re-assignments below. Patching one alone is silently undone by the other,
//  and which one is in force depends on whether InitCommonString has run yet.
//
//  ★ WITH W906_INIDATA_ROOT UNSET THIS RETURNS THE GOLDEN LITERAL BYTE FOR
//  BYTE -- including DIOCFGPath's lowercase "iniData" (golden common.cpp:35),
//  a golden quirk that Windows' case-insensitive filesystem hides. That is why
//  this is a PREFIX SUBSTITUTION on each literal and not a root+tail rebuild:
//  rebuilding would silently normalise the quirk away. Production is
//  unaffected; only ctest sets the variable (tests/CMakeLists.txt).
// ===========================================================================
static AnsiString W906IniDataRedirect(const char* goldenLiteral)
{
    const char* env = getenv("W906_INIDATA_ROOT");
    if (env == 0 || *env == '\0')
        return AnsiString(goldenLiteral);                 // production: verbatim golden

    static const char kGoldenRoot[] = "D:\\HT9045\\IniData";
    const int n = (int)(sizeof(kGoldenRoot) - 1);
    for (int i = 0; i < n; ++i)
    {
        char a = goldenLiteral[i];
        char b = kGoldenRoot[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b)
            return AnsiString(goldenLiteral);             // not under the IniData root
    }
    return AnsiString(env) + AnsiString(goldenLiteral + n);
}

AnsiString DefaultPath              =W906IniDataRedirect("D:\\HT9045\\IniData\\");                        // common.cpp:32
AnsiString DataPath                 =W906IniDataRedirect("D:\\HT9045\\IniData\\Data\\");                  // common.cpp:33
AnsiString OffsetPath               =W906IniDataRedirect("D:\\HT9045\\IniData\\Offset\\");                // common.cpp:34
AnsiString DIOCFGPath               =W906IniDataRedirect("D:\\HT9045\\iniData\\DioCfg\\");               // common.cpp:35
AnsiString asErrNotePath            ="D:\\HT9045\\system\\SpecialErrNote.ini";        // common.cpp:36
AnsiString ConfigMemoPath           ="D:\\HT9045\\config\\Description.ini";          // common.cpp:37
AnsiString LastDataPath             =W906EnvPathOr("W906_SETUPINF_PATH", "D:\\HT9045\\SetUp.inf");                        // common.cpp:38   //AI(W906-IOWEB-P11) 20260924: seam, see :146
AnsiString BmpPath                  ="D:\\HT9045\\IMG\\BMP\\";                       // common.cpp:39
AnsiString IoTablePath              ="D:\\HT9045\\System\\IO_Table.csv";             // common.cpp:40
AnsiString MotTablePath             ="D:\\HT9045\\System\\Mot_Table.csv";            // common.cpp:41
AnsiString TrayTablePath            ="D:\\HT9045\\System\\TrayForm.csv";             // common.cpp:42
AnsiString PlateTablePath           ="D:\\HT9045\\System\\PlateForm.csv";            // common.cpp:43
AnsiString GrapicPath               ="D:\\HT9045\\IMG\\Graphic\\";                   // common.cpp:44
AnsiString SecsGemPath              ="D:\\HT9045\\SECS\\SECS\\SYSTEM\\Gerneral.ini"; // common.cpp:45
static AnsiString asKeyStr          ="HontechPassword";                              // common.cpp:46 (file-local)
AnsiString as9045UPH                ="D:\\HT9045_Log\\UPH";                          // common.cpp:47
AnsiString as9045LogPath            ="D:\\HT9045_Log";                               // common.cpp:48
AnsiString asEPLogPath              ="D:\\HT9045_Log\\EP";                           // common.cpp:49
AnsiString asHiSiLogPath            ="D:\\HT9045_Log\\Temperature";                  // common.cpp:50
AnsiString asLbTempLogPath          ="D:\\HT9045_Log\\LB_Temperature";               // common.cpp:51
AnsiString asShtLogPath             ="D:\\HT9045_Log\\ShuttleLog";                   // common.cpp:52
AnsiString asHomeLogPath            ="D:\\HT9045_Log\\HomeLog";                      // common.cpp:53
AnsiString asTorqLogPath            ="D:\\HT9045_Log\\TorqueLog";                    // common.cpp:54
AnsiString asACSmartIntervalLogPath ="D:\\HT9045_Log\\ACSmartIntervalLog";           // common.cpp:55
AnsiString asTrayLogPath            ="D:\\HT9045_Log\\EditTray";                     // common.cpp:56
AnsiString asTempLogPath            ="D:\\HT9045_Log\\Temp";                         // common.cpp:57
AnsiString asEJ1NLogPath            ="D:\\HT9045_Log\\EJ1N_Log";                     // common.cpp:58
AnsiString asLaserLogPath           ="D:\\HT9045_Log\\LaserLog";                     // common.cpp:59
AnsiString asASELogPath             ="D:\\SaveRecord";                               // common.cpp:60
AnsiString asTravelingLogPath       =(getenv("W906_PRODLOG_ROOT") && *getenv("W906_PRODLOG_ROOT")) ? AnsiString(getenv("W906_PRODLOG_ROOT")) : AnsiString("D:\\HT9045_Log\\Production_Log");               // common.cpp:61 -- AI(W906-W2-PRODLOG) 20260926: ctest 接縫（只有 tests/CMakeLists.txt 設 W906_PRODLOG_ROOT；沒設＝golden 字面，量產不變），G-5 解開後 SaveRecord 會真的寫這裡
AnsiString asOLPLogPath             ="D:\\HT9045_Log\\Automation";                   // common.cpp:62
AnsiString asBarCodeLogPath1        ="D:\\HT9045_Log\\BarCode_LOG";                  // common.cpp:65
AnsiString asHeaterOnLogPath        ="D:\\HT9045_Log\\Heater_On_Off_LOG";            // common.cpp:66
AnsiString asYieldRecordPath        ="D:\\HT9045_Log\\Yield";                        // common.cpp:67
AnsiString asESDLogPath             ="D:\\HT9045_Log\\ESD";                          // common.cpp:68
AnsiString asAlarmLogXmlPath        ="D:\\HT9045_Log\\AlarmLogXml";                  // common.cpp:69
AnsiString asPadCommLogPath         ="D:\\HT9045_Log\\PadCommLog";                   // common.cpp:71
AnsiString asUDPLogPath             ="D:\\HT9045_Log\\UDP";                          // common.cpp:72
AnsiString asVibrateMotCommLogPath  ="D:\\HT9045_Log\\VibrateMotorCommLog";          // common.cpp:73
AnsiString asProductionLogPath      ="D:\\RMS";                                      // common.cpp:74
AnsiString asPMAlarmPath            ="D:\\HT9045\\PMAlarm";                          // common.cpp:75
AnsiString asProductDataPath        ="D:\\HT9045_Log\\ProductData\\";                // common.cpp:76
AnsiString asProductRecordPath      ="D:\\HT9045_Log\\ProductRecord";                // common.cpp:77
AnsiString asSocketIDLogPath        ="D:\\HT9045_Log\\SocketIDLog\\";                // common.cpp:78
AnsiString asQtyDataPath            ="D:\\HT9045_Log\\QtyData\\";                    // common.cpp:79
AnsiString sPMList_Month            ="D:\\HT9045\\PMAlarm\\PM_Month.ini";            // common.cpp:80
AnsiString sPMList_Quarter          ="D:\\HT9045\\PMAlarm\\PM_Quarter.ini";          // common.cpp:81
AnsiString sPMList_Year             ="D:\\HT9045\\PMAlarm\\PM_Year.ini";             // common.cpp:82
AnsiString sPMList_Temperature      ="D:\\HT9045\\PMAlarm\\PM_Temperature.ini";      // common.cpp:83
AnsiString sPMList_ESD              ="D:\\HT9045\\PMAlarm\\PM_ESD.ini";              // common.cpp:84
AnsiString sPMList_IonFan           ="D:\\HT9045\\PMAlarm\\PM_IonFan.ini";           // common.cpp:85
AnsiString sPMItem_List             ="D:\\HT9045\\PMAlarm\\PM_List.ini";             // common.cpp:86
AnsiString sPMItem_Check            ="D:\\HT9045\\PMAlarm\\PM_Check.ini";            // common.cpp:87
AnsiString sPMItem_ListTemperature  ="D:\\HT9045\\PMAlarm\\PM_ListTemperature.ini";  // common.cpp:88
AnsiString sPMItem_CheckTemperature ="D:\\HT9045\\PMAlarm\\PM_CheckTemperature.ini"; // common.cpp:89
AnsiString sPMItem_ListESD          ="D:\\HT9045\\PMAlarm\\PM_ListESD.ini";          // common.cpp:90
AnsiString sPMItem_CheckESD         ="D:\\HT9045\\PMAlarm\\PM_CheckESD.ini";         // common.cpp:91
AnsiString sPMItem_ListIonFan       ="D:\\HT9045\\PMAlarm\\PM_ListIonFan.ini";       // common.cpp:92
AnsiString sPMItem_CheckIonFan      ="D:\\HT9045\\PMAlarm\\PM_CheckIonFan.ini";      // common.cpp:93
AnsiString sPMSetting               ="D:\\HT9045\\PMAlarm\\PM_Setting.ini";          // common.cpp:94
AnsiString JOBFILEDataPath          ="JOBFILE";                                      // common.cpp:95
AnsiString JOBFILEOffsetPath        ="JOBFILE";                                      // common.cpp:96
AnsiString sBarcodeFile             ="D:\\HT9045\\Barcode_File\\";                   // common.cpp:97
AnsiString asBarCodeDownLot         ="D:\\HT9045_Log\\2DBarCode\\LotCheckData.txt";  // common.cpp:98
AnsiString asSearch2DIDByLot        ="D:\\HT9045_Log\\2DBarCode\\Search2DIDByLot.txt"; // common.cpp:99
AnsiString asSortingBy2DID          ="D:\\HT9045_Log\\2DBarCode\\SortingBy2DID.txt"; // common.cpp:100
AnsiString as2DIDSortToError        ="D:\\HT9045_Log\\2DBarCode\\2DID_list.txt";     // common.cpp:101
AnsiString asIndexZphasePath        ="D:\\HT9045_Log\\Zphase";                       // common.cpp:102
AnsiString asARSMParaPath           ="D:\\HT9045\\system\\ARMS.ini";                 // common.cpp:103
AnsiString sProductionInfoFilePath  ="D:\\HT9045_log\\ProductionInfo";               // common.cpp:104
AnsiString asLightScalePath         ="D:\\HT9045_Log\\LightScale\\";                 // common.cpp:105
AnsiString asTCPIPPath              ="D:\\HT9045_Log\\TCPIP_Log";                    // common.cpp:106
AnsiString asRenesasCommPath        ="D:\\HT9045_Log\\RenesasComm_Log";              // common.cpp:107
AnsiString asRenesasTaskPath        ="D:\\HT9045_Log\\RenesasTask_Log";              // common.cpp:108
AnsiString aESDSetDataFileName      ="D:\\ESD_Program\\system\\General.ini";         // common.cpp:109
AnsiString asGalilCmdPath           ="D:\\HT9045_Log\\Galil_Log";                    // common.cpp:110
AnsiString asASECLFTPPath           ="D:\\ASECLFTP";                                 // common.cpp:111
AnsiString asChipMosFTPPath         ="D:\\HT9045_Log\\FTP";                          // common.cpp:112
AnsiString asChipMosFTPPath2        ="D:\\HT9045_Log\\ChipMosFTP";                   // common.cpp:113
AnsiString asSaveEventLogPath       ="D:\\HT9045_Log\\SaveEventLog";                 // common.cpp:114
AnsiString asOCRSaveMane            ="D:\\HT9045_Log\\OCR\\Alarm\\Bad";              // common.cpp:115
AnsiString asTeachPath              =W906EnvPathOr("W906_TEACH_INI_PATH", "D:\\HT9045\\system\\teach.ini");                // common.cpp:117   //AI(W906-IOWEB-P13) 20260925: seam (see :146) -- measured: wb_serve boot seeds seteditContactZ1/Z2Relative into this file
AnsiString asHandlePath             ="D:\\HT9045_Log\\Handlelog";                    // common.cpp:118
AnsiString sJamRatePath             ="D:\\HT9045_Log\\JamRate";                      // common.cpp:119
AnsiString sDailyJamPath            ="D:\\HT9045_Log\\JamRate_Daily";                // common.cpp:120
AnsiString sFTPSetupFileLogPath     ="D:\\HT9045_Log\\FTPSetupFileChangeLog";        // common.cpp:121
AnsiString sSaveByMachine           ="D:\\HT9045\\IniData\\SaveByMachine\\";         // common.cpp:122
AnsiString asBarCodeInspReportPath  ="D:\\HT9045_Log\\2DIDParameterReport";          // common.cpp:123
AnsiString as3SigmaLogPath          ="D:\\HT9045_log\\TemperatureLog_3Sigma";        // common.cpp:125
AnsiString asTrayStepSpeedByMachinePatch ="D:\\HT9045\\system\\TrayStepSpeed.ini";   // common.cpp:126
AnsiString asProduct_LoaderPath     ="D:\\HT9045_Log\\Production_Loader";            // common.cpp:127
AnsiString asVacuumOnOffLogPath     ="D:\\HT9045_log\\VacuumOnOffLog";               // common.cpp:128
AnsiString asHPCardPath             ="D:\\HT9045_log\\HPCARD";                       // common.cpp:129
AnsiString asDefaultRecipeChangeLogPath ="D:\\HT9045_Log\\DefaultRecipeChangeLog";   // common.cpp:130
AnsiString asTestTCPIPLogPath       ="D:\\HT9045_Log\\Test_TCPIP";                   // common.cpp:131
AnsiString sGroundESDLogPath        ="D:\\HT9045_Log\\GroundESDLog";                 // common.cpp:132
AnsiString aAutoTempureOffsetFTP    ="D:\\HT9045_Log\\AutoTempureOffsetFTP";         // common.cpp:133
AnsiString aAutoTempureOffsetFTPBackup ="D:\\HT9045_Log\\AutoTempureOffsetFTPBackup"; // common.cpp:134
AnsiString aDownloadUpdateAutomaticallyPath ="D:\\HT9045_Log\\Installer";            // common.cpp:135
AnsiString asGroundManPath          ="D:\\HT9045_log\\GroundManLog\\GroundManValue";  // common.cpp:136
AnsiString asShtSenLogPath          ="D:\\HT9045_Log\\ShuttleSensorLog\\";           // common.cpp:137
AnsiString asInArmAutoAlignmentPath ="D:\\HT9045_log\\AOA_Log\\InArm";               // common.cpp:138
AnsiString asOutArmAutoAlignmentPath ="D:\\HT9045_log\\AOA_Log\\OutArm";             // common.cpp:139
AnsiString abHandlerStateChangeUploadServerPath ="D:\\HT9045_Log\\HandlerStateChangeUploadServer"; // common.cpp:140
AnsiString asCleanPadLogPath        =(getenv("W906_CLEANPADLOG_ROOT") && *getenv("W906_CLEANPADLOG_ROOT")) ? AnsiString(getenv("W906_CLEANPADLOG_ROOT")) : AnsiString("D:\\HT9045_Log\\CleanPad_Log");                 // common.cpp:141 -- AI(W906-W2-CLEANPADLOG) 20260926: ctest 接縫（只有 tests/CMakeLists.txt 設；沒設＝golden 字面），G-6 解開後 SaveRecordCleanPad 會真的寫這裡
AnsiString asBackup2DSortListPath   ="D:\\HT9045_log\\2D_SortList_Backup\\";         // common.cpp:142
AnsiString asOCRDownLoadLotPath     ="D:\\HT9045_Log\\OCR\\CompareFile";             // common.cpp:143
AnsiString asOCRLotPath             ="D:\\HT9045_Log\\OCR\\OCRLot.txt";              // common.cpp:144
AnsiString asProductionByFilePath   ="D:\\HT9045_Log\\Production_ByFile";            // common.cpp:145
AnsiString asTesterReportPath       ="C:\\HontechTestLogs";                          // common.cpp:146
AnsiString asMachineLifePath        ="D:\\HT9045\\system\\MachineLife.ini";          // common.cpp:147

// INI singleton globals (common.cpp:148-151)
TIniFile    *INIFileGeneral = nullptr;   // common.cpp:148
TIniFile    *INIFile        = nullptr;   // common.cpp:149
TMemIniFile *INIFileMem     = nullptr;   // common.cpp:151

AnsiString asCheckIndexYLogPath     ="D:\\HT9045_log\\CheckIndexYLog";               // common.cpp:153
AnsiString asBackup2DSummaryPath    ="D:\\HT9045_log\\Summary_Backup\\";             // common.cpp:154
AnsiString asDupBundleID            ="D:\\HT9045_Log\\BundleID\\DupBundleID.txt";    // common.cpp:155
AnsiString aslDupUnloadBundlID      ="D:\\HT9045_Log\\BundleID\\DupUnloadBundleID.txt"; // common.cpp:156
AnsiString as2DWhiteListLog         ="D:\\HT9045_Log\\2DBarCode\\CheckResult\\";     // common.cpp:157
AnsiString as2DWhiteListLogName     ="";                                             // common.cpp:158
AnsiString SVNRevision              ="";                                             // common.cpp:159
AnsiString asSWVersionPath          ="D:\\HT9045_log\\SW_Version\\";                 // common.cpp:160
AnsiString asJamStatPath            ="D:\\HT9045_Log\\JamStatList";                  // common.cpp:164
AnsiString asPowerSavePath          ="D:\\HT9045_Log\\Power_EnergySaving";           // common.cpp:165
AnsiString asATCPowerSavePath       ="D:\\HT9045_Log\\ATC_EnergySaving";             // common.cpp:166
AnsiString asTempLog_HS             ="D:\\HandlerLog\\Temperature_Log\\";            // common.cpp:167
AnsiString asEPLog_HS               ="D:\\HandlerLog\\EP_Log\\";                     // common.cpp:168
AnsiString asESDLog_HS              ="D:\\HandlerLog\\ESD_Log\\";                    // common.cpp:169
AnsiString asATCEventLog_HS         ="D:\\HandlerLog\\ATCEven_Log\\";               // common.cpp:170
AnsiString asArmTestLog_HS          ="D:\\HandlerLog\\ArmTestInfo\\";                // common.cpp:171
AnsiString sGroundESDInterValPath   ="D:\\HT9045_Log\\RecordGroundESDInterval";      // common.cpp:172
AnsiString asATCFileTransferPath    ="D:\\ATC\\Data\\SaveFile\\";                    // common.cpp:173
AnsiString sWhite2DIDListLoc        ="D:\\HT9045_Log\\White2DIDListTmp\\";           // common.cpp:174

// ===========================================================================
//  InitCommonString (common.cpp:177-265)
//  jou 2016-08-24 : Initial Common String
//  AI(W906-CommonCompletion) 20260721: un-gated -- reassigns ~90 already-
//  declared AnsiString path globals to runtime defaults.
//
//  VERIFIED DEVIATION from this wave's plan text (which assumed, subject to
//  verification, that every reassignment here is identical to its
//  declaration-site default above): diffed golden's declaration block
//  against golden's InitCommonString body line-by-line (not merely assumed)
//  and found THREE real discrepancies, all preserved verbatim below (golden's
//  own actual behavior -- NOT "fixed" to match the declaration-site value):
//    * SecsGemPath: declared "D:\\HT9045\\SECS\\SECS\\SYSTEM\\Gerneral.ini"
//      (common.cpp:116, double "SECS") but reassigned here to the
//      single-SECS "D:\\HT9045\\SECS\\SYSTEM\\Gerneral.ini" (golden
//      common.cpp:200).
//    * asBarCodeDownLot: declared "...\\2DBarCode\\LotCheckData.txt"
//      (common.cpp:166) but reassigned here to "...\\2DBarCode\\LotData1.txt"
//      (golden common.cpp:243) -- a different filename.
//    * asSortingBy2DID: declared as a FILE path
//      "...\\2DBarCode\\SortingBy2DID.txt" (common.cpp:168) but reassigned
//      here to the DIRECTORY "...\\2DBarCode\\" (golden common.cpp:245).
//  Net effect: which value is actually in force at runtime depends on
//  whether InitCommonString has been called yet -- a real (if latent) golden
//  quirk, not a translation error.
//
//  asBackup2DSortListPath is also assigned TWICE to the identical value
//  (golden common.cpp:253 and :257, under two different attribution
//  comments) -- a harmless duplicate, preserved as-is rather than deduped.
// ===========================================================================
void InitCommonString()                                                      // common.cpp:177
{
    asGeneralPath        =W906EnvPathOr("W906_GENERAL_INI_PATH", "d:\\HT9045\\system\\Gerneral.ini");   //AI(W906-IOWEB-P11) 20260924: seam (golden lowercase d: kept)
    asBarCodeLogPath     ="D:\\HT9045_Log\\2DBarCode";
    asBarCodeLot         ="D:\\HT9045_Log\\2DBarCode\\LotData.txt";
    asBarCodeCommLogPath ="D:\\HT9045_Log\\2DBarCodeComm";
    asTrayMapCommLogPath ="D:\\HT9045_Log\\TrayMapComm";
    asKeyenceCommLogPath ="D:\\HT9045_Log\\KeyenceComm";
    asTrayMapLogPath     ="D:\\HT9045_Log\\TrayMapLog";
    asTrayMapDataPath    ="D:\\HT9045_Log\\TrayMapLog\\TrayData";
    asFix2AICCDDataPath  ="D:\\HT9045_Log\\Fix2AICCDLog\\Fix2AICCDData";
    asSystemPath         ="d:\\HT9045\\system\\";
    asDBPath             ="D:\\HT9045\\MDB\\Handler.db3";
    //AI(W906-LOT-W1) 20260919: 第二層 —— 與上面宣告處成對，理由見
    //  W906AuthPathRedirect 的註解（只補一處會被這裡蓋掉）。
    AuthPath             =W906AuthPathRedirect("D:\\HT9045\\config\\");
    //  AI(W906-BA-BU0g) 20260911: the second half of the IniData containment
    //  seam. The decl-site initialisers above already call
    //  W906IniDataRedirect, but InitCommonString re-assigns the same four
    //  globals with plain literals, so whichever runs last wins -- and which
    //  that is depends on whether InitCommonString has been called yet.
    //  Patching one site alone is silently undone by the other.
    DefaultPath          =W906IniDataRedirect("D:\\HT9045\\IniData\\");
    DataPath             =W906IniDataRedirect("D:\\HT9045\\IniData\\Data\\");
    OffsetPath           =W906IniDataRedirect("D:\\HT9045\\IniData\\Offset\\");
    DIOCFGPath           =W906IniDataRedirect("D:\\HT9045\\iniData\\DioCfg\\");
    asErrNotePath        ="D:\\HT9045\\system\\SpecialErrNote.ini";
    ConfigMemoPath       ="D:\\HT9045\\config\\Description.ini";
    LastDataPath         =W906EnvPathOr("W906_SETUPINF_PATH", "D:\\HT9045\\SetUp.inf");   //AI(W906-IOWEB-P11) 20260924: seam
    BmpPath              ="D:\\HT9045\\IMG\\BMP\\";
    GrapicPath           ="D:\\HT9045\\IMG\\Graphic\\";
    SecsGemPath          ="D:\\HT9045\\SECS\\SYSTEM\\Gerneral.ini";           // DEVIATION from decl-site default -- see banner above
    asKeyStr             ="HontechPassword";
    as9045UPH            ="D:\\HT9045_Log\\UPH";
    as9045LogPath        ="D:\\HT9045_Log";
    asEPLogPath          ="D:\\HT9045_Log\\EP";
    asHiSiLogPath        ="D:\\HT9045_Log\\Temperature";
    asShtLogPath         ="D:\\HT9045_Log\\ShuttleLog";
    asHomeLogPath        ="D:\\HT9045_Log\\HomeLog";
    asTorqLogPath        ="D:\\HT9045_Log\\TorqueLog";
    asACSmartIntervalLogPath ="D:\\HT9045_Log\\ACSmartIntervalLog";
    asTrayLogPath        ="D:\\HT9045_Log\\EditTray";
    asTempLogPath        ="D:\\HT9045_Log\\Temp";
    asEJ1NLogPath        ="D:\\HT9045_Log\\EJ1N_Log";
    asLaserLogPath       ="D:\\HT9045_Log\\LaserLog";
    asASELogPath         ="D:\\SaveRecord";
    asTravelingLogPath   =(getenv("W906_PRODLOG_ROOT") && *getenv("W906_PRODLOG_ROOT")) ? AnsiString(getenv("W906_PRODLOG_ROOT")) : AnsiString("D:\\HT9045_Log\\Production_Log");   // AI(W906-W2-PRODLOG) 20260926: 同上（InitCommonString 會重設一次）
    asOLPLogPath         ="D:\\HT9045_Log\\Automation";
    asBarCodeLogPath1    ="D:\\HT9045_Log\\BarCode_LOG";
    asHeaterOnLogPath    ="D:\\HT9045_Log\\Heater_On_Off_LOG";
    asYieldRecordPath    ="D:\\HT9045_Log\\Yield";
    asProductionLogPath  ="D:\\RMS";
    asPMAlarmPath        ="D:\\HT9045\\PMAlarm";
    asProductDataPath    ="D:\\HT9045_Log\\ProductData\\";
    sPMList_Month            ="D:\\HT9045\\PMAlarm\\PM_Month.ini";
    sPMList_Quarter          ="D:\\HT9045\\PMAlarm\\PM_Quarter.ini";
    sPMList_Year             ="D:\\HT9045\\PMAlarm\\PM_Year.ini";
    sPMList_Temperature      ="D:\\HT9045\\PMAlarm\\PM_Temperature.ini";
    sPMList_ESD              ="D:\\HT9045\\PMAlarm\\PM_ESD.ini";
    sPMList_IonFan           ="D:\\HT9045\\PMAlarm\\PM_IonFan.ini";
    sPMItem_List             ="D:\\HT9045\\PMAlarm\\PM_List.ini";
    sPMItem_Check            ="D:\\HT9045\\PMAlarm\\PM_Check.ini";
    sPMItem_ListTemperature  ="D:\\HT9045\\PMAlarm\\PM_ListTemperature.ini";
    sPMItem_CheckTemperature ="D:\\HT9045\\PMAlarm\\PM_CheckTemperature.ini";
    sPMItem_ListESD          ="D:\\HT9045\\PMAlarm\\PM_ListESD.ini";
    sPMItem_CheckESD         ="D:\\HT9045\\PMAlarm\\PM_CheckESD.ini";
    sPMItem_ListIonFan       ="D:\\HT9045\\PMAlarm\\PM_ListIonFan.ini";
    sPMItem_CheckIonFan      ="D:\\HT9045\\PMAlarm\\PM_CheckIonFan.ini";
    sPMSetting               ="D:\\HT9045\\PMAlarm\\PM_Setting.ini";
    JOBFILEDataPath          ="JOBFILE";
    JOBFILEOffsetPath        ="JOBFILE";
    sBarcodeFile             ="D:\\HT9045\\Barcode_File\\";
    asBarCodeDownLot         ="D:\\HT9045_Log\\2DBarCode\\LotData1.txt";      // DEVIATION from decl-site default -- see banner above
    asSearch2DIDByLot        ="D:\\HT9045_Log\\2DBarCode\\Search2DIDByLot.txt";
    asSortingBy2DID          ="D:\\HT9045_Log\\2DBarCode\\";                  // DEVIATION from decl-site default -- see banner above
    asGalilCmdPath           ="D:\\HT9045_Log\\Galil_Log";
    asProduct_LoaderPath     ="D:\\HT9045_Log\\Production_Loader";
    asAlarmLogXmlPath        ="D:\\HT9045_Log\\AlarmLogXml";
    sFTPSetupFileLogPath     ="D:\\HT9045_Log\\FTPSetupFileChangeLog";
    asTestTCPIPLogPath       ="D:\\HT9045_Log\\Test_TCPIP";
    asShtSenLogPath          ="D:\\HT9045_Log\\ShuttleSensorLog\\";
    asCleanPadLogPath        =(getenv("W906_CLEANPADLOG_ROOT") && *getenv("W906_CLEANPADLOG_ROOT")) ? AnsiString(getenv("W906_CLEANPADLOG_ROOT")) : AnsiString("D:\\HT9045_Log\\CleanPad_Log");   // AI(W906-W2-CLEANPADLOG) 20260926: 同上（InitCommonString 會重設一次）
    asBackup2DSortListPath   ="D:\\HT9045_log\\2D_SortList_Backup\\";
    asOCRDownLoadLotPath     ="D:\\HT9045_Log\\OCR\\CompareFile";
    asOCRLotPath             ="D:\\HT9045_Log\\OCR\\OCRLot.txt";
    asProductionByFilePath   ="D:\\HT9045_Log\\Production_ByFile";
    asBackup2DSortListPath   ="D:\\HT9045_log\\2D_SortList_Backup\\";         // golden's own duplicate assignment (see banner above), preserved
    asBackup2DSummaryPath    ="D:\\HT9045_log\\Summary_Backup\\";
    asPowerSavePath          ="D:\\HT9045_Log\\Power_EnergySaving";
    asATCPowerSavePath       ="D:\\HT9045_Log\\ATC_EnergySaving";
    asBarCodeInspReportPath  ="D:\\HT9045_Log\\2DIDParameterReport";
    asDupBundleID            ="D:\\HT9045_Log\\BundleID\\DupBundleID.txt";
    aslDupUnloadBundlID      ="D:\\HT9045_Log\\BundleID\\DupUnloadBundleID.txt";
    asMachineLifePath        ="D:\\HT9045\\system\\MachineLife.ini";
}

// ===========================================================================
//  EncodeStr / DecodeStr (common.cpp:267-321)
//  jou 2013-01-04 : Password Txt XOR encode/decode against file-local asKeyStr.
//  AI(W906-CommonCompletion) 20260721: un-gated -- pure XOR password codec,
//  no external dependency beyond the already file-local `asKeyStr`
//  (common.cpp:117) and the `byte` typedef (already visible project-wide via
//  windows.h's rpcndr.h, non-lean per vcl_compat.h's own precedent/note).
// ===========================================================================
AnsiString EncodeStr(AnsiString sourceStr)                                   // common.cpp:267
{
    int l1, l2, p1, p2;
    byte ch, ctemp;
    AnsiString Result;
    l1=sourceStr.Length();
    p1=1;
    l2=asKeyStr.Length();
    p2=1;
    Result="";
    while(p1<=l1)
    {
        ctemp=(byte(sourceStr[p1])-1)^byte(asKeyStr[p2]);
        if(ctemp==0 || ctemp=='\0')
            ch=(byte(asKeyStr[p2]));
        else
            ch=ctemp;

        Result=Result+char(ch);
        p1++;
        if(p2>=l2)
            p2=1;
        else
            p2++;
    }
    return Result;
}

AnsiString DecodeStr(AnsiString sourceStr)                                   // common.cpp:295
{
    int l1, l2, p1, p2;
    byte ch;
    AnsiString Result;
    l1=sourceStr.Length();
    p1=1;
    l2=asKeyStr.Length();
    p2=1;
    Result="";
    while(p1<=l1)
    {
        ch=byte(sourceStr[p1]);
        if(ch==byte(asKeyStr[p2]))                                           // Steven 20210330 : fix rare collision where an encoded byte equals the key byte
            ch=byte(asKeyStr[p2]);
        else
            ch=ch^byte(asKeyStr[p2]);

        Result=Result+char(ch+1);
        p1++;
        if(p2>=l2)
            p2=1;
        else
            p2++;
    }
    return Result;
}

// ===========================================================================
//  OpenIniFile / CloseIniFile (common.cpp:323-343)
// ===========================================================================

// ---------------------------------------------------------------------------
//  OpenIniFile (common.cpp:323-334)
//  Caching singleton: if INIFile is NULL or bound to a different path, close
//  and reopen.  Returns false (without opening) if FileName is empty.
//  Steven 20141120 : Add Read/Write IniFile Speed
// ---------------------------------------------------------------------------
bool OpenIniFile(AnsiString FileName)   // common.cpp:323
{
    if (FileName == "")                 // common.cpp:325 -- empty-path guard
        return false;

    if (INIFile == nullptr || INIFile->FileName != FileName)  // common.cpp:328
    {
        CloseIniFile();
        INIFile = new TIniFile(FileName);
    }
    return true;
}

// ---------------------------------------------------------------------------
//  CloseIniFile (common.cpp:336-343)
//  FAITHFUL BUG preserved: INIFile is NOT set to NULL after delete.
//  The next OpenIniFile with the same path will see a non-NULL stale pointer
//  whose FileName still matches, and will reuse the freed object.
//  This matches BCB6 behaviour exactly (common.cpp:336-343).
//  Steven 20141120 : Add Read/Write IniFile Speed
// ---------------------------------------------------------------------------
void CloseIniFile()                     // common.cpp:336
{
    if (INIFile != nullptr)
    {
        INIFile->UpdateFile();
        delete INIFile;
        // NOTE: INIFile intentionally NOT set to nullptr here -- faithful bug.
    }
}

// ===========================================================================
//  OpenIniFileMem / CloseIniFileMem (common.cpp:345-365)
//  RogerYang 20260214 : load entire file into memory to bypass 32KB/section cap
// ===========================================================================

// ---------------------------------------------------------------------------
//  OpenIniFileMem (common.cpp:345-356)
// ---------------------------------------------------------------------------
bool OpenIniFileMem(AnsiString FileName)    // common.cpp:345
{
    if (FileName == "")
        return false;

    if (INIFileMem == nullptr || INIFileMem->FileName != FileName)  // common.cpp:350
    {
        CloseIniFileMem();
        INIFileMem = new TMemIniFile(FileName);
    }
    return true;
}

// ---------------------------------------------------------------------------
//  CloseIniFileMem (common.cpp:358-365)
//  FAITHFUL BUG preserved: INIFileMem NOT set to nullptr after delete.
// ---------------------------------------------------------------------------
void CloseIniFileMem()                      // common.cpp:358
{
    if (INIFileMem != nullptr)
    {
        INIFileMem->UpdateFile();
        delete INIFileMem;
        // NOTE: INIFileMem intentionally NOT set to nullptr -- faithful bug.
    }
}

// ===========================================================================
//  CheckIniData / CheckIniDataMem (common.cpp:367-395)
// ===========================================================================

// ---------------------------------------------------------------------------
//  CheckIniData (common.cpp:367-380)
// ---------------------------------------------------------------------------
bool __fastcall CheckIniData(AnsiString FileName, AnsiString Group, AnsiString Name)  // common.cpp:367
{
    bool bTemp = false;
    AnsiString Str;
    if (OpenIniFile(FileName) == false)    // common.cpp:371
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);                // TODO(wave-cMyDB): real cMyDB::RecordProcess
        return bTemp;
    }

    bTemp = INIFile->ValueExists(Group, Name);   // common.cpp:378
    return bTemp;
}

// ---------------------------------------------------------------------------
//  CheckIniDataMem (common.cpp:382-395)
//  RogerYang 20260214
// ---------------------------------------------------------------------------
bool __fastcall CheckIniDataMem(AnsiString FileName, AnsiString Group, AnsiString Name)  // common.cpp:382
{
    bool bTemp = false;
    AnsiString Str;
    if (OpenIniFileMem(FileName) == false)
    {
        Str.sprintf("Read NULL INI_Mem on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return bTemp;
    }

    bTemp = INIFileMem->ValueExists(Group, Name);   // common.cpp:393
    return bTemp;
}

// ===========================================================================
//  CheckAndReadIniData overloads (common.cpp:397-508)
// ===========================================================================

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(double) (common.cpp:397-413)
//  Missing key -> seed with sprintf "%0.4f" text via WriteString, return Value.
//  Present key -> ReadFloat; return updated Value.
// ---------------------------------------------------------------------------
double __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  // common.cpp:397
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    Str.sprintf("%0.4f", Value);                                  // common.cpp:407
    if (!INIFile->ValueExists(Group, Name))
        INIFile->WriteString(Group, Name, Str);                   // common.cpp:409 -- seed default
    else
        Value = INIFile->ReadFloat(Group, Name, Value);           // common.cpp:411
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(unsigned long) (common.cpp:415-430)
//  ODD/FAITHFUL: missing key path calls ReadInteger with result DISCARDED --
//  no seed write, unlike every other CheckAndReadIniData overload.
// ---------------------------------------------------------------------------
unsigned long __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value)  // common.cpp:415
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    if (!INIFile->ValueExists(Group, Name))
        INIFile->ReadInteger(Group, Name, static_cast<int>(Value));  // common.cpp:426 -- READ, result discarded
    else
        Value = static_cast<unsigned long>(INIFile->ReadInteger(Group, Name, static_cast<int>(Value))); // common.cpp:428
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(int) (common.cpp:432-447)
//  Missing key -> WriteInteger (seed); present -> ReadInteger.
// ---------------------------------------------------------------------------
int __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  // common.cpp:432
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    if (!INIFile->ValueExists(Group, Name))
        INIFile->WriteInteger(Group, Name, Value);               // common.cpp:443 -- seed
    else
        Value = INIFile->ReadInteger(Group, Name, Value);        // common.cpp:445
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(bool) (common.cpp:449-464)
//  Missing key -> WriteBool (seed); present -> ReadBool.
// ---------------------------------------------------------------------------
bool __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value)  // common.cpp:449
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    if (!INIFile->ValueExists(Group, Name))
        INIFile->WriteBool(Group, Name, Value);                  // common.cpp:460 -- seed
    else
        Value = INIFile->ReadBool(Group, Name, Value);           // common.cpp:462
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(AnsiString) (common.cpp:466-491)
//  Missing key -> WriteString (seed), Str=Value.
//  Present key -> ReadString; EXTRA REPAIR: if stored value is "" and default
//  is not "", re-seed with WriteString (Steven 20160323 + JerryYang 20170711).
// ---------------------------------------------------------------------------
AnsiString __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:466
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    if (!INIFile->ValueExists(Group, Name))
    {
        INIFile->WriteString(Group, Name, Value);                // common.cpp:478 -- seed
        Str = Value;                                             // common.cpp:479 -- JerryYang 20170711
    }
    else
    {
        Str = INIFile->ReadString(Group, Name, Value);           // common.cpp:483
        if (Str == "" && Value != "")                            // common.cpp:484 -- Steven 20160323
        {
            Str = Value;
            INIFile->WriteString(Group, Name, Value);            // re-seed empty stored value
        }
    }
    return Str;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniData(TDateTime) (common.cpp:493-508)
//  Missing key -> WriteDateTime (seed); present -> ReadDateTime.
// ---------------------------------------------------------------------------
TDateTime __fastcall CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value)  // common.cpp:493
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }

    if (!INIFile->ValueExists(Group, Name))
        INIFile->WriteDateTime(Group, Name, Value);              // common.cpp:504 -- seed
    else
        Value = INIFile->ReadDateTime(Group, Name, Value);       // common.cpp:506
    return Value;
}

// ===========================================================================
//  ReadIniData overloads (common.cpp:510-564)
//  Pure reads: no seeding/writes; default = passed value.
// ===========================================================================

// ---------------------------------------------------------------------------
//  ReadIniData(bool) (common.cpp:510-522)
// ---------------------------------------------------------------------------
bool __fastcall ReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)  // common.cpp:510
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return bValue;
    }
    return INIFile->ReadBool(Group, Name, bValue);               // common.cpp:520
}

// ---------------------------------------------------------------------------
//  ReadIniData(double) (common.cpp:524-536)
// ---------------------------------------------------------------------------
double __fastcall ReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  // common.cpp:524
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFile->ReadFloat(Group, Name, Value);               // common.cpp:534
}

// ---------------------------------------------------------------------------
//  ReadIniData(int) (common.cpp:538-550)
// ---------------------------------------------------------------------------
int __fastcall ReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  // common.cpp:538
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFile->ReadInteger(Group, Name, Value);             // common.cpp:548
}

// ---------------------------------------------------------------------------
//  ReadIniData(AnsiString) (common.cpp:552-564)
//  No empty-string repair here (unlike CheckAndReadIniData(AnsiString)).
// ---------------------------------------------------------------------------
AnsiString __fastcall ReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:552
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Read NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFile->ReadString(Group, Name, Value);              // common.cpp:562
}

// ===========================================================================
//  ReadIniDataMem overloads (common.cpp:566-620)
//  Identical to ReadIniData but via OpenIniFileMem / INIFileMem.
//  RogerYang 20260214
// ===========================================================================

// ---------------------------------------------------------------------------
//  ReadIniDataMem(bool) (common.cpp:566-578)
// ---------------------------------------------------------------------------
bool __fastcall ReadIniDataMem(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)  // common.cpp:566
{
    AnsiString Str;
    if (OpenIniFileMem(FileName) == false)
    {
        Str.sprintf("Read NULL INI_Mem on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return bValue;
    }
    return INIFileMem->ReadBool(Group, Name, bValue);            // common.cpp:576
}

// ---------------------------------------------------------------------------
//  ReadIniDataMem(double) (common.cpp:580-592)
// ---------------------------------------------------------------------------
double __fastcall ReadIniDataMem(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  // common.cpp:580
{
    AnsiString Str;
    if (OpenIniFileMem(FileName) == false)
    {
        Str.sprintf("Read NULL INI_Mem on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFileMem->ReadFloat(Group, Name, Value);            // common.cpp:590
}

// ---------------------------------------------------------------------------
//  ReadIniDataMem(int) (common.cpp:594-606)
// ---------------------------------------------------------------------------
int __fastcall ReadIniDataMem(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  // common.cpp:594
{
    AnsiString Str;
    if (OpenIniFileMem(FileName) == false)
    {
        Str.sprintf("Read NULL INI_Mem on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFileMem->ReadInteger(Group, Name, Value);          // common.cpp:604
}

// ---------------------------------------------------------------------------
//  ReadIniDataMem(AnsiString) (common.cpp:608-620)
// ---------------------------------------------------------------------------
AnsiString __fastcall ReadIniDataMem(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:608
{
    AnsiString Str;
    if (OpenIniFileMem(FileName) == false)
    {
        Str.sprintf("Read NULL INI_Mem on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return Value;
    }
    return INIFileMem->ReadString(Group, Name, Value);           // common.cpp:618
}

// ===========================================================================
//  WriteIniData overloads (common.cpp:622-1120)
//  INI-CORE only translated this wave.
//  Change-log side-effects (TempChangeLog / RecordChangeLogProcess /
//  FormHS->RecordChangeLogByLot / ConvertToMMType / UI deref fContact /
//  FTestIF / fCleaning + globals InitialOK / bSysLotStart /
//  CosFunction.bUseChangeLogByLot) are gated with
//    #if 0 // TODO(wave-logging/UI)
//  The ini-core (Read-old / try{Write*} catch RecordProcess) is active.
// ===========================================================================

// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- WriteIniData
//  golden HT9011UC_Code_V3.33.906.0_20260618/common.cpp:622-689  (68 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN WriteIniData common.cpp:622-689
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)  //Steven 20090731
{
    AnsiString Str, Str1, Str2;
    bool ret;
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name
    bool bHasChange=false;                                                      //Ifor 20191004 : Change Log By Lot 需放在最下面，避免開檔後存錯位置
    bool bStr2HasFind=false;                                                    //Ifor 20191021 : 整理Even Log
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group, Name);
        RecordProcess(Str);
        return;
    }

    ret=INIFile->ReadBool(Group, Name, bValue);
    if(ret!=bValue && InitialOK==true)                                          //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        StrChangeName=TempChangeLog(Group,Name);                                //Ifor 20190930 : add Display temperature switch Site Name
        if(FileName.Pos("Offset")==0)                                           //Ifor 20191004 : add Change Log 是否為Offset 資料
        {
            Str1.sprintf("%s_%s change Value",Group , StrChangeName);
        }
        else
        {
            Str1.sprintf("%s_%s Offset change Value",Group , StrChangeName);
        }

        if(FileName.Pos("HandlerCondition.Data")!=0)
        {
            if(Group=="Configuration")
            {
                bStr2HasFind=true;
                if(Name=="bAutoClean_UseTray")
                {
                    Str2.sprintf("%s ==> %s", fCleaning->rgCleanKitType->Items->Strings[ret], fCleaning->rgCleanKitType->Items->Strings[bValue]);
                }
                else
                {
                    bStr2HasFind=false;
                }
            }
        }

        if(bStr2HasFind==false)
        {
            Str2.sprintf("%d==>%d", ret, bValue);
        }
        RecordChangeLogProcess(Str1.c_str(), Str2.c_str());                     //wei 20180625 offset Change log紀錄
        bHasChange=true;
    }

    try
    {
        INIFile->WriteBool(Group, Name, bValue);
    }
    catch(...)
    {
        Str1.sprintf("WriteIniData:%s", FileName);
        RecordProcess("Exception", Str1);
    }

    if(CosFunction.bUseChangeLogByLot==true &&                                  //Ifor 20191004 : Change Log By Lot 需放在最下面，避免開檔後存錯位置
       bSysLotStart==true &&
       bHasChange==true)                                                        //Ifor 20191002 : add Change Log By Lot
    {
        FormHS->RecordChangeLogByLot(Str1, Str2);
    }
}
#endif // AI-W6C-GOLDEN-END WriteIniData common.cpp:622-689

// ---------------------------------------------------------------------------
//  WriteIniData(bool) (common.cpp:622-689)
//  Core: ReadBool old value; try{ WriteBool } catch.
//  Change-log block gated (InitialOK/TempChangeLog/RecordChangeLogProcess/
//  FormHS->RecordChangeLogByLot / fCleaning UI deref).
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)  // common.cpp:622
{
    AnsiString Str, Str1, Str2;
    bool ret;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    ret = INIFile->ReadBool(Group, Name, bValue);               // common.cpp:636

#if 0 // TODO(wave-logging/UI): change-log block (common.cpp:637-688)
    // if (ret != bValue && InitialOK == true) {
    //     ... TempChangeLog / RecordChangeLogProcess / fCleaning UI / RecordChangeLogByLot
    // }
#endif // TODO(wave-logging/UI)

    try
    {
        INIFile->WriteBool(Group, Name, bValue);                 // common.cpp:675
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }

#if 0 // TODO(wave-logging/UI): RecordChangeLogByLot (common.cpp:683-688)
    // if (CosFunction.bUseChangeLogByLot && bSysLotStart && bHasChange)
    //     FormHS->RecordChangeLogByLot(Str1, Str2);
#endif // TODO(wave-logging/UI)

    (void)ret;   // suppress unused-variable warning while change-log is gated
    (void)Str2;
}

// ---------------------------------------------------------------------------
//  WriteIniData(int) (common.cpp:691-873)
//  Core: ReadInteger old value; try{ WriteInteger } catch.
//  Change-log block gated (large UI-coupled block + ConvertToMMType).
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  // common.cpp:691
{
    AnsiString Str, Str1, Str2;
    int ret;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    ret = INIFile->ReadInteger(Group, Name, Value);              // common.cpp:705

#if 0 // TODO(wave-logging/UI): change-log block (common.cpp:706-855)
    // if (ret != Value && InitialOK == true) {
    //     ... TempChangeLog / fContact->cbContactMode->Items / FTestIF->rgInterfaceType->Items
    //     ... fCleaning->rgAutoCleanSelectArm->Items / ConvertToMMType / RecordChangeLogProcess
    // }
#endif // TODO(wave-logging/UI)

    try                                                          // common.cpp:857
    {
        INIFile->WriteInteger(Group, Name, Value);
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }

#if 0 // TODO(wave-logging/UI): RecordChangeLogByLot (common.cpp:867-872)
#endif // TODO(wave-logging/UI)

    (void)ret; (void)Str2;
}

// ---------------------------------------------------------------------------
//  WriteIniData(double) (common.cpp:875-972)
//  Core: sprintf "%0.4f" -> Str; ReadFloat old; try{ WriteString(Str) } catch.
//  SUBTLETY (common.cpp:891): BCB6 compares `ret` (double) with `Str`
//  (AnsiString) -- BCB6 resolves via AnsiString implicit cast from double
//  (effectively comparing the 4-decimal formatted string of ret with Str).
//  The change-log check is fully gated, so only the ini-core (WriteString)
//  matters here.
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  // common.cpp:875
{
    AnsiString Str, Str1, Str2;
    double ret;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }
    Str.sprintf("%0.4f", Value);                                 // common.cpp:888

    ret = INIFile->ReadFloat(Group, Name, Value);                // common.cpp:890

#if 0 // TODO(wave-logging/UI): change-log block (common.cpp:891-954)
    // if (ret != Str && InitialOK == true) { ... ConvertToMMType / RecordChangeLogProcess }
    // NOTE: `ret != Str` in BCB6 coerces Str to double via AnsiString operator double
    // (effectively sprintf("%0.4f", ret) != Str, i.e. the stored value differs from new).
#endif // TODO(wave-logging/UI)

    try                                                          // common.cpp:956
    {
        INIFile->WriteString(Group, Name, Str);                  // common.cpp:958 -- NOT WriteFloat
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }

#if 0 // TODO(wave-logging/UI): RecordChangeLogByLot (common.cpp:966-971)
#endif // TODO(wave-logging/UI)

    (void)ret; (void)Str2;
}

// ---------------------------------------------------------------------------
//  WriteIniData(unsigned long) (common.cpp:974-1019)
//  Core: ReadInteger old; try{ WriteInteger } catch.
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value)  // common.cpp:974
{
    AnsiString Str, Str1, Str2;
    unsigned long ret;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    ret = static_cast<unsigned long>(INIFile->ReadInteger(Group, Name, static_cast<int>(Value)));  // common.cpp:987

#if 0 // TODO(wave-logging/UI): change-log block (common.cpp:988-1001)
    // if (ret != Value && InitialOK == true) { TempChangeLog / RecordChangeLogProcess }
#endif // TODO(wave-logging/UI)

    try                                                          // common.cpp:1003
    {
        INIFile->WriteInteger(Group, Name, static_cast<int>(Value));  // common.cpp:1005
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }

#if 0 // TODO(wave-logging/UI): RecordChangeLogByLot (common.cpp:1013-1018)
#endif // TODO(wave-logging/UI)

    (void)ret; (void)Str2;
}

// ---------------------------------------------------------------------------
//  WriteIniData(AnsiString) (common.cpp:1021-1099)
//  Core: ReadString old; TryStrToFloat to decide numeric vs string compare;
//  try{ WriteString } catch.
//  CHANGE-DETECT: if both old/new parse as float -> compare atof() != atof();
//  else string compare ret != Value (common.cpp:1057-1058).
//  Change-log action (RecordChangeLogProcess / FormHS) is gated; the detect
//  logic is also gated since all its outputs feed only the log block.
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:1021
{
    AnsiString Str, Str1, Str2;
    AnsiString ret;

#if 0 // TODO(wave-logging/UI): change-log variables (common.cpp:1025-1029)
    bool bStrIsFloat1 = false;
    bool bStrIsFloat2 = false;
    double freg       = 0;
#endif // TODO(wave-logging/UI)

    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    ret = INIFile->ReadString(Group, Name, Value);               // common.cpp:1037

#if 0 // TODO(wave-logging/UI): float-detect change-log block (common.cpp:1053-1081)
    // bStrIsFloat1 = TryStrToFloat(ret.c_str(), freg);
    // bStrIsFloat2 = TryStrToFloat(Value.c_str(), freg);
    // if (InitialOK == true) {
    //     if ((bStrIsFloat1 && bStrIsFloat2 && atof(ret.c_str()) != atof(Value.c_str())) ||
    //         (!bStrIsFloat1 && !bStrIsFloat2 && ret != Value)) {
    //         ... TempChangeLog / RecordChangeLogProcess
    //     }
    // }
#endif // TODO(wave-logging/UI)

    try                                                          // common.cpp:1083
    {
        INIFile->WriteString(Group, Name, Value);                // common.cpp:1085
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }

#if 0 // TODO(wave-logging/UI): RecordChangeLogByLot (common.cpp:1093-1098)
#endif // TODO(wave-logging/UI)

    (void)ret; (void)Str2;
}

// ---------------------------------------------------------------------------
//  WriteIniData(TDateTime) (common.cpp:1101-1120)
//  No change-log.  try{ WriteDateTime } catch.
// ---------------------------------------------------------------------------
void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value)  // common.cpp:1101
{
    AnsiString Str, Str1;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    try                                                          // common.cpp:1111
    {
        INIFile->WriteDateTime(Group, Name, Value);              // common.cpp:1113
    }
    catch (...)
    {
        Str1.sprintf("WriteIniData:%s", FileName.c_str());
        RecordProcess("Exception", Str1);
    }
}

// ===========================================================================
//  WriteIniDataNoLog overloads (common.cpp:1122-1184)
//  No change-log, no RecordChangeLog.  Open -> Write inside try/catch.
//  Steven 20200526 : Add Write ini without event log
// ===========================================================================

// ---------------------------------------------------------------------------
//  WriteIniDataNoLog(int) (common.cpp:1122-1141)
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataNoLog(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  // common.cpp:1122
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    try                                                          // common.cpp:1132
    {
        INIFile->WriteInteger(Group, Name, Value);               // common.cpp:1134
    }
    catch (...)
    {
        Str.sprintf("WriteIniDataNoLog:%s", FileName.c_str());
        RecordProcess("Exception", Str);
    }
}

// ---------------------------------------------------------------------------
//  WriteIniDataNoLog(AnsiString) (common.cpp:1143-1162)
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataNoLog(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:1143
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }

    try                                                          // common.cpp:1153
    {
        INIFile->WriteString(Group, Name, Value);                // common.cpp:1155
    }
    catch (...)
    {
        Str.sprintf("WriteIniDataNoLog:%s", FileName.c_str());
        RecordProcess("Exception", Str);
    }
}

// ---------------------------------------------------------------------------
//  WriteIniDataNoLog(double) (common.cpp:1164-1184)
//  sprintf "%0.4f" -> Str; WriteString (NOT WriteFloat).
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataNoLog(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  // common.cpp:1164
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Write NULL INI on [%s] %s", Group.c_str(), Name.c_str());
        RecordProcess(Str);
        return;
    }
    Str.sprintf("%0.4f", Value);                                 // common.cpp:1173

    try
    {
        INIFile->WriteString(Group, Name, Str);                  // common.cpp:1177
    }
    catch (...)
    {
        Str.sprintf("WriteIniDataNoLog:%s", FileName.c_str());
        RecordProcess("Exception", Str);
    }
}

// ===========================================================================
//  WriteIniData1 (common.cpp:1186-1237)
//  kevin 20220623
//  Like WriteIniData(AnsiString) but:
//    * open-fail returns SILENTLY (no RecordProcess) (common.cpp:1194-1196)
//    * builds change strings but does NOT call RecordChangeLogProcess
//    * does NOT call RecordChangeLogByLot
//  The change-detection block is gated (it only feeds the discarded strings).
// ===========================================================================
void __fastcall WriteIniData1(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:1186
{
    AnsiString Str, Str1, Str2;
    AnsiString ret;

    if (OpenIniFile(FileName) == false)                          // common.cpp:1194
    {
        return;                                                  // common.cpp:1196 -- silent, no RecordProcess
    }

    ret = INIFile->ReadString(Group, Name, Value);               // common.cpp:1199

#if 0 // TODO(wave-logging/UI): change-detect block (common.cpp:1200-1225)
    // bool bStrIsFloat1 = false, bStrIsFloat2 = false;
    // double freg = 0;
    // bStrIsFloat1 = TryStrToFloat(ret.c_str(), freg);
    // bStrIsFloat2 = TryStrToFloat(Value.c_str(), freg);
    // if (InitialOK == true) {
    //     if ((bStrIsFloat1 && bStrIsFloat2 && atof(ret.c_str()) != atof(Value.c_str())) ||
    //         (!bStrIsFloat1 && !bStrIsFloat2 && ret != Value)) {
    //         StrChangeName = TempChangeLog(Group, Name);
    //         if (FileName.Pos("Offset") == 0)
    //             Str1.sprintf("%s_%s change Value", Group, StrChangeName);
    //         else
    //             Str1.sprintf("%s_%s Offset change Value", Group, StrChangeName);
    //         if (FileName.Pos("Contact.Data") != 0 && Group == "Mode" &&
    //             Name == "fSocketInitialICCheckPositionOffset")
    //             Str2.sprintf("%0.2fmm ==> %0.2fmm", atof(ret.c_str()), atof(Value.c_str()));
    //         else
    //             Str2.sprintf("%s==>%s", ret, Value);
    //         // NOTE: unlike WriteIniData(AnsiString), NO RecordChangeLogProcess call here.
    //     }
    // }
#endif // TODO(wave-logging/UI)

    try
    {
        INIFile->WriteString(Group, Name, Value);                // common.cpp:1230
    }
    catch (...)
    {
        Str.sprintf("WriteIniData1:%s", FileName.c_str());
        RecordProcess("Exception", Str);
    }

    (void)ret; (void)Str1; (void)Str2;
}

// ===========================================================================
//  ReplaceIniData (common.cpp:1239-1250)
//  Copies the stored value of key Name2 into key Name1.
// ===========================================================================
void __fastcall ReplaceIniData(AnsiString FileName, AnsiString Group, AnsiString Name1, AnsiString Name2)  // common.cpp:1239
{
    AnsiString Str;
    if (OpenIniFile(FileName) == false)
    {
        Str.sprintf("Replace NULL INI on [%s] %s", Group.c_str(), Name1.c_str());
        RecordProcess(Str);
        return;
    }
    AnsiString ret = INIFile->ReadString(Group, Name2, " ");     // common.cpp:1248 -- default single space
    INIFile->WriteString(Group, Name1, ret);                     // common.cpp:1249
}

// ===========================================================================
//  GetLastOpenFN (common.cpp:1252-1281)
// ===========================================================================
// AI(W906-CommonWaveFile) 20260721: un-gated -- TStringList::LoadFromFile /
// FileExists are already-active vclcompat primitives; ShowMyMessage resolves
// to the local no-op forward-stub declared near RecordProcess above.
AnsiString __fastcall GetLastOpenFN()                                        // common.cpp:1252
{
    AnsiString Str="Fail Open";
    TStringList *MyList= new TStringList();

    if(FileExists(LastDataPath))
    {
        MyList->LoadFromFile(LastDataPath);
        if(MyList->Count==0)                                                 // Jimmychiu 20250426 : content of setup is empty
        {
            ShowMyMessage("The content of setup.inf is empty!");
        }
        else if(AnsiString(MyList->Strings[0]).Trim()=="")                   // StringsProxy has no Trim() -- explicit AnsiString cast (established idiom, e.g. Automation/auto9045.cpp:84)
        {
            ShowMyMessage("The content of setup.inf is NULL!");
        }
        else
        {
            Str=MyList->Strings[0];
        }
    }
    else
    {
        ShowMyMessage("LastData does not exist");
    }
    MyList->Clear();                                                         // Ifor 20170603 : clear TStringList before delete
    delete MyList;

    return Str;
}

// ===========================================================================
//  CheckSectionExist / CheckKeyExist (common.cpp:1283-1309)
// ===========================================================================

// ---------------------------------------------------------------------------
//  CheckSectionExist (common.cpp:1283-1295)
//  Returns false if Group=="" || FileName=="".
// ---------------------------------------------------------------------------
bool __fastcall CheckSectionExist(AnsiString FileName, AnsiString Group)  // common.cpp:1283
{
    if (Group == "" || FileName == "")                           // common.cpp:1285
        return false;

    if (OpenIniFile(FileName) == false)
        return false;

    return INIFile->SectionExists(Group);                        // common.cpp:1293
}

// ---------------------------------------------------------------------------
//  CheckKeyExist (common.cpp:1297-1309)
//  Returns false if Group=="" || FileName=="".
// ---------------------------------------------------------------------------
bool __fastcall CheckKeyExist(AnsiString FileName, AnsiString Group, AnsiString Name)  // common.cpp:1297
{
    if (Group == "" || FileName == "")                           // common.cpp:1299
        return false;

    if (OpenIniFile(FileName) == false)
        return false;

    return INIFile->ValueExists(Group, Name);                    // common.cpp:1307
}

// ===========================================================================
//  WriteLastDataFN (common.cpp:1311-1331)
// ===========================================================================
// AI(W906-CommonWaveFile) 20260721: un-gated. Golden's bootstrap (else,
// LastDataPath does not yet exist) branch used raw BCB6 int-handle
// FileCreate/FileWrite/FileClose (common.cpp:1325-1327) -- no vclcompat shim
// for that trio exists (confirmed via grep across this tree), and adding one
// is deliberately out of scope this wave (avoid growing new Win32 shim
// surface for a single bootstrap call). DELIBERATE SUBSTITUTION: reuse the
// same TStringList Add()+SaveToFile() path as the if-branch instead --
// functionally equivalent to golden's "create the file with SName as its
// sole line" bootstrap behavior. `iFileHandle` (golden's local var for the
// removed FileCreate/FileWrite/FileClose calls) is dropped as unused.
void __fastcall WriteLastDataFN(AnsiString SName)                            // common.cpp:1311
{
    TStringList *MyList= new TStringList();

    if(FileExists(LastDataPath))
    {
        MyList->LoadFromFile(LastDataPath);
        MyList->Clear();
        MyList->Add(SName);
        MyList->SaveToFile(LastDataPath);
    }
    else
    {
        MyList->Add(SName);
        MyList->SaveToFile(LastDataPath);
    }
    MyList->Clear();                                                         // Ifor 20170603 : clear TStringList before delete
    delete MyList;
}

// ===========================================================================
//  AddSpace (common.cpp:1333-1336) -- body is a golden no-op return.
//  AI(W906-CommonCompletion) 20260721: un-gated -- the draft body already
//  sitting in this #if 0 block matched golden exactly (return; with no
//  side effect). Takes an AnsiString, not a VCL control -- no dependency gap.
// ===========================================================================
void __fastcall AddSpace(AnsiString /*File*/) { return; }

// ===========================================================================
//  OnlyNumberInPut / OnlyNumberAndDotInPut / OnlyMakeFileDataInPut
//  (common.cpp:1338-1358) -- key-filter helpers; pure logic, no deps.
//  AI(W906-CommonCompletion) 20260721: un-gated -- verified the draft bodies
//  already sitting in this #if 0 block against golden common.cpp:1338-1358:
//  logically identical (golden's if(cond) return true; return false; here
//  collapsed to a single `return cond;`; OnlyMakeFileDataInPut's golden
//  if(cond) return false; return true; here is De Morgan'd to
//  `return !(cond);`) -- no behavior discrepancy, just a compacter
//  expression form. None take a VCL control (plain int keycode) -- no
//  dependency gap either.
// ===========================================================================
bool __fastcall OnlyNumberInPut(int iKey)          { return (iKey>='0'&&iKey<='9')||iKey==8||iKey=='-'; }
bool __fastcall OnlyNumberAndDotInPut(int iKey)    { return (iKey>='0'&&iKey<='9')||iKey=='.'||iKey==8||iKey=='-'; }
bool __fastcall OnlyMakeFileDataInPut(int iKey)    { return !(iKey=='\''||iKey=='/'||iKey==':'||iKey=='*'||iKey=='?'||iKey=='"'||iKey=='<'||iKey=='|'); }

// ===========================================================================
//  GATED: MyDrawText overloads (common.cpp:1360-1404)
//  WAVE: canvas/UI -- TCanvas / TRect / TColor VCL GDI types.
// ===========================================================================
#if 0 // TODO(wave-canvas): MyDrawText x6 (common.cpp:1360-1404)
#endif // TODO(wave-canvas)

// ===========================================================================
//  OpenGeneralIniFile / CloseGeneralIniFile (common.cpp:1408-1421)
// ===========================================================================

// ---------------------------------------------------------------------------
//  OpenGeneralIniFile (common.cpp:1408-1412)
//  UNCONDITIONALLY calls CloseGeneralIniFile() then creates a new TIniFile
//  bound to asGeneralPath.  No caching guard, no return value.
//  Steven 20141120 : Add Read/Write IniFile Speed
// ---------------------------------------------------------------------------
void OpenGeneralIniFile()                                        // common.cpp:1408
{
    CloseGeneralIniFile();
    INIFileGeneral = new TIniFile(asGeneralPath);
}

// ---------------------------------------------------------------------------
//  CloseGeneralIniFile (common.cpp:1414-1421)
//  FAITHFUL BUG: INIFileGeneral NOT set to nullptr after delete.
// ---------------------------------------------------------------------------
void CloseGeneralIniFile()                                       // common.cpp:1414
{
    if (INIFileGeneral != nullptr)
    {
        INIFileGeneral->UpdateFile();
        delete INIFileGeneral;
        // NOTE: INIFileGeneral intentionally NOT set to nullptr -- faithful bug.
    }
}

// ===========================================================================
//  CheckAndReadIniDataGeneral overloads (common.cpp:1423-1464)
//  Operate DIRECTLY on global INIFileGeneral -- NO open/close guard, NO NULL
//  check.  Caller MUST have called OpenGeneralIniFile() first; otherwise NULL
//  deref is faithful BCB6 behaviour.
// ===========================================================================

// ---------------------------------------------------------------------------
//  CheckAndReadIniDataGeneral(AnsiString) (common.cpp:1423-1430)
// ---------------------------------------------------------------------------
AnsiString __fastcall CheckAndReadIniDataGeneral(AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:1423
{
    if (!INIFileGeneral->ValueExists(Group, Name))
        INIFileGeneral->WriteString(Group, Name, Value);         // common.cpp:1426 -- seed
    else
        Value = INIFileGeneral->ReadString(Group, Name, Value);  // common.cpp:1428
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniDataGeneral(double) (common.cpp:1432-1442)
//  Seed via WriteString (NOT WriteFloat), 4-decimal text.
// ---------------------------------------------------------------------------
double __fastcall CheckAndReadIniDataGeneral(AnsiString Group, AnsiString Name, double Value)  // common.cpp:1432
{
    AnsiString Str;
    Str.sprintf("%0.4f", Value);                                 // common.cpp:1435

    if (!INIFileGeneral->ValueExists(Group, Name))
        INIFileGeneral->WriteString(Group, Name, Str);           // common.cpp:1438 -- seed as 4-decimal text
    else
        Value = INIFileGeneral->ReadFloat(Group, Name, Value);   // common.cpp:1440
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniDataGeneral(int) (common.cpp:1444-1455)
// ---------------------------------------------------------------------------
int __fastcall CheckAndReadIniDataGeneral(AnsiString Group, AnsiString Name, int Value)  // common.cpp:1444
{
    if (!INIFileGeneral->ValueExists(Group, Name))
    {
        INIFileGeneral->WriteInteger(Group, Name, Value);        // common.cpp:1448 -- seed
    }
    else
    {
        Value = INIFileGeneral->ReadInteger(Group, Name, Value); // common.cpp:1452
    }
    return Value;
}

// ---------------------------------------------------------------------------
//  CheckAndReadIniDataGeneral(bool) (common.cpp:1457-1464)
// ---------------------------------------------------------------------------
bool __fastcall CheckAndReadIniDataGeneral(AnsiString Group, AnsiString Name, bool Value)  // common.cpp:1457
{
    if (!INIFileGeneral->ValueExists(Group, Name))
        INIFileGeneral->WriteBool(Group, Name, Value);           // common.cpp:1460 -- seed
    else
        Value = INIFileGeneral->ReadBool(Group, Name, Value);    // common.cpp:1462
    return Value;
}

// ===========================================================================
//  WriteIniDataGeneral overloads (common.cpp:1466-1487)
//  Operate directly on INIFileGeneral.  No guard, no log.
//  Steven 20090731
// ===========================================================================

// ---------------------------------------------------------------------------
//  WriteIniDataGeneral(bool) (common.cpp:1466-1469)
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataGeneral(AnsiString Group, AnsiString Name, bool bValue)  // common.cpp:1466
{
    INIFileGeneral->WriteBool(Group, Name, bValue);
}

// ---------------------------------------------------------------------------
//  WriteIniDataGeneral(int) (common.cpp:1471-1474)
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataGeneral(AnsiString Group, AnsiString Name, int Value)  // common.cpp:1471
{
    INIFileGeneral->WriteInteger(Group, Name, Value);
}

// ---------------------------------------------------------------------------
//  WriteIniDataGeneral(double) (common.cpp:1476-1482)
//  sprintf "%0.4f" -> WriteString (NOT WriteFloat). Steven 20150723.
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataGeneral(AnsiString Group, AnsiString Name, double Value)  // common.cpp:1476
{
    AnsiString Str;
    Str.sprintf("%0.4f", Value);                                 // common.cpp:1479
    INIFileGeneral->WriteString(Group, Name, Str);
}

// ---------------------------------------------------------------------------
//  WriteIniDataGeneral(AnsiString) (common.cpp:1484-1487)
// ---------------------------------------------------------------------------
void __fastcall WriteIniDataGeneral(AnsiString Group, AnsiString Name, AnsiString Value)  // common.cpp:1484
{
    INIFileGeneral->WriteString(Group, Name, Value);
}

// ===========================================================================
//  ReadWriteIni overloads (common.cpp:1489-1605)
//  Unified read-or-write facade.
//  bIsRead==true:  Value = CheckAndReadIniData(FileName,Group,Name,DefaultValue)
//                  [NOTE: DefaultValue, not Value, is the seed/default arg]
//                  then clamp via CheckRange if bCheckRange.
//  bIsRead==false: clamp Value via CheckRange if bCheckRange;
//                  WriteIniData(FileName,Group,Name,clamped-Value).
//  ASYMMETRY (double/unsigned long): extra pre-clamp of Value before the
//  if(bIsRead) branch (common.cpp:1564-1565, 1587-1588).
//
//  Depends on: CheckAndReadIniData / WriteIniData (this batch) +
//              CheckRange (MachineType.h template, already in scope).
// ===========================================================================

// ---------------------------------------------------------------------------
//  ReadWriteIni(bool) (common.cpp:1489-1503)
//  No bCheckRange / Max / Min params (common.h:52 signature).
// ---------------------------------------------------------------------------
bool __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value, bool DefaultValue, bool bIsRead)  // common.cpp:1489
{
    bool tValue = Value;

    if (bIsRead)
    {
        Value  = CheckAndReadIniData(FileName, Group, Name, DefaultValue);  // common.cpp:1495
        tValue = Value;
    }
    else
    {
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1500
    }
    return tValue;
}

// ---------------------------------------------------------------------------
//  ReadWriteIni(AnsiString) (common.cpp:1505-1518)
//  No bCheckRange / Max / Min params (common.h:51 signature).
// ---------------------------------------------------------------------------
AnsiString __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value, AnsiString DefaultValue, bool bIsRead)  // common.cpp:1505
{
    AnsiString tValue = Value;

    if (bIsRead)
    {
        Value  = CheckAndReadIniData(FileName, Group, Name, DefaultValue);  // common.cpp:1510
        tValue = Value;
    }
    else
    {
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1515
    }
    return tValue;
}

// ---------------------------------------------------------------------------
//  ReadWriteIni(TDateTime) (common.cpp:1520-1538)
// ---------------------------------------------------------------------------
TDateTime __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value, TDateTime DefaultValue, bool bIsRead, bool bCheckRange, TDateTime Maximum, TDateTime Minimum)  // common.cpp:1520
{
    TDateTime tValue = Value;

    if (bIsRead)
    {
        Value = CheckAndReadIniData(FileName, Group, Name, DefaultValue);  // common.cpp:1525
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1527
        else
            tValue = Value;
    }
    else
    {
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1534
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1535
    }
    return tValue;
}

// ---------------------------------------------------------------------------
//  ReadWriteIni(int) (common.cpp:1540-1559)
// ---------------------------------------------------------------------------
int __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, int Value, int DefaultValue, bool bIsRead, bool bCheckRange, int Maximum, int Minimum)  // common.cpp:1540
{
    int tValue = Value;

    if (bIsRead)                                                 // common.cpp:1544
    {
        Value = CheckAndReadIniData(FileName, Group, Name, DefaultValue);
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1548
        else
            tValue = Value;
    }
    else
    {
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1555
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1556
    }
    return tValue;
}

// ---------------------------------------------------------------------------
//  ReadWriteIni(double) (common.cpp:1561-1582)
//  ASYMMETRY: extra pre-clamp before if(bIsRead) (common.cpp:1564-1565).
// ---------------------------------------------------------------------------
double __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, double Value, double DefaultValue, bool bIsRead, bool bCheckRange, double Maximum, double Minimum)  // common.cpp:1561
{
    double tValue = Value;
    if (bCheckRange)
        Value = CheckRange(Value, Maximum, Minimum);             // common.cpp:1564-1565 -- pre-clamp

    if (bIsRead)
    {
        Value = CheckAndReadIniData(FileName, Group, Name, DefaultValue);
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1571
        else
            tValue = Value;
    }
    else
    {
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1578
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1579
    }
    return tValue;
}

// ---------------------------------------------------------------------------
//  ReadWriteIni(unsigned long) (common.cpp:1584-1605)
//  ASYMMETRY: extra pre-clamp before if(bIsRead) (common.cpp:1587-1588).
// ---------------------------------------------------------------------------
unsigned long __fastcall ReadWriteIni(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value, unsigned long DefaultValue, bool bIsRead, bool bCheckRange, unsigned long Maximum, unsigned long Minimum)  // common.cpp:1584
{
    unsigned long tValue = Value;
    if (bCheckRange)
        Value = CheckRange(Value, Maximum, Minimum);             // common.cpp:1587-1588 -- pre-clamp

    if (bIsRead)
    {
        Value = CheckAndReadIniData(FileName, Group, Name, DefaultValue);
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1593
        else
            tValue = Value;
    }
    else
    {
        if (bCheckRange)
            tValue = CheckRange(Value, Maximum, Minimum);        // common.cpp:1600
        WriteIniData(FileName, Group, Name, tValue);             // common.cpp:1601
    }
    return tValue;
}

// ===========================================================================
//  GATED: most remaining common.cpp regions (lines 1607-end) -- EXCEPT the
//  wave-file cluster immediately below, un-gated this wave (AI(W906-
//  CommonWaveFile) 20260721).
// ===========================================================================

// ===========================================================================
//  WriteDataToFile x2 / CheckFileIsEmpty / ReadDataFromFile / MyForceDirectories
//  (common.cpp:1607-1721)
// ===========================================================================
// AI(W906-CommonWaveFile) 20260721: un-gated -- all five functions translated
// below. See each function's own note for golden bugs/quirks preserved
// verbatim.

// ---------------------------------------------------------------------------
//  WriteDataToFile(char*,char*,bool) (common.cpp:1607-1625)
//  Pure CRT: fopen(mode "w"/"a") -> fputs(cData) -> fputs("\n") -> fclose.
//  Silent no-op if fopen fails (pFile==NULL) -- no error signalled to caller.
//  DEVIATION from golden's literal `char*` params: `const char*` here (see
//  common.h's declaration-site note for why -- fixes a real infinite-
//  recursion bug found via a build+gdb backtrace, not a stylistic choice).
// ---------------------------------------------------------------------------
void WriteDataToFile(const char* cFilePath, const char* cData, bool bOverWrite) // common.cpp:1607
{
    FILE *pFile;
    if(bOverWrite)
    {
        pFile=fopen(cFilePath, "w");
    }
    else
    {
        pFile=fopen(cFilePath, "a");
    }

    if(pFile!=NULL)
    {
        fputs(cData, pFile);
        fputs("\n", pFile);
        fclose(pFile);
    }
}

// ---------------------------------------------------------------------------
//  CheckFileIsEmpty(AnsiString) (common.cpp:1627-1643)
//  GOLDEN BUG, preserved verbatim: fclose(pFile) is called UNCONDITIONALLY,
//  even when pFile==NULL (fopen failed) -- passing a NULL FILE* to fclose is
//  UB in ISO C. Do NOT add a NULL guard; this is golden's actual behavior.
//
//  GOLDEN BUG (found and VERIFIED empirically this wave via tests/
//  test_common.cpp -- NOT just a static reading; the naive expectation below
//  was wrong the first time round too), preserved verbatim: the return value
//  is effectively INVERTED from what the function's name suggests. `bResult`
//  starts `true` and is only ever forced to `false` in the one case where the
//  file opens AND the very first fgetc() immediately hits EOF. The three
//  real outcomes are:
//    - file does not exist            -> returns true
//    - file exists, has content       -> returns true
//    - file exists, genuinely empty   -> returns false
//  In other words: true means "missing OR non-empty", false means "exists
//  and is empty" -- the opposite of an intuitive "IsEmpty" predicate for two
//  of the three cases. Do not "fix" this; callers elsewhere in golden
//  presumably already accommodate (or never depend on) this inversion.
// ---------------------------------------------------------------------------
bool CheckFileIsEmpty(AnsiString cFilePath)                                  // common.cpp:1627
{
    bool bResult=true;
    FILE *pFile;

    pFile=fopen(cFilePath.c_str(), "r");
    if(pFile!=NULL)
    {
        int first_char=fgetc(pFile);
        if(first_char==EOF)
        {
            bResult=false;
        }
    }
    fclose(pFile);                                                          // golden bug: unconditional, even if pFile==NULL
    return bResult;
}

// ---------------------------------------------------------------------------
//  WriteDataToFile(AnsiString,AnsiString,bool) (common.cpp:1645-1648)
//  1-line forwarder to the char* overload via .c_str().
// ---------------------------------------------------------------------------
void WriteDataToFile(AnsiString cFilePath, AnsiString cData, bool bOverWrite) // common.cpp:1645
{
    WriteDataToFile(cFilePath.c_str(), cData.c_str(), bOverWrite);
}

// ---------------------------------------------------------------------------
//  ReadDataFromFile(AnsiString) (common.cpp:1650-1665)
//  OWNERSHIP CONTRACT (golden, preserved verbatim -- this is a deliberate
//  C-style raw pointer hand-off, not a bug to "modernize" away): returns a
//  raw malloc()'d buffer on success; the CALLER is responsible for free()'ing
//  it. Signature kept exactly as golden has it (raw char*/malloc, no smart
//  pointer, no signature change).
//
//  GOLDEN BUG (found and VERIFIED empirically this wave via a standalone gdb/
//  diagnostic repro, not just inferred): `fopen(...,"r")` here is TEXT mode
//  (golden's literal mode string, no "b"), so on Windows the CRLF<->LF
//  translation applies. `file_size` is the ON-DISK byte count (from ftell()),
//  used as (a) the malloc() size, (b) the fread() request size, AND (c) the
//  index where the forced '\0' terminator is written. A text-mode fread()
//  DELIVERS FEWER bytes than requested whenever the file contains any "\r\n"
//  (each collapses to one delivered "\n"), stopping at EOF -- so for a file
//  with N embedded newlines, exactly N bytes at the TAIL of the malloc'd
//  buffer (immediately before the forced '\0') are LEFT UNINITIALIZED
//  (whatever malloc() happened to hand back), not zeroed and not part of the
//  real file content. Any file written by WriteDataToFile ALWAYS has this
//  problem, since it unconditionally appends one "\n" per call. Preserved
//  verbatim (golden's fopen mode strings are exactly "r"/"w"/"a", no "b" --
//  see also WriteDataToFile above); documented here rather than silently
//  switched to binary mode. Callers must not assume the returned buffer's
//  tail bytes (near the end, before the final NUL) are meaningful when the
//  source file has embedded newlines -- see tests/test_common.cpp for a
//  reproduction and the safe (prefix-only) way to check round-tripped content.
// ---------------------------------------------------------------------------
char* ReadDataFromFile(AnsiString cFilePath)                                 // common.cpp:1650
{
    FILE* pFile=fopen(cFilePath.c_str(), "r");
    if(pFile!=NULL)
    {
        fseek(pFile, 0, SEEK_END);
        long file_size=ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        char* file_buf=(char*)malloc(file_size + 1);
        fread(file_buf, file_size, 1, pFile);
        file_buf[file_size]='\0';
        fclose(pFile);
        return file_buf;
    }
    return NULL;
}

// ---------------------------------------------------------------------------
//  MyForceDirectories(AnsiString,AnsiString) (common.cpp:1667-1721)
//  GOLDEN QUIRK, preserved verbatim: the directory-vs-filename auto-detect
//  only strips a trailing filename component for exactly ".txt"/".csv"
//  extensions (hardcoded) when the path does not exist and has no trailing
//  '\\' -- any other extension is (mis)treated as a directory name to create.
//  catch(Exception&)/catch(...) MERGED into a single generic catch(...) --
//  there is no vclcompat Exception base class to catch. Follows the
//  established project precedent in Interface/TesterTCP.cpp:79
//  (Gated_MyForceDirectories) and its sibling copy in
//  Interface/TesterTCP_Socket.cpp. Documented accepted behavior-diff: loses
//  golden's distinct return-code/e.Message text split (-2 with e.Message vs
//  -3 with a fixed string) -- both call sites currently in this project
//  discard the return value, so this is harmless today; -3 with the fixed
//  message is returned uniformly instead.
// ---------------------------------------------------------------------------
int MyForceDirectories(AnsiString Directory, AnsiString Function)            // common.cpp:1667
{
    AnsiString Str;
    if(Directory=="")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }

    if(Directory[Directory.Length()]!='\\')
    {
        DWORD attr=GetFileAttributes(Directory.c_str());
        if(attr!=INVALID_FILE_ATTRIBUTES)
        {
            if(!(attr & FILE_ATTRIBUTE_DIRECTORY))
                Directory=ExtractFilePath(Directory);                       // path exists: strip filename only if it's a file
        }
        else
        {
            AnsiString sExt=ExtractFileExt(Directory).LowerCase();          // only these 2 known file extensions get stripped (golden quirk, see banner above)
            if(sExt==".txt" || sExt==".csv")
                Directory=ExtractFilePath(Directory);
        }
    }

    if(Directory=="")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }
    else
    {
        try
        {
            if(DirectoryExists(Directory)==false)
            {
                ForceDirectories(Directory);
            }
        }
        catch(...)
        {
            Str.sprintf("%s -- %s", Directory, Function);
            ShowMyMessage("Create directory fail!", Str, Str);
            return -3;
        }
    }

    return 1;
}

// ===========================================================================
//  MyTickCount / MySleepEx / MySleep (common.cpp:1726-1796)
//  AI(W906-CommonCompletion) 20260721: un-gated -- 3 trivial timing wrappers.
//
//  MySleepEx CRITICAL NOTE: golden gates its live body behind
//  #ifdef USE_EC_CHANGE { ~50-line EC-change-report polling loop, needs
//  HGem/TQPF_Timer/iSleepCount/bRunTimer/bAlreadySleep, none translated }
//  #else return SleepEx(dwMilliseconds, bAlertable); #endif (common.cpp:
//  1737-1790). Confirmed USE_EC_CHANGE is commented out
//  (`//#define USE_EC_CHANGE`) in BOTH golden and this tree's MachineType.h,
//  so the #ifdef branch is genuinely dead code the preprocessor strips under
//  this build's config -- intentionally NOT ported (not silently dropped;
//  this is the one case in this wave where a whole branch is skipped rather
//  than translated, because it is unreachable regardless). Only the live
//  #else branch is translated below. golden's iSleepCount/bRunTimer/
//  bAlreadySleep/MySpeelTimer file-scope globals (common.cpp:1731-1734) exist
//  ONLY to serve that dead branch -- grepped the whole golden tree and
//  confirmed no other file references them (the same-named `bRunTimer`(1)/
//  `bRunTimer1` identifiers elsewhere are unrelated function-local statics or
//  class members) -- so they are correctly omitted here too, not just the
//  dead branch alone.
//
//  COLLISION FIX (required for these 3 to link): acarry_shims.cpp:152-153 and
//  ainarm9045_2x4_16_shims.cpp:69 previously carried offline stand-in
//  DEFINITIONS of exactly these 3 symbol names (as a substitute while these
//  common.cpp bodies were gated). Verified via those shim headers' own
//  comments (acarry_shims.h:219-232, ainarm9045_2x4_16_shims.h:89) that they
//  already cite these as "golden common.h:259-261" -- i.e. the callers only
//  ever needed a link-time symbol satisfying that exact declaration, so
//  removing the stand-in DEFINITIONS (while leaving each shim header's own
//  prototype re-declaration alone) needs zero caller changes. Those 2 stand-in
//  bodies are removed as part of this same change (see acarry_shims.cpp /
//  ainarm9045_2x4_16_shims.cpp) so the bodies below are the sole definitions
//  within the ht9045_core/ht9045_sm link set (the pair this plan named and
//  the pair CMakeLists.txt:1217-1219 confirms actually link together).
//  atester.cpp:1515's `static DWORD W7T1_MyTickCount()` + its `#define
//  MyTickCount W7T1_MyTickCount` macro-redirect is a separate, non-colliding
//  TU-local identifier (internal linkage) -- left untouched, out of scope
//  per plan. ADDITIONAL FINDING beyond the plan's own recon (reported, not
//  fixed -- out of scope this wave): MyPLC/MyPLC_IO_Modbus.cpp:61 ALSO
//  defines a global (non-static) `DWORD MySleepEx(...)`, in the ht9045_comms
//  library. This is a LATENT, pre-existing collision risk with the bodies
//  below -- NOT introduced by this change (ht9045_comms already defined it
//  unconditionally, before this wave, regardless of common.cpp's gating
//  state) -- that would only surface if some future target ever links BOTH
//  ht9045_core and ht9045_comms together; grepped tests/CMakeLists.txt +
//  this file: no current target does (test_MyCCLink links ht9045_comms/
//  ht9045_globals/ht9045_public/vclcompat, never ht9045_core), so it does
//  not affect this wave's build. Flagged here for whoever eventually wires
//  those two libraries together.
// ===========================================================================
DWORD MyTickCount()                                                          // common.cpp:1726
{
    return GetTickCount();
}

// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- MySleepEx
//  golden HT9011UC_Code_V3.33.906.0_20260618/common.cpp:1735-1791  (57 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN MySleepEx common.cpp:1735-1791
DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable)                          //Steven 20200807 : 趁著Sleep的時候去檢查EC change report
{
    #ifdef USE_EC_CHANGE
    int iFlag;
    bool bStopWhile=false;
    static int iInputCnt=0;
    int iLeftTime;

    if(bAlreadySleep==false)                                                    //避免多執行緒進來
    {
        bAlreadySleep=true;
    }
    else
    {
        return SleepEx(dwMilliseconds, bAlertable);
    }

    if(bRunTimer==false)
    {
        MySpeelTimer.SetMSAndOn(dwMilliseconds);
        MySpeelTimer.LatchCycleTime(true);
        bRunTimer=true;
    }

    iInputCnt=iSleepCount;
    do
    {
        iFlag=HGem->DoReportECChange(iSleepCount);
        if(iFlag==1)
        {
            iSleepCount++;
            if(iSleepCount>=HGem->EC_ID->Count)
                iSleepCount=0;
        }

        if(iSleepCount==iInputCnt || iFlag!=1)                                  //假設已經繞了一圈, 或者不需要執行EC check, 就停止while loop
        {
            bStopWhile=true;
            break;
        }
    }
    while(MySpeelTimer.Off()==false && bStopWhile==false);

    if(bStopWhile==true)
    {
        iLeftTime=dwMilliseconds-MySpeelTimer.LatchCycleTime();

        if(iLeftTime>0)
            SleepEx(iLeftTime, bAlertable);
    }
    bRunTimer=false;
    bAlreadySleep=false;
    return 1;
    #else
    return SleepEx(dwMilliseconds, bAlertable);
    #endif
}
#endif // AI-W6C-GOLDEN-END MySleepEx common.cpp:1735-1791

DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable)                       // common.cpp:1735 -- Steven 20200807
{
    // golden's #ifdef USE_EC_CHANGE branch (EC-change-report polling loop) is
    // dead code under this build's config -- see banner above; not ported.
    return SleepEx(dwMilliseconds, bAlertable);
}

void MySleep(DWORD dwMilliseconds)                                           // common.cpp:1793
{
    ::Sleep(dwMilliseconds);
}

// ===========================================================================
//  TempChangeLog (common.cpp:1802-2037) -- Ifor 20190930
//  AI(W906-CommonCompletion) 20260721: ATTEMPTED then RE-GATED. The plan's
//  premise ("zero current callers -> translating the body now is zero-risk
//  prep work") is WRONG, verified via a real build+link, not just reasoning:
//  the body reads asTempCtrl[] (defined cmydef.cpp) and TestIF_File (defined
//  cprod.cpp) directly -- both real, both already declared/visible via
//  cmydef.h/cprod.h -- but those two .cpp files are compiled into the
//  ht9045_globals static library, which ht9045_core (common.cpp's own
//  library) deliberately does NOT link (see this project's CMakeLists.txt
//  ht9045_core banner / cContact.cpp's own "confirmed empirically: the clean
//  build below links fine without it" precedent -- ht9045_globals is not
//  linked into any executable yet because its active globals' constructors
//  reach still-gated regions). Static-library linking is per-.o-file, not
//  per-function: once ANY symbol in common.cpp.obj is referenced (which
//  happens the moment any executable calls ANY other common.cpp function --
//  WriteIniData, OpenIniFile, etc.), the WHOLE object file is pulled in,
//  including TempChangeLog's asTempCtrl/TestIF_File references, regardless
//  of whether TempChangeLog itself is ever called. Un-gating this produced 2
//  concrete link failures (`undefined reference to asTempCtrl` /
//  `TestIF_File`) in test_common.exe and test_ini_helpers.exe the moment
//  they linked ht9045_core. Re-gated rather than also wiring
//  target_link_libraries(ht9045_core ... ht9045_globals) project-wide --
//  that is a much larger structural change (every ht9045_core consumer,
//  which is most of the test suite) than this wave's stated scope, and the
//  existing banner explicitly warns it isn't safe yet. Genuinely still
//  blocked -- NOT stale info, contrary to this wave's plan text; left gated
//  exactly as before this wave, draft body below preserved unchanged (a
//  faithful, verified-correct translation -- see the body itself and its own
//  inline comments -- ready to un-gate the moment ht9045_core can safely
//  reach ht9045_globals, or TestIF_File/asTempCtrl gain a lighter-weight
//  home).
// ===========================================================================
#if 0 // TODO(wave-logging): TempChangeLog (common.cpp:1802-2037) -- body verified correct, blocked on ht9045_core/ht9045_globals link boundary, see banner above
AnsiString __fastcall TempChangeLog(AnsiString Group, AnsiString Name)       // common.cpp:1802
{
    int iSiteAdd=0;
    AnsiString asATCTempName[32]=   {"Aa1", "Ab1", "Ac1", "Ad1",
                                     "Ae1", "Af1", "Ag1", "Ah1",
                                     "Ba1", "Bb1", "Bc1", "Bd1",
                                     "Be1", "Bf1", "Bg1", "Bh1",
                                     "Aa2", "Ab2", "Ac2", "Ad2",
                                     "Ae2", "Af2", "Ag2", "Ah2",
                                     "Ba2", "Bb2", "Bc2", "Bd2",
                                     "Be2", "Bf2", "Bg2", "Bh2"
                                    };

    AnsiString asIniDelayName[10]=  {"Every first devices",
                                     "After ShowAlarm Message",
                                     "After Auto Clean Function",
                                     "When happen tested time below",
                                     "After Open Heat Door",
                                     "When Press Stop Over",
                                     "When No Full Site",
                                     "EOT monitor time",
                                     "OTD unlock",
                                     "SOT monitor time"
                                    };

    AnsiString asAutoCleanSpeed[4]= {"Auto Clean Input Arm Speed",
                                     "Auto Clean Shuttle Speed",
                                     "Auto Clean Index Arm Speed",
                                     "Auto Clean Input Arm Z Speed"
                                    };

    if(Group.Pos("AmbientHotLowOffSet")     ==1     ||                       // Ifor 20190930 : add Temp Change Site
       Group.Pos("AmbientHotMidOffSet")     ==1     ||
       Group.Pos("Low OffSet")              ==1     ||
       Group.Pos("Mid. OffSet")             ==1     ||
       Group.Pos("High OffSet")             ==1     ||
       Group.Pos("User OffSet")             ==1     ||
       Group.Pos("SingleTempLimit")         ==1     ||
       Group.Pos("Init Temp OffSet")        ==1     ||
       Group.Pos("TestOverTime Temp OffSet")==1     )
    {
        if(Name.Pos("CH")==1)                                                // Ifor 20191015 : Fix temp sw. testing use non-CH parameter causes exception unable to write file
        {
            iSiteAdd=atoi(Name.SubString(3,Name.Length()-2).c_str());
            if(Group.Pos("SingleTempLimit")==1)                              // Ifor 20190930 : add Single Temp Limit index-1
            {
                Name=asTempCtrl[iSiteAdd];
            }
            else
            {
                Name=asTempCtrl[iSiteAdd-1];
            }
        }
    }
    else if(Group.Pos("ATC")==1 && Name.Pos("ATCTempOffset")==1)
    {
        iSiteAdd=atoi(Name.SubString(15,Name.Length()-15).c_str());

        if(TestIF_File.iTestMode==_8Site2X4 && TestIF_File.bOctal_16Kit==true) // JerryYang 20230828 : fix change log
        {
            if(iSiteAdd>=2)                                                  // avoid negative index
            {
                iSiteAdd-=2;
            }
        }

        Name="OffSet_"+asATCTempName[iSiteAdd];
    }
    else if(Group.Pos("InitialMode")==1)
    {
        if(Name.Pos("iInitialDelay")==1)
        {
            if(Name.Pos("iInitialDelay_")==1)
            {
                iSiteAdd=atoi(Name.SubString(15,Name.Length()-14).c_str())-1;
            }
            else
            {
                iSiteAdd=0;
            }
            Name=asIniDelayName[iSiteAdd];
        }
        else if(Name.Pos("dInitialDelay_10")==1)
        {
            iSiteAdd=atoi(Name.SubString(15,Name.Length()-14).c_str())-1;
            Name=asIniDelayName[iSiteAdd];
        }
        else if(Name=="bEveryFirstDeviceUseInitialDelay")
        {
            Name="bEvery First Device Use Initial Delay";
        }
        else if(Name=="bAfterShowAlarmMessageUseInitialDelay")
        {
            Name="bAfter Show Alarm Message Use Initial Delay";
        }
        else if(Name=="bWhenHappenTestedTimeBelowUseInitialDelay")
        {
            Name="bWhen Happen Tested Time Below Use Initial Delay";
        }
        else if(Name=="bAfterAutoCleanFunctionUseInitialDelay")
        {
            Name="bAfter Auto Clean Function Use Initial Delay";
        }
        else if(Name=="bAfterOpenHeatDoorUseInitialDelay")
        {
            Name="bAfter Open Heat Door Use Initial Delay";
        }
        else if(Name=="bWhenPressStopOverUseInitialDelay")
        {
            Name="bWhen Press Stop Over Use Initial Delay";
        }
        else if(Name=="bTestFinishToNextTestOver")
        {
            Name="bEOT monitor time Use Initial Delay";
        }
        else if(Name=="bTestStartToNextTestStart")
        {
            Name="bSOT monitor time Use Initial Delay";
        }
        else if(Name=="bOTDUnlockDelay")
        {
            Name="bOTD Unlock Use Initial Delay";
        }
        else if(Name=="iEveryFirstDeviceUseInitialDelay")
        {
            Name="When happen tested time below Trigger time (Sec)";
        }
        else if(Name=="iWhenPressStopOver")
        {
            Name="When Press Stop Over Trigger time (Sec)";
        }
        else if(Name=="iTestFinishToNextTestOver")
        {
            Name="EOT monitor time Trigger time (Sec)";
        }
        else if(Name=="dTeststartToNextTestStart")
        {
            Name="SOT monitor time Trigger time (Sec)";
        }
    }
    else if(Group.Pos("Time")==1)
    {
        if(Name=="Stary Delay")
        {
            Name="Start Delay";
        }
        else if(Name=="Initial Stary Delay")
        {
            Name="Initial Start Delay";
        }
        else if(Name=="Initial Stary Delay CT")
        {
            Name="Initial Start Delay Count";
        }
    }
    else if(Group.Pos("Mode")==1)
    {
        if(Name=="fSocketInitialICCheckPositionOffset")
        {
            Name="fSocket Initial IC Check Position Offset";
        }
        else if(Name=="iSocketInitialICCheckPosition")
        {
            Name="iSocket Initial IC Check Position";
        }
    }
    else if(Group.Pos("Configuration")==1)
    {
        if(Name.Pos("iAutoClean_MotorSpeed[")==1)
        {
            iSiteAdd=atoi(Name.SubString(23,1).c_str());
            Name=asAutoCleanSpeed[iSiteAdd];
        }
        else if(Name=="iAutoClean_AlarmCount")
        {
            Name="Auto Clean Alarm Count";
        }
        else if(Name=="iAutoClean_iPadThickness")
        {
            Name="Auto Clean Clean Pad Deviation";
        }
        else if(Name=="iAutoClean_ContactCleanHeight")
        {
            Name="Auto Clean Socket Position Offset";
        }
        else if(Name=="iAutoClean_IndexPickOffset")
        {
            Name="Auto Clean Index to Shuttle Pick Offset";
        }
        else if(Name=="iAutoClean_IndexReleaseOffset")
        {
            Name="Auto Clean Index to Shuttle Release Offset";
        }
        else if(Name=="iAutoClean_Shuttle1PickOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Pick Offset";
        }
        else if(Name=="iAutoClean_Shuttle1PlaceOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Place Offset";
        }
        else if(Name=="iAutoClean_Shuttle1XOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 X Offset";
        }
        else if(Name=="iAutoClean_Shuttle1YOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Y Offset";
        }
        else if(Name=="iAutoClean_Shuttle2PickOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Pick Offset";
        }
        else if(Name=="iAutoClean_Shuttle2PlaceOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Place Offset";
        }
        else if(Name=="iAutoClean_Shuttle2XOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 X Offset";
        }
        else if(Name=="iAutoClean_Shuttle2YOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Y Offset";
        }
        else if(Name=="ShuttlePitchOffset")
        {
            Name="Auto Clean In Arm Shuttle Pitch Offset";
        }
    }
    return Name;
}
#endif // TODO(wave-logging)

// ===========================================================================
//  Common_PathCombin -- TU-local static helper, NOT a partial FileInfo class.
//  AI(W906-CommonCompletion) 20260721: golden's GetRecipeFileName /
//  CopyAndCompressFile both call `FileInfo().PathCombin(...)`
//  (ProductionInfo/FileInfo.cpp:290-311), but `class FileInfo` does not exist
//  anywhere in this translated tree (ProductionInfo/FileInfo.cpp itself is
//  untranslated). Rather than dragging in the whole untranslated FileInfo
//  class for one small path-join helper, inline just that logic here as a
//  TU-local static function -- same "don't drag in a whole untranslated class
//  for one small piece of it" precedent already established by
//  Automation/SCK_ART_Remainder.cpp's W5SckArtRem_GetRecipePath/
//  W5SckArtRem_GetRecipeFileName/W5SckArtRem_MyForceDirectories stand-ins
//  (see that file's own Gate #1 comment block). Faithful copy of golden
//  FileInfo::PathCombin's body verbatim: append '/' for FTP-style paths
//  (already containing a '/', or already ending in '/'), else '\\' for local
//  paths, unless the base path already ends with the right separator; then
//  append the filename.
// ===========================================================================
static AnsiString Common_PathCombin(const AnsiString &sPath, const AnsiString &sFile)
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
            if(combinedPath[combinedPath.Length()]!='\\')
            {
                combinedPath+="\\";
            }
        }
    }
    combinedPath+=sFile;
    return combinedPath;
}

// ===========================================================================
//  GetRecipePath / GetRecipeFileName (common.cpp:2039-2048)
//  JimmyChiu 20220601 / Steven 20241030
//  AI(W906-CommonCompletion) 20260721: un-gated. GetRecipePath has zero
//  dependency beyond already-real DataPath/GetLastOpenFN(). GetRecipeFileName
//  uses Common_PathCombin (above) instead of golden's FileInfo().PathCombin
//  -- see that helper's banner for why. Golden itself keeps a commented-out
//  one-line equivalent inside GetRecipeFileName (common.cpp:2047); reproduced
//  below for provenance.
//
//  KNOWN INTEGRATION OPPORTUNITY (out of scope this wave): Automation/
//  SCK_ART_Remainder.cpp:120-138 has local W5SckArtRem_GetRecipePath /
//  W5SckArtRem_GetRecipeFileName stand-ins built specifically because these
//  were gated (explicit TODO(wave-path) comments there). Now that the real
//  functions exist, deleting those stand-ins and wiring that file to the real
//  functions below is a candidate for a future wave -- SCK_ART_Remainder.cpp
//  is intentionally left untouched this wave.
// ===========================================================================
AnsiString GetRecipePath()                                                   // common.cpp:2039
{
    return AnsiString().sprintf("%s%s\\", DataPath, GetLastOpenFN());
}

AnsiString GetRecipeFileName(AnsiString FileName)                            // common.cpp:2044
{
    return Common_PathCombin(AnsiString().sprintf("%s%s", DataPath, GetLastOpenFN()), FileName);
    // golden's own commented-out one-line equivalent (common.cpp:2047):
    // return AnsiString().sprintf("%s%s\\%s", DataPath, GetLastOpenFN(), FileName);
}

// ===========================================================================
//  SGDToCSV (common.cpp:2050-2064) -- Sam 20231106 / Sam 20240104
//  AI(W906-CommonCompletion) 20260721: un-gated. Iterates a TStringGrid's
//  RowCount/ColCount, reads Cells[iCol][iRow], StringReplace()s a split
//  character, appends via WriteDataToFile (already real, landed earlier
//  today). vclcompat::TStringGrid's Cells[ACol][ARow] is confirmed 0-based on
//  BOTH axes (vclcompat/StringGrid.h's own file-head note) -- matches golden's
//  loop exactly (both loops start at 0, no header-row skip here; golden's own
//  "skip row 0" idiom used elsewhere, e.g. uHGemEquipment.cpp, is a caller
//  choice, not something SGDToCSV itself does), so the loop bounds/indexing
//  below are a direct, unmodified translation.
// ===========================================================================
void SGDToCSV(vclcompat::TStringGrid *strGrid, AnsiString sSplit, AnsiString sReplaceSplit, AnsiString Path)  // common.cpp:2050
{
    AnsiString sTemp="", sStr="";

    for(int iRow=0; iRow<strGrid->RowCount; iRow++)
    {
        sTemp="";
        for(int iCol=0; iCol<strGrid->ColCount; iCol++)
        {
            sStr=StringReplace(strGrid->Cells[iCol][iRow], sSplit, sReplaceSplit, TReplaceFlags());  // common.cpp:2059 -- NOTE: no rfReplaceAll here (golden's own choice -- only the FIRST occurrence is replaced)
            sTemp+=sStr+sSplit;
        }
        WriteDataToFile(Path, sTemp);
    }
}

// ===========================================================================
//  IsFileInUse (common.cpp:2066-2090) -- Jimmychiu 20241121
//  AI(W906-CommonCompletion) 20260721: un-gated -- pure Win32
//  CreateFile/GetLastError/CloseHandle, zero dependency gap. Zero current
//  callers.
// ===========================================================================
bool IsFileInUse(const char* filePath)                                       // common.cpp:2066
{
    HANDLE hFile=CreateFile(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(hFile==INVALID_HANDLE_VALUE)
    {
        DWORD error=GetLastError();
        if(error==ERROR_SHARING_VIOLATION || error==ERROR_LOCK_VIOLATION)
        {
            return true;
        }
    } else
    {
        CloseHandle(hFile);
    }
    return false;
}

// ===========================================================================
//  CopyAndCompressFile (common.cpp:2092-2120)
//  AI(W906-CommonCompletion) 20260721: un-gated. Calls Common_PathCombin
//  TWICE (same helper as GetRecipeFileName above -- reused, not duplicated a
//  third time) + MySleep(iDelayMS) (real as of this same wave, Item 9 above)
//  + plain Win32 FileExists/CopyFile against hardcoded d:\HT9045\7z.exe /
//  C:\Program Files\7-Zip\7z.exe paths (preserved exactly, not parameterized).
//
//  DEVIATION from this wave's plan text (which asserted ShellExecute is a
//  "plain Win32 ... no gap" call): VERIFIED this is NOT true in this tree --
//  Interface/TesterTCP.cpp's CopyRecipeToTester/CopyRecipeFromTester (golden
//  TesterTCP.cpp:1070/1089) already hit this EXACT same golden idiom
//  (ShellExecute against this EXACT same d:\HT9045\7z.exe) and explicitly
//  documented (TesterTCP.cpp:144-146) that "VCL ShellExecute has no
//  vclcompat/portable-C++ equivalent anywhere in this tree (would pull in a
//  new <shellapi.h>+shell32 link dependency not shared by anything else
//  translated so far)". Reused that exact same established project idiom here
//  instead of re-deriving a third variant: a detached `cmd /c start "" /B`
//  launch via system() (async fire-and-forget, matching golden's own
//  ShellExecute+MySleep(iDelayMS) best-effort throttle semantics -- NOT a
//  blocking call). Zero current callers of this function either way.
// ===========================================================================
bool CopyAndCompressFile(const AnsiString &sourcePath, const AnsiString &targetPath, const AnsiString &sSourFileName, const AnsiString &sTarFileName, const int &iDelayMS, AnsiString &sMsg)  // common.cpp:2092
{
    AnsiString sSourceFilePath=Common_PathCombin(sourcePath, sSourFileName);
    AnsiString sTargetFilePath=Common_PathCombin(targetPath, sTarFileName);
    if(FileExists(sSourceFilePath))
    {
        if(FileExists("d:\\HT9045\\7z.exe")==false)                          // common.cpp:2098 -- prepare 7-Zip
        {
            CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe", false); // common.cpp:2100
        }
        AnsiString Str=AnsiString().sprintf("a -tzip \"%s\" \"%s\"", sTargetFilePath, sSourceFilePath);

        // golden: ShellExecute(NULL,"open","d:\\HT9045\\7z.exe",Str.c_str(),NULL,SW_HIDE);
        // -- see banner above for why this is routed through system() instead.
        AnsiString cmd;
        cmd.sprintf("d:\\HT9045\\7z.exe %s", Str);
        AnsiString shellCmd;
        shellCmd.sprintf("start \"\" /B %s", cmd);
        system(shellCmd.c_str());

        MySleep(iDelayMS);                                                   // common.cpp:2104
        if(FileExists(sTargetFilePath))
        {
            sMsg=AnsiString().sprintf("Copy file from %s to %s .", sSourceFilePath, sTargetFilePath);
            return true;
        }
        else
        {
            sMsg=AnsiString().sprintf("Failed to copy file from %s to %s .", sSourceFilePath, sTargetFilePath);
        }
    }
    else
    {
        sMsg=AnsiString().sprintf("File %s does not exists!", sSourceFilePath);
    }
    return false;
}

// ===========================================================================
//  ChangeSaveFileName (common.cpp:2124-2183) -- ben 20240927
//  AI(W906-CommonCompletion) 20260721: un-gated. Pure AnsiString::Pos +
//  StringReplace(...,TReplaceFlags()<<rfReplaceAll) chain sanitizing
//  \ / : * ? " < > | characters. All primitives already exist in
//  vclcompat/SysUtils.h. Zero current callers.
//
//  GOLDEN BUG, preserved verbatim (found and verified via a careful reading of
//  the raw source bytes, not assumed): the `"` (double-quote) branch below
//  uses the literal token `""""` (four raw double-quote characters, no
//  backslash escape) as both the Pos() search string and the StringReplace()
//  OldPattern. Under standard C++ string-literal grammar (this file has
//  always been compiled as C++, not Pascal, so BCB6 doubled-quote escaping
//  does not apply) two adjacent `""` empty-string literals concatenate to a
//  single empty string "" -- so this branch actually checks/replaces an EMPTY
//  pattern, not a literal `"` character. Per this tree's own StringReplace
//  contract ("An empty OldPattern is a no-op") and BCB6's own Pos("")==0
//  convention, `asFileName.Pos("""")` is always 0, so `!=0` is always false --
//  this whole branch is DEAD CODE in golden itself. The apparent intent (strip
//  embedded `"` characters from a save filename, matching the other 8
//  separator branches) never actually fires. Reproduced verbatim below,
//  4-quote token included, rather than "fixed" to `"\""` -- that would be a
//  silent behavior change from what golden's compiled binary actually does.
// ===========================================================================
AnsiString __fastcall ChangeSaveFileName(AnsiString asFileName)              // common.cpp:2124
{
    AnsiString Str1;

    if(asFileName.Pos("\\")!=0)
    {
        Str1=StringReplace(asFileName, "\\", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("/")!=0)
    {
        Str1=StringReplace(asFileName, "/", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos(":")!=0)
    {
        Str1=StringReplace(asFileName, ":", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("*")!=0)
    {
        Str1=StringReplace(asFileName, "*", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("?")!=0)
    {
        Str1=StringReplace(asFileName, "?", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("""")!=0)                                              // golden bug: always-false dead branch -- see banner above
    {
        Str1=StringReplace(asFileName, """", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("<")!=0)
    {
        Str1=StringReplace(asFileName, "<", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos(">")!=0)
    {
        Str1=StringReplace(asFileName, ">", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    if(asFileName.Pos("|")!=0)
    {
        Str1=StringReplace(asFileName, "|", "_", TReplaceFlags()<<rfReplaceAll);
        asFileName=Str1;
    }

    return asFileName;
}

// ===========================================================================
//  MySecondsBetween (common.cpp:2188-2199) -- Sam 20251031
//  AI(W906-CommonCompletion) 20260721: un-gated. TDateTime subtraction +
//  DecodeDate/DecodeTime, both already in vclcompat/TDateTime.h and brought
//  into the global namespace via vcl_compat.h. Zero current callers.
// ===========================================================================
int __fastcall MySecondsBetween(TDateTime DT1, TDateTime DT2)                // common.cpp:2188
{
    int iTempSec=0;
    Word year, month, day;
    Word hour, min, sec, msec;
    TDateTime TempDT;
    TempDT=DT2-DT1;
    DecodeDate(TempDT, year, month, day);
    DecodeTime(TempDT, hour, min, sec, msec);
    iTempSec=day*24*60*60+hour*60*60+min*60+sec;
    return iTempSec;
}
