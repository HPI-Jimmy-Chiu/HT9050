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
        // golden V912 :13283-13301 另一臂：fSetup／fNote／MyMessageBox／fYieldMonitoring／fBinSel 自己要密碼的時候（旗標 g_W906SetupAsksPassword 與那一臂 W906_StOperatorClickAskArm 宣告在 WebLogin.h 檔尾）
        if(g_W906SetupAsksPassword) W906_StOperatorClickAskArm(S1); else   //AI(W906-Q45-B5) 20260929 (St02-E): V906 只有 SetUp 的重新登入會問（W906_Reauth 設旗標）；那一臂的本體在檔尾。主畫面登入照舊只走下面這臂
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
int WebLogin_BookCompare(const AnsiString& user, const AnsiString& password, const AnsiString& bookOverride,   //AI(W906-Q45-B5) 20260929 (St02-E): golden cbUserSelectChange 密碼本分支本身（DoPassword 重新登入直接叫它）；btLoginClick 那一層＝檔尾的 WebLogin_BookLogin
                       std::string* msg)
{
    //if(s_btLoginIsLogout)   //AI(W906-Q45-B5) 20260929 (St02-E): 這一段是 golden btLoginClick 的 Caption 判斷，搬到檔尾的 WebLogin_BookLogin（DoPassword 不經過它）
    //{
    //    // 審查 20260924 M4：golden btLoginClick :28082 只在 Caption=="Login" 時登入；Caption 是 Logout 時按下是登出
    //    *msg = "already logged in (btLogin shows Logout): log out first";
    //    return WEBLOGIN_ALREADY_IN;
    //}
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

    bool W906_PwBookBinaryOverride(const AnsiString& ov); if((CosFunction.bUseLoginDatToSetLevel && bookOverride == "") || W906_PwBookBinaryOverride(bookOverride))   //Steven 20170301 (wei) : 使用Login.dat當密碼本   //AI(W906-SEC-Q9) 20260927: + a binary W906_PWBOOK_PATH override (probe use only; no effect when it is unset)
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
                    //s_btLoginIsLogout=true;                                   // golden :28088 btLogin->Caption="Logout"   //AI(W906-Q45-B5) 20260929 (St02-E): golden btLoginClick :28091 在叫 cbUserSelectChange 之前設，不在這裡 —— 檔尾 WebLogin_BookLogin 在 WEBLOGIN_OK 時設
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
//  AI(W906-D4) 20260928 (St02-E): D4 —— golden TfMain::ChangeTesterConnect 在 Off-Line → On-Line（Mode==10 && Msg、非 I27 手動整盤）時的
//  「強制回到 Operator」段（golden 906_0625_Steven main.cpp:12104-12131；912 :12621-12648，本體相同；註解「jou 2014-03-28 SPIL Handler
//  On-line & Offline Switch Flow」，但程式沒有客戶碼條件 —— golden 所有機台都會跑，照翻）。
//  呼叫端：forms/fMain.cpp TfMain::ChangeTesterConnect（原本 #if 0 的 TODO(W906-GB-P2d) D4 那一格）經 W906_WebLoginForceOperatorHook
//  （forms 庫不能直接連到本檔：很多 ctest 只連機台庫）；本檔在靜態初始化時裝上。安裝座本身定義在 LogObjects.cpp 檔尾（ht9045_db，跟
//  ht9045_forms 一定一起連）而不是 fMain.cpp —— fMain.cpp 那兩格（:1056 宣告、:1148 呼叫）可以單獨撤掉，本檔照樣連得起來。
//  ctest：WebLogin_ForceOperator（tests/test_weblogin_force_operator.cpp，三個分支＋安裝座；login.dat／levelset.dat 走測試縫）。
//  golden 的 VCL 元件照本檔既有的對應：cbUserSelect->ItemIndex＝s_itemIndex、spbUserName->Caption＝s_userCaption、
//  btLogin->Caption＝s_btLoginIsLogout、cbUserSelect->Text＝fMain->cbUserSelect->Text（user.level tag）；ChangeLevelAttr＝本檔的
//  DoChangeLevel 轉接（:57）。TemperatureEditDisable 是真的 TfMain 成員（forms/fMain_OperateMode.cpp）。
//  不在任何 WS 指令裡也可能被叫（遠端／機台流程切 On-Line）：不回訊息，只 printf 一行。
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // ---- JsonBridge/FormJson.cpp (the lock pw::Lock takes, below)
void W906_WebLoginForceOperator()
{   struct FormGuard { FormGuard() { ht9045::formjson::FormLock(); } ~FormGuard() { ht9045::formjson::FormUnlock(); } } formLock;   // AI(W906-ELA-REV) 20260928 (St02-E): the FormJson lock, as the WS ops (pw::Lock) that touch the same s_* / AccessLevel; the caller is the main-loop thread
    //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow
    s_itemIndex=0;                                                              // golden cbUserSelect->ItemIndex=0;
    AccessLevel=0;
    ChangeLevelAttr();

    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //wei 20160505 增加PE權限
        {
            fMain->cbUserSelect->Text="Operator";
            s_userCaption="Operator";                                           // golden spbUserName->Caption="Operator";
            NewRecordProcess("MES2140", "======== Operator login ========");
        }
        else
        {
            fMain->cbUserSelect->Text="Open";
            s_userCaption="Open";                                               // golden spbUserName->Caption="Open";
        }
    }
    else
    {
        fMain->cbUserSelect->Text="Operator";
        s_userCaption="Operator";                                               // golden spbUserName->Caption="Operator";
        NewRecordProcess("MES2140", "======== Operator login ========");
    }

    s_btLoginIsLogout=false;                                                    // golden btLogin->Caption="Login";
    fMain->TemperatureEditDisable();
    std::printf("[WebLogin] D4 Off-Line -> On-Line: forced back to %s (AccessLevel=0) -- golden 906_0625_Steven main.cpp:12104-12131 (912 :12621-12648)\n",
                s_userCaption.c_str());
}
extern void (*W906_WebLoginForceOperatorHook)();                               // LogObjects.cpp 檔尾（呼叫端 forms/fMain.cpp ChangeTesterConnect）
namespace { struct W906_D4HookInstall { W906_D4HookInstall() { W906_WebLoginForceOperatorHook = &W906_WebLoginForceOperator; } } g_d4HookInstall; }


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

// =============================================================================
//  //AI(W906-SEC-S55) 20260926: WS security.passwd —— Status.Security 頁的四顆改密碼鈕（只加，不改上面的登入）。
//
//  Steven S55「名單可以，密碼需要加密」。golden（V912，cp950，唯讀）：
//    按鈕  btnOperatorClick cSecurity.cpp:1667-1670 → ChangePassword(1)
//          sbEngineerClick  :944-950  → ChangePassword(5 階?2:1)
//          sbSupervisorClick:936-942  → ChangePassword(5 階?3:2)
//          btnHonPrecClick  :928-934  → ChangePassword(5 階?4:3)
//    權限  main.cpp:28588-28595 sbPasswordClick：Insufficient(29)（[29] Config - Password）不夠就不開表單；
//          FormShow :313-381 依 AccessLevel 決定三顆鈕看不看得到（看不到＝按不到），:429 btnOperator＝bSecurityHave5Level。
//          ⚠ golden 下拉選單模式**不驗舊密碼**（:795-811 只看新密碼非空）—— 權限檢查就只有上面兩道，照做。
//    本體  ChangePassword(iLevel) :600-811，對話框 TfLogin（login.cpp／login.dfm）：
//      * FileExists(pwPath)（密碼本模式，:611-794）：文字檔每行「帳號 等級 密碼」（SplitStrByDotSpaceOnly 切）。
//          cbLoginUserName 列出該等級的帳號；rgLoginOption New／Delete／Edit；存檔 List->SaveToFile(pwPath)。
//          IniConfig.bPasswordSecret：密碼欄用 EncodeStr（common.cpp:267-293，與 "HontechPassword" 逐字 XOR）；
//          第一次（Gerneral.ini [Password] Change=0）按任一顆鈕 → 整本轉成編碼、Change=1、"Password Secret finish!!"，不開對話框。
//      * 否則（下拉選單模式，:795-811）：HonPrec 那一顆直接 return；新密碼非空 → USER.ID[iLevel-1]=""、
//          USER.PassWord[iLevel-1]=新密碼（**明文**，stOperatorClick 用明文比對，見本檔 stOperatorClick）→ SavePassword
//          （cprod.cpp:1360-1371，整個 PASS_WORD 寫 d:\HT9045\system\login.dat）。
//      * bUseLoginDatToSetLevel（pwPath＝login.dat，二進位 USER 結構、帳號密碼都 EncodeStr，外部工具 PW_Editor 編）：golden 走的是
//          上面「文字密碼本」那條（FileExists 為真）—— 把二進位檔當文字讀、SaveToFile 蓋掉，關表單 FormClose :464 SavePassword
//          再用記憶體的 USER 蓋回來。淨效果＝什麼都沒改、中間 login.dat 是壞的。**V906 支援新增／刪除／修改**（Steven S131 Q9＝B，
//          記成新增；本檔尾的 W906_PwBinaryBook＋LoginDatBook.h）。偵測：密碼本檔內含 NUL 位元組（文字密碼本不會有）。
//
//  網頁端沒有模態框：op "open"＝golden 按下按鈕到 fLogin->ShowModal() 之前（含可能的整本轉碼）；op "apply"＝使用者按 sbOk 之後
//  （把 ShowModal 前那段重跑一次再接後半，檔案若在中間被改，以 apply 當下為準）。對話框右上角 X 在 golden 也會繼續往下跑
//  （ShowModal 回來照樣存）—— 網頁的取消就是什麼都不送（同 WebLogin 審查 L3 的刻意差異）。
//  錯誤碼：golden ShowErrorMessage 會 StopAllMotor；這裡不呼叫，回 code 讓頁面發不停機告警（比照 Steven 20260924 登入的裁決）。
//
//  **不送明文密碼**：所有回應只有帳號、等級、畫面可見性與結果碼。密碼欄（明文或 EncodeStr 後）一律不帶；不 printf、不寫 log。
//  測試縫：W906_PWBOOK_PATH（同 auth.login）指定文字密碼本；W906_LOGINDAT_PATH 指定下拉模式存檔的 login.dat（預設 golden 字面值）。
//
//  op（value＝JSON 字串）
//    state                         {"mode":"select|book|book-binary","allowed":b,"buttons":{名:{"visible":b,"level":n}},...}
//    list                          看得到的鈕對應等級的帳號名單 [{"name","level"}]（下拉模式沒有帳號，只回 slots）
//    open   {"button":"sbEngineer"}  golden ChangePassword 前半 → {"dialog":{TfLogin 各元件可見性／cbLoginUserName 名單}}
//    apply  {"button":..,"option":0|1|2,"user":"..","oldPassword":"..","newPassword":"..","confirmDelete":b}
//                                  golden 後半 → {"code":"MES1673|MES1674|MES1675|WAR1677|cancelled",...}／下拉模式 {"result":"saved"}；
//                                  擋下＝executed:false＋code（WAR1678／WAR1672／WAR1682）或 guard（WAR1676／not-authorized／binary-book／book-full／bad-value…）
// =============================================================================
#include <memory>                  // 放在這裡而不是檔頭：不位移上面既有程式的行號
#include "forms/fSecurity.h"       // fSecurity->Insufficient(29)
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp

