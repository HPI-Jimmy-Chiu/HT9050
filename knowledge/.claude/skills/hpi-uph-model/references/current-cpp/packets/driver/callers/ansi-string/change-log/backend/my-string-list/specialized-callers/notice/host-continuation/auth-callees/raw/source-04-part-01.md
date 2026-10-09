# 原文 04／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `DoPassword`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `b5f496e12432e9e307ff8d34114eb3f0621346a0b0361ed1cfb16beaf4a481c0`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
bool DoPassword(Note& n, int sel, const Cred* cred, Out* o)
{
    bool bFlag=true;

    // golden :5281-5288 -- CC_Greatek + IniConfig.bN15UserLevelByTxt downloads the password book by FTP
    //   (ESDForm->FTP_PasswordFile_Download) into pwPath: ESDForm is not in this tree, pwPath is kept (banner).

    bool bTechComExist=FileExists(reauth::BookPath());                          //2012-01-03    Dell modify
    int iLevel=0;
    static bool bEnter=false;                                                   //Steven 20151015 : To avoid double click

    o->mode = bTechComExist ? "book" : "select";
    if(bWaitSecsGemReply==true)                                                 //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
    {
        o->reason = "bWaitSecsGemReply: golden returns true (note.cpp:5294-5297)";
        return bFlag;
    }

    iLevel=fSecurity->GetJamLevel(n.sJamArea, n.sJamCode);

    if(CosFunction.bConAlarmNeedKeyInPassword==true &&                          //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password
       IniConfig.bO16ConAlarmNeedKeyInPasswordCT)
    {
        if(n.bNeedPassWord==true && iLevel<=0)
        {
            iLevel=LevelSet.AccessLevel[35];
        }
    }
    else if(CosFunction.bConAlarmInTimeLevelUp &&                               //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
            IniConfig.bO17EnableLevelUpWhenContiAlarm)
    {
        if(n.bNeedPassWord==true && iLevel<=0)
        {
            iLevel=LevelSet.AccessLevel[166];
        }
    }

    //AI(W906-E030) 20261003 (St01): golden 906 DoPassword（note.cpp:5237-5382）O16 / O17 那段（906 :5261-5276，V912 :5301-5316）之後直接是
    //  SCC（906 :5278，V912 :5325）。V912 :5318-5323 夾在中間的 WAR04217 一段（CSV Compare 開著就 iLevel=1，連 JAM 等級表設 2 / 3 也蓋成 1）
    //  906 沒有，拿掉（RULINGS_20261002 #20 / #23-6）：WAR04217 的等級照 906 = GetJamLevel 再走下面 SCC / VTEST / 統計 / TCP。
    if(CUSTOMER_CODE==CC_SCC)                                                   //Ifor 20181024 add :SCC針對特定Alarm Code解除時需權限密碼
    {
        if(n.sJamCode=="WAR07301" ||                                            //Socket consecutive failure
           n.sJamCode=="WAR07321" ||                                            //Arm1: consecutive failure
           n.sJamCode=="WAR07329" ||                                            //Arm2: consecutive failure
           n.sJamCode=="JAM0540"  || n.sJamCode=="JAM0550" ||                   //Device lose at Output Shuttle
           n.sJamCode=="JAM0303"  || n.sJamCode=="JAM0304" ||                   //Device drop error (Arm 1) & Device drop error (Arm 2)  //Jou 20140604 Add for SCC
           n.sJamCode=="WAR0310"  ||                                            //Socket has IC error!                                   //Jou 20140604 Add for SCC
           n.sJamCode=="JAM0314"  || n.sJamCode=="JAM0315")                     //JerryYang 20160516 add JAM0314 JAM0315 "Clean pad drop error"
        {
            if(iLevel==0)
                iLevel=1;
        }
    }
    else if(IniConfig.bVTESTFunction==true)
    {
        if(n.sJamCode=="WAR16123" && sel==1)                                    // Select[1]==true (K_RETRY)
        {
            o->reason = "VTEST WAR16123 with RETRY: golden returns true (note.cpp:5337-5338)";
            return true;
        }

        if(n.sJamCode=="WAR07460" && iLevel<1)                                  //AI(ht9045-config) 20260507 (RogerYang) : VTEST銦片超壽命解除需工程師密碼
            iLevel=1;
    }

    if(CosFunction.bStatisticsJamCount==true)                                   //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    {
        if(n.bNeedHighLevelPassword==true)
        {
            iLevel++;
            if(CosFunction.bSecurityHave5Level==true)
            {
                if(iLevel>=4)
                    iLevel=4;
            }
            else
            {
                if(iLevel>=3)
                    iLevel=3;
            }
        }
    }

    if(CosFunction.bEnableHandlerResultServer &&
       fSecurity->GetJemTCPAlarm(n.sJamArea, n.sJamCode))                       //Sam 20230620 : 面板已經鎖定需要 IT 下命令解鎖
    {
        iLevel=1;
    }
    o->required = iLevel;
    //jou 2012-11-28 Bin Yield Failure 的百分比值要可以輸入到小數點一位,另外也要紅底+密碼
    //jou 2016-11-17 make code修改Security password 設定不卡客戶碼
    {
        if(n.bNeedPassWord==true && /*fInput->fShow==false*/ true && bEnter==false)   // fInput: no keypad form on the server (banner)
        {
            o->asks = true;
            if(!cred) return false;                                             // [W906] asked only "would golden stop here"
            AnsiString user = cred->cancelled ? AnsiString("") : reauth::Typed(cred->userId);    reauth::WipeOnExit wipeUser(&user);   // Q45-4 = A: cancel = the cleared box
            AnsiString pass = cred->cancelled ? AnsiString("") : reauth::Typed(cred->password);  reauth::WipeOnExit wipePass(&pass);
            std::string why;
            bEnter=true;                                                        //Steven 20151015 : To avoid double click
            if(bTechComExist)
            {
                std::string msg;
                const int lr = WebLogin_BookCompare(user, pass, reauth::BookOverride(), &msg);   // golden fMain->cbUserSelectChange(NULL);
                if(lr==WEBLOGIN_BAD_CREDENTIALS)
                {
                    o->alarm = "WAR1677";                                       // banner: golden's ShowErrorMessage returns at :814 ("Alarm at same time")
                    why = "wrong user name or password (golden bError main.cpp:15333-15344: Operator; WAR1677 is \"Alarm at same time\" under the note)";
                }
                else if(lr==WEBLOGIN_NO_BOOK)
                {
                    o->alarm = "WAR1681";
                    why = "the password book cannot be read (golden WAR1681; level unchanged)";
                }
                else
                {
                    why = "password book login";
                }
            }
            else
            {
                reauth::SetupArm arm(true);                                     // golden main.cpp:13285 fNote->bNeedPassWord && fNote->fShow
                stOperatorClick(pass);                                          // golden fMain->stOperatorClick(fMain);
                why = "drop-down password compare, the note's arm (main.cpp:13283-13301)";
            }
            bEnter=false;                                                       //Steven 20151015 : To avoid double click
            o->accessLevel = AccessLevel;
            if(AccessLevel<iLevel)
            {
                bFlag=false;
                // golden :5387-5395 fPassword2->Label3/Label4->Visible=true (book) / palWrongPW (select): the login box shows them
            }
            else
            {
                RecordProcess("==Login for unlock alarm.==", AnsiString(AccessLevel));      //Steven 20220615 : Log for unlock alarm
                // golden :5400-5408 Label3/Label4 hidden / palWrongPW->Parent=tsRedAlarm (display)
            }
            if(cred->cancelled)
                why = "login box cancelled = blank user name and password (Q45-4), " + why;
            why += "; level " + reauth::Num(AccessLevel) + (bFlag ? " >= " : " < ") + reauth::Num(iLevel);

            if(CUSTOMER_CODE==CC_PTI)                                           //Sam 20230112 : 力成呂其名要求解除 Alarm 後要維持權限。
            {
            }
            else if(bTechComExist)
            {
                s_btLoginIsLogout=false;                                        // fMain->btLogin->Caption="Login";
                s_userCaption="Operator";                                       // fMain->spbUserName->Caption="Operator";
                s_itemIndex=0;                                                  // fMain->cbUserSelect->ItemIndex=0;
                AccessLevel=0;
                ChangeLevelAttr();                                              // fMain->ChangeLevelAttr();
                o->loggedOut = true;
                why += "; then logged out to Operator (note.cpp:5418-5424)";
            }
            o->reason = why;
        }
        else if(bEnter==true)                                                   //Steven 20151015 : To avoid double click
        {
            bFlag=false;
        }
        else
        {
            o->reason = "bNeedPassWord is false: golden does not ask";
        }
    }
    return bFlag;
}

<!-- preserved-content:end -->
```
