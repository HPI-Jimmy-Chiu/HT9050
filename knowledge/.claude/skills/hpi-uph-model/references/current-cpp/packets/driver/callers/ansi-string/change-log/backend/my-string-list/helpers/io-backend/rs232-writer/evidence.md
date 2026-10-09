# RS232 writer證據、caller範圍與機型版本

## 相同名字不能視為相同program

| 來源 | 身分／介面 | 本次界線 |
| --- | --- | --- |
| `TesterComm/Rs232/Rs232Globals.cpp`＋`Rs232Bridge.h` | tester-side RS232Standard，namespace rs232std，char*／char*／int writer | 完整選定本體與宣告已保存 |
| `gpibbridge::TfRS232Main` | header原banner指出是GPIB program內AMD／ATC aux RS232 | 不能替代本單元；未追其完整caller |
| 共用common writer | 全域const char*或AnsiString writer與append參數 | 既有IO文件，只作比較，不重算完成 |

source／header banner明列歷史RS232 `Rev12.13.902.0_20260410`、20260926移植；
這是tester program的版本軸，與Handler V906／V912／V913／客戶V899分開。
歷史golden路徑、來源行號、build-switch註解與裁決文字完整保存為選定region，
本次沒有讀取該原始902樹，沒有新golden／913／BCB6比對結論。

`TRANSLATION_RULES.md` 原文的版本、namespace與忠實保留規則1～3一併保存；
舊worker的共享checkout／工具流程只屬歷史移植角色，不代替本次ST-GPT的隔離Git與文件授權。
不依其文字啟動worker、build或其他session。

## CMake與編譯條件

目前root CMake將Rs232Globals.cpp列入 `ht9045_testercomm STATIC`；
GNU／Clang條件的source property有unused／sign compare／write strings等warning flags。
原文在manifest保留target與RS232 registration兩段，以及完整選定warning block。
這能證明source registration文字，不能證明實際link、caller可達或ABI。

CPP先套 `RS232STD_GOLDEN_DEBUG` 與 `RS232STD_GOLDEN_SOFT_SIMULTE` 定義，
但writer定義自身的preprocessor guard stack為空；不因此宣稱SIM會攔截寫檔。
Header宣告的include guard與namespace分別保存；本體的Win32 calls沒有runtime模擬分支。

## 限定範圍的靜態spelling搜尋

這次在同一來源commit讀15個cpp／h：TesterComm/Rs232、root rs232.cpp、
檔名含rs232的tests。WriteDataToFile文字僅命中writer定義、header宣告及banner／comment，
未找到直接呼叫表達式；逐檔blob／byte與命中原行見manifest的spelling_scan。
這不是全repo call graph，也不排除alias、巨集、address-taking或外部link caller。
`tests/test_testercomm_rs232.cpp` 該spelling數為0，沒有據此執行或宣稱writer測試通過。

writer原comment的「golden沒有caller」完整保留，是20260926歷史作者敘述，
本次有限搜尋不會把它升格為新原始golden驗證。

## 同題機型與UPH界線

選定本體沒有Type_HT9050／其他Handler機型、客戶或UPH容量分支，
作同題共用IO契約閱讀；各機台是否選用這個tester program仍需追設定與caller。
HT9050工單、snapshot與runtime身分沿用 [目前C++機型入口](../../../../../../../../../../index.md)，
不把source registration當作HT9050已部署，也不把寫檔文件算成UPH S8或實機完成。
其餘RS232／upload路徑、完整startup／parser子callee、CRT、版本機型與實際IO待續。