namespace {
namespace pw {

struct Lock { Lock() { ht9045::formjson::FormLock(); } ~Lock() { ht9045::formjson::FormUnlock(); } };

const char* const kButton[4] = { "btnOperator", "sbEngineer", "sbSupervisor", "btnHonPrec" };   // cSecurity.dfm 元件名＝頁面 id
enum Mode { kSelect = 0, kBook = 1, kBookBinary = 2 };
const char* ModeName(Mode m) { return m == kSelect ? "select" : (m == kBook ? "book" : "book-binary"); }

// golden 各按鈕的 ChangePassword 參數
int ButtonLevel(int b)
{
    const bool five = (CosFunction.bSecurityHave5Level==true);
    switch (b) {
        case 0: return 1;                                                       // btnOperatorClick :1667-1670
        case 1: return five ? 2 : 1;                                            // sbEngineerClick  :944-950
        case 2: return five ? 3 : 2;                                            // sbSupervisorClick:936-942
        case 3: return five ? 4 : 3;                                            // btnHonPrecClick  :928-934
    }
    return -1;
}

// golden FormShow :313-381（switch(AccessLevel)）＋ :429（btnOperator）
bool ButtonVisible(int b)
{
    if (b == 0) return CosFunction.bSecurityHave5Level;                         // :429
    bool hon = false, sup = false, eng = false;
    if (CosFunction.bSecurityHave5Level==true) {
        switch (AccessLevel) {
            case 4: hon = true; sup = true; eng = true; break;                  // HonPrec
            case 3: sup = true; eng = true; break;                              // Supervisor
            case 2: eng = true; break;                                          // Engineer
            default: break;                                                     // 1 OP／0 Open
        }
    } else {
        switch (AccessLevel) {
            case 3: hon = true; sup = true; eng = true; break;                  // HonPrec
            case 2: sup = true; eng = true; break;                              // Supervisor
            case 1: eng = true; break;                                          // Engineer
            default: break;                                                     // 0 OP
        }
    }
    return b == 3 ? hon : (b == 2 ? sup : eng);
}

int ButtonIndex(const std::string& name)
{
    for (int i = 0; i < 4; ++i) if (name == kButton[i]) return i;
    return -1;
}

AnsiString BookPath()                                                           // golden pwPath；測試縫同 wb_serve auth.login
{
    const char* e = std::getenv("W906_PWBOOK_PATH");
    return (e && *e) ? AnsiString(e) : pwPath;
}
AnsiString LoginDatPath()                                                       // golden SavePassword／ReadPassword 的字面值（V912 cprod.cpp:1362／:1376）
{
    const char* e = std::getenv("W906_LOGINDAT_PATH");
    return (e && *e) ? AnsiString(e) : AnsiString("d:\\HT9045\\system\\login.dat");
}

bool FileHasNul(const AnsiString& path)
{
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) return false;
    char buf[4096]; std::size_t n; bool nul = false;
    while (!nul && (n = std::fread(buf, 1, sizeof(buf), fp)) > 0) nul = (std::memchr(buf, 0, n) != 0);
    std::fclose(fp);
    return nul;
}

Mode GetMode(AnsiString* book)
{
    *book = BookPath();
    if (!FileExists(*book)) return kSelect;                                     // golden :611 if(FileExists(pwPath)) … else :795
    return FileHasNul(*book) ? kBookBinary : kBook;
}

// JSON（UTF-8）→ ANSI（cp950）。golden 的帳號／密碼是 ANSI 位元組；轉不過去的字不可能出現在 golden 的密碼本裡。
bool Acp(const std::string& u, AnsiString* out)
{
    bool ascii = true;
    for (std::size_t i = 0; i < u.size(); ++i) if ((unsigned char)u[i] >= 0x80) { ascii = false; break; }
    if (ascii) { *out = u.c_str(); return true; }
    const int wn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u.c_str(), (int)u.size(), NULL, 0);
    if (wn <= 0) return false;
    std::wstring w((std::size_t)wn, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, u.c_str(), (int)u.size(), &w[0], wn);
    BOOL usedDef = FALSE;
    const int an = WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, NULL, 0, NULL, &usedDef);
    if (an <= 0 || usedDef) return false;
    std::string a((std::size_t)an, '\0');
    WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, &a[0], an, NULL, &usedDef);
    if (usedDef) return false;
    *out = a.c_str();
    return true;
}

// golden SavePassword（V912 cprod.cpp:1360-1371）；失敗時 golden ShowErrorMessage("WAR1682")（會停機）→ 這裡回 false，頁面發不停機告警
bool SaveUser()
{
    FILE *Fp=std::fopen(LoginDatPath().c_str(), "wb");
    if(Fp!=NULL)
    {
        std::fwrite((char *)&USER.RecordCT, sizeof(PASS_WORD), 1, Fp);
        std::fclose(Fp);
        return true;
    }
    return false;
}

struct BookLine { AnsiString name; int level; };

// 文字密碼本的「帳號 等級」（golden :630-642 同一套切法；密碼欄不讀出來）
void ReadBookNames(TStringList& List, std::vector<BookLine>* out)
{
    char cStr[256];
    char dest[20] = {0};                                                        // golden 未初始化；第一行切不到時是垃圾 —— 這裡清成 ""（安全性差異）
    for(int i=0; i<List.Count; i++)
    {
        std::strncpy(cStr, AnsiString(List.Strings[i]).c_str(), sizeof(cStr)-1);   // golden sizeof(cStr)（256 字以上沒有結尾 NUL）—— 同 WebLogin_BookLogin 補結尾
        cStr[sizeof(cStr)-1]=0;
        SplitStrByDotSpaceOnly(cStr, dest, 20);                                 //Name
        AnsiString asName=dest;
        SplitStrByDotSpaceOnly(cStr, dest, 20);                                 //Level
        out->push_back(BookLine{asName, std::atoi(dest)});
    }
}

int BoundedLen(const char* p, int n) { int i = 0; while (i < n && p[i]) ++i; return i; }   // MinGW.org 沒有 strnlen

// 二進位密碼本（login.dat，bUseLoginDatToSetLevel）的帳號／等級 —— 讀到自己的複本，不動 USER（golden main.cpp:15003-15020 同一套解法）
bool ReadBinaryNames(const AnsiString& path, std::vector<BookLine>* out)
{
    std::unique_ptr<PASS_WORD> u(new PASS_WORD());
    FILE* Fp = std::fopen(path.c_str(), "rb");
    if (!Fp) return false;
    const std::size_t got = std::fread((char *)&u->RecordCT, sizeof(PASS_WORD), 1, Fp);
    std::fclose(Fp);
    (void)got;                                                                  // golden 不看回傳值（短檔＝後面維持 0）
    for (int i = 0; i < 1000; i++) {
        const AnsiString S3 = DecodeStr(AnsiString(u->ID[i], BoundedLen(u->ID[i], (int)sizeof(u->ID[i]))));   // golden AnsiString(USER.ID[i]) 無界；30 字滿格時會讀進下一格 —— 這裡限在 30（安全性差異）
        if (S3 != "") out->push_back(BookLine{S3, u->Level[i]});
    }
    return true;
}

void WriteGuard(webbridge::JsonWriter& w, const std::string& op, const char* guard, const char* golden, const std::string& detail)
{
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(golden);
    w.Key("detail").String(detail);
}

const char* CodeMessage(const std::string& c)                                  // ShowErrorMessage 代碼的英文標題（golden 行號見呼叫處）
{
    if (c == "WAR1678") return "User Name or Password need KeyIn";
    if (c == "WAR1672") return "User Name Already exists";
    if (c == "MES1673") return "New User Finish";
    if (c == "MES1674") return "Delete User Finish";
    if (c == "WAR1677") return "UserName or PassWord Error";
    if (c == "MES1675") return "Modify User Finish";
    if (c == "WAR1682") return "login.dat cannot be written";
    if (c == "cancelled") return "Sure delete?? -> NO";
    return "";
}

