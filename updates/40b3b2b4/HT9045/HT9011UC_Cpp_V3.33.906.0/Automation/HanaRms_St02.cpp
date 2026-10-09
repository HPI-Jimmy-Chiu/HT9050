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
//    K1  retired 20261009 (W-195 H2): connect on demand -- HANARMSSendCmd queues and calls HANARMSConnect (golden 913).
//    K2  retired 20261009 (W-195 H2 / H3): HDNAME = HANARMSGetHDName() -- HANARMS.ini HDName, else Gerneral.ini Machine ID
//        (golden 913 :3349-3358).
//    K3  retired 20261009 (W-195 H2): LoadHANARMSSetting defaults "" / 14140; an empty IP = "RMS server IP/Port not set".
//    K4  retired 20261009 (W-195 H1): HANARMS_ContactMethod now sends the HANA 1002 mode names (golden 913).
//  AI(W906-W195) 20261009 (St02-E) H1: the recipe compare follows golden 913 (HANA 1002 rules, RogerYang 20261005) --
//    HANARMS_ContactMethod, HANARMS_IsLowSpeed, the 1002 helpers (AddFail .. CmpList), HANARMSCompare and the four
//    HANARMSParseJobInfoRep notes; those cite "golden 913 :N" (Automation/automation.cpp of
//    D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven).  Every other "golden :N" in this file is still a 912 line.
//  AI(W906-W195) 20261009 (St02-E) H2: connect on demand (golden 913, RogerYang 20260916) -- HANARMSClientConnect /
//    ClientRead's HDNAME / ClientError / the three button templates / LoadHANARMSSetting / HANARMSSendCmd /
//    HANARMSIsConnected / HANARMSConnect / HANARMSFlushPending / HANARMSGetHDName / HANARMSQueryJobInfo, and
//    PrepareHANARMSConnect (HanaRms_Prepare_St02.cpp) -- cite "golden 913 :N" too.
//    K5  retired 20261009 (W-195 H2): [TX] only when really sent, [QUEUE] otherwise (golden 913 :3261, :3274).
//    K6  the CHECKSTR check also warns on EVENT_REPORT_REP (which has no CHECKSTR).
//    K7  the [WARN] text says "A76" although the flag is A77 (A76 went to SPIL in 912) -- text kept verbatim.
//    K8  retired 20261009 (W-195 H3): btnRMS_GetLotClick runs the lot query (golden 913 :3137-3153).
//  AI(W906-W195) 20261009 (St02-E) H3: HANARMSCut / HANARMSFailForDialog, btnRMS_GetLotClick, HANARMSCheckRecipe (graded
//    "no baseline" reasons) and HANARMSRunCheckOK (re-query, the big dialog) follow golden 913 (RogerYang 20260916).
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
static const int HANARMS_START_WAIT_MS = 3000;   //RogerYang 20260916 : Start 前置連線等待上限   (AI(W906-W195) 20261009 H4: golden 913 :2524)

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
//AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : CON_MTD 改送 Contact mode 名稱(HANA 1002 對照表),RELEASE/DIRECT 停用
static AnsiString HANARMS_ContactMethod(int iContactMode)   // golden 913 :2598-2616
{
    switch(iContactMode)
    {
        case DirectContactMode:             return "Direct Contact";
        case DropContact:                   return "Drop Contact";
        case DirectContactModeDiffentSpeed: return "Direct & Slow Contact";
        case TMove:                         return "TMOVE Contact";
        case TMoveDrop:                     return "TMOVE Drop Contact";
        case DirectContactSoftEP:           return "Direct & Soft EP Contact";
        case DropContactSoftEP:             return "Drop & Soft EP Contact";
//        case DropPlaceShiftContact:         return "Shift Contact for 8Site 1x4";     //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : HANA 1002 表未列,落 default=UNSUPPORTED
        case DropContactModeDiffentSpeed:   return "Drop & Slow Contact";
        case TMoveSlowContact:              return "TMOVE Slow Contact";
        case TMoveDropSlowContact:          return "TMOVE Drop & Slow Contact";
        default:                            return "UNSUPPORTED";
    }
}
//---------------------------------------------------------------------------
//AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : LOW_SPD 依 HANA 1002 對照表(Slow 與 Soft EP 為 E)
static bool HANARMS_IsLowSpeed(int iContactMode)   // golden 913 :2618-2633 (912 :2612-2624 HANARMS_LowSpeed returned "E"/"-")
{
    switch(iContactMode)
    {
        case DirectContactModeDiffentSpeed: //2  Direct & Slow
        case DirectContactSoftEP:           //5  Direct & Soft EP
        case DropContactSoftEP:             //6  Drop & Soft EP
        case DropContactModeDiffentSpeed:   //8  Drop & Slow
        case TMoveSlowContact:              //9  TMOVE Slow
        case TMoveDropSlowContact:          //10 TMOVE Drop & Slow
            return true;
        default:
            return false;
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
//AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : 以下為 HANA 1002 新規則用 helper
static void HANARMS_AddFail(AnsiString &asFail, AnsiString asItem)   // golden 913 :2670-2675
{
    if(asFail.Length()) asFail += " ; ";
    asFail += asItem;
}
//---------------------------------------------------------------------------
//"-" 或 "D" 皆為停用(HANA 1002 : no use = disabled)
static bool HANARMS_IsOff(AnsiString s)   // golden 913 :2677-2682
{
    s = s.Trim().UpperCase();
    return (s == "-" || s == "D");
}
//---------------------------------------------------------------------------
//轉大寫、去頭尾空白、連續空白併成一個底線("Drop & Slow Contact"="DROP_&_SLOW_CONTACT")
static AnsiString HANARMS_Norm(AnsiString s)   // golden 913 :2684-2704
{
    s = s.Trim().UpperCase();
    AnsiString r = "";
    bool bSp = false;
    for(int i=1; i<=s.Length(); i++)
    {
        if(s[i]==' ' || s[i]=='\t' || s[i]=='_')
        {
            if(!bSp) r += '_';
            bSp = true;
        }
        else
        {
            r += s[i];
            bSp = false;
        }
    }
    return r;
}
//---------------------------------------------------------------------------
//字串欄(CON_MTD):正規化後比對;未送或 "-" 跳過
static bool HANARMS_CmpNorm(AnsiString asName, AnsiString asServer, bool bFound, AnsiString asHandler, AnsiString &asFail)   // golden 913 :2706-2715
{
    if(bFound==false || asServer=="-")
        return true;
    if(HANARMS_Norm(asServer) == HANARMS_Norm(asHandler))
        return true;
    HANARMS_AddFail(asFail, asName + "(S=" + asServer + "/H=" + asHandler + ")");
    return false;
}
//---------------------------------------------------------------------------
//開關欄(AT_OFF/LOW_SPD):伺服器 E=須啟用,"-"/D=須停用;Handler 回 E/D
static bool HANARMS_CmpFlag(AnsiString asName, AnsiString asServer, bool bFound, bool bHandlerOn, AnsiString &asFail)   // golden 913 :2717-2728
{
    if(bFound==false)
        return true;
    bool bNeedOn = (asServer.Trim().UpperCase() == "E");
    if(( bNeedOn &&  bHandlerOn) ||
       (!bNeedOn && !bHandlerOn && HANARMS_IsOff(asServer)))
        return true;
    HANARMS_AddFail(asFail, asName + "(S=" + asServer + "/H=" + (bHandlerOn ? "E" : "D") + ")");
    return false;
}
//---------------------------------------------------------------------------
//數值欄:伺服器送數值=功能須啟用且數值相同;"-"/D=功能須停用。停用時 H 顯示 D
static bool HANARMS_CmpFunc(AnsiString asName, AnsiString asServer, bool bFound, bool bOn, double dHandler, AnsiString &asFail)   // golden 913 :2730-2748
{
    if(bFound==false)
        return true;
    if(HANARMS_IsOff(asServer))
    {
        if(!bOn) return true;
    }
    else if(bOn)
    {
        double d = atof(asServer.c_str()) - dHandler;
        if(d < 0) d = -d;
        if(d < 0.0005) return true;
    }
    AnsiString asH = bOn ? AnsiString().sprintf("%g", dHandler) : AnsiString("D");
    HANARMS_AddFail(asFail, asName + "(S=" + asServer + "/H=" + asH + ")");
    return false;
}
//---------------------------------------------------------------------------
//SB 逗號串:"-"/D=Handler 須無 Special Bin;其餘去空白後比對。Handler 無設定時 H 顯示 D
static bool HANARMS_CmpList(AnsiString asName, AnsiString asServer, bool bFound, AnsiString asHandler, AnsiString &asFail)   // golden 913 :2750-2760
{
    if(bFound==false)
        return true;
    bool bHOff = (asHandler == "-");
    if(HANARMS_IsOff(asServer) ? bHOff : (HANARMS_StripSpace(asServer) == asHandler))
        return true;
    HANARMS_AddFail(asFail, asName + "(S=" + asServer + "/H=" + (bHOff ? AnsiString("D") : asHandler) + ")");
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
//---------------------------------------------------------------------------
//RogerYang 20260916 : 依標籤寬度截斷,避免單欄自己折成兩行
static AnsiString HANARMSCut(AnsiString asStr, int iMax)   // golden 913 :2783-2789 (AI(W906-W195) 20261009 H3)
{
    if(asStr.Length() <= iMax)
        return asStr;
    return asStr.SubString(1, iMax-3) + "...";
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : 組裝彈窗訊息。MyMessageBox 兩個 Label 固定大小且不捲動,
//  搭配 bBigMyMessage(570px 寬/左對齊)與暫時加高後約 主3行 + 次8行,每行約 71 字。
//  20 欄全列約 519 字會被無聲裁掉,故逐欄分行並限量;完整清單一律看 RMS 畫面與 log。
//  訊息全用英文(HANA 為韓國客戶,中文對現場無用)。
static void HANARMSFailForDialog(AnsiString asFail, AnsiString &asMain, AnsiString &asSub)   // golden 913 :2791-2844 (H3)
{
    const int iMainMax=2, iSubMax=6, iCutW=58;                              //實測 570px 約 8.8px/字 -> 每行 64 字,
                                                                            //  截 58 保留餘裕(超過會 WordWrap 折行吃掉下一欄);
                                                                            //  次標籤 150px/19px=7.9 行 -> 6 欄 + 1 總數行
    AnsiString asShow[9];
    int iTotal=0, iKeep=0;

    AnsiString asWork = asFail;
    while(asWork.Length())                                                  //以 " ; " 逐欄切開
    {
        int p = asWork.Pos(" ; ");
        AnsiString asOne = (p > 0) ? asWork.SubString(1, p-1) : asWork;
        if(p > 0)
            asWork.Delete(1, p+2);
        else
            asWork = "";
        asOne = asOne.Trim();
        if(asOne.Length() == 0)
            continue;
        iTotal++;
        if(iKeep < (iMainMax+iSubMax))
            asShow[iKeep++] = asOne;
    }

    if(asFail.Pos("(S=") <= 0)                                              //連線類訊息(不含欄位格式)原樣帶出
    {                                                                       //  不可用欄位數判斷:只錯一欄時也沒有 " ; ",會誤判成連線訊息而漏掉標題
        asMain = asFail;
        asSub  = "See RMS screen and log for details.";                      //此分支涵蓋「未開批」等非連線原因,
                                                                            //  措辭不可寫死成查連線,主訊息本身已說明原因
        return;
    }

    asMain.sprintf("HANA RMS recipe check FAIL!!  (%d item%s)", iTotal, (iTotal>1)?"s":"");
    asSub = "";
    for(int i=0; i<iKeep; i++)
    {
        if(i < iMainMax)
            asMain += "\r\n" + HANARMSCut(asShow[i], iCutW);
        else
            asSub  += (asSub.Length()?AnsiString("\r\n"):AnsiString("")) + HANARMSCut(asShow[i], iCutW);
    }

    AnsiString asTail;                                                      //最後一行永遠可見
    if(iTotal > iKeep)
        asTail.sprintf("... +%d more, see RMS screen for the full list", iTotal-iKeep);
    else
        asTail = "Correct the item(s) above, then press START again.";       //全部列完了,給下一步指引比叫人去看 log 有用
    asSub += (asSub.Length()?AnsiString("\r\n"):AnsiString("")) + asTail;
}
//RogerYang 20260827 : GET_JOB_INFO_REP 與本機 recipe 逐欄比對。true=全過;asFail 列出所有不符
//AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : 規則依 HANA 1002:"-"/D=功能須停用、數值欄須啟用且相等、RT_AT_OFF 固定 D。細節見 HANA RMS 技能
static bool HANARMSCompare(AnsiString asLine, AnsiString &asFail)   // golden 913 :2845-2936 (912 :2683-2769)
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
        else                                                                    //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : 室溫一律比 Ambient temp. 設定值,不看 Ambient check(HANA 1001)
            bOK &= HANARMS_CmpNum("TEMP", sv, true, Temperature.fAbitTemp, asFail);
//        else if(Temperature.bUseAbitCHK)
//            bOK &= HANARMS_CmpNum("TEMP", sv, true, Temperature.fAbitTemp, asFail);
//        else if(sv != "-")
//        {
//            bOK = false;
//            if(asFail.Length()) asFail += " ; ";
//            asFail += "TEMP(S=" + sv + "/H=RoomTemp)";
//        }
    }

    sv = HANARMSGetKV(asLine, "PIN_CNT", bF);
    bOK &= HANARMS_CmpNum("PIN_CNT", sv, bF, (double)DeviceForm_File.iPinCT, asFail);

    sv = HANARMSGetKV(asLine, "PIN_FC", bF);                                    //單位 gf
    bOK &= HANARMS_CmpNum("PIN_FC", sv, bF, DeviceForm_File.ForcePerPinG, asFail);

    sv = HANARMSGetKV(asLine, "CON_MTD", bF);                                   //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : Contact mode 名稱,大小寫/空白不拘
    bOK &= HANARMS_CmpNorm("CON_MTD", sv, bF, HANARMS_ContactMethod(DeviceForm_File.ContactMode), asFail);

    sv = HANARMSGetKV(asLine, "WIN_CNT", bF);                                   //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : 改比 Low Yields after test count(含 Enable)
    bOK &= HANARMS_CmpFunc("WIN_CNT", sv, bF, TestIF.bFailAlarmLowYield, (double)TestIF.iLowYieldCount, asFail);
//    bOK &= HANARMS_CmpNum("WIN_CNT", sv, bF, (double)TestIF.iSlidingWindowSize, asFail);

    sv = HANARMSGetKV(asLine, "LOW_SPD", bF);
    bOK &= HANARMS_CmpFlag("LOW_SPD", sv, bF, HANARMS_IsLowSpeed(DeviceForm_File.ContactMode), asFail);

    //--- FT ---(數值欄一併檢查功能 Enable)
    sv = HANARMSGetKV(asLine, "FT.C_FAIL_SCK", bF);
    bOK &= HANARMS_CmpFunc("FT.C_FAIL_SCK", sv, bF, TestIF.bContsFailBySocket, (double)TestIF.iContsFailSocketAlarmCT, asFail);
    sv = HANARMSGetKV(asLine, "FT.S_T_S", bF);
    bOK &= HANARMS_CmpFunc("FT.S_T_S", sv, bF, TestIF.bFailAlarmSiteYieldDifferent, TestIF.dFailAlarmSiteYield, asFail);
    sv = HANARMSGetKV(asLine, "FT.LOW_YD", bF);
    bOK &= HANARMS_CmpFunc("FT.LOW_YD", sv, bF, TestIF.bFailAlarmLowYield, TestIF.dLowYieldLimit, asFail);
    sv = HANARMSGetKV(asLine, "FT.AT_OFF", bF);                                 //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : 改比 Consecutive Failure(Socket) 的 Trigger
    bOK &= HANARMS_CmpFlag("FT.AT_OFF", sv, bF, TestIF.bLowYieldAutoSiteOffByContiFail, asFail);

    HANARMS_GetSpecialBinStr(true, eBinFT, hb, hc);
    sv = HANARMSGetKV(asLine, "FT.SB_ARM", bF);
    bOK &= HANARMS_CmpList("FT.SB_ARM", sv, bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "FT.SB_ARM_CNT", bF);
    bOK &= HANARMS_CmpList("FT.SB_ARM_CNT", sv, bF, hc, asFail);
    HANARMS_GetSpecialBinStr(false, eBinFT, hb, hc);
    sv = HANARMSGetKV(asLine, "FT.SB_SCK", bF);
    bOK &= HANARMS_CmpList("FT.SB_SCK", sv, bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "FT.SB_SCK_CNT", bF);
    bOK &= HANARMS_CmpList("FT.SB_SCK_CNT", sv, bF, hc, asFail);

    //--- RT ---
    sv = HANARMSGetKV(asLine, "RT.C_FAIL_SCK", bF);
    bOK &= HANARMS_CmpFunc("RT.C_FAIL_SCK", sv, bF, TestIF.bContsFailBySocket_RT, (double)TestIF.iContsFailSocketAlarmCT_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.S_T_S", bF);
    bOK &= HANARMS_CmpFunc("RT.S_T_S", sv, bF, TestIF.bFailAlarmSiteYieldDifferent_RT, TestIF.dFailAlarmSiteYield_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.LOW_YD", bF);
    bOK &= HANARMS_CmpFunc("RT.LOW_YD", sv, bF, TestIF.bFailAlarmLowYield_RT, TestIF.dLowYieldLimit_RT, asFail);
    sv = HANARMSGetKV(asLine, "RT.AT_OFF", bF);                                 //AI(ht9045-hana-rms-interlock) 20261005 (RogerYang) : RT 無此功能(Auto Site Off 只在 FT 生效),固定回 D
    bOK &= HANARMS_CmpFlag("RT.AT_OFF", sv, bF, false, asFail);

    HANARMS_GetSpecialBinStr(true, eBinRT, hb, hc);
    sv = HANARMSGetKV(asLine, "RT.SB_ARM", bF);
    bOK &= HANARMS_CmpList("RT.SB_ARM", sv, bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "RT.SB_ARM_CNT", bF);
    bOK &= HANARMS_CmpList("RT.SB_ARM_CNT", sv, bF, hc, asFail);
    HANARMS_GetSpecialBinStr(false, eBinRT, hb, hc);
    sv = HANARMSGetKV(asLine, "RT.SB_SCK", bF);
    bOK &= HANARMS_CmpList("RT.SB_SCK", sv, bF, hb, asFail);
    sv = HANARMSGetKV(asLine, "RT.SB_SCK_CNT", bF);
    bOK &= HANARMS_CmpList("RT.SB_SCK_CNT", sv, bF, hc, asFail);

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
    HANARMSAddField(mmo, "CON_MTD",  asLine, "CON_MTD",  "(contact mode)");                         // golden 913 :2971
    HANARMSAddField(mmo, "WIN_CNT",  asLine, "WIN_CNT",  "");
    //FT group
    HANARMSAddField(mmo, "FT.C_FAIL_SCK", asLine, "FT.C_FAIL_SCK", "");
    HANARMSAddField(mmo, "FT.S_T_S",      asLine, "FT.S_T_S",      "");
    HANARMSAddField(mmo, "FT.LOW_YD",     asLine, "FT.LOW_YD",     "");
    HANARMSAddField(mmo, "FT.AT_OFF",     asLine, "FT.AT_OFF",     "(E=enable/D=disable)");     // golden 913 :2977
    HANARMSAddField(mmo, "FT.SB_ARM",     asLine, "FT.SB_ARM",     "");
    HANARMSAddField(mmo, "FT.SB_ARM_CNT", asLine, "FT.SB_ARM_CNT", "");
    HANARMSAddField(mmo, "FT.SB_SCK",     asLine, "FT.SB_SCK",     "");
    HANARMSAddField(mmo, "FT.SB_SCK_CNT", asLine, "FT.SB_SCK_CNT", "");
    //RT group
    HANARMSAddField(mmo, "RT.C_FAIL_SCK", asLine, "RT.C_FAIL_SCK", "");
    HANARMSAddField(mmo, "RT.S_T_S",      asLine, "RT.S_T_S",      "");
    HANARMSAddField(mmo, "RT.LOW_YD",     asLine, "RT.LOW_YD",     "");
    HANARMSAddField(mmo, "RT.AT_OFF",     asLine, "RT.AT_OFF",     "(E=enable/D=disable)");     // golden 913 :2986
    HANARMSAddField(mmo, "RT.SB_ARM",     asLine, "RT.SB_ARM",     "");
    HANARMSAddField(mmo, "RT.SB_ARM_CNT", asLine, "RT.SB_ARM_CNT", "");
    HANARMSAddField(mmo, "RT.SB_SCK",     asLine, "RT.SB_SCK",     "");
    HANARMSAddField(mmo, "RT.SB_SCK_CNT", asLine, "RT.SB_SCK_CNT", "");
    //0722 additions
    HANARMSAddField(mmo, "LOW_SPD",  asLine, "LOW_SPD",  "(E=enable/D=disable)");                   // golden 913 :2992
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
      Scktcomp::TCustomWinSocket * /*Socket*/)                                 // golden 913 :3005-3014 (912 :2838-2845)
{
    //RogerYang 20260727 : connected
    bW906HANARMSConnecting = false;                                             // [W906] 5
    g_asHANARMSRecvBuf = "";
    if(lblRMS_Status) lblRMS_Status->Caption = "Connected";
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "connected");
    sHANARMSLinkErr = "";                                                   //RogerYang 20260916 : 連上了,清掉舊錯誤
    HANARMSFlushPending();                                                  //RogerYang 20260916 : 把排隊中的指令送出
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
            AnsiString asHD  = HANARMSGetHDName();                          //RogerYang 20260916 : 預設用機台 Machine ID   (golden 913 :3059; K2 retired)
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
      Scktcomp::TCustomWinSocket * /*Socket*/, Scktcomp::TErrorEvent /*ErrorEvent*/, int &ErrorCode)   // golden 913 :3078-3089 (912 :2909-2916)
{
    //RogerYang 20260727 : swallow error code (no system dialog), just log + reset status
    //RogerYang 20260916 : 記下可讀的失敗原因,供互鎖訊息區分「連不上」與「參數不符」
    bW906HANARMSConnecting = false;                                             // [W906] 5
    sHANARMSLinkErr.sprintf("cannot connect to %s:%s (socket error %d)",
                            edtHANARMSIP->Text.c_str(), edtHANARMSPort->Text.c_str(), ErrorCode);
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", sHANARMSLinkErr);
    sHANARMSPending = "";                                                   //連不上就不留著,避免稍後莫名送出
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
void THanaRmsMembers::btnRMS_GetLotClick(TObject * /*Sender*/)                // golden 913 :3137-3153 (912 :2964-2968; AI(W906-W195) 20261009 H3: K8 retired)
{
    //RogerYang 20260727 : fill GET_LOT_INFO request template (Handler -> Server)
//    edtHANARMSManCmd->Text = "CMD=\"GET_LOT_INFO\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" CHECKSTR=\"END\"";
    //RogerYang 20260916 : 改為直接執行查詢(與開批 LOTON 同一路徑),兼作互鎖失敗後的手動重試
    bool bF;
    AnsiString asLot = sHANARMSLot.Trim();                                  //1.開批取得的批號
    if(asLot == "")
        asLot = HANARMSGetKV(edtHANARMSManCmd->Text, "LOT", bF).Trim();     //2.手動欄位的 LOT="..."
    if(asLot == "")
    {
        edtHANARMSManCmd->Text = "CMD=\"GET_LOT_INFO\" LOT=\"TLOTNO\" HDNAME=\"" + HANARMSGetHDName() + "\" CHECKSTR=\"END\"";
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [WARN] ", "no lot : start a lot first, or edit LOT= in the command box");
        return;
    }
    HANARMSQueryJobInfo(asLot);
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_GetJobClick(TObject * /*Sender*/)                // golden 913 :3155-3159 (912 :2970-2974)
{
    //RogerYang 20260727 : fill GET_JOB_INFO request template
    edtHANARMSManCmd->Text = "CMD=\"GET_JOB_INFO\" LOT=\"TLOTNO\" HDNAME=\"" + HANARMSGetHDName() + "\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"XXX_XXXX_XXX_XXX\" CHECKSTR=\"END\"";   //RogerYang 20260916 : HDNAME 用機號
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_EventClick(TObject * /*Sender*/)                 // golden 913 :3161-3165 (912 :2976-2980)
{
    //RogerYang 20260727 : fill EVENT_REPORT request template (PJ_FG=YES ok / NO fail ; PJ_CD=10 ok)
    edtHANARMSManCmd->Text = "CMD=\"EVENT_REPORT\" LOT=\"TLOTNO\" HDNAME=\"" + HANARMSGetHDName() + "\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"XXX_XXXX_XXX_XXX\" RCP_NAME=\"EXEC_RCPNAME_TEMPO\" PJ_FG=\"YES\" PJ_CD=\"10\" COMMENT=\"-\" CHECKSTR=\"END\"";   //RogerYang 20260916 : HDNAME 用機號
}
//---------------------------------------------------------------------------
void THanaRmsMembers::btnRMS_GetTimeClick(TObject * /*Sender*/)               // golden 913 :3167-3171 (912 :2982-2986)
{
    //RogerYang 20260727 : fill GET_TIME_INFO request template
    edtHANARMSManCmd->Text = "CMD=\"GET_TIME_INFO\" HDNAME=\"" + HANARMSGetHDName() + "\" CHECKSTR=\"END\"";   //RogerYang 20260916 : HDNAME 用機號
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
void THanaRmsMembers::LoadHANARMSSetting()                                      // golden 913 :3227-3233 (912 :3042-3047; K3 retired)
{
    AnsiString ini = w906hanarms::IniPath();                                    // [W906] 3: golden AnsiString ini = "D:\\HT9045\\config\\HANARMS.ini";
    //RogerYang 20260916 : 預設留空(=未設定);原本的 127.0.0.1 佔位值會讓人誤以為已設定
    edtHANARMSIP->Text   = ReadIniData(ini, "Setting", "IP",   AnsiString(""));
    edtHANARMSPort->Text = ReadIniData(ini, "Setting", "Port", AnsiString("14140"));
}

//  golden :3049-3074 PrepareHANARMSConnect -> Automation/HanaRms_Prepare_St02.cpp

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//RogerYang 20260902 : 送一句 RMS 命令(自動補\n);未連線只記 log
void THanaRmsMembers::HANARMSSendCmd(AnsiString asCmd)                          // golden 913 :3249-3276 (912 :3078-3101; K1 / K5 retired)
{
    asCmd = asCmd.Trim();
    if(asCmd.Length() == 0)
        return;

    if(HANARMSIsConnected())                                                //RogerYang 20260916 : 已連線直接送
    {
        try
        {
            HANARMSClient->Socket->SendText(asCmd + "\n");
            HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [TX] ", asCmd);        //真的送出才記 TX
        }
        catch(...)
        {
            HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", "send exception");
        }
        return;
    }

    //RogerYang 20260916 : 未連線 -> 排隊後發起連線,連上再送(不可同步等,會凍住 MainProc)
    if(sHANARMSPending.Length())
        sHANARMSPending += "\n";
    sHANARMSPending += asCmd;
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [QUEUE] ", asCmd);
    HANARMSConnect();
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : 連線是否真的可用(Active 不等於連得上)
bool THanaRmsMembers::HANARMSIsConnected()                                      // golden 913 :3278-3290
{
    try
    {
        return (HANARMSClient != NULL && HANARMSClient->Active &&
                HANARMSClient->Socket != NULL && HANARMSClient->Socket->Connected);
    }
    catch(...)
    {
        return false;
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : 發起非阻塞連線;連上由 HANARMSClientConnect 沖出待送指令
void THanaRmsMembers::HANARMSConnect()                                          // golden 913 :3292-3326
{
    // [W906] 1: golden `if(HANARMSClient == NULL) return;` (:3295-3296) -- the dfm component always exists there; here it
    //   is made on first use, right before the first Open (below), so "IP/Port not set" makes no socket.
    if(HANARMSIsConnected())
        return;
    if(bW906HANARMSConnecting)                                                  // [W906] 5: a polled connect is pending (Active stays false until it completes, so golden's Close below would not run and the polled socket ignores a second Open) -- skipping only saves a second "connecting to" line
        return;

    LoadHANARMSSetting();
    AnsiString asIP = edtHANARMSIP->Text.Trim();
    int iPort = atoi(edtHANARMSPort->Text.c_str());
    if(asIP == "" || iPort <= 0)
    {
        sHANARMSLinkErr = "RMS server IP/Port not set";
        sHANARMSPending = "";
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", sHANARMSLinkErr);
        return;
    }

    try
    {
        W906_HANARMSClientCreate();                                             // [W906] 1: see above
        if(HANARMSClient->Active)                                            //清掉已斷的舊 socket 才能重連
            HANARMSClient->Close();
        HANARMSClient->Address = asIP;
        HANARMSClient->Port    = iPort;
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [INFO] ", "connecting to "+asIP+":"+edtHANARMSPort->Text);
        bW906HANARMSConnecting = true;                                          // [W906] 5: before Open -- a Sim Open connects inside the call and OnConnect clears it
        HANARMSClient->Open();
    }
    catch(...)
    {
        bW906HANARMSConnecting = false;                                         // [W906] 5
        sHANARMSLinkErr = "connect exception";
        sHANARMSPending = "";
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [ERR] ", sHANARMSLinkErr);
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : 把連線期間排隊的指令依序送出
void THanaRmsMembers::HANARMSFlushPending()                                     // golden 913 :3328-3347
{
    if(sHANARMSPending.Length() == 0)
        return;

    AnsiString asAll = sHANARMSPending;
    sHANARMSPending  = "";                                                  //先清,避免重入時重送
    while(asAll.Length())
    {
        int iPos = asAll.Pos("\n");
        AnsiString asOne = (iPos > 0) ? asAll.SubString(1, iPos-1) : asAll;
        if(iPos > 0)
            asAll.Delete(1, iPos);
        else
            asAll = "";
        if(asOne.Trim().Length())
            HANARMSSendCmd(asOne);
    }
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : HDNAME 預設=機台 Machine ID(Gerneral.ini [Version] Machine ID)
//  HANARMS.ini 的 HDName 有填才覆寫(現場若 RMS 註冊名與機台不同時用)
AnsiString THanaRmsMembers::HANARMSGetHDName()                                  // golden 913 :3349-3358
{
    AnsiString asHD = ReadIniData(w906hanarms::IniPath(), "Setting", "HDName", AnsiString(""));   // [W906] 3: golden "D:\\HT9045\\config\\HANARMS.ini"
    asHD = asHD.Trim();
    if(asHD == "")
        asHD = AnsiString(IniConfig.sGPIBMachineID).Trim();                  //Gerneral.ini 的值常帶尾端空白
    return asHD;
}
//---------------------------------------------------------------------------
//RogerYang 20260902 : 開批自動查詢。先送 GET_LOT_INFO,收到 REP 由 ClientRead 接送 GET_JOB_INFO
void THanaRmsMembers::HANARMSQueryJobInfo(AnsiString asLot)                     // golden 913 :3360-3377 (912 :3104-3114)
{
    if(IniConfig.bA77_EnableHanaRMSInterlock==false)
        return;
    //RogerYang 20260916 : 只有換批才作廢舊基準;同批重查(手動 Get Lot/自動重試)失敗時保留原基準,
    //  否則按一下 Get Lot 就可能把原本有效的基準弄丟,反而把機台擋住
    AnsiString asNewLot = asLot.Trim();
    if(asNewLot == "")
        return;                                                             //無批號:不動基準也不查詢
    if(sHANARMSLot.Trim() != asNewLot)
        sHANARMSJobInfoLine = "";                                           //換批,舊基準作廢
    sHANARMSLot = asNewLot;
    sHANARMSPending   = "";                                                 //RogerYang 20260916 : 丟掉上一批沒送出的指令
    dwHANARMSSentTick = GetTickCount();                                     //RogerYang 20260916 : 查詢起算時刻
    AnsiString asHD = HANARMSGetHDName();
    HANARMSSendCmd("CMD=\"GET_LOT_INFO\" LOT=\"" + sHANARMSLot + "\" HDNAME=\"" + asHD + "\" CHECKSTR=\"END\"");
}
//---------------------------------------------------------------------------
//RogerYang 20260902 : 以最後收到的 JOB 資訊基準重比本機 recipe。無基準=false
bool THanaRmsMembers::HANARMSCheckRecipe(AnsiString &asFail)                    // golden 913 :3380-3408 (912 :3117-3134; AI(W906-W195) 20261009 H3)
{
    bool bRet;
    if(sHANARMSJobInfoLine == "")
    {
        //RogerYang 20260916 : 依連線狀態分級,讓操作員看得出是通訊問題還是參數問題
        if(sHANARMSLot.Trim() == "")
            asFail = "RMS not queried : no lot started";
        else if(sHANARMSLinkErr.Length())
            asFail = "RMS link fail : " + sHANARMSLinkErr;
        else if(HANARMSIsConnected() == false)
            asFail = "RMS server not connected : " + edtHANARMSIP->Text + ":" + edtHANARMSPort->Text;
        else if(dwHANARMSSentTick != 0)
            asFail.sprintf("RMS no reply from server (%d sec, lot=%s)",
                           (int)((GetTickCount()-dwHANARMSSentTick)/1000), sHANARMSLot.c_str());
        else
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
bool THanaRmsMembers::HANARMSRunCheckOK(bool bShowAlarm)                        // golden 913 :3410-3460 (912 :3137-3158; AI(W906-W195) 20261009 H3)
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

    //RogerYang 20260916 : 沒基準就就地重發查詢,操作員再按 Start 即可重試,不必重開程式
    if(sHANARMSJobInfoLine == "" && sHANARMSLot.Trim() != "" &&             //5 秒節流,避免連續循環送爆
       (dwHANARMSSentTick == 0 || (GetTickCount()-dwHANARMSSentTick) > 5000))
        HANARMSQueryJobInfo(sHANARMSLot);

    if(bShowAlarm)
    {
        AnsiString asDlgMain, asDlgSub;                                     //RogerYang 20260916 : 依標籤容量逐欄分行
        HANARMSFailForDialog(asFail, asDlgMain, asDlgSub);
        bBigMyMessage = true;                                               //放大視窗(590x381)+左對齊,既有通用旗標
        // [W906] golden :3440-3444 / :3451-3455 set and restore MyMessageBox->lblMainMsg / lblChineseMsg Height (60 / 150, then
        //   49 / 62): the port's TMyMessageBoxShim (mymessbox_shim.h) has no labels -- the web page lays out the "bigLeft" box
        //   by name (tools/wb_serve.cpp MbLayoutOnShow, which reads and clears bBigMyMessage inside ShowMyMessage).
        try
        {
            ShowMyMessage(asDlgMain, asDlgSub, "HANARMSRunCheckOK");
        }
        catch(...)                                                          // [W906] golden __finally: reset, then rethrow
        {
            bBigMyMessage = false;
            throw;
        }
        bBigMyMessage = false;                                              //ShowMyMessage 若因重入提早返回,旗標不可殘留
    }
    return false;
}
//---------------------------------------------------------------------------
//RogerYang 20260916 : Start 前置檢查(照偉測 fMesSystem->CheckLotInfor 樣式,擋在進料之前)
//  A77 開啟時先確認 RMS 連線可用;連不上就拒絕 Start,料一顆都不會進機台。
//  參數比對無法在此做(批號要等 Tester 送 LOTON),仍由測試位閘門負責。
bool THanaRmsMembers::HANARMSCheckLinkBeforeStart()                             // golden 913 :3462-3503 (AI(W906-W195) 20261009 H4)
{
    if(IniConfig.bA77_EnableHanaRMSInterlock == false ||                     //CUSTOMER_CODE 判斷在 main.cpp 呼叫端
       LastSet.iTester != ON_LINE ||                                        //離線不卡(同偉測 CheckLotInfor)
       LastSet.iRunStartMode == rsmAutoSiteMap)                             //ASM 不卡(同 HANARMSRunCheckOK)
        return true;

    if(HANARMSIsConnected())
        return true;

    sHANARMSLinkErr = "";
    HANARMSConnect();                                                       //非阻塞;連上與否由 socket 事件回報

    unsigned long dwT0 = GetTickCount();                                    //有限時等待:Start 是按鍵觸發,不在主迴圈內
    while(HANARMSIsConnected() == false &&
          (GetTickCount() - dwT0) < (unsigned long)HANARMS_START_WAIT_MS)
    {
        W906_HanaRmsPumpTick();                                             // [W906] golden Application->ProcessMessages(): the port has no message loop; the polled socket finishes a connect only inside Poll() (HanaRmsPump_St02.cpp, re-entry guarded) -- without it a real link never connects here and every START is refused
        Sleep(50);
        if(sHANARMSLinkErr.Length())                                        //已明確失敗就不必等滿
            break;
    }

    if(HANARMSIsConnected())
    {
        HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [CHK] ", "link OK before start");
        return true;
    }

    AnsiString asMsg = "HANA RMS server not available, start refused!!\r\n";
    if(sHANARMSLinkErr.Length())
        asMsg += sHANARMSLinkErr;
    else
        asMsg += "no response from " + edtHANARMSIP->Text + ":" + edtHANARMSPort->Text;
    HANARMSAddLog(mmoHANARMS, GetTimeInfo()+" [CHK] ", asMsg);
    ShowMyMessage(asMsg, "Check the RMS server and network, then press START again.",
                  "HANARMSCheckLinkBeforeStart");
    return false;
}
//---------------------------------------------------------------------------
