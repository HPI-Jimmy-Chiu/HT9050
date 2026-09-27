// =============================================================================
//  WebLogin.cpp -- 主畫面登入（golden TfMain 的登入三支）給網頁用。
//
//  Steven 20260924.  NOT in golden as a file; 每一段都是 golden main.cpp 的逐段移植，行號寫在段落上。
//  使用者 20260924：「預設使用下拉選單內含四個等級的方式，也就是 TfMain::cbUserSelectChange」
//  「是否使用密碼本需要看 FileExists(pwPath) || CosFunction.bUseLoginDatToSetLevel」「還有 btLoginClick」
//  「debug 模式，預設就是登入 HonPrec 權限」。
//
//  兩種模式（golden FormShow main.cpp:10863-10879）：
//    * 密碼本：FileExists(pwPath) || CosFunction.bUseLoginDatToSetLevel → btLogin＋spbUserName 顯示、
//      cbUserSelect 隱藏。按 btLogin → btLoginClick（main.cpp:28078）→ cbUserSelectChange 的密碼本分支
//      （帳號＋密碼；驗證本體在 WebAuth.cpp WebAuthVerify，既有的 WS auth.login）。
//    * 下拉選單（本機預設：pwPath=C:\winnt\system32\tech.com 不存在）：cbUserSelect（Operator／Engineer／
//      Supervisor／HonPrec，main.dfm:1679-1683）→ cbUserSelectChange 的「沒有密碼本」分支（:15342-15448）
//      → 升等才進 stOperatorClick（:13191）輸入密碼。
//
//  網頁端沒有模態框：golden 在 stOperatorClick 裡開 fQwertyKey 問密碼，這裡改成「需要密碼時回
//  needPassword，頁面開小鍵盤後帶密碼再送一次」—— 沒帶密碼時**不動任何狀態**。
//  ⚠ 刻意的差異（審查 20260924 L3）：golden 按「取消」關掉小鍵盤仍會執行 stOperatorClick 的 AccessLevel=0
//  （:13217 無條件）→ 降回 Operator；網頁版取消就是什麼都沒送，等級不變。
//  ⚠ 刻意的差異（使用者 20260924）：密碼錯誤 golden 是 ShowErrorMessage("WAR1677")（會 StopAllMotor），
//  網頁版 C++ 不呼叫，回 WEBLOGIN_BAD_CREDENTIALS／WEBLOGIN_BAD_PASSWORD，由頁面發不停機告警。
// =============================================================================
#include "WebLogin.h"

#include "WebAuth.h"
#include "cmydef.h"        // AccessLevel, pwPath, pwName, sSuperVisorString, iDefHonPrecLevel, SystemStart, CUSTOMER_CODE
#include "common.h"        // FileExists, CheckAndReadIniDataGeneral, WriteIniDataGeneral, CheckKeyExist, MyForceDirectories
#include "CosFunction.h"   // CosFunction.bUseLoginDatToSetLevel / bOEEFunction / bSecurityHave5Level
#include "Config.h"        // IniConfig.bSPILFunction
#include "LastSet.h"       // LastSet.szSupervisor
#include "MachineType.h"   // CC_*
#include "cprod.h"         // USER（PASS_WORD，login.dat）
#include "cMyDB.h"         // NewRecordProcess
#include "forms/fMain.h"   // fMain->DoChangeLevel / UpdateMainOperateMode / cbUserSelect

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "cpublic.h"       // SplitStrByDotSpaceOnly
#include "vclcompat/TStringList.h"

namespace {
// golden main.dfm:1674 cbUserSelect ItemIndex = 0；移植樹的 fMain->cbUserSelect 門面只有 Text，
// ItemIndex 在這裡保存（與 golden 的 cbUserSelect->ItemIndex 同一個意義）。
int s_itemIndex = 0;
// golden btLogin->Caption（"Login"／"Logout"）
bool s_btLoginIsLogout = false;
const char* const kItems[4] = {"Operator", "Engineer", "Supervisor", "HonPrec"};   // main.dfm:1680-1683
AnsiString UseName;                                                             // golden main.cpp:209（檔案層級）
AnsiString s_userCaption = "Operator";                                          // golden spbUserName->Caption

// golden ChangeLevelAttr 第一行就是 DoChangeLevel（main.cpp:12410）；移植樹的 ChangeLevelAttr 是空殼，
// 所以跟既有 auth.login 一樣直接呼叫 DoChangeLevel（fMain.h 說明）。
void ChangeLevelAttr() { fMain->DoChangeLevel(); }
}  // namespace