void WriteCode(webbridge::JsonWriter& w, const std::string& code, const char* golden)
{
    w.Key("code").String(code);
    w.Key("message").String(CodeMessage(code));
    w.Key("alarm").Bool(code.compare(0, 3, "WAR") == 0);                       // 頁面據此發不停機告警
    w.Key("goldenLine").String(golden);
}

// TfLogin 的元件可見性（login.dfm 預設＋FormShow login.cpp:17-67＋rgLoginOptionClick :77-101）
void WriteDialog(webbridge::JsonWriter& w, bool book, int option, bool optionVisible, const std::vector<AnsiString>& names)
{
    w.Key("dialog").BeginObject();
    w.Key("caption").String("Login");                                           // login.dfm:6
    w.Key("labUserName").Bool(book);                                            // dfm:34 Visible=False；ChangePassword :624 設 true
    w.Key("edUserName").Bool(book);                                             // dfm:259／:625
    w.Key("cbLoginUserName").BeginObject();                                     // dfm:324／:627-628（名單＝該等級的帳號 :636-639）
    w.Key("visible").Bool(book);
    w.Key("items").BeginArray();
    for (std::size_t i = 0; i < names.size(); ++i) w.String(std::string(names[i].c_str()));
    w.EndArray();
    w.EndObject();
    w.Key("rgLoginOption").BeginObject();                                       // dfm:275-294 New／Delete／Edit
    w.Key("visible").Bool(book && optionVisible);
    w.Key("itemIndex").Number((wb_int64)option);
    w.Key("items").BeginArray(); w.String("New"); w.String("Delete"); w.String("Edit"); w.EndArray();
    w.EndObject();
    // 下拉模式：FormShow :53 只藏 rgLoginOption，舊／新密碼都照 dfm 顯示（舊密碼欄 golden 不讀）
    // 密碼本：FormShow :29-30 藏新密碼；rgLoginOptionClick：Delete 藏舊密碼、Edit 顯示新密碼並把標籤改成 OldPassword
    const bool oldVis = !book || option != 1;
    const bool newVis = !book || option == 2;
    w.Key("labOldPassword").BeginObject();
    w.Key("visible").Bool(oldVis);
    w.Key("caption").String(book && option == 2 ? "OldPassword" : "Password");
    w.EndObject();
    w.Key("edLoginOldPassword").Bool(oldVis);
    w.Key("labNewPassword").Bool(newVis);
    w.Key("edLoginNewPassword").Bool(newVis);
    w.EndObject();
}

std::string Done(webbridge::JsonWriter& w, bool* ok)
{
    w.EndObject();
    if (ok) *ok = true;
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

}  // namespace pw
}  // namespace

