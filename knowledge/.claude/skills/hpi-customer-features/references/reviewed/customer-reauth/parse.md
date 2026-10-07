# reauth解析與頁面對照

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `W906_ReauthTake`／`reauth::PointOf`／`kSetupTag`／`kConfigTag`；宣告見[WebReauth.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebReauth.h)；HSys tag見[HSys.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/FileRW/HSys.cpp)的 `kPage`／PageRegistrar。

## 解析不是登入驗證

W906_ReauthTake先Clear舊暫存；root不存在或不是object時返回空拒絕理由。它從root分離reauth，也從object型widgets分離內嵌reauth。內嵌位置存在即拒絕；有root reauth時可用object或非空array，逐項檢查object、point字串、PointOf對照、cancelled布林、可選userId字串，以及未取消時必須有password字串。同一point重複也拒絕；沒有reauth時此函式可返回空理由。

通過解析後將答案放入g.answers、g.used，設定g.present／g.tag。`cancelled:true`不要求password，也不把userId／password複製進該答案；這只是答案表示法，不等於存檔流程已取消、不需要其他守衛或已成功登入。空拒絕理由只代表這個解析階段沒有拒絕，實際DoPassword、權限與callee仍要另外追。

## 支援頁面

| tag | PointOf接受的point | 本次界線 |
|---|---|---|
| TestIF_File_SetUp（kSetupTag） | rtcOff、ocrOff | 只核對對照表／解析；完整登入與功能勾選流程未在本次驗證 |
| IniConfig（kConfigTag） | m01、i37_1 | 同上；不宣稱其他point或所有Config存檔都會重新登入 |
| HSys | 無 | kPage的tag是HSys，PointOf兩個分支之外返回false；帶任何reauth.point會在解析被拒絕 |

HSys沒帶reauth時，W906_ReauthTake可返回空理由；仍須通過[WS接收、主迴圈、OpenGateRefused／PageSave與HSys存檔守衛](../customer-web-save/index.md)。此表不是owner例外或權限豁免，也不能將「沒帶reauth」推成沒有登入／權限要求。

這是V906頁面分流，不新增客戶或機型分支；HT9050與其他Handler的活動版本／tag／權限／runtime另核對。V912表單路徑按[原存檔caller](../customer-persistence.md)及[先前HSys對照](../customer-web-save/hsys.md)讀，不能將Web解析表當成BCB行為已驗證。
