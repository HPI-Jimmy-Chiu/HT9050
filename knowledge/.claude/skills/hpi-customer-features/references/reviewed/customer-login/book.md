# WebLogin_BookCompare：來源、比較與返回碼

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `WebLogin_BookCompare`；定位 `bookOverride`、`pwPath`、`IniConfig.bPasswordSecret`、`bChange`、`CosFunction.bUseLoginDatToSetLevel`、`JCET_FOR_EVAN`、`ids／levels／pws` 及 `AccessLevel`。

## 讀取分流（程式靜態）

bookOverride非空時選override，否則pwPath；FileExists失敗回WEBLOGIN_NO_BOOK。bPasswordSecret開啟時才透過CheckAndReadIniDataGeneral讀Password／Change進bChange。binary分支由CosFunction旗標與override條件或W906_PwBookBinaryOverride決定；fopen失敗清USER.RecordCT後回NO_BOOK，fopen成功則fread／fclose後掃描紀錄與DecodeStr。所讀body沒有比較fread的返回值，binary helper／結構、短讀後果及全部檔案行為仍未查完。

其他情形用TStringList::LoadFromFile，逐行以str[512]／dest[20]暫存、SplitStrByDotSpaceOnly取得欄位；bPasswordSecret且bChange==true才走密碼字串DecodeStr。這裡只記已讀呼叫／容量及條件，不替未讀完的字串解析、解碼、檔案callee或例外行為補成功保證。

## 比較與狀態變更

所讀bCheckOK把user／id與password／pws各自UpperCase比較；JCET_FOR_EVAN==1只在user條件的OR位置，password條件仍存在。這是變數條件，尚未核對它的來源／客戶與現場啟用值，不能從名稱推定客戶或取消所有檢查。

命中且解析出的level落0～3時，更新AccessLevel、s_itemIndex、UseName，呼叫ChangeLevelAttr，再依等級及CosFunction.bLoginShowUserName設定顯示名稱／asUser，NewRecordProcess後回WEBLOGIN_OK。用返回碼確認這個函式選讀路徑，仍不能證明後續儲存或所有caller已成功。

沒有合法命中則走末段Operator／AccessLevel=0／UI與登入旗標重設，再回WEBLOGIN_BAD_CREDENTIALS。NO_BOOK的兩個已讀早退位置未走此末段，不能把所有失敗都套同一個權限重設；此前的其他callee效果、throw／清理與全域寫者仍待查。MBox／Reauth caller在返回後另有門檻與登出，見[MBox](mbox.md)與[前層](../customer-reauth-callee/authentication.md)。

本次只讀程式，沒有打開密碼本、讀取登入環境變數值或執行任何驗證／檔案寫入。

20261007 續補[讀取／解碼／分欄與 V906 字串](../customer-login-readers/index.md)，局部核對此 caller 下層的指定 body；舊來源釘點與當時待查說明保留，完整 reader／認證／磁碟與現場結果仍未完成。