std::string W906_SecurityPasswdOp(const std::string& payloadJson, bool* ok)
{
    using namespace pw;
    if (ok) *ok = false;
    Lock lock;
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (!root) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"value is not JSON\"}";
    struct RootGuard { cJSON* r; ~RootGuard() { cJSON_Delete(r); } } rg{root};
    auto str = [root](const char* k) -> std::string {
        const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, k);
        return (j && cJSON_IsString(j) && j->valuestring) ? std::string(j->valuestring) : std::string();
    };
    const std::string op = str("op");
    if (op != "state" && op != "list" && op != "open" && op != "apply")
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"op must be state|list|open|apply\"}";

    AnsiString book;
    const Mode mode = GetMode(&book);
    const bool allowed = fSecurity->Insufficient(29, false);                   // main.cpp:28588-28595 sbPasswordClick
    webbridge::JsonWriter w;
    w.BeginObject();

    if (op == "state") {
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        w.Key("mode").String(ModeName(mode));
        w.Key("pwPath").String(std::string(book.c_str()));
        w.Key("accessLevel").Number((wb_int64)AccessLevel);
        w.Key("have5Level").Bool(CosFunction.bSecurityHave5Level);
        w.Key("allowed").Bool(allowed);
        w.Key("passwordSecret").Bool(IniConfig.bPasswordSecret);
        w.Key("buttons").BeginObject();
        for (int b = 0; b < 4; ++b) {
            w.Key(kButton[b]).BeginObject();
            w.Key("visible").Bool(ButtonVisible(b));
            w.Key("level").Number((wb_int64)ButtonLevel(b));
            w.EndObject();
        }
        w.EndObject();
        return Done(w, ok);
    }

    if (!allowed) {
        WriteGuard(w, op, "WAR1676", "main.cpp:28588-28595", "Insufficient(29): this AccessLevel cannot open the Security form");
        return Done(w, ok);
    }

    if (op == "list") {
        std::vector<BookLine> lines;
        bool readOk = true;
        if (mode == kBook) { TStringList List; List.LoadFromFile(book); ReadBookNames(List, &lines); }
        else if (mode == kBookBinary) readOk = ReadBinaryNames(book, &lines);
        w.Key("executed").Bool(readOk);
        w.Key("op").String(op);
        w.Key("mode").String(ModeName(mode));
        w.Key("users").BeginArray();                                            // 只有帳號與等級（S55：名單可以）；只列看得到的鈕對應的等級（golden 對話框的名單範圍）
        for (std::size_t i = 0; i < lines.size(); ++i) {
            bool vis = false;
            for (int b = 0; b < 4; ++b) if (ButtonVisible(b) && ButtonLevel(b) == lines[i].level) vis = true;
            if (!vis) continue;
            w.BeginObject();
            w.Key("name").String(std::string(lines[i].name.c_str()));
            w.Key("level").Number((wb_int64)lines[i].level);
            w.EndObject();
        }
        w.EndArray();
        w.Key("slots").BeginArray();                                            // 下拉模式：golden 沒有帳號，只有「哪個等級的密碼」
        for (int b = 0; b < 4; ++b) {
            if (!ButtonVisible(b)) continue;
            w.BeginObject(); w.Key("button").String(kButton[b]); w.Key("level").Number((wb_int64)ButtonLevel(b)); w.EndObject();
        }
        w.EndArray();
        return Done(w, ok);
    }

    const std::string btnName = str("button");
    const int b = ButtonIndex(btnName);
    if (b < 0) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"button must be btnOperator|sbEngineer|sbSupervisor|btnHonPrec\"}";
    if (CosFunction.bSecurityHave5Level) {                                      // 同 WebLogin_Select：golden FormShow :11191-11196 改 iDefHonPrecLevel=4 那段尚未移植
        WriteGuard(w, op, "five-level-not-ported", "main.cpp:11191-11196", "5-level security (CosFunction.bSecurityHave5Level) is not ported yet");
        return Done(w, ok);
    }
    if (!ButtonVisible(b)) {
        WriteGuard(w, op, "not-authorized", "cSecurity.cpp:313-381", "golden FormShow hides this button at the current AccessLevel");
        return Done(w, ok);
    }
    const int iLevel = ButtonLevel(b);
    const AnsiString asLevel = iLevel;                                          // golden :602 AnsiString asLevel=iLevel;

    w.Key("op").String(op);
    w.Key("button").String(btnName);
    w.Key("level").Number((wb_int64)iLevel);
    w.Key("mode").String(ModeName(mode));

    // ---- bUseLoginDatToSetLevel 的二進位密碼本（login.dat） ----
    //AI(W906-SEC-Q9) 20260927 (St02-E): Steven S131 Q9 = B -- New / Delete / Edit on the binary book are SUPPORTED now (a
    //  recorded addition: golden ChangePassword treats this file as text and FormClose restores it from memory, so golden
    //  never changes it; PW_Editor is golden's only writer).  The body is W906_PwBinaryBook at the end of this file
    //  (LoginDatBook.h: EncodeStr, the 64004-byte PASS_WORD layout, backup -> write -> verify -> restore).  It still answers
    //  the old "binary-book" guard when the book is not writable (bUseLoginDatToSetLevel off and no W906_PWBOOK_PATH probe
    //  override, or the file is not exactly 64004 bytes).
    if (mode == kBookBinary) {
        extern std::string W906_PwBinaryBook(const std::string& op, const cJSON* root, const AnsiString& book, int iLevel,
                                             webbridge::JsonWriter& w, bool* ok);
        return W906_PwBinaryBook(op, root, book, iLevel, w, ok);
    }
    // (the old guard -- ReadBinaryNames, executed:false, guard "binary-book", goldenLine "cSecurity.cpp:611-794 + :464",
    //  the users at iLevel -- is W906_PwBinaryBook's refusal branch now; these lines keep the rest of the file in place)
    //
    //
    //

    // ---- 下拉選單模式（golden :795-811） ----
    if (mode == kSelect) {
        if(iLevel>=iDefHonPrecLevel)                                            // :797-798
        {
            w.Key("executed").Bool(true);
            w.Key("result").String("noop");
            w.Key("goldenLine").String("cSecurity.cpp:797-798");
            w.Key("detail").String("golden returns without a dialog for the HonPrec level when there is no password book");
            return Done(w, ok);
        }
        if (op == "open") {
            w.Key("executed").Bool(true);
            WriteDialog(w, false, 0, false, std::vector<AnsiString>());
            return Done(w, ok);
        }
        AnsiString newPw;                                                       // edLoginNewPassword（這個分支 golden 不 Trim）
        if (!Acp(str("newPassword"), &newPw))
            return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"password has characters outside the ANSI code page\"}";
        if(newPw.IsEmpty())                                                     // :802-806
        {
            w.Key("executed").Bool(false);
            WriteCode(w, "WAR1678", "cSecurity.cpp:802-806");
            return Done(w, ok);
        }
        const int iUseID=iLevel-1;                                              // :603
        std::strncpy(USER.ID[iUseID]      , "", sizeof(USER.ID[iUseID]));        // :808 edUserName->Text 在這個分支恆為 ""（FormShow login.cpp:23 清空、dfm 看不到、sbOkClick :71 cbLoginUserName 看不到不複製）
        std::strncpy(USER.PassWord[iUseID], newPw.c_str(), sizeof(USER.PassWord[iUseID]));   // :809（golden 原樣：30 字以上沒有結尾 NUL）
        if (!SaveUser()) {                                                      // :810 SavePassword()
            w.Key("executed").Bool(false);
            WriteCode(w, "WAR1682", "cprod.cpp:1360-1371");
            return Done(w, ok);
        }
        w.Key("executed").Bool(true);
        w.Key("result").String("saved");
        w.Key("saved").Bool(true);
        w.Key("goldenLine").String("cSecurity.cpp:808-810");
        return Done(w, ok);
    }

    // ---- 文字密碼本模式（golden :611-794） ----
    bool bChange=false;
    if(IniConfig.bPasswordSecret)                                               // :613-621
        bChange=CheckAndReadIniDataGeneral("Password", "Change", false);

    TStringList List;
    List.LoadFromFile(book);                                                    // :625 List->LoadFromFile(pwPath)
    TStringList ListChange;
    std::vector<AnsiString> names;
    {
        char cStr[256];
        char dest[20] = {0};                                                    // golden 未初始化（見 ReadBookNames）
        AnsiString asName, asLevel1, asPass, asList;
        for(int i=0; i<List.Count; i++)                                         // :630-653
        {
            std::strncpy(cStr, AnsiString(List.Strings[i]).c_str(), sizeof(cStr)-1);
            cStr[sizeof(cStr)-1]=0;
            SplitStrByDotSpaceOnly(cStr, dest, 20);                             //Name
            asName=dest;
            SplitStrByDotSpaceOnly(cStr, dest, 20);                             //Level
            asLevel1=dest;
            if(std::atoi(dest)==std::atoi(asLevel.c_str()))
                names.push_back(asName);                                        // fLogin->cbLoginUserName->Items->Add(asName)

            if(IniConfig.bPasswordSecret && bChange==false)                     //jou 2013-01-04 Password Txt 加密
            {
                SplitStrByDotSpaceOnly(cStr, dest, 20);                         //Password
                asPass=dest;
                asPass=EncodeStr(asPass);
                asList.sprintf("%s %s %s", asName, asLevel1, asPass);
                ListChange.Add(asList);
            }
        }
    }
    if(IniConfig.bPasswordSecret && bChange==false)                             // :656-666 第一次：整本轉成編碼，不開對話框
    {
        WriteIniDataGeneral("Password","Change",true);
        ListChange.SaveToFile(book);
        w.Key("executed").Bool(true);
        w.Key("result").String("secret-converted");
        w.Key("message").String("Password Secret finish!!");                    // golden ShowMyMessage（頁面顯示）
        w.Key("saved").Bool(true);
        w.Key("goldenLine").String("cSecurity.cpp:656-666");
        return Done(w, ok);
    }

    // FormShow（login.cpp:27-50）：AMKOR China／QUALCOMM 且 AccessLevel<=Engineer → 只能 Edit、rgLoginOption 看不到
    const bool amkorEditOnly = ((CUSTOMER_CODE==CC_AMKOR_China || CUSTOMER_CODE==CC_QUALCOMM) && AccessLevel<=iDefEngineerLevel);
    if (op == "open") {
        w.Key("executed").Bool(true);
        WriteDialog(w, true, amkorEditOnly ? 2 : 0, !amkorEditOnly, names);
        return Done(w, ok);
    }

    // ---- sbOk 之後（golden :669-791） ----
    const cJSON* jopt = cJSON_GetObjectItemCaseSensitive(root, "option");
    int option = (jopt && cJSON_IsNumber(jopt)) ? jopt->valueint : -1;
    if (amkorEditOnly) option = 2;                                              // rgLoginOption 看不到，操作員改不了
    if (option < 0 || option > 2)
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"option must be 0 (New), 1 (Delete) or 2 (Edit)\"}";
    AnsiString user, oldPw, newPw;
    if (!Acp(str("user"), &user) || !Acp(str("oldPassword"), &oldPw) || !Acp(str("newPassword"), &newPw))
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"user or password has characters outside the ANSI code page\"}";
    const cJSON* jcd = cJSON_GetObjectItemCaseSensitive(root, "confirmDelete");
    const bool confirmDelete = (jcd && cJSON_IsTrue(jcd));
    user=user.Trim();                                                           // :673 edUserName（sbOkClick :71-72 取 cbLoginUserName->Text）
    oldPw=oldPw.Trim();                                                         // :674
    w.Key("option").Number((wb_int64)option);
    w.Key("user").String(std::string(user.c_str()));

    if(user.IsEmpty())                                                          // :682-689
    {
        w.Key("executed").Bool(false);
        WriteCode(w, "WAR1678", "cSecurity.cpp:682-689");
        return Done(w, ok);
    }
    else if(oldPw.IsEmpty())                                                    // :690-700
    {
        if(option!=1)                                                           // 刪除不需要 password
        {
            w.Key("executed").Bool(false);
            WriteCode(w, "WAR1678", "cSecurity.cpp:690-700");
            return Done(w, ok);
        }
    }

    AnsiString s, asPass;
    std::string code;
    const char* golden = "";
    int index = -1;
    switch(option)
    {
        case 0:                                                                 //New :703-727
            for(int i=0; i<List.Count; i++)
            {
                if(AnsiString(List.Strings[i]).Pos(user)==1)                    // golden 原樣：前綴比對（"ab" 也會擋 "abc"）
                {
                    w.Key("executed").Bool(false);
                    WriteCode(w, "WAR1672", "cSecurity.cpp:704-714");
                    return Done(w, ok);                                         // golden return：不存檔
                }
            }
            if(IniConfig.bPasswordSecret)
            {
                asPass=EncodeStr(oldPw);
                s=user+" "+asLevel+" "+asPass;
            }
            else
            {
                s=user+" "+asLevel+" "+oldPw;
            }
            List.Add(s);
            code="MES1673"; golden="cSecurity.cpp:715-726";
            break;
        case 1:                                                                 //Delete :728-755
            s=user+" "+asLevel;
            index=-1;
            for(int i=0; i<List.Count; i++)
            {
                if(AnsiString(List.Strings[i]).Pos(s)>0)                        // golden 原樣：包含即算
                {
                    index=i;
                    break;
                }
            }
            if(!confirmDelete)                                                  // :741-744 MessageBox "Sure delete??" 按 NO → break（仍會 SaveToFile）
            {
                code="cancelled"; golden="cSecurity.cpp:741-744";
                break;
            }
            if(index!=-1)
            {
                List.Delete(index);
                code="MES1674"; golden="cSecurity.cpp:746-750";
            }
            else
            {
                code="WAR1677"; golden="cSecurity.cpp:751-754";
            }
            break;
        default:                                                                //Edit :756-789（option==2）
            //AI(W906-FRW-S55) 20260927 [W906] 偏離 golden（Steven 20260927 ★ Q11＝B，RULINGS_20260926 S133）：新密碼空白就擋、不動密碼本。
            //  golden 的 Edit 沒檢查新密碼；修好下面的缺陷之後，空白新密碼會存成「帳號 等級 」，讀密碼本時第三欄切不到、沿用等級字
            //  （cpublic.cpp:317-320），等於密碼變成等級數字。比照 golden 下拉模式 cSecurity.cpp:802-806 回 WAR1678。加密、非加密兩支都擋。
            if(newPw.IsEmpty())
            {
                w.Key("executed").Bool(false);
                WriteCode(w, "WAR1678", "cSecurity.cpp:802-806 (W906: Edit with an empty new password, Steven Q11=B)");
                return Done(w, ok);
            }
            if(IniConfig.bPasswordSecret)
            {
                asPass=EncodeStr(oldPw);
                s=user+" "+asLevel+" "+asPass;
            }
            else
            {
                s=user+" "+asLevel+" "+oldPw;
            }
            index=List.IndexOf(s);                                              // 整行相等＝舊密碼對了（golden 唯一驗舊密碼的地方）
            if(index!=-1)
            {
                List.Delete(index);
                if(IniConfig.bPasswordSecret)
                {
                    asPass=EncodeStr(newPw);
                    s=user+" "+asLevel+" "+asPass;
                }
                else
                {
                    s=user+" "+asLevel+" "+newPw;                               //AI(W906-FRW-S55) 20260927 [W906] 修 golden 缺陷（Steven ★ Q11＝B，S133）：golden :779 寫的是 edLoginOldPassword，
                }                                                               //    非加密模式把舊密碼寫回、新密碼被丟掉（畫面卻顯示 MES1675）。V899 :777、V912 :779 都有同一個缺陷（不改 BCB，只通報 Jimmy）
                List.Add(s);
                code="MES1675"; golden="cSecurity.cpp:767-783";
            }
            else
            {
                code="WAR1677"; golden="cSecurity.cpp:784-787";
            }
            break;
    }

    List.SaveToFile(book);                                                      // :791
    w.Key("executed").Bool(true);
    w.Key("saved").Bool(true);
    WriteCode(w, code, golden);
    return Done(w, ok);
}
//---------------------------------------------------------------------------
//AI(W906-SEC-Q9) 20260927 (St02-E): Steven S131 Q9 = B -- New / Delete / Edit on the BINARY password book (login.dat,
//  CosFunction.bUseLoginDatToSetLevel).  A RECORDED ADDITION: golden's handler never writes this book (see the note at the
//  kBookBinary branch above and LoginDatBook.h).  The checks and codes follow the golden text-book branch above
//  (cSecurity.cpp:682-789):
//    * empty user, or empty old password except for Delete -> WAR1678;
//    * New: the user already exists at any level (UpperCase, like the login compare) -> WAR1672; else the first free slot
//      (PW_Editor's rule) at this button's level -> MES1673; no free slot -> guard "book-full" (new, no golden code);
//    * Delete: needs confirmDelete ("cancelled", nothing written); user + level not found -> WAR1677 (nothing written);
//      else the slot is cleared (PW_Editor's delete) -> MES1674;
//    * Edit: empty new password -> WAR1678 (the Q11 rule); user + level + old password must match, else WAR1677 (nothing
//      written); else only that slot's PassWord changes -> MES1675.
//  A value EncodeStr cannot store safely (empty, > 29 bytes, a byte < 0x20) -> guard "bad-value", nothing written.
//  The write is backup -> write -> verify (whole image, and nothing outside the slot changed) -> restore on failure (WAR1682).
//  After a verified write of the file memory USER mirrors (LoginDatPath(): the literal golden ReadPassword / SavePassword
//  path, or its W906_LOGINDAT_PATH seam), USER is reloaded from the written image: the levelset save (golden FormClose
//  SavePassword) writes USER back, and a stale USER would silently undo this edit.  A W906_PWBOOK_PATH probe book that is
//  another file leaves USER alone.
//  No password is ever returned, printed or logged.
#include "LoginDatBook.h"
#include <cstddef>
#include <cstdlib>