// ---------------------------------------------------------------------------
//  開機（wb_serve 在開機序列尾段呼叫一次）
// ---------------------------------------------------------------------------
// golden TfMain::InitialSuperVisorPassword（main.cpp:26458-26477），由 TfMain 建構子 :2057 呼叫
static void InitialSuperVisorPassword(int CustomerCode)
{
    switch(CustomerCode)
    {
        case CC_KYEC_CHEN:
            sSuperVisorString="922996522";  break;
        case CC_ASE_SG:
            sSuperVisorString="26338033";   break;
        case CC_AMKOR_Philippines:
        case CC_Microchip_Phil:                                                 //JerryYang 20180414 (Steven) Microchip菲律賓
            sSuperVisorString="07012015";   break;                              //JerryYang 20160227 新增密碼
        case CC_Greatek:
            sSuperVisorString="50005000";   break;                              //Sam 20171011 (wei) : 超豐新增密碼
        case CC_UTAC_TW:
            sSuperVisorString="50005000";   break;                              //Sam 20240606 : 客戶羅德修要求更換密碼
        default:
            sSuperVisorString="16943420";   break;
    }
}

void WebLogin_Boot()
{
    // golden TfMain 建構子 main.cpp:2057
    InitialSuperVisorPassword(CUSTOMER_CODE);

    // golden FormShow main.cpp:10566-10567
    if(std::strcmp(LastSet.szSupervisor,"")!=0)
        sSuperVisorString=LastSet.szSupervisor;

    // golden FormShow main.cpp:10837-10860（pwPath 依客戶選）
    MyForceDirectories("C:\\winnt\\system32\\");
    if(CUSTOMER_CODE==CC_AMKOR_China ||
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 (Steven) add QUALCOMM
    {
        pwPath="D:\\HT9045\\system\\userid.com";
        pwName="userid.com";                                                    //Steven 20221216 : 吳如春要改成從網路抓
    }
    else if(CosFunction.bUseLoginDatToSetLevel)                                 //Steven 20170301 (wei) : 使用Login.dat當密碼本
    {
        pwPath="D:\\HT9045\\system\\login.dat";
        pwName="login.dat";
    }
    else if(CosFunction.bOEEFunction)                                           //JimmyChiu 20220901 不需要密碼預設讀取
    {
        pwPath="";
        pwName="";                                                              //Steven 20221215
    }
    else
    {
        pwPath="C:\\winnt\\system32\\tech.com";
        pwName="tech.com";
    }

    // golden FormShow main.cpp:11062-11069：模擬／除錯組態預設 HonPrec（使用者 20260924「debug 模式，預設就是登入 HonPrec 權限」）
#if defined(SOFT_SIMULTE) || defined(DEBUG)
    AccessLevel=iDefHonPrecLevel;
    s_itemIndex=3;
    fMain->cbUserSelect->Text="HonPrec";
    ChangeLevelAttr();
    #ifdef SOFT_SIMULTE
    // golden :10869-10872（密碼本模式時 btLogin 顯示 Logout、spbUserName 顯示 HonPrec）
    s_btLoginIsLogout=WebLogin_UsesBook();
    s_userCaption="HonPrec";
    #endif
#else
    // golden :11070-11081（出貨組態）
    if(CUSTOMER_CODE==CC_HONPREC_QC)
    {
        fMain->cbUserSelect->Text="HonPrec";
        AccessLevel=iDefHonPrecLevel;                                           //最高權限
        s_itemIndex=3;
        ChangeLevelAttr();
    }
    else
    {
        if(FileExists(pwPath)==false)                                           //jou 2012-09-20 password formshow error
        {
            std::string m;
            WebLogin_Select(0, false, "", &m);                                  // cbUserSelect->ItemIndex=0; cbUserSelectChange(this);
        }
    }
#endif
    // 審查 20260924 M1：開機就講清楚——模擬組態會自動給 HonPrec
    std::printf("login: mode=%s pwPath=%s boot AccessLevel=%d (%s)%s\n",
                WebLogin_UsesBook() ? "book (btLogin)" : "select (cbUserSelect)", pwPath.c_str(), AccessLevel,
                fMain->cbUserSelect->Text.c_str(),
#if defined(SOFT_SIMULTE)
                "  <- SOFT_SIMULTE build: golden FormShow :11062 gives HonPrec at boot"
#else
                ""
#endif
                );
}

// golden FormShow main.cpp:10863-10864
bool WebLogin_UsesBook()
{
    return FileExists(pwPath) || CosFunction.bUseLoginDatToSetLevel;
}

// ---------------------------------------------------------------------------
//  golden TfMain::stOperatorClick（main.cpp:13191-13322）—— 密碼由參數帶進來（golden 是 fQwertyKey）
// ---------------------------------------------------------------------------
static void stOperatorClick(const AnsiString& password)
{
    int i, iLevel=0;
    AnsiString S1, S3, S4;
    // golden :13196-13202 KYEC_LEE 先刷 Barcode；:13206-13215 SCC／USE_BARCODE_AS_KEYBOARD 用 Barcode 輸入 ——
    // 網頁端的密碼一律由頁面小鍵盤給（見 WebLogin.h）

    AccessLevel=0;

    AnsiString HonPrecPassword;
    if(CheckKeyExist(asGeneralPath, "VENDER", "HONPREC")==false &&
       CheckKeyExist(asGeneralPath, "VENDER", "HONTECH"))                       //Sam 20250122 :　最高權限舊密碼進版相容
    {
        HonPrecPassword=CheckAndReadIniDataGeneral("VENDER", "HONTECH", AnsiString("27025312"));
        WriteIniDataGeneral("VENDER", "HONPREC", HonPrecPassword);
    }
    else
    {
        HonPrecPassword=CheckAndReadIniDataGeneral("VENDER", "HONPREC", AnsiString("27025312"));
        if(HonPrecPassword=="27025312")
            WriteIniDataGeneral("VENDER", "HONPREC", AnsiString("27025312"));
    }
    //Eliot 2008_08_26 Start
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        if(password==sSuperVisorString ||
           password=="100")
        {
            AccessLevel=iDefHonPrecLevel;                                       //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
        }
        else if(fMain->cbUserSelect->Text!="")                                  // golden: fPassword->edUserName->Text（cbUserSelectChange :15431 設成 cbUserSelect->Text）
        {
            S1=password.UpperCase();

            if(CosFunction.bSecurityHave5Level==true)                           //jou 2014-06-19 Security Have 5 Level
                iLevel=3;
            else
                iLevel=2;

            for(i=0; i<iLevel; i++)
            {
                S3=USER.ID[i];
                S3=S3.UpperCase();
                S4=USER.PassWord[i];
                S4=S4.UpperCase();
                if(S1==S4 && (s_itemIndex==i+1))
                {
                    AccessLevel=i+1;
                    break;
                }
            }
        }
    }
    else if(password==sSuperVisorString ||
            password==HonPrecPassword)
    {
        if((CUSTOMER_CODE==CC_KYEC_LEE  ||                                      //20140325 wei  KYEC如果HontechPassword=27025312 無法登入
            CUSTOMER_CODE==CC_KYEC_XILINX) &&
           password==HonPrecPassword)
        {
            return;
        }
        AccessLevel=iDefHonPrecLevel;                                           //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
        if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)         //20140308 Wei 需要知道哪個權限登入 KYEC
        {
            NewRecordProcess("MES2143", "======== HonPrec login ========");
        }
    }
    else if(password!="")
    {
        S1=password.UpperCase();
        // golden :13274-13295 另一臂：fSetup／fNote／MyMessageBox／fYieldMonitoring／fBinSel 自己要密碼的時候
        // （那些表單開著、正在問密碼）—— 主畫面登入不會走到，網頁端也沒有那些模態，只走下面這臂。
        {
            if(CosFunction.bSecurityHave5Level==true)                           //jou 2014-06-19 Security Have 5 Level
                iLevel=3;
            else
                iLevel=2;

            for(i=0; i<iLevel; i++)
            {
                S3=USER.ID[i];
                S3=S3.UpperCase();
                S4=USER.PassWord[i];
                S4=S4.UpperCase();
                if(S1==S4 && (s_itemIndex==i+1))
                {
                    AccessLevel=i+1;
                    break;
                }
            }
        }
    }
    //Eliot 2008_08_26 end
}

