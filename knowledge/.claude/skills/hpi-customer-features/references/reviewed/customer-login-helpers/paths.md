# BookPath 的 namespace 與模式判斷

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的匿名namespace內 `pw::BookPath`／`reauth::BookPath` 及全域 `WebLogin_UsesBook`；前層已讀 `BookOverride`、`W906_Reauth`、`W906_DoPasswordMBox` 與 `WebLogin_StateJson`，見[前層 helper](../customer-login/state.md)／[callee](../customer-reauth-callee/authentication.md)／[MBox](../customer-login/mbox.md)。BookPath 的相同函式名與signature不代表同一個定義；manifest保留 namespace 與definition_index。

## 路徑選擇

pw::BookPath直接讀getenv(W906_PWBOOK_PATH)，存在且非空用override，否則pwPath。reauth::BookPath經BookOverride取override，非空才用，否則pwPath；BookOverride的所讀body同樣使用該環境變數。這只是這些表達式的對照，本次沒有讀環境值或檢查哪個檔在機台實際存在。

## mode 的資料源不相同

| 已讀函式 | 判斷表達式／輸出用途 |
| --- | --- |
| WebLogin_UsesBook | FileExists(pwPath) OR CosFunction.bUseLoginDatToSetLevel；供StateJson選mode |
| W906_Reauth／W906_DoPasswordMBox | FileExists(reauth::BookPath())；用有效路徑是否存在選book／select |
| WebLogin_BookCompare | 以傳入的bookOverride是否非空選override／pwPath；binary條件另讀CosFunction及[override探測](binary.md) |

因此不能拿StateJson的mode替代存檔caller實際使用的路徑或格式；它們檢查的變數／條件不同。pwPath、CosFunction旗標、環境與其他caller的全部來源仍待查，當前現場配置沒有驗證。

沒有把兩個namespace合成同一個callee，也未宣稱其他檔案的同名namespace或同名helper都等效。HT9050／其他Handler的客戶、機型、版本與runtime要沿自己的caller核對；V912 UI仍用前層指定body作局部對照。