static_assert(sizeof(PASS_WORD) == logindat::kSize, "PASS_WORD must be the golden 64004-byte login.dat record");
static_assert(offsetof(PASS_WORD, ID) == logindat::kIdOff, "PASS_WORD.ID offset (golden 4)");
static_assert(offsetof(PASS_WORD, PassWord) == logindat::kPwOff, "PASS_WORD.PassWord offset (golden 30004)");
static_assert(offsetof(PASS_WORD, Level) == logindat::kLvOff, "PASS_WORD.Level offset (golden 60004)");

// A W906_PWBOOK_PATH probe override that is a binary book (exactly 64004 bytes, with a NUL) -- WebLogin_BookLogin :411.
bool W906_PwBookBinaryOverride(const AnsiString& ov)
{
    if (ov == "") return false;
    logindat::Image img;
    if (!logindat::Read(std::string(ov.c_str()), &img)) return false;
    for (std::size_t i = 0; i < img.size(); ++i) if (img[i] == 0) return true;
    return false;
}

std::string W906_PwBinaryBook(const std::string& op, const cJSON* root, const AnsiString& book, int iLevel,
                              webbridge::JsonWriter& w, bool* ok)
{
    using namespace pw;
    const char* ovEnv = std::getenv("W906_PWBOOK_PATH");
    const bool overrideSet = (ovEnv && *ovEnv);
    logindat::Image before;
    const bool readable = logindat::Read(std::string(book.c_str()), &before);
    if (!(CosFunction.bUseLoginDatToSetLevel || overrideSet) || !readable) {   // the old guard (golden defect, list only)
        std::vector<BookLine> lines;
        ReadBinaryNames(book, &lines);
        w.Key("executed").Bool(false);
        w.Key("guard").String("binary-book");
        w.Key("goldenLine").String("cSecurity.cpp:611-794 + :464");
        w.Key("detail").String(readable
            ? "the password book is binary but bUseLoginDatToSetLevel is off; golden never edits it -- edit it with PW_Editor"
            : "the binary password book is not exactly 64004 bytes (golden PASS_WORD); edit it with PW_Editor");
        w.Key("users").BeginArray();
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (lines[i].level != iLevel) continue;
            w.BeginObject(); w.Key("name").String(std::string(lines[i].name.c_str())); w.Key("level").Number((wb_int64)lines[i].level); w.EndObject();
        }
        w.EndArray();
        return Done(w, ok);
    }
    if (op == "open") {
        std::vector<AnsiString> names;
        for (int i = 0; i < logindat::kSlots; ++i)
            if (logindat::IdField(before, i)[0] != 0 && logindat::Level(before, i) == iLevel) {
                const AnsiString n = logindat::DecodedId(before, i);
                if (n != "") names.push_back(n);
            }
        w.Key("executed").Bool(true);
        WriteDialog(w, true, 0, true, names);
        return Done(w, ok);
    }

    auto str = [root](const char* k) -> std::string {
        const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, k);
        return (j && cJSON_IsString(j) && j->valuestring) ? std::string(j->valuestring) : std::string();
    };
    const cJSON* jopt = cJSON_GetObjectItemCaseSensitive(root, "option");
    const int option = (jopt && cJSON_IsNumber(jopt)) ? jopt->valueint : -1;
    if (option < 0 || option > 2)
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"option must be 0 (New), 1 (Delete) or 2 (Edit)\"}";
    AnsiString user, oldPw, newPw;
    if (!Acp(str("user"), &user) || !Acp(str("oldPassword"), &oldPw) || !Acp(str("newPassword"), &newPw))
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"user or password has characters outside the ANSI code page\"}";
    const cJSON* jcd = cJSON_GetObjectItemCaseSensitive(root, "confirmDelete");
    const bool confirmDelete = (jcd && cJSON_IsTrue(jcd));
    user = user.Trim();                                                          // as golden :673-674
    oldPw = oldPw.Trim();
    w.Key("option").Number((wb_int64)option);
    w.Key("user").String(std::string(user.c_str()));
    const char* const kQ9 = "W906 Q9 addition (Steven S131=B); checks as golden cSecurity.cpp:682-789";
    if (user.IsEmpty() || (oldPw.IsEmpty() && option != 1)) {
        w.Key("executed").Bool(false);
        WriteCode(w, "WAR1678", kQ9);
        return Done(w, ok);
    }
    logindat::Image after = before;
    int slot = -1;
    const char* code = "";
    auto refuse = [&](const char* guard, const char* detail) -> std::string {
        w.Key("executed").Bool(false);
        w.Key("guard").String(guard);
        w.Key("detail").String(detail);
        return Done(w, ok);
    };
    auto noWrite = [&](const char* c) -> std::string {                          // answered with the code, nothing saved
        w.Key("executed").Bool(true);
        w.Key("saved").Bool(false);
        WriteCode(w, c, kQ9);
        return Done(w, ok);
    };
    const char* const kBad = "a user name / password must be 1..29 characters with no control characters (EncodeStr, 30-byte field)";
    switch (option) {
    case 0:                                                                     // New
        if (logindat::UserExists(before, user)) { w.Key("executed").Bool(false); WriteCode(w, "WAR1672", kQ9); return Done(w, ok); }
        slot = logindat::FirstFree(before);
        if (slot < 0) return refuse("book-full", "all 1000 login.dat slots are used");
        if (!logindat::SetField(logindat::IdField(after, slot), user) || !logindat::SetField(logindat::PwField(after, slot), oldPw))
            return refuse("bad-value", kBad);
        logindat::SetLevel(after, slot, iLevel);
        code = "MES1673";
        break;
    case 1:                                                                     // Delete
        if (!confirmDelete) return noWrite("cancelled");
        slot = logindat::FindUser(before, user, iLevel);
        if (slot < 0) return noWrite("WAR1677");
        logindat::ClearSlot(after, slot);
        code = "MES1674";
        break;
    default:                                                                    // Edit
        if (newPw.IsEmpty()) { w.Key("executed").Bool(false); WriteCode(w, "WAR1678", kQ9); return Done(w, ok); }
        slot = logindat::FindUser(before, user, iLevel);
        if (slot < 0 || !(logindat::DecodedPw(before, slot) == oldPw)) return noWrite("WAR1677");
        if (!logindat::SetField(logindat::PwField(after, slot), newPw)) return refuse("bad-value", kBad);
        code = "MES1675";
        break;
    }
    std::string err;
    if (!logindat::WriteVerified(std::string(book.c_str()), before, after, slot, &err)) {
        w.Key("executed").Bool(false);
        WriteCode(w, "WAR1682", kQ9);
        w.Key("detail").String(err);
        return Done(w, ok);
    }
    if (book.UpperCase() == LoginDatPath().UpperCase())                          // memory == file (see the note above)
        std::memcpy(&USER.RecordCT, &after[0], logindat::kSize);
    w.Key("executed").Bool(true);
    w.Key("saved").Bool(true);
    WriteCode(w, code, kQ9);
    return Done(w, ok);
}

// =============================================================================
//  AI(W906-Q45-B5) 20260929 (St02-E): Q45 re-login pieces (St01 B5; design .claude/skills/ht9050-st01-evaluations/references/
//  q45-web-password.md 3.2).  golden DoPassword (V912 cSetUp.cpp:4325-4362 / cConfiguration.cpp:6482-6529) goes past
//  btLoginClick and calls
//    book mode    fMain->cbUserSelectChange(NULL)  -> WebLogin_BookCompare (:383, the password-book branch itself)
//    select mode  fMain->stOperatorClick(fMain)    -> stOperatorClick (:167), with SetUp asking -> its other arm (below)
//  The re-login itself (W906_Reauth) is St01's and goes AFTER this block.  Kept at the file end so that lines 1-1369 do
//  not move (WebCmdGuard.cpp:841 and tools/webprobe/wbrun_guard.py:574 cite them); the 10 lines changed above are same-line.
// =============================================================================