// ---------------------------------------------------------------------------
//  golden TfMain::cbUserSelectChange 的「沒有密碼本」分支（main.cpp:15342-15448）
// ---------------------------------------------------------------------------
int WebLogin_Select(int itemIndex, bool hasPassword, const AnsiString& password, std::string* msg)
{
    if(WebLogin_UsesBook())
    {
        *msg = "this machine uses the password book (btLogin + user/password): use auth.login";
        return WEBLOGIN_WRONG_MODE;
    }
    if(CosFunction.bSecurityHave5Level)
    {
        // 審查 20260924 M2：golden FormShow :11191-11196 會改 iDefHonPrecLevel=4 並重建 5 項下拉選單 —— 尚未移植
        *msg = "5-level security (CosFunction.bSecurityHave5Level) is not ported yet";
        return WEBLOGIN_BAD_ARG;
    }
    if(itemIndex<0 || itemIndex>3)
    {
        *msg = "itemIndex must be 0..3 (Operator/Engineer/Supervisor/HonPrec)";
        return WEBLOGIN_BAD_ARG;
    }
    const int oldIndex = s_itemIndex;
    s_itemIndex = itemIndex;                                                    // 使用者在下拉選單選了這一項（golden OnChange 時 ItemIndex 已是新值）

    // golden :15342-15348 chk_Honprec_Use（主畫面除錯勾選）—— 網頁端沒有這個勾選（預設未勾）

    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        for(int i=0; i<4; i++)
        {
            if(s_itemIndex==i && AccessLevel>=i)
            {
                AccessLevel=i;
                ChangeLevelAttr();
                fMain->UpdateMainOperateMode();
                return WEBLOGIN_OK;
            }
        }
    }
    else
    {
        if(s_itemIndex==0)
        {
            AccessLevel=0;
            ChangeLevelAttr();
            fMain->UpdateMainOperateMode();
            return WEBLOGIN_OK;
        }

        if(s_itemIndex==1 && AccessLevel>=1)
        {
            AccessLevel=1;
            ChangeLevelAttr();
            fMain->UpdateMainOperateMode();
            return WEBLOGIN_OK;
        }

        if(s_itemIndex==2 && AccessLevel>=2)
        {
            AccessLevel=2;
            ChangeLevelAttr();
            fMain->UpdateMainOperateMode();
            return WEBLOGIN_OK;
        }
    }

    // 以下是 golden 開小鍵盤問密碼（stOperatorClick）那一段：網頁先回 needPassword，帶密碼再來
    if(!hasPassword)
    {
        s_itemIndex = oldIndex;                                                 // 框還沒按 OK：什麼都沒變
        *msg = std::string("password required for ") + kItems[itemIndex];
        return WEBLOGIN_NEED_PASSWORD;
    }

    fMain->cbUserSelect->Text=kItems[s_itemIndex];                              // golden :15431 fPassword->edUserName->Text=cbUserSelect->Text
    const int wanted = s_itemIndex;
    stOperatorClick(password);
    if(AccessLevel<=0)
        s_itemIndex=0;
    ChangeLevelAttr();

    fMain->UpdateMainOperateMode();

    if(FileExists(pwPath)==false)                                               //Steven 20140815
    {
        if(AccessLevel==0)
        {
            asUser="Operator";
            NewRecordProcess("MES2140", "======== Operator login ========");
        }
        else if(AccessLevel==1)
        {
            asUser="Engineer";
            NewRecordProcess("MES2141", "======== Engineer login ========");
        }
        else if(AccessLevel==2)
        {
            asUser="Supervisor";
            NewRecordProcess("MES2142", "======== Supervisor login ========");
        }
        else if(AccessLevel==3)
        {
            asUser="HonPrec";
            NewRecordProcess("MES2143", "======== HonPrec login ========");
        }
    }
    // golden :15446-15447 SECS/GEM EventReport(SECS_EVENT.SwitchUser) —— 移植樹沒有 SECS 事件（列 todo）
    if(AccessLevel!=wanted)                                                     // 審查 20260924 H1：密碼不對（狀態已照 golden 回到 Operator）
    {
        *msg = std::string("WAR1677 wrong password for ") + kItems[wanted];
        return WEBLOGIN_BAD_PASSWORD;
    }
    return WEBLOGIN_OK;
}

