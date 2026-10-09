# stOperatorClick：先清零，再按客戶與用途分流

[上層](index.md)；[先前Press／DoPassword](../password.md)；[原文](raw/source-01.md)。

`WebLogin.cpp::stOperatorClick` 是void函式，先寫 `AccessLevel=0`。
接著呼叫VENDER設定的CheckKeyExist、CheckAndReadIniDataGeneral及WriteIniDataGeneral，
處理最高權限舊key相容與預設值回寫；這些callee位在credential比對之前。
因此不能把本函式當成純比較器；本輪只讀正文，未執行INI讀寫或取實際設定值。

## SPIL與USER比較

`IniConfig.bSPILFunction` 臂先比sSuperVisorString或源碼內建值，match設iDefHonPrecLevel。
其餘只在cbUserSelect->Text不是Operator時比USER.PassWord。
CosFunction.bSecurityHave5Level決定掃三項或兩項；S1與S4轉UpperCase，
還必須s_itemIndex==i+1，才設AccessLevel=i+1並break。
S3取USER.ID並轉UpperCase，但這份正文的match條件沒有比較S3，不能稱ID已驗證。

## 非SPIL、KYEC與Setup ask

非SPIL先比sSuperVisorString或載入的HonPrecPassword。
CUSTOMER_CODE為CC_KYEC_LEE／CC_KYEC_XILINX且password等於HonPrecPassword時直接return；
此前AccessLevel已清零。其餘match設iDefHonPrecLevel，KYEC臂另呼叫NewRecordProcess。
即使外層另一條件也match，內層仍以password==HonPrecPassword判斷，不能推論分支優先級豁免。

其他非空password轉UpperCase；g_W906SetupAsksPassword才走W906_StOperatorClickAskArm。
否則主畫面臂沿用二／三項USER.PassWord與selected index比對。
SPIL臂與最高權限臂不走這個Setup ask分派，不能把它套用到所有登入。

正文沒有共同的ChangeLevelAttr／caption／btLogin尾段；後續UI與紀錄副作用依caller核對。
保留golden／Barcode／V912歷史comment，不代表新比較過913或網頁輸入已完成同等驗證。
AskArm、設定讀寫及完整使用者選單callback仍待另讀。