// golden btLoginClick (V912 main.cpp:28080-28094): Caption "Login" -> "Logout", then cbUserSelectChange.  auth.login
//   (tools/wb_serve.cpp:5511) calls this; its behaviour is unchanged: "Logout" only when the compare succeeded (as before).
int WebLogin_BookLogin(const AnsiString& user, const AnsiString& password, const AnsiString& bookOverride,
                       std::string* msg)
{
    if(s_btLoginIsLogout)
    {
        // 審查 20260924 M4：golden btLoginClick :28082 只在 Caption=="Login" 時登入；Caption 是 Logout 時按下是登出
        *msg = "already logged in (btLogin shows Logout): log out first";
        return WEBLOGIN_ALREADY_IN;
    }
    const int r = WebLogin_BookCompare(user, password, bookOverride, msg);
    if(r==WEBLOGIN_OK)
        s_btLoginIsLogout=true;                                                 // golden :28091 btLogin->Caption="Logout"
    return r;
}

// golden stOperatorClick's other arm (V912 main.cpp:13283-13301), taken when (fSetup->fShow==true && fSetup->bNeedPassword==true)
//   || fNote / MyMessageBox / fYieldMonitoring / fBinSel ask for the password.  V906: only SetUp's re-login sets the flag
//   (W906_Reauth, around its select-mode stOperatorClick call).  The drop-down is not looked at: Supervisor's password
//   (USER.PassWord[1]) first, then Engineer's ([0]); S1 is already upper-cased by the caller (golden :13281).
bool g_W906SetupAsksPassword = false;
void W906_StOperatorClickAskArm(const AnsiString& S1)
{
    int i;
    AnsiString S3, S4;
    for(i=1; i>=0; i--)
    {
        S3=USER.ID[i];
        S3=S3.UpperCase();
        S4=USER.PassWord[i];
        S4=S4.UpperCase();
        if(S1==S4 )
        {
            AccessLevel=i+1;
            break;
        }
    }
}

// =============================================================================
//  AI(W906-Q45-B5) 20260930 (St01): W906_Reauth -- Q45 "甲" re-login: #4 Configuration [M01], #6 / #7 SetUp RTC-off / OCR-off.
//  Declarations and the message format: WebReauth.h.  Design: .claude/skills/ht9050-st01-evaluations/references/
//  q45-web-password.md 2.0 / 3.2 (Steven Q45 "按照你的建議執行" = 1A 2A 3A 4A 5A 6A 7B 8A 9A 10A; 20260929 "請按照bcb的邏輯處理").
//  Appended only: St02's lines 1-1419 above are unchanged and do not move.
//
//  golden (V912, cp950, read-only), the two DoPassword functions side by side:
//    TfSetup::DoPassword  cSetUp.cpp:4325-4362             TfConfiguration::DoPassword  cConfiguration.cpp:6482-6529
//      bFlag=true; bTechComExist=FileExists(pwPath);          the same, plus
//      iLevel=LevelSet.AccessLevel[37];                       iLevel=LevelSet.AccessLevel[92]; if(iLevel==0) return true;
//      if(REAL_TIME_CCD) { if(fQwertyKey->bShow==false) {     if(REAL_TIME_CCD) { if(fInput->fShow==false) {
//        bTechComExist ? fMain->cbUserSelectChange(NULL)        the same login and level compare, then
//                      : fMain->stOperatorClick(fMain);         if(bTechComExist) { btLogin "Login"; spbUserName "Operator";
//        if(AccessLevel<iLevel) bFlag=false;                      cbUserSelect->ItemIndex=0; AccessLevel=0; ChangeLevelAttr(); }
//      } }  return bFlag;                                     } }  return bFlag;
//  Port mapping (VCL pieces as this file already maps them, :46-57 and the D4 block):
//    cbUserSelectChange(NULL) = WebLogin_BookCompare (St02, :383) -- NOT WebLogin_BookLogin: golden DoPassword does not go through
//      btLoginClick, so an operator who is already logged in is asked too.  The book is W906_PWBOOK_PATH when that test seam is
//      set (exactly as auth.login, tools/wb_serve.cpp:5501-5511), otherwise golden's pwPath.
//    stOperatorClick(fMain) = stOperatorClick (:167).  SetUp: g_W906SetupAsksPassword = golden (fSetup->fShow && fSetup->bNeedPassword)
//      for the call, so its :239 arm runs W906_StOperatorClickAskArm (St02, :1403 = golden main.cpp:13283-13301).  Configuration:
//      fSetup is never open at the same time (both forms are ShowModal, golden main.cpp:28471 / :28624) -> the normal arm, only the
//      drop-down's current level (:13304-13320).  golden quirk kept (Q45-5 = A): stOperatorClick only sets AccessLevel -- no
//      ChangeLevelAttr, the drop-down does not move, and Configuration does not log out in this mode (golden logs out only when
//      bTechComExist).
//    fQwertyKey->bShow / fInput->fShow: the server has no keypad form, it is never showing -> golden always asks here.
//    Cancel (Q45-4 = A): golden clears the box before ShowModal (main.cpp:15170 / :15184 book, :13205 select) and compares after it
//      returns whichever button closed it -> blank user name + blank password -> wrong.
//  Event-log rows are golden's own: MES2144 "USER login" inside WebLogin_BookCompare, the KYEC HonPrec row in stOperatorClick,
//    SetUp's "RealTimeCCD Disabled" / "OCR Disabled" in the generated sbUpdateClick.  The Configuration logout writes no row (golden
//    :6518-6525 has none).  A wrong book password = golden bError (main.cpp:15333-15344) ShowErrorMessage("WAR1677"): as auth.login,
//    C++ does not raise it (it stops the motors); r->alarm tells the page to raise the non-stop alarm (Steven 20260924).
//  Not here (the page does it): the login box (HTQwerty) and its Label3 / Label4 "level too low" captions (r->reason instead).
//  [W906] one deviation (Q45-2 = A): golden asks on every M01 click; the web asks once when Save is pressed, for all changed cells.
//  No password is printed, logged or returned; the stash zeroes its copies (and the JSON strings) when it is done.  The callers
//  hold FormLock (editlist.save) -- nothing here takes it (FormLock is not re-entrant, see the D4 block).
// =============================================================================
#include "WebReauth.h"
#include <algorithm>

namespace {
namespace reauth {

const char* const kM01[]  = {"cbM01", "cbM01_01", "cbM01_02", "cbM01_03", "cbM01_04", "cbM01_05", "cbM01_06", "cbM01_07",
                             "cbM01_08", "cbM01_09", "cbM01_10", "cbM01_11", "cbM01_12", "cbM01_13", "cbM01_14", "cbM01_15"};   // golden cConfiguration.dfm:21162-21305 OnClick=cbM01Click
const char* const kRtc[]  = {"cbEnableRealTimeCCD"};                             // golden cSetUp.cpp:3573
const char* const kOcr[]  = {"cbOcrFunction"};                                   // golden cSetUp.cpp:3574
const char* const kC12[]  = {"cbC12", "cbC13", "chkC14", "cbC17", "cbC24"};       // golden cConfiguration.cpp:6775 cbC12Click (dfm:3178 / :3161 / :3238 / :3533 / :3903)
const char* const kA27[]  = {"cbA27"};                                           // golden :6824 cbA27Click (dfm:1323)
const char* const kN075[] = {"cbN07_EnableEmployeeCheak"};                       // golden :6860 cbN07_EnableEmployeeCheakClick (dfm:15080)
const char* const kSgpw[] = {"Image1", "grpA32_1"};                              // golden :7274 Image1DblClick (dfm:1478) -> grpA32_1 visible

const char kSetupTag[]  = "TestIF_File_SetUp";
const char kConfigTag[] = "IniConfig";
const char kVendorWhy[] = "這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改";   // Q45 #1..#3 = C (design 3.2.1)
const char kSgpwWhy[]   = "Check List Enable 區塊（[A32_1]）要 SG_PW.ini 的密碼，網頁版不提供，請到 BCB 版機台改";   // Q45 #5 = C

struct Stash {                   // one editlist.save
    bool present = false;        // the value carried "reauth" (beside widgets)
    std::string tag;
    W906ReauthAnswer answer;
    bool used = false;           // a golden DoPassword took it
    bool haveResult = false;     // a golden DoPassword ran (with or without an answer)
    W906ReauthResult result;
};
Stash g;

void Wipe(std::string* s)
{
    if (!s) return;
    std::fill(s->begin(), s->end(), '\0');
    s->clear();
}
void WipeNode(cJSON* n)          // every string value under n -> zero bytes (n itself included)
{
    for (; n; n = n->next) {
        if (cJSON_IsString(n) && n->valuestring) std::memset(n->valuestring, 0, std::strlen(n->valuestring));
        WipeNode(n->child);
    }
}
void WipeJson(cJSON* j)          // a DETACHED item: zero its strings, then free it
{
    if (!j) return;
    if (cJSON_IsString(j) && j->valuestring) std::memset(j->valuestring, 0, std::strlen(j->valuestring));
    WipeNode(j->child);
    cJSON_Delete(j);
}
bool IsConfig(const char* tag) { return tag && std::strcmp(tag, kConfigTag) == 0; }
bool IsSetup(const char* tag)  { return tag && std::strcmp(tag, kSetupTag) == 0; }
bool PointOf(const std::string& tag, const std::string& point)
{
    if (tag == kSetupTag)  return point == "rtcOff" || point == "ocrOff";
    if (tag == kConfigTag) return point == "m01";
    return false;
}
// golden FileExists(pwPath); the test seam W906_PWBOOK_PATH replaces pwPath as auth.login does (tools/wb_serve.cpp:5501-5511)
AnsiString BookOverride()
{
    const char* e = std::getenv("W906_PWBOOK_PATH");
    return (e && *e) ? AnsiString(e) : AnsiString("");
}
AnsiString BookPath() { const AnsiString o = BookOverride(); return o != "" ? o : pwPath; }
// UTF-8 from the page -> the ANSI bytes the golden keypad would have typed (pw::Acp, as security.passwd).  Text with no ANSI form
// cannot be in a golden book: kept as its raw bytes, so it matches nothing (as auth.login today).
AnsiString Typed(const std::string& u)
{
    AnsiString a;
    if (pw::Acp(u, &a)) return a;
    return AnsiString(u.c_str());
}
struct SetupArm {                // golden (fSetup->fShow==true && fSetup->bNeedPassword==true) while stOperatorClick runs
    explicit SetupArm(bool on) : was(g_W906SetupAsksPassword) { g_W906SetupAsksPassword = on; }
    ~SetupArm() { g_W906SetupAsksPassword = was; }
    bool was;
};  struct WipeOnExit { explicit WipeOnExit(AnsiString* s) : p(s) {} ~WipeOnExit() { for (int i = 1; i <= p->Length(); ++i) (*p)[i] = '\0'; } AnsiString* p; };   // AI(W906-Q45-B5) 20260930: zeroes a typed user name / password copy when its block ends (ST01-E review of 699dc06d, note 1); same line
W906ReauthAnswer TakeAnswer(const char* tag)   // the page's answer for this page, once
{
    W906ReauthAnswer a;
    if (g.present && !g.used && g.tag == tag) {
        a = g.answer;
        g.used = true;
        Wipe(&g.answer.password);
        Wipe(&g.answer.userId);
    }
    return a;
}
void Keep(const W906ReauthResult& r) { g.haveResult = true; g.result = r; }
std::string Num(int v) { return std::to_string(v); }
void Controls(webbridge::JsonWriter& w, const char* const* c, int n)
{
    w.Key("controls").BeginArray();
    for (int i = 0; i < n; ++i) w.String(c[i]);
    w.EndArray();
}

}  // namespace reauth
}  // namespace