// ---------------------------------------------------------------------------
//  golden TfMain::cbUserSelectChange 的密碼本分支（main.cpp:14968-15418；btLoginClick :28087-28089 從這裡進）
//  只移植本機會走到的兩臂：bUseLoginDatToSetLevel（login.dat）與預設文字密碼本（PassList[3]）。
//  N15UserLevelByTxt＋Greatek（FTP 下載密碼本）、bLoginASECL、bUseBarCoderAutoLogin 列 todo。
// ---------------------------------------------------------------------------
int WebLogin_BookLogin(const AnsiString& user, const AnsiString& password, const AnsiString& bookOverride,
                       std::string* msg)
{
    if(s_btLoginIsLogout)
    {
        // 審查 20260924 M4：golden btLoginClick :28082 只在 Caption=="Login" 時登入；Caption 是 Logout 時按下是登出
        *msg = "already logged in (btLogin shows Logout): log out first";
        return WEBLOGIN_ALREADY_IN;
    }
    const AnsiString book = bookOverride != "" ? bookOverride : pwPath;
    std::vector<AnsiString> ids, levels, pws;          // golden PassList[0]／[1]／[2]
    char dest[20];
    char str[512];

    if(!FileExists(book))
    {
        // golden :15420-15426：密碼本不見了 → WAR1681（這條是設定錯誤，不是密碼錯，照 golden 報、交給呼叫端）
        // 使用者 20260924「使用 nonstop」：golden ShowErrorMessage("WAR1681")（會 StopAllMotor）→ 頁面發不停機告警
        *msg = std::string("WAR1681 password book not found: ") + book.c_str();
        return WEBLOGIN_NO_BOOK;
    }

    bool bChange=false;
    if(IniConfig.bPasswordSecret)                                               //jou 2013-01-04 Password Txt 加密
    {
        bChange=CheckAndReadIniDataGeneral("Password", "Change", false);
    }

    if(CosFunction.bUseLoginDatToSetLevel && bookOverride == "")                //Steven 20170301 (wei) : 使用Login.dat當密碼本
    {
        FILE *Fp = std::fopen(book.c_str(), "rb");
        if(Fp!=NULL)                                                            //Steven 20200507 : 修正密碼本讀檔
        {
            std::fread((char *)&USER.RecordCT, sizeof(PASS_WORD), 1, Fp);
            std::fclose(Fp);
        }
        else
        {
            std::memset(&USER.RecordCT, 0, sizeof(PASS_WORD));
            *msg = "WAR1681 login.dat cannot be read";                          // 同上：不停機告警由頁面發
            return WEBLOGIN_NO_BOOK;
        }

        for(int i=0; i<1000; i++)                                               //Steven 20200507 : 修正密碼本讀檔 1000 --> USER.RecordCT
        {
            AnsiString S2=AnsiString(USER.ID[i]);                               //jou 2016-08-31 修正USER.ID NULL時會造成異常
            AnsiString S3=DecodeStr(S2);
            if(S3!="")                                                          //Alick 20160728 排除空白ID
            {
                ids.push_back(S3);
                S2.sprintf("%d", USER.Level[i]);
                levels.push_back(S2);
                S2=AnsiString(USER.PassWord[i]);                                //Steven 20200508 : 修正USER.PassWord NULL時會造成異常
                S3=DecodeStr(S2);
                pws.push_back(S3);
            }
        }
    }
    else
    {
        TStringList* L = new TStringList();                                     // golden PassList[3]->LoadFromFile(pwPath)
        L->LoadFromFile(book);
        for(int i=0; i<L->Count; i++)
        {
            std::strncpy(str, AnsiString(L->Strings[i]).c_str(), sizeof(str)-1);
            str[sizeof(str)-1]=0;
            SplitStrByDotSpaceOnly(str, dest, 20);
            ids.push_back(dest);
            SplitStrByDotSpaceOnly(str, dest, 20);
            levels.push_back(dest);
            SplitStrByDotSpaceOnly(str, dest, 20);
            AnsiString asPassword=dest;
            if(IniConfig.bPasswordSecret && bChange==true)                      //jou 2013-01-04 Password Txt 加密
            {
                asPassword=DecodeStr(asPassword);
            }
            pws.push_back(asPassword);
        }
        delete L;
    }

    if(!ids.empty())
    {
        for(std::size_t i=0; i<ids.size(); i++)
        {
            const bool bCheckOK=((user.UpperCase()==ids[i].UpperCase() ||
                                  JCET_FOR_EVAN==1) &&                          //Steven 20221215
                                 password.UpperCase()==pws[i].UpperCase());
            if(bCheckOK)
            {
                int l=std::atoi(levels[i].c_str());
                if(l>=0 && l<=3)
                {
                    AccessLevel=l;
                    s_itemIndex=l;
                    UseName=ids[i];
                    ChangeLevelAttr();
                    if(l==3)
                    {
                        fMain->cbUserSelect->Text="HonPrec";
                        s_userCaption="HonPrec";
                    }
                    else
                    {
                        s_userCaption=kItems[l];
                    }

                    if(CosFunction.bLoginShowUserName)                          //Steven 20230317 : 登入時顯示帳號名稱
                    {
                        s_userCaption=UseName;
                    }

                    asUser=UseName;
                    AnsiString S=UseName+AnsiString("===")+s_userCaption;
                    NewRecordProcess("MES2144", "======== USER login ========", S);
                    // golden :15393-15415 EnabledSetupFile(...)（主畫面 UI）—— HTML 端依 user.level 處理
                    s_btLoginIsLogout=true;                                     // golden :28088 btLogin->Caption="Logout"
                    return WEBLOGIN_OK;
                }
            }
        }
    }

    // golden :15419-15432 bError
    s_itemIndex=0;
    AccessLevel=0;
    ChangeLevelAttr();
    s_btLoginIsLogout=false;                                                    // btLogin->Caption="Login";
    fMain->cbUserSelect->Text="Operator";
    s_userCaption="Operator";
    // golden：ShowErrorMessage("WAR1677", 0, MMSystem, false, "Main--UserSelectChange") —— 會 StopAllMotor。
    // 使用者 20260924：密碼錯誤要發不停機告警 → C++ 不呼叫，頁面依回傳碼發 WAR1677（web/config/AlarmNonStop.json）
    *msg = "WAR1677 UserName or PassWord Error";
    return WEBLOGIN_BAD_CREDENTIALS;
}

