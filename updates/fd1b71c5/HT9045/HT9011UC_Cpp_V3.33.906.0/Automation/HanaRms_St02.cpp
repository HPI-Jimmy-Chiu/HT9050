// ===========================================================================
//  Automation/HanaRms_St02.cpp -- golden 912 HANA RMS Interlock: Automation/automation.cpp:2514-3158 except
//  PrepareHANARMSConnect (HanaRms_Prepare_St02.cpp), plus GetTimeInfo (:280-289).
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  Steven W64 = A (1002 08:0x): port 912.0; where 912 has a RogerYang
//  comment, the comment wins.  Header, file map and the numbered [W906] list: Automation/HanaRms_St02.h.
//  Golden: D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\automation.cpp (Big5, read as cp950); comments
//  below are golden's, decoded to UTF-8.
//
//  [912 known defect] kept as 912.0 (HUMAN_REVIEW C; ht9045-hana-rms-interlock skill sections 2.1 / 4 / 7.1 / 8 say what
//  RogerYang changed after 912.0 -- that tree is not in this repo, and no 912.0 RogerYang comment asks for it):
//    K1  one connect, at WakeupGPIB (G9) only; the HDServer closes an idle link after 30 s, nothing reconnects, later
//        commands log "[WARN] not connected, not sent" (HANARMSSendCmd).
//    K2  HDNAME = HANARMS.ini [Setting] HDName, default "HD_01" (not the machine ID) -- HANARMSClientRead's auto chain
//        and HANARMSQueryJobInfo; the button templates hard-code HD_01.
//    K3  LoadHANARMSSetting defaults 127.0.0.1 / 6670; 127.0.0.1 means "not configured" to PrepareHANARMSConnect.
//    K4  HANARMS_ContactMethod: index 9 (TMoveSlowContact) is RELEASE (912.0 code AND its RogerYang comment
//        {1,4,6,8,9,10}; the skill's 20260930 "9 -> DIRECT" is 912.2).
//    K5  HANARMSSendCmd logs [TX] before it knows whether the line can be sent.
//    K6  the CHECKSTR check also warns on EVENT_REPORT_REP (which has no CHECKSTR).
//    K7  the [WARN] text says "A76" although the flag is A77 (A76 went to SPIL in 912) -- text kept verbatim.
//    K8  btnRMS_GetLotClick only fills a template (912.1 made it the production query).
// ===========================================================================
#include "MachineDefine.h"      // the tree's include hub (golden automation.cpp:1)
#include "Automation/HanaRms_St02.h"
#include "Automation/HanaRms_Internal_St02.h"
#include "atester_shims.h"      // fAutomation (golden automation.cpp:2829-2830 writes fAutomation->sHANARMSJobInfoLine)
#include "MachineType.h"        // CC_HANA_MICRON, eBinFT / eBinRT, TEST_MAX_BIN, DropContact..TMoveDropSlowContact, rsmAutoSiteMap
#include "cprod.h"              // TestIF, DeviceForm_File, Temperature, BinSelect; IniConfig (Config.h)
#include "cmydef.h"             // CUSTOMER_CODE, Tempture_Hot / Tempture_AmbientHot, SystemYear .. SystemSec
#include "LastSet.h"            // LastSet.iTemperature / iRunStartMode
#include "Config.h"             // IniConfig.bA77_EnableHanaRMSInterlock
#include "common.h"             // MyForceDirectories, WriteDataToFile, ReadIniData, WriteIniData, GetLastOpenFN, AuthPath, as9045LogPath
#include "canary_support.h"     // ShowMyMessage (golden mymessbox.h:58)

#include <cstddef>              // NULL
#include <cstdlib>              // atoi / atof

using w906hanarms::HANARMSAddLog;

// ---------------------------------------------------------------------------------------------------------------------
//  [W906] 3: the two golden literal paths, on the tree's existing seams (unset = the golden value byte for byte)
// ---------------------------------------------------------------------------------------------------------------------
AnsiString w906hanarms::LogDir()  { return as9045LogPath + "\\HANARMS"; }   // golden "D:\\HT9045_Log\\HANARMS" (as9045LogPath = common.cpp:240, golden common.cpp:48)
AnsiString w906hanarms::IniPath() { return AuthPath + "HANARMS.ini"; }       // golden "D:\\HT9045\\config\\HANARMS.ini" (AuthPath = common.cpp:168, golden common.cpp:31 "D:\\HT9045\\config\\")

// ---------------------------------------------------------------------------------------------------------------------
//  golden 912 automation.cpp:280-289 -- the TfAutomation member every HANA RMS body calls for its log time stamp
// ---------------------------------------------------------------------------------------------------------------------
AnsiString THanaRmsMembers::GetTimeInfo()
{
    static TDateTime dtPresent;
    AnsiString TimeString;
    dtPresent= Now();
    DecodeDate(dtPresent, aSystemYear, aSystemMonth, aSystemDate);
    DecodeTime(dtPresent, aSystemHour, aSystemMin, aSystemSec, aSystemMSec);
    TimeString.sprintf("%04d%02d%02d%02d%02d%02d", aSystemYear, aSystemMonth, aSystemDate, aSystemHour, aSystemMin, aSystemSec);
    return TimeString;
}

