# StateJson 與 IsConfig／BookOverride／Typed

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `WebLogin_StateJson`、`IsConfig`、`BookOverride`、`Typed`。本層只補這四個body，不改[既有Ack查證](../customer-reauth/clear-ack.md)的日期與來源pin。

## StateJson 的已讀欄位

StateJson組mode（WebLogin_UsesBook）、level（AccessLevel）、itemIndex（s_itemIndex）、levelName（cbUserSelect->Text）、userCaption（s_userCaption）、固定items、btLogin（s_btLoginIsLogout）與systemStart（SystemStart）。本body沒有直接輸出password；這不是所有登入回覆或憑證處理的全量驗證。

levelName與userCaption在所讀body直接串接，未另呼叫JSON escaping；值的全部來源、轉碼、其他caller的回覆組裝／解析、UI同步及並行寫者仍待查。這裡描述直接表達式，不宣稱端到端JSON或所有來源字串已驗證。

## 三個 helper 的分工

- IsConfig要求tag非空，與kConfigTag做strcmp==0；它不驗登入或客戶碼。
- BookOverride讀環境變數W906_PWBOOK_PATH的指標，非空才回路徑字串，否則空。這是程式分流，本次沒有讀實際環境變數值；兩個不同namespace的BookPath仍待分開追查，不把同名helper合併。
- Typed先嘗試pw::Acp，失敗則AnsiString(u.c_str())；Acp與字元轉換細節仍待查，不由函式名稱宣稱無損轉碼或密碼比對已相容。

同題供HT9050／其他Handler使用，版本／客戶／機型／runtime另分流。後續仍需全部caller、登入／權限狀態寫者、解碼與UI／字串callees、owner／HTTP時序、consumer與最終落盤。未操作runtime、現場帳密、機台、API或build。
