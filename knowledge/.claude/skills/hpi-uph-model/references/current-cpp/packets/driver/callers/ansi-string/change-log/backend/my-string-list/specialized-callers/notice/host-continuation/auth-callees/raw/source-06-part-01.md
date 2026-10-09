# 原文 06／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `WebLogin_BookCompare`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `1223dee36e9280986b6a0c7702d33359640891caa08e4a3a7c19ab9ec6fe70a6`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