// ---------------------------------------------------------------------------
//  golden TfMain::btLoginClick 的 Logout 半邊（main.cpp:28095-28127）；Login 半邊 → cbUserSelectChange
//  的密碼本分支＝既有 WS auth.login（呼叫端先檢 WebLogin_LoginAllowed）
// ---------------------------------------------------------------------------
bool WebLogin_LoginAllowed() { return !SystemStart; }                            // golden :28080-28081 if(SystemStart) return;

void WebLogin_LoggedIn() { s_btLoginIsLogout = true; }                          // golden :28088 btLogin->Caption="Logout"

bool WebLogin_Logout(std::string* msg)
{
    if(SystemStart)                                                             // golden :28080-28081
    {
        *msg = "machine is running (SystemStart): golden btLoginClick ignores the click";
        return false;
    }
    s_btLoginIsLogout = false;                                                  // golden: btLogin->Caption="Login";
    s_itemIndex=0;
    AccessLevel=0;
    s_userCaption="Operator";                                                   // golden :28101 spbUserName->Caption="Operator"
    // golden :28099 fMain->EnabledSetupFile(false) —— 主畫面 UI（HTML 端依 user.level 處理）
    ChangeLevelAttr();
    NewRecordProcess("MES2140", "======== Operator login ========");
    // golden :28102 TemperatureEditDisable()；:28104-28108 VTEST 關 fOffSet；:28110-28122 SPIL OFF-Line 切 ON-Line —— 列 todo
    return true;
}

std::string WebLogin_StateJson()
{
    std::string j = "{\"mode\":\"";
    j += WebLogin_UsesBook() ? "book" : "select";
    j += "\",\"level\":" + std::to_string(AccessLevel);
    j += ",\"itemIndex\":" + std::to_string(s_itemIndex);
    j += ",\"levelName\":\"" + std::string(fMain->cbUserSelect->Text.c_str()) + "\"";
    j += ",\"userCaption\":\"" + std::string(s_userCaption.c_str()) + "\"";   // golden spbUserName->Caption
    j += ",\"items\":[\"Operator\",\"Engineer\",\"Supervisor\",\"HonPrec\"]";
    j += ",\"btLogin\":\"";
    j += s_btLoginIsLogout ? "Logout" : "Login";
    j += "\",\"systemStart\":";
    j += SystemStart ? "true" : "false";
    j += "}";
    return j;
}