bool W906_Reauth(const char* tag, bool setupNeedPassword, const W906ReauthAnswer& a, W906ReauthResult* r)
{
    using namespace reauth;
    W906ReauthResult out;
    const bool config = IsConfig(tag);
    out.point = a.point;
    out.answered = a.present;
    out.cancelled = a.present && a.cancelled;
    out.levelItem = config ? 92 : 37;
    out.golden = config ? "V912 cConfiguration.cpp:6482-6529 TfConfiguration::DoPassword (from cbM01Click :6531-6544)"
                        : "V912 cSetUp.cpp:4325-4362 TfSetup::DoPassword (from sbUpdateClick :3573-3606)";
    out.levelBefore = AccessLevel;

    bool bFlag=true;
    bool bTechComExist=FileExists(BookPath());                                  //2012-01-03    Dell modify
    int iLevel=0;

    iLevel=LevelSet.AccessLevel[out.levelItem];                                 // [37] SetUp :4331 / [92] Configuration :6488
    out.required = iLevel;
    out.mode = bTechComExist ? "book" : "select";
    if(config && iLevel==0)                                                     //ChungHung 20140331 alter   (Configuration only, :6489-6490)
    {
        out.reason = "levelset item 92 is 0: golden returns true without asking (cConfiguration.cpp:6489-6490)";
        out.level = AccessLevel;
        out.login = WebLogin_StateJson();
        *r = out;
        return true;
    }
    if(REAL_TIME_CCD)
    {
        // golden :4335 if(fQwertyKey->bShow==false) / :6493 if(fInput->fShow==false): no keypad form on the server -> always taken
        out.asked = true;
        if(!a.present)
        {
            // golden shows the login box here; this save brought no answer -> the caller keeps its refusal, nothing changed
            out.handled = false;
            out.passed = false;
            out.reason = "golden asks for a re-login here, but this save carried no reauth answer";
            out.level = AccessLevel;
            out.login = WebLogin_StateJson();
            *r = out;
            return false;
        }
        AnsiString user = a.cancelled ? AnsiString("") : Typed(a.userId);  WipeOnExit wipeUser(&user);   // Q45-4 = A: cancel = the cleared box; zeroed when this block ends
        AnsiString pass = a.cancelled ? AnsiString("") : Typed(a.password);  WipeOnExit wipePass(&pass);
        std::string why;
        if(bTechComExist)
        {
            std::string msg;
            const int lr = WebLogin_BookCompare(user, pass, BookOverride(), &msg);   // golden fMain->cbUserSelectChange(NULL);
            if(lr==WEBLOGIN_BAD_CREDENTIALS)
            {
                out.alarm = "WAR1677";
                why = "wrong user name or password (golden bError main.cpp:15333-15344: Operator + WAR1677)";
            }
            else if(lr==WEBLOGIN_NO_BOOK)
            {
                out.alarm = "WAR1681";
                why = "the password book cannot be read (golden WAR1681; level unchanged)";
            }
            else
            {
                why = "password book login";
            }
        }
        else
        {
            SetupArm arm(!config && setupNeedPassword);
            stOperatorClick(pass);                                              // golden fMain->stOperatorClick(fMain);
            why = !config && setupNeedPassword ? "drop-down password compare, SetUp arm (main.cpp:13283-13301)"
                                               : "drop-down password compare, current drop-down level only (main.cpp:13304-13320)";
        }

        if(AccessLevel<iLevel)
        {
            bFlag=false;
            // golden :4344-4349 / :6503-6508 fPassword->Label3/Label4->Visible (the box's "level too low" captions) -> out.reason
        }
        if(a.cancelled)
            why = "login box cancelled = blank user name and password (Q45-4), " + why;
        why += "; level " + Num(AccessLevel) + (bFlag ? " >= " : " < ") + Num(iLevel) + " (levelset item " + Num(out.levelItem) + ")";

        if(config && bTechComExist)                                             // golden cConfiguration.cpp:6518-6525
        {
            s_btLoginIsLogout=false;                                            // fMain->btLogin->Caption="Login";
            s_userCaption="Operator";                                           // fMain->spbUserName->Caption="Operator";
            s_itemIndex=0;                                                      // fMain->cbUserSelect->ItemIndex=0;
            AccessLevel=0;
            ChangeLevelAttr();                                                  // fMain->ChangeLevelAttr();
            out.loggedOut = true;
            why += "; then logged out to Operator (cConfiguration.cpp:6518-6525)";
        }
        out.reason = why;
    }
    else
    {
        out.reason = "REAL_TIME_CCD is off (Gerneral.ini [System]): golden does not ask";
    }
    out.passed = bFlag;
    out.level = AccessLevel;
    out.login = WebLogin_StateJson();
    *r = out;
    return bFlag;
}

// ---- tools/wb_serve.cpp editlist.save ------------------------------------------------------------------------------
std::string W906_ReauthTake(cJSON* root, const std::string& tag)
{
    using namespace reauth;
    W906_ReauthClear();                                                         // a stale answer never carries over
    if (!root || !cJSON_IsObject(root)) return std::string();                   // the shape error is wb_serve's
    cJSON* jw = cJSON_GetObjectItemCaseSensitive(root, "widgets");
    cJSON* inside = (jw && cJSON_IsObject(jw)) ? cJSON_DetachItemFromObjectCaseSensitive(jw, "reauth") : nullptr;
    cJSON* ra = cJSON_DetachItemFromObjectCaseSensitive(root, "reauth");
    std::string refused;
    if (inside) {
        refused = "reauth must sit beside widgets, never inside it (Q45 design 3.2.2 / 3.2.5) -- nothing saved";
    } else if (ra) {
        const cJSON* jp  = cJSON_IsObject(ra) ? cJSON_GetObjectItemCaseSensitive(ra, "point") : nullptr;
        const cJSON* jc  = cJSON_IsObject(ra) ? cJSON_GetObjectItemCaseSensitive(ra, "cancelled") : nullptr;
        const cJSON* ju  = cJSON_IsObject(ra) ? cJSON_GetObjectItemCaseSensitive(ra, "userId") : nullptr;
        const cJSON* jpw = cJSON_IsObject(ra) ? cJSON_GetObjectItemCaseSensitive(ra, "password") : nullptr;
        if (!cJSON_IsObject(ra))
            refused = "reauth must be an object: {point, userId?, password} or {point, cancelled:true}";
        else if (!jp || !cJSON_IsString(jp) || !jp->valuestring)
            refused = "reauth.point must be a string";
        else if (!PointOf(tag, jp->valuestring))
            refused = "reauth.point is not a re-login point of " + tag + " (TestIF_File_SetUp: rtcOff / ocrOff; IniConfig: m01) -- nothing saved";
        else if (jc && !cJSON_IsBool(jc))
            refused = "reauth.cancelled must be true or false";
        else if (ju && !(cJSON_IsString(ju) && ju->valuestring))
            refused = "reauth.userId must be a string";
        else if (!cJSON_IsTrue(jc) && !(jpw && cJSON_IsString(jpw) && jpw->valuestring))
            refused = "reauth.password must be a string (or send cancelled:true)";
        else {
            g.present = true;
            g.tag = tag;
            g.answer.present = true;
            g.answer.point = jp->valuestring;
            g.answer.cancelled = cJSON_IsTrue(jc) != 0;
            if (!g.answer.cancelled) {                                          // cancelled: whatever else was typed is dropped
                if (ju) g.answer.userId = ju->valuestring;
                g.answer.password = jpw->valuestring;
            }
        }
    }
    WipeJson(inside);
    WipeJson(ra);
    if (!refused.empty()) W906_ReauthClear();
    return refused;
}