//---------------------------------------------------------------------------
//RogerYang 20260727 : ===== HANA RMS Interlock simulator (only fAutomation touched) =====
//  Role      : this Handler acts as TCP CLIENT, connects out to HANA HDServer.
//  Protocol  : key="value" pairs, space separated, message ends with CHECKSTR="END" then '\n'.
//              (per HANA HDServer Communication Spec 20260720 ; ASCII, Korean excluded)
//  Isolation : own HANARMSClient socket + own parse ; does NOT reuse OLP (STX/ETX) path.
//  NOTE      : HANA server IP/Port are NOT given in HANA documents -> must be confirmed by HANA.
static AnsiString g_asHANARMSRecvBuf = "";   //RogerYang 20260727 : RMS receive accumulation, split by '\n'   (golden :2520)

//RogerYang 20260727 : append one line to the RMS log memo (auto-trim when too long)
void w906hanarms::HANARMSAddLog(TMemo *mmo, AnsiString asTag, AnsiString asMsg)   // golden :2523-2540 (file-static there; [W906] shared, HanaRms_Internal_St02.h)
{
    try                                                                         //RogerYang 20260902 : 逐行落地日檔(memo 滿1000行自清,檔案才是除錯依據)
    {
        AnsiString asDir = w906hanarms::LogDir();                               // [W906] 3: golden AnsiString asDir = "D:\\HT9045_Log\\HANARMS";
        MyForceDirectories(asDir);
        AnsiString asFile;
        asFile.sprintf("%s\\HANARMS_%04d%02d%02d.txt", asDir.c_str(), SystemYear, SystemMonth, SystemDate);
        WriteDataToFile(asFile, asTag + asMsg);
    }
    catch(...)
    {
    }
    if(mmo == NULL) return;
    if(mmo->Lines->Count > 1000)
        mmo->Lines->Clear();
    mmo->Lines->Add(asTag + asMsg);
}
//---------------------------------------------------------------------------
//RogerYang 20260727 : quote-aware key="value" extractor for HANA RMS parsing.
//  - key match is CASE-INSENSITIVE (RCP_Name vs RCP_NAME both hit).
//  - tries the 0722 dot key first, then underscore variant as fallback (FT.C_FAIL_SCK / FT_C_FAIL_SCK).
//  - bFound reports whether the key exists at all ; value is returned quote-stripped.
static AnsiString HANARMSGetKV(AnsiString asLine, AnsiString asKey, bool &bFound)   // golden :2546-2592
{
    //RogerYang 20260827 : 取 key="value" 的值。容忍 = 前後與值前後空白、大小寫不分、點號找不到改底線
    bFound = false;
    AnsiString asUpLine = asLine.UpperCase();
    int iLen = asLine.Length();
    for(int k=0; k<2; k++)
    {
        AnsiString asK = asKey;
        if(k == 1)                                       //點號轉底線再試
        {
            asK = "";
            for(int i=1; i<=asKey.Length(); i++)
                asK += (asKey[i]=='.') ? '_' : asKey[i];
            if(asK == asKey) break;
        }
        AnsiString asKU = asK.UpperCase();
        int iFrom = 1;
        while(iFrom <= iLen)
        {
            int p = asUpLine.SubString(iFrom, iLen).Pos(asKU);
            if(p <= 0) break;
            int iKeyStart = iFrom + p - 1;
            int i = iKeyStart + asKU.Length();
            //key 與開頭引號間只允許空白與一個 '='
            bool bSeenEq = false, bOK = true;
            while(i <= iLen && asLine[i] != '"')
            {
                char c = asLine[i];
                if(c == ' ' || c == '\t') { i++; continue; }
                if(c == '=' && !bSeenEq)  { bSeenEq = true; i++; continue; }
                bOK = false; break;
            }
            if(bOK && bSeenEq && i <= iLen && asLine[i]=='"')
            {
                i++;
                int iValStart = i;
                while(i <= iLen && asLine[i] != '"') i++;
                bFound = true;
                AnsiString v = (i > iValStart) ? asLine.SubString(iValStart, i-iValStart) : AnsiString("");
                return v.Trim();                          //去值前後空白
            }
            iFrom = iKeyStart + asKU.Length();            //繼續找(避開子字串誤中)
        }
    }
    return "";
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : CON_MTD - ContactMode 分組 (RELEASE=Drop系{1,4,6,8,9,10} / 其餘 DIRECT)
static AnsiString HANARMS_ContactMethod(int iContactMode)   // golden :2595-2609
{
    switch(iContactMode)
    {
        case DropContact:                   //1  Drop
        case TMoveDrop:                     //4  TMOVE Drop
        case DropContactSoftEP:             //6  Drop & Soft EP
        case DropContactModeDiffentSpeed:   //8  Drop & Slow
        case TMoveSlowContact:              //9  TMOVE Slow      [912 known defect] K4: 912.0 code and comment say RELEASE (kept)
        case TMoveDropSlowContact:          //10 TMOVE Drop & Slow
            return "RELEASE";
        default:                            //0,2,3,5,7
            return "DIRECT";
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : LOW_SPD - 慢放模式回 "E",否則 "-"
static AnsiString HANARMS_LowSpeed(int iContactMode)   // golden :2612-2624
{
    switch(iContactMode)
    {
        case DirectContactModeDiffentSpeed: //2  Direct & Slow
        case DropContactModeDiffentSpeed:   //8  Drop & Slow
        case TMoveSlowContact:              //9  TMOVE Slow
        case TMoveDropSlowContact:          //10 TMOVE Drop & Slow
            return "E";
        default:
            return "-";
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : 去掉字串內全部空白 (SB 逗號串比對用)
static AnsiString HANARMS_StripSpace(AnsiString s)   // golden :2627-2633
{
    AnsiString r = "";
    for(int i=1; i<=s.Length(); i++)
        if(s[i] != ' ') r += s[i];
    return r;
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : 單欄字串比對。欄位未送或 "-" 皆跳過;不符則把原因累加到 asFail
static bool HANARMS_CmpEq(AnsiString asName, AnsiString asServer, bool bFound, AnsiString asHandler, AnsiString &asFail)   // golden :2636-2645
{
    if(bFound==false || asServer=="-")
        return true;
    if(asServer == asHandler)
        return true;
    if(asFail.Length()) asFail += " ; ";
    asFail += asName + "(S=" + asServer + "/H=" + asHandler + ")";
    return false;
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : 單欄數值比對(完全相等,容 0.0005 浮點誤差)
static bool HANARMS_CmpNum(AnsiString asName, AnsiString asServer, bool bFound, double dHandler, AnsiString &asFail)   // golden :2648-2659
{
    if(bFound==false || asServer=="-")
        return true;
    double d = atof(asServer.c_str()) - dHandler;
    if(d < 0) d = -d;
    if(d < 0.0005)
        return true;
    if(asFail.Length()) asFail += " ; ";
    asFail += asName + "(S=" + asServer + AnsiString().sprintf("/H=%g)", dHandler);
    return false;
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : 取 Special Bin 逗號串(bin 與 count 各一串,遞增排序);未啟用回 "-"
static void HANARMS_GetSpecialBinStr(bool bByArm, int iBinIdx, AnsiString &asBin, AnsiString &asCnt)   // golden :2662-2679
{
    asBin = ""; asCnt = "";
    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        bool bOn = bByArm ? BinSelect[iBinIdx].bSpecialBinByArm[i]
                          : BinSelect[iBinIdx].bSpecialBinBySocket[i];
        if(bOn)
        {
            unsigned int iCnt = bByArm ? BinSelect[iBinIdx].iSpecialBinCountByArm[i]
                                       : BinSelect[iBinIdx].iSpecialBinCountBySocket[i];
            if(asBin.Length()) { asBin += ","; asCnt += ","; }
            asBin += IntToStr(i);
            asCnt += IntToStr((int)iCnt);
        }
    }
    if(asBin == "") { asBin = "-"; asCnt = "-"; }
}
//---------------------------------------------------------------------------
//RogerYang 20260827 : GET_JOB_INFO_REP 與本機 recipe 逐欄比對。true=全過;asFail 列出所有不符
//  規則:數值完全相等、"-"跳過、AT_OFF 無 RT 變體故 FT/RT 比同一旗標。細節見 HANA RMS 技能
static bool HANARMSCompare(AnsiString asLine, AnsiString &asFail)   // golden :2683-2769
{
    bool bOK = true;
    bool bF;
    AnsiString sv, hb, hc;
    asFail = "";

    sv = HANARMSGetKV(asLine, "RCP_NAME", bF);
    bOK &= HANARMS_CmpEq("RCP_NAME", sv, bF, GetLastOpenFN(), asFail);

    sv = HANARMSGetKV(asLine, "GB_CT", bF);
#if 0 // GATE(W906-ST02-C10) [W906] 4: fMain->hanaART is TfMainHanaART (forms/fMain.h:80-96), no GetHD_CT; uHANA_ART (Automation/HANA_ART.cpp:287) has no instance -- golden 912 automation.cpp:2694
    bOK &= HANARMS_CmpEq("GB_CT", sv, bF, fMain->hanaART->GetHD_CT(), asFail);
#else
    bOK &= HANARMS_CmpEq("GB_CT", sv, bF, AnsiString("(N/A)"), asFail);       // [W906] 4: the Handler value is unknown -> a GB_CT the server sends (anything but "-") FAILS; "-" / not sent = skipped as golden
#endif

    sv = HANARMSGetKV(asLine, "TEMP", bF);                                      //"-"=室溫;數值依模式(同 DoSetUpInfo)
    if(bF)
    {
        if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
            bOK &= HANARMS_CmpNum("TEMP", sv, true, Temperature.fWorkTemperBase, asFail);
        else if(Temperature.bUseAbitCHK)
            bOK &= HANARMS_CmpNum("TEMP", sv, true, Temperature.fAbitTemp, asFail);
        else if(sv != "-")
        {
            bOK = false;
            if(asFail.Length()) asFail += " ; ";
            asFail += "TEMP(S=" + sv + "/H=RoomTemp)";
        }
    }

    sv = HANARMSGetKV(asLine, "PIN_CNT", bF);
    bOK &= HANARMS_CmpNum("PIN_CNT", sv, bF, (double)DeviceForm_File.iPinCT, asFail);

    sv = HANARMSGetKV(asLine, "PIN_FC", bF);                                    //單位 gf
    bOK &= HANARMS_CmpNum("PIN_FC", sv, bF, DeviceForm_File.ForcePerPinG, asFail);

    sv = HANARMSGetKV(asLine, "CON_MTD", bF);
    bOK &= HANARMS_CmpEq("CON_MTD", sv, bF, HANARMS_ContactMethod(DeviceForm_File.ContactMode), asFail);

    sv = HANARMSGetKV(asLine, "WIN_CNT", bF);
    bOK &= HANARMS_CmpNum("WIN_CNT", sv, bF, (double)TestIF.iSlidingWindowSize, asFail);

    sv = HANARMSGetKV(asLine, "LOW_SPD", bF);
    bOK &= HANARMS_CmpEq("LOW_SPD", sv, bF, HANARMS_LowSpeed(DeviceForm_File.ContactMode), asFail);

    //--- FT ---
    sv = HANARMSGetKV(asLine, "FT.C_FAIL_SCK", bF);
    bOK &= HANARMS_CmpNum("FT.C_FAIL_SCK", sv, bF, (double)TestIF.iContsFailSocketAlarmCT, asFail);
    sv = HANARMSGetKV(asLine, "FT.S_T_S", bF);
    bOK &= HANARMS_CmpNum("FT.S_T_S", sv, bF, TestIF.dFailAlarmSiteYield, asFail);
    sv = HANARMSGetKV(asLine, "FT.LOW_YD", bF);
    bOK &= HANARMS_CmpNum("FT.LOW_YD", sv, bF, TestIF.dLowYieldLimit, asFail);
    sv = HANARMSGetKV(asLine, "FT.AT_OFF", bF);
    bOK &= HANARMS_CmpEq("FT.AT_OFF", sv, bF, TestIF.bLowYieldAutoSiteOff ? AnsiString("E") : AnsiString("-"), asFail);

    HANARMS_GetSpecialBinStr(true, eBinFT, hb, hc);
    sv = HANARMSGetKV(asLine, "FT.SB_ARM", bF);
    bOK &= HANARMS_CmpEq("FT.SB_ARM", HANARMS_StripSpace(sv), bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "FT.SB_ARM_CNT", bF);
    bOK &= HANARMS_CmpEq("FT.SB_ARM_CNT", HANARMS_StripSpace(sv), bF, hc, asFail);
    HANARMS_GetSpecialBinStr(false, eBinFT, hb, hc);
    sv = HANARMSGetKV(asLine, "FT.SB_SCK", bF);
    bOK &= HANARMS_CmpEq("FT.SB_SCK", HANARMS_StripSpace(sv), bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "FT.SB_SCK_CNT", bF);
    bOK &= HANARMS_CmpEq("FT.SB_SCK_CNT", HANARMS_StripSpace(sv), bF, hc, asFail);

    //--- RT ---
    sv = HANARMSGetKV(asLine, "RT.C_FAIL_SCK", bF);
    bOK &= HANARMS_CmpNum("RT.C_FAIL_SCK", sv, bF, (double)TestIF.iContsFailSocketAlarmCT_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.S_T_S", bF);
    bOK &= HANARMS_CmpNum("RT.S_T_S", sv, bF, TestIF.dFailAlarmSiteYield_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.LOW_YD", bF);
    bOK &= HANARMS_CmpNum("RT.LOW_YD", sv, bF, TestIF.dLowYieldLimit_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.AT_OFF", bF);
    bOK &= HANARMS_CmpEq("RT.AT_OFF", sv, bF, TestIF.bLowYieldAutoSiteOff ? AnsiString("E") : AnsiString("-"), asFail);

    HANARMS_GetSpecialBinStr(true, eBinRT, hb, hc);
    sv = HANARMSGetKV(asLine, "RT.SB_ARM", bF);
    bOK &= HANARMS_CmpEq("RT.SB_ARM", HANARMS_StripSpace(sv), bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "RT.SB_ARM_CNT", bF);
    bOK &= HANARMS_CmpEq("RT.SB_ARM_CNT", HANARMS_StripSpace(sv), bF, hc, asFail);
    HANARMS_GetSpecialBinStr(false, eBinRT, hb, hc);
    sv = HANARMSGetKV(asLine, "RT.SB_SCK", bF);
    bOK &= HANARMS_CmpEq("RT.SB_SCK", HANARMS_StripSpace(sv), bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "RT.SB_SCK_CNT", bF);
    bOK &= HANARMS_CmpEq("RT.SB_SCK_CNT", HANARMS_StripSpace(sv), bF, hc, asFail);

    return bOK;
}
//---------------------------------------------------------------------------
//RogerYang 20260727 : print one parsed field, indented under the [RX] line.
//  "-" is shown as (not used) ; asNote is an optional unit / enum hint. Missing field is skipped.
static void HANARMSAddField(TMemo *mmo, AnsiString asLabel, AnsiString asLine, AnsiString asKey, AnsiString asNote)   // golden :2773-2785
{
    bool bFound;
    AnsiString v = HANARMSGetKV(asLine, asKey, bFound);
    if(!bFound) return;

    AnsiString asShow;
    if(v == "-") asShow = "(not used)";
    else         asShow = v + (asNote.Length() ? (" " + asNote) : AnsiString(""));

    while(asLabel.Length() < 14) asLabel += " ";                               //fixed-width label
    HANARMSAddLog(mmo, "   ", asLabel + ": " + asShow);
}
//---------------------------------------------------------------------------
//RogerYang 20260727 : if the received line carries GET_JOB_INFO_REP fields (0722 spec),
//  print each field indented under [RX]. Envelope-tolerant : works with or without the
//  outer CMD="GET_JOB_INFO_REP" ... CHECKSTR="END" wrapper (it just scans known keys).
static void HANARMSParseJobInfoRep(TMemo *mmo, AnsiString asLine)   // golden :2790-2836
{
    bool bHasRcp, bDummy;
    HANARMSGetKV(asLine, "RCP_Name", bHasRcp);
    AnsiString asCmd = HANARMSGetKV(asLine, "CMD", bDummy);
    if(!bHasRcp && asCmd.UpperCase() != "GET_JOB_INFO_REP")
        return;                                                                //not a job-info reply

    //basic parameters
    HANARMSAddField(mmo, "RCP_Name", asLine, "RCP_Name", "");
    HANARMSAddField(mmo, "GB_CT",    asLine, "GB_CT",    "");
    HANARMSAddField(mmo, "TEMP",     asLine, "TEMP",     "(C ; -=room temp)");
    HANARMSAddField(mmo, "PIN_CNT",  asLine, "PIN_CNT",  "");
    HANARMSAddField(mmo, "PIN_FC",   asLine, "PIN_FC",   "(gf)");
    HANARMSAddField(mmo, "CON_MTD",  asLine, "CON_MTD",  "(RELEASE/DIRECT)");
    HANARMSAddField(mmo, "WIN_CNT",  asLine, "WIN_CNT",  "");
    //FT group
    HANARMSAddField(mmo, "FT.C_FAIL_SCK", asLine, "FT.C_FAIL_SCK", "");
    HANARMSAddField(mmo, "FT.S_T_S",      asLine, "FT.S_T_S",      "");
    HANARMSAddField(mmo, "FT.LOW_YD",     asLine, "FT.LOW_YD",     "");
    HANARMSAddField(mmo, "FT.AT_OFF",     asLine, "FT.AT_OFF",     "(E=enable)");
    HANARMSAddField(mmo, "FT.SB_ARM",     asLine, "FT.SB_ARM",     "");
    HANARMSAddField(mmo, "FT.SB_ARM_CNT", asLine, "FT.SB_ARM_CNT", "");
    HANARMSAddField(mmo, "FT.SB_SCK",     asLine, "FT.SB_SCK",     "");
    HANARMSAddField(mmo, "FT.SB_SCK_CNT", asLine, "FT.SB_SCK_CNT", "");
    //RT group
    HANARMSAddField(mmo, "RT.C_FAIL_SCK", asLine, "RT.C_FAIL_SCK", "");
    HANARMSAddField(mmo, "RT.S_T_S",      asLine, "RT.S_T_S",      "");
    HANARMSAddField(mmo, "RT.LOW_YD",     asLine, "RT.LOW_YD",     "");
    HANARMSAddField(mmo, "RT.AT_OFF",     asLine, "RT.AT_OFF",     "(E=enable)");
    HANARMSAddField(mmo, "RT.SB_ARM",     asLine, "RT.SB_ARM",     "");
    HANARMSAddField(mmo, "RT.SB_ARM_CNT", asLine, "RT.SB_ARM_CNT", "");
    HANARMSAddField(mmo, "RT.SB_SCK",     asLine, "RT.SB_SCK",     "");
    HANARMSAddField(mmo, "RT.SB_SCK_CNT", asLine, "RT.SB_SCK_CNT", "");
    //0722 additions
    HANARMSAddField(mmo, "LOW_SPD",  asLine, "LOW_SPD",  "(E=enable)");
    HANARMSAddField(mmo, "GET_RDY",  asLine, "GET_RDY",  "");

    //RogerYang 20260827 : 收到 JOB 資訊即與本機 recipe 比對,結果寫 log
    if(fAutomation)
        fAutomation->sHANARMSJobInfoLine = asLine;                          //RogerYang 20260902 : 存為互鎖比對基準
    AnsiString asFail = "";
    if(HANARMSCompare(asLine, asFail))
        HANARMSAddLog(mmo, "   ", "[CMP] PASS");
    else
        HANARMSAddLog(mmo, "   ", "[CMP] FAIL : " + asFail);
}
//---------------------------------------------------------------------------
//  [W906] 1: golden's dfm component (automation.dfm:765-775: Active = False, ClientType = ctNonBlocking, Port = 6670,
//  OnConnect / OnDisconnect / OnRead / OnError), made the first time golden dereferences HANARMSClient.
// ---------------------------------------------------------------------------------------------------------------------
static void W906HanaRmsClientFree(Scktcomp::TClientSocket *p)   // [W906] 1: THanaRmsMembers::~ calls this (HanaRmsMembers_St02.cpp)
{
    if(p == NULL) return;
    p->OnConnect = nullptr;      // no golden event into a dying owner
    p->OnDisconnect = nullptr;
    p->OnRead = nullptr;
    p->OnError = nullptr;
    delete p;
}

void THanaRmsMembers::W906_HANARMSClientCreate()
{
    if(HANARMSClient != NULL) return;
    HANARMSClient = new Scktcomp::TClientSocket(NULL);
    W906_pfnHANARMSClientFree = &W906HanaRmsClientFree;
    HANARMSClient->Port = 6670;                                                 // dfm :768
    HANARMSClient->OnConnect    = [this](TObject *Sender, Scktcomp::TCustomWinSocket *Socket) { HANARMSClientConnect(Sender, Socket); };      // dfm :769
    HANARMSClient->OnDisconnect = [this](TObject *Sender, Scktcomp::TCustomWinSocket *Socket) { HANARMSClientDisconnect(Sender, Socket); };   // dfm :770
    HANARMSClient->OnRead       = [this](TObject *Sender, Scktcomp::TCustomWinSocket *Socket) { HANARMSClientRead(Sender, Socket); };         // dfm :771
    HANARMSClient->OnError      = [this](TObject *Sender, Scktcomp::TCustomWinSocket *Socket, Scktcomp::TErrorEvent ErrorEvent, int &ErrorCode) { HANARMSClientError(Sender, Socket, ErrorEvent, ErrorCode); };   // dfm :772
    HANARMSClient->SetPolled(true);                                             // [W906] 1: ctNonBlocking -> POLLED (H-008): Open() never blocks; W906_HanaRmsPumpTick's Poll() fires the events on the Handler tick
    HANARMSClient->SetSimMode(!w906hanarms::RealSocket());                      // [W906] 1: real only after the composition root's W906_HanaRmsSetRealSocket(true)
}
//---------------------------------------------------------------------------
void THanaRmsMembers::HANARMSClientConnect(TObject * /*Sender*/,
      Scktcomp::TCustomWinSocket * /*Socket*/)                                 // golden :2838-2845
{
    //RogerYang 20260727 : connected
    bW906HANARMSConnecting = false;                                             // [W906] 5
    g_asHANARMSRecvBuf = "";
    if(lblRMS_Status) lblRMS_Status->Caption = "Connected";
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "connected");
}
//---------------------------------------------------------------------------
void THanaRmsMembers::HANARMSClientDisconnect(TObject * /*Sender*/,
      Scktcomp::TCustomWinSocket * /*Socket*/)                                 // golden :2847-2854
{
    //RogerYang 20260727 : disconnected
    bW906HANARMSConnecting = false;                                             // [W906] 5
    g_asHANARMSRecvBuf = "";
    if(lblRMS_Status) lblRMS_Status->Caption = "Disconnected";
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "disconnected");
}
//---------------------------------------------------------------------------
void THanaRmsMembers::HANARMSClientRead(TObject * /*Sender*/,
      Scktcomp::TCustomWinSocket *Socket)                                      // golden :2856-2907
{
    //RogerYang 20260727 : accumulate received bytes, split by '\n' (HANA message terminator)
    try
    {
        g_asHANARMSRecvBuf += Socket->ReceiveText();
    }
    catch(...)
    {
        return;
    }

    int iPos;
    while((iPos = g_asHANARMSRecvBuf.Pos("\n")) > 0)
    {
        AnsiString asLine = g_asHANARMSRecvBuf.SubString(1, iPos - 1);   //one message without '\n'
        g_asHANARMSRecvBuf.Delete(1, iPos);

        //strip trailing CR when the server sends CRLF
        while(asLine.Length() > 0 && asLine[asLine.Length()] == '\r')
            asLine.Delete(asLine.Length(), 1);

        if(asLine.Trim().Length() == 0)
            continue;

        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [RX] ", asLine);

        HANARMSParseJobInfoRep(mmoHANARMS, asLine);                             //RogerYang 20260727 : job-info reply -> show fields indented

        //RogerYang 20260902 : 自動鏈:收到 LOT 資訊接著要 JOB 資訊
        if(sHANARMSLot != "" && asLine.Pos("GET_LOT_INFO_REP") > 0)
        {
            bool bF;
            AnsiString asHD  = ReadIniData(w906hanarms::IniPath(), "Setting", "HDName", AnsiString("HD_01"));   // [W906] 3 path; [912 known defect] K2 HD_01
            AnsiString asJob = "CMD=\"GET_JOB_INFO\" LOT=\"" + sHANARMSLot + "\"";
            asJob += " HDNAME=\""   + asHD + "\"";
            asJob += " LOCATION=\"" + HANARMSGetKV(asLine, "LOCATION", bF) + "\"";
            asJob += " CUSTCD=\""   + HANARMSGetKV(asLine, "CUSTCD",   bF) + "\"";
            asJob += " PARTNO=\""   + HANARMSGetKV(asLine, "PARTNO",   bF) + "\"";
            asJob += " CHECKSTR=\"END\"";
            HANARMSSendCmd(asJob);
        }

        //RogerYang 20260727 : quick format check - each reply should end with CHECKSTR="END"
        if(asLine.Pos("CHECKSTR=\"END\"") <= 0)                                // [912 known defect] K6: also warns on EVENT_REPORT_REP
            HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [WARN] ", "reply without CHECKSTR=\"END\"");
    }

    if(g_asHANARMSRecvBuf.Length() > 8192)   //safety : drop garbage if terminator never comes
        g_asHANARMSRecvBuf = "";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::HANARMSClientError(TObject * /*Sender*/,
      Scktcomp::TCustomWinSocket * /*Socket*/, Scktcomp::TErrorEvent /*ErrorEvent*/, int &ErrorCode)   // golden :2909-2916
{
    //RogerYang 20260727 : swallow error code (no system dialog), just log + reset status
    bW906HANARMSConnecting = false;                                             // [W906] 5
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", "socket error "+IntToStr(ErrorCode));
    ErrorCode = 0;
    if(lblRMS_Status) lblRMS_Status->Caption = "Disconnected";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnHANARMSConnClick(TObject * /*Sender*/)               // golden :2918-2936
{
    //RogerYang 20260727 : connect to HANA HDServer.
    //  IP/Port come from edtHANARMSIP/edtHANARMSPort. Those values are NOT from HANA docs
    //  and must be confirmed by HANA before real testing.
    try
    {
        W906_HANARMSClientCreate();                                             // [W906] 1
        if(HANARMSClient->Active)
            HANARMSClient->Close();
        HANARMSClient->Address = edtHANARMSIP->Text;
        HANARMSClient->Port    = atoi(edtHANARMSPort->Text.c_str());
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "[Man]connecting to "+edtHANARMSIP->Text+":"+edtHANARMSPort->Text);
        bW906HANARMSConnecting = true;                                          // [W906] 5: before Open -- a Sim Open connects inside the call and OnConnect clears it
        HANARMSClient->Open();
    }
    catch(...)
    {
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", "[Man]connect exception");
    }
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnHANARMSDisConnClick(TObject * /*Sender*/)            // golden :2938-2949
{
    //RogerYang 20260727 : disconnect
    try
    {
        if(HANARMSClient && HANARMSClient->Active)
            HANARMSClient->Close();
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnHANARMSManCmdClick(TObject * /*Sender*/)             // golden :2951-2955
{
    //RogerYang 20260727 : send the string in edtHANARMSManCmd; auto append '\n' terminator.
    HANARMSSendCmd(edtHANARMSManCmd->Text);                                 //RogerYang 20260902 : 改共用送出函式
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_ClearLogClick(TObject * /*Sender*/)              // golden :2957-2962
{
    //RogerYang 20260727 : clear RMS log
    if(mmoHANARMS)
        mmoHANARMS->Lines->Clear();
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_GetLotClick(TObject * /*Sender*/)                // golden :2964-2968   [912 known defect] K2 / K8
{
    //RogerYang 20260727 : fill GET_LOT_INFO request template (Handler -> Server)
    edtHANARMSManCmd->Text = "CMD=\"GET_LOT_INFO\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" CHECKSTR=\"END\"";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_GetJobClick(TObject * /*Sender*/)                // golden :2970-2974
{
    //RogerYang 20260727 : fill GET_JOB_INFO request template
    edtHANARMSManCmd->Text = "CMD=\"GET_JOB_INFO\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"XXX_XXXX_XXX_XXX\" CHECKSTR=\"END\"";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_EventClick(TObject * /*Sender*/)                 // golden :2976-2980
{
    //RogerYang 20260727 : fill EVENT_REPORT request template (PJ_FG=YES ok / NO fail ; PJ_CD=10 ok)
    edtHANARMSManCmd->Text = "CMD=\"EVENT_REPORT\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"XXX_XXXX_XXX_XXX\" RCP_NAME=\"EXEC_RCPNAME_TEMPO\" PJ_FG=\"YES\" PJ_CD=\"10\" COMMENT=\"-\" CHECKSTR=\"END\"";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_GetTimeClick(TObject * /*Sender*/)               // golden :2982-2986
{
    //RogerYang 20260727 : fill GET_TIME_INFO request template
    edtHANARMSManCmd->Text = "CMD=\"GET_TIME_INFO\" HDNAME=\"HD_01\" CHECKSTR=\"END\"";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_AliveClick(TObject * /*Sender*/)                 // golden :2988-2992
{
    //RogerYang 20260727 : fill ALIVECHK heartbeat template
    edtHANARMSManCmd->Text = "CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\"";
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnSaveHANARMSLogClick(TObject * /*Sender*/)            // golden :2994-3011
{
    AnsiString asDir = w906hanarms::LogDir();                                   // [W906] 3: golden AnsiString asDir = "D:\\HT9045_Log\\HANARMS";
    MyForceDirectories(asDir);                          //既有函式，會自動建資料夾
    AnsiString asFile;
    asFile.sprintf("%s\\HANARMS_%04d%02d%02d_%02d%02d%02d.log",
                   asDir.c_str(), SystemYear, SystemMonth, SystemDate,
                   SystemHour, SystemMin, SystemSec);
    try
    {
        mmoHANARMS->Lines->SaveToFile(asFile);
    }
    catch(...)
    {

    }
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "log saved: "+asFile);
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnHANARMSSettingClick(TObject * /*Sender*/)            // golden :3013-3019
{
    AnsiString ini = w906hanarms::IniPath();                                    // [W906] 3: golden AnsiString ini = "D:\\HT9045\\Config\\HANARMS.ini"; (same file: NTFS ignores the case of "Config")
    WriteIniData(ini, "Setting", "IP",   edtHANARMSIP->Text);
    WriteIniData(ini, "Setting", "Port", edtHANARMSPort->Text);
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "setting saved");
}
//---------------------------------------------------------------------------
//  golden :3021-3039 ShowFormAsOLP / ShowFormAsHANARMS -- NOT translated: they only switch the form's pages
//  (tsOLP / tsHANARMSInterlock TabVisible, pgcSocketTCPIP->ActivePage, Caption "Automation" / "HANA RMS", Show()).
//  The HANA RMS web screen is a new design (question for ST01-M); golden's only callers are main.cpp:28034
//  (labAutomationDblClick, the OLP page) and cConfiguration.cpp:7933-7937 (btnA77RMSSettingClick).
//---------------------------------------------------------------------------
//RogerYang : load HANA RMS IP/Port from dedicated ini into the edit fields
void THanaRmsMembers::LoadHANARMSSetting()                                      // golden :3042-3047   [912 known defect] K3 defaults
{
    AnsiString ini = w906hanarms::IniPath();                                    // [W906] 3: golden AnsiString ini = "D:\\HT9045\\config\\HANARMS.ini";
    edtHANARMSIP->Text   = ReadIniData(ini, "Setting", "IP",   AnsiString("127.0.0.1"));
    edtHANARMSPort->Text = ReadIniData(ini, "Setting", "Port", AnsiString("6670"));
}

//  golden :3049-3074 PrepareHANARMSConnect -> Automation/HanaRms_Prepare_St02.cpp

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//RogerYang 20260902 : 送一句 RMS 命令(自動補\n);未連線只記 log
void THanaRmsMembers::HANARMSSendCmd(AnsiString asCmd)                          // golden :3078-3101
{
    asCmd = asCmd.Trim();
    if(asCmd.Length() == 0)
        return;

    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [TX] ", asCmd);                  // [912 known defect] K5: logged before the link is checked

    if(HANARMSClient && HANARMSClient->Active && HANARMSClient->Socket->Connected)
    {
        try
        {
            HANARMSClient->Socket->SendText(asCmd + "\n");
        }
        catch(...)
        {
            HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", "send exception");
        }
    }
    else
    {
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [WARN] ", "not connected, not sent");   // [912 known defect] K1: no reconnect here
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260902 : 開批自動查詢。先送 GET_LOT_INFO,收到 REP 由 ClientRead 接送 GET_JOB_INFO
void THanaRmsMembers::HANARMSQueryJobInfo(AnsiString asLot)                     // golden :3104-3114
{
    if(IniConfig.bA77_EnableHanaRMSInterlock==false)
        return;
    sHANARMSLot = asLot.Trim();                                                 // golden :3108
    sHANARMSJobInfoLine = "";                                                   //換批,舊基準作廢   golden :3109 verbatim (St02-M 1002 19:2x: RogerYang's comment and the code agree -- this is the lot-open query -- so the old [W906] 6 "same lot keeps it" is dropped)
    if(sHANARMSLot == "")
        return;
    AnsiString asHD = ReadIniData(w906hanarms::IniPath(), "Setting", "HDName", AnsiString("HD_01"));   // [W906] 3 path; [912 known defect] K2 HD_01
    HANARMSSendCmd("CMD=\"GET_LOT_INFO\" LOT=\"" + sHANARMSLot + "\" HDNAME=\"" + asHD + "\" CHECKSTR=\"END\"");
}
//---------------------------------------------------------------------------
//RogerYang 20260902 : 以最後收到的 JOB 資訊基準重比本機 recipe。無基準=false
bool THanaRmsMembers::HANARMSCheckRecipe(AnsiString &asFail)                    // golden :3117-3134
{
    bool bRet;
    if(sHANARMSJobInfoLine == "")
    {
        asFail = "RMS JOB_INFO not received";
        bRet = false;
    }
    else
    {
        bRet = HANARMSCompare(sHANARMSJobInfoLine, asFail);
    }
    if(bRet)                                                                    //RogerYang 20260902 : 每次互鎖比對結果都留檔
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [CHK] ", "recipe check PASS");
    else
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [CHK] ", "recipe check FAIL : " + asFail);
    return bRet;
}
//---------------------------------------------------------------------------
//RogerYang 20260902 : 測試起點互鎖閘門(Stop/Start)。未啟用/非HANA/ASM一律放行
bool THanaRmsMembers::HANARMSRunCheckOK(bool bShowAlarm)                        // golden :3137-3158
{
    if(CUSTOMER_CODE != CC_HANA_MICRON ||
       IniConfig.bA77_EnableHanaRMSInterlock == false ||
       LastSet.iRunStartMode == rsmAutoSiteMap)
        return true;

    if(bHANARMSNeedRunCheck == false)                                           //RogerYang 20260902 : 事件式:只比 Start 後第一次,過了就不再比
        return true;

    AnsiString asFail = "";
    if(HANARMSCheckRecipe(asFail))
    {
        bHANARMSNeedRunCheck = false;                                           //RogerYang 20260902 : 比對過關,清旗標到下次 Start
        return true;
    }

    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [CMP] ", "RunCheck FAIL : " + asFail);
    if(bShowAlarm)
        ShowMyMessage("HANA RMS recipe check FAIL!!\r\n" + asFail, "HANA RMS recipe 比對失敗!!", "HANARMSRunCheckOK");
    return false;
}
//---------------------------------------------------------------------------
