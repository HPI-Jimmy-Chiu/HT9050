# 原文 01：stOperatorClick

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `stOperatorClick`，完整CPP正文。
來源 commit `367d9d85792fa756db6e898c950d6d79931cbf74`；摘錄 SHA256 `ac5e1aa522df1d37514e0f129865d129c735b0fdb3fd6d14692c08bfa16a4f50`。
歷史註解、客戶條件、裁決與常數保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