void W906_ReauthAck(std::string* ack)
{
    using namespace reauth;
    if (!ack || ack->size() < 2 || (*ack)[0] != '{') return;
    if (!g.present && !g.haveResult) return;
    webbridge::JsonWriter w;
    w.BeginObject();
    if (g.haveResult) {
        const W906ReauthResult& r = g.result;
        w.Key("point").String(r.point.empty() ? g.answer.point : r.point);
        w.Key("answered").Bool(r.answered);
        w.Key("asked").Bool(r.asked);
        w.Key("handled").Bool(r.handled);
        w.Key("passed").Bool(r.passed);
        w.Key("cancelled").Bool(r.cancelled);
        w.Key("mode").String(r.mode);
        w.Key("levelItem").Number((wb_int64)r.levelItem);
        w.Key("required").Number((wb_int64)r.required);
        w.Key("levelBefore").Number((wb_int64)r.levelBefore);
        w.Key("level").Number((wb_int64)r.level);
        w.Key("loggedOut").Bool(r.loggedOut);
        if (!r.alarm.empty()) w.Key("alarm").String(r.alarm);
        w.Key("reason").String(r.reason);
        w.Key("reverted").BeginArray();
        for (std::size_t i = 0; i < r.reverted.size(); ++i) w.String(r.reverted[i]);
        w.EndArray();
        w.Key("golden").String(r.golden);
        w.Key("login").RawValue(r.login.empty() ? WebLogin_StateJson() : r.login);
    } else {                                                                    // brought, but golden never reached DoPassword
        w.Key("point").String(g.answer.point);
        w.Key("answered").Bool(true);
        w.Key("asked").Bool(false);
        w.Key("reason").String("golden did not reach DoPassword in this save (no re-login cell changed, or golden skipped that "
                               "branch) -- the answer was not used, the login did not change");
        w.Key("reverted").BeginArray();
        w.EndArray();
        w.Key("login").RawValue(WebLogin_StateJson());
    }
    w.EndObject();
    if (!w.Ok()) return;
    ack->insert(1, "\"reauth\":" + w.Str() + (ack->size() > 2 ? "," : ""));
}

void W906_ReauthClear()
{
    using namespace reauth;
    Wipe(&g.answer.password);
    Wipe(&g.answer.userId);
    g = Stash();
}

bool W906_ReauthHasAnswer(const char* tag)
{
    using namespace reauth;
    return tag && g.present && !g.used && g.tag == tag;
}

// ---- the two save-flow call sites -----------------------------------------------------------------------------------
bool W906_ReauthSetupDoPassword(bool bNeedPassword, bool bOCRNeedPassword, bool bRtcChecked, bool bOcrChecked, bool* bHandled)
{
    using namespace reauth;
    W906ReauthAnswer a = TakeAnswer(kSetupTag);
    W906ReauthResult r;
    const bool bFlag = W906_Reauth(kSetupTag, bNeedPassword, a, &r);
    Wipe(&a.password);
    Wipe(&a.userId);
    if (!bFlag) {                                   // golden cSetUp.cpp:3578-3581 (the generated sbUpdateClick reverts; this only reports it)
        if (bNeedPassword && !bRtcChecked) r.reverted.push_back(kRtc[0]);
        if (bOCRNeedPassword && !bOcrChecked) r.reverted.push_back(kOcr[0]);
    }
    Keep(r);
    if (bHandled) *bHandled = r.handled;
    return bFlag;
}

bool W906_ReauthConfigM01(const std::vector<std::string>& changed, const std::function<void(const std::string&)>& revert,
                          bool* bHandled)
{
    using namespace reauth;
    W906ReauthAnswer a = TakeAnswer(kConfigTag);
    W906ReauthResult r;
    const bool bFlag = W906_Reauth(kConfigTag, false, a, &r);
    Wipe(&a.password);
    Wipe(&a.userId);
    if (!bFlag)                                     // golden cbM01Click :6538-6541 (each changed cell was one click; Q45-2 = A: all of them)
        for (std::size_t i = 0; i < changed.size(); ++i) {
            if (revert) revert(changed[i]);
            r.reverted.push_back(changed[i]);
        }
    Keep(r);
    if (bHandled) *bHandled = r.handled;
    return bFlag;
}

// ---- editlist.get extra.auth ----------------------------------------------------------------------------------------
const char* const* W906_ReauthControls(const char* point, int* n)
{
    using namespace reauth;
    struct P { const char* id; const char* const* c; int n; };
    static const P kP[] = {
        {"m01", kM01, (int)(sizeof(kM01) / sizeof(kM01[0]))}, {"rtcOff", kRtc, 1}, {"ocrOff", kOcr, 1},
        {"c12", kC12, (int)(sizeof(kC12) / sizeof(kC12[0]))}, {"a27", kA27, 1}, {"n07_5", kN075, 1}, {"sgpw", kSgpw, 2},
    };
    for (std::size_t i = 0; point && i < sizeof(kP) / sizeof(kP[0]); ++i)
        if (std::strcmp(point, kP[i].id) == 0) { if (n) *n = kP[i].n; return kP[i].c; }
    if (n) *n = 0;
    return nullptr;
}

std::string W906_ReauthOpenJson(const char* tag, bool rtcArmed, bool rtcAtOpen, bool ocrArmed, bool ocrAtOpen)
{
    using namespace reauth;
    const bool book = FileExists(BookPath());                                   // golden bTechComExist (DoPassword :4328 / :6485)
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("mode").String(book ? "book" : "select");
    w.Key("realTimeCcd").Bool(REAL_TIME_CCD);
    w.Key("points").BeginArray();
    if (IsSetup(tag)) {
        const int lv = LevelSet.AccessLevel[37];
        for (int k = 0; k < 2; ++k) {
            w.BeginObject();
            w.Key("id").String(k == 0 ? "rtcOff" : "ocrOff");
            Controls(w, k == 0 ? kRtc : kOcr, 1);
            w.Key("kind").String("relogin");
            w.Key("armed").Bool(REAL_TIME_CCD && (k == 0 ? rtcArmed : ocrArmed));
            w.Key("armedAtOpen").Bool(k == 0 ? rtcAtOpen : ocrAtOpen);
            w.Key("levelItem").Number((wb_int64)37);
            w.Key("level").Number((wb_int64)lv);
            w.Key("levelZeroSkips").Bool(false);                                // golden SetUp has no "item 37 = 0" shortcut
            w.Key("logoutAfter").Bool(false);
            w.Key("golden").String(k == 0 ? "V912 cSetUp.cpp:3573 (bNeedPassword, gbRTC) -> DoPassword :4325-4362"
                                          : "V912 cSetUp.cpp:3574 (bOCRNeedPassword, gbOcr) -> DoPassword :4325-4362");
            w.EndObject();
        }
    } else if (IsConfig(tag)) {
        const int lv = LevelSet.AccessLevel[92];
        w.BeginObject();
        w.Key("id").String("m01");
        Controls(w, kM01, (int)(sizeof(kM01) / sizeof(kM01[0])));
        w.Key("kind").String("relogin");
        w.Key("armed").Bool(REAL_TIME_CCD && lv != 0);
        w.Key("levelItem").Number((wb_int64)92);
        w.Key("level").Number((wb_int64)lv);
        w.Key("levelZeroSkips").Bool(true);                                     // golden :6489-6490
        w.Key("logoutAfter").Bool(book);                                        // golden :6518-6525 (only with a password book)
        w.Key("golden").String("V912 cConfiguration.cpp:6531-6544 cbM01Click -> DoPassword :6482-6529");
        w.EndObject();
        struct V { const char* id; const char* const* c; int n; const char* kind; const char* why; const char* golden; };
        const V kV[] = {
            {"c12", kC12, (int)(sizeof(kC12) / sizeof(kC12[0])), "vendor", kVendorWhy, "V912 cConfiguration.cpp:6775-6791 cbC12Click"},
            {"a27", kA27, 1, "vendor", kVendorWhy, "V912 cConfiguration.cpp:6824-6858 cbA27Click"},
            {"n07_5", kN075, 1, "vendor", kVendorWhy, "V912 cConfiguration.cpp:6860-6885 cbN07_EnableEmployeeCheakClick"},
            {"sgpw", kSgpw, 2, "sgpw", kSgpwWhy, "V912 cConfiguration.cpp:7274-7289 Image1DblClick"},
        };
        for (std::size_t i = 0; i < sizeof(kV) / sizeof(kV[0]); ++i) {
            w.BeginObject();
            w.Key("id").String(kV[i].id);
            Controls(w, kV[i].c, kV[i].n);
            w.Key("kind").String(kV[i].kind);
            w.Key("armed").Bool(false);
            w.Key("why").String(kV[i].why);
            w.Key("golden").String(kV[i].golden);
            w.EndObject();
        }
    }
    w.EndArray();
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"points\":[]}");
}
