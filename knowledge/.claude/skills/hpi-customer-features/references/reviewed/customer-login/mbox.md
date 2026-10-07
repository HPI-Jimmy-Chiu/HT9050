# I37、DoPassword_MBox 與 PTI 建置分流

來源：[V906 WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `W906_DoPasswordMBox`；[V912 mymessbox.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/mymessbox.cpp)的 `TMyMessageBox::DoPassword_MBox`。數值見[V906 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/MachineType.h)與[V912 MachineType.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MachineType.h)的 `CC_PTI`。活定位用 function、`CUSTOMER_CODE`、`SOFT_SIMULTE`、`fInput->fShow` 與 `LevelSet.AccessLevel[35]`。

## 局部客戶差異一列

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
| --- | --- | --- | --- | --- | --- |
| CC_PTI／957（兩版定義） | V906 W906_DoPasswordMBox；V912 TMyMessageBox::DoPassword_MBox | 直接比較CUSTOMER_CODE；V906另有編譯期SOFT_SIMULTE，V912所讀body另受fInput->fShow門檻 | V906 PTI分支非模擬回false、模擬沿初值true；V912在fInput未顯示時走PTI密碼輸入及比對，不能宣稱兩分支完整相同 | 兩版指定body／定義已讀；不是912-only；未核對目前實際建置組態或全部caller | HT9050與其他Handler同題，作用中客戶／機型／runtime需另查 <!-- review-record:reauth-mbox-pti-build --> |

## V906 的返回與結果

I37 wrapper 見[前一層](../customer-reauth-callee/answers.md)：呼叫 W906_DoPasswordMBox 時 mboxArm=false。本函式設定 levelItem=35、required=LevelSet.AccessLevel[35]、mode 依 BookPath 的 FileExists 分流；PTI 選取分支不呼叫一般 book／select 登入，bFlag 依 SOFT_SIMULTE 分流，再填 passed／level／login。原因字串描述 golden／未支援背景，不能當成這次已執行機台或驗過密碼。

其他客戶分支先記 asked；沒有答案則 handled=false／passed=false 並回false。有答案時，取消送空 user／password；book 路徑呼叫[BookCompare](book.md)，select 路徑使用 SetupArm(mboxArm) 後呼叫 stOperatorClick。所讀body在 AccessLevel<iLevel 且 iLevel>0 才使bFlag=false；book存在時之後另重設登入狀態與AccessLevel=0、ChangeLevelAttr並記loggedOut，因此比較時與最後結果level可不同。REAL_TIME_CCD不是此body的包覆條件，不能套用[W906_Reauth](../customer-reauth-callee/authentication.md)的條件。

## V912 對照界線

V912 body 初始bFlag=true，只有 fInput->fShow==false 才進客戶分支。PTI走fPassword／ShowQwertyKey，再把輸入與CheckAndReadIniDataGeneral回傳的HonPrecPassword比較；本次沒有讀取或記錄密碼值。所讀body沒有SOFT_SIMULTE條件，V906理由字串的歷史行號不能替代這個差異。

其他客戶依FileExists(pwPath)選cbUserSelectChange或stOperatorClick，再查AccessLevel<iLevel且iLevel>0；book分支會更動Label3／Label4顯示，最後重設UI登入／名稱與AccessLevel=0。完整UI、INI讀取、其他callee／caller與存檔作用尚未驗證。V906 C++樹此pin沒有mymessbox.cpp檔，不從歷史golden路徑推定它存在或已對等。
